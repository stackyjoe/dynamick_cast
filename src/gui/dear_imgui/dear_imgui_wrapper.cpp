#include <exception>
#include <filesystem>
#include <system_error>

#include <SDL.h>
#include <fmt/core.h>

#include <boost/iostreams/stream.hpp>

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_sdl.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/backends/imgui_impl_opengl3_loader.h"

#include "dear_imgui_wrapper.hpp"

#include "library/rss_parser.hpp"
#include "shared/string_functions.hpp"
#include "networking/url_parser.hpp"

#include "dynamick_cast/generic_exception_handling.hpp"
#include "dynamick_cast/debug_print.hpp"


using namespace std::string_literals;

dear_imgui_wrapper::dear_imgui_wrapper(int &argc, char **argv, thread_safe_interface<audio_abstraction> &&audio_handle)
    : should_continue(true),
    open_window(false),
    audio_handle(std::move(audio_handle)),
    library_handle(thread_safe_interface<library>::template make<library>()),
    state(UserDesiredState::stop),
    url_input_buffer(std::make_unique<char[]>(buffer_size)),
    vol(100.0f),
    user_holds_volume_slider(false),
    track_position(0.0f),
    user_holds_track_slider(false),
    menu_size(ImVec2(0,20))
    {
    auto [afp, ns] = library_handle.perform([](auto &l){
        return std::make_pair(l.app_file_path(), l.native_sep());
    });
    project_directory = afp;
    native_separator = ns;
    // Setup SDL
    // (Some versions of SDL before <2.0.10 appears to have performance/stalling issues on a minority of Windows systems,    // depending on whether SDL_INIT_GAMECONTROLLER is enabled or disabled.. updating to latest version of SDL is recommended!)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        throw std::runtime_error( SDL_GetError());
    }

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
    // GL 3.2 Core + GLSL 150
    const char* glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    window = SDL_CreateWindow("Dynamick Cast", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync


    // Initialize OpenGL loader
    bool err = imgl3wInit() != 0;
    if (err)
    {
        throw std::runtime_error("Failed to initialize OpenGL loader!\n");
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    io_context = std::addressof(ImGui::GetIO());

    if(io_context == nullptr) {
        throw std::runtime_error("Could not get IO context from Dear ImGui.\n");
    }

    io_context->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io_context->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    }

dear_imgui_wrapper::~dear_imgui_wrapper() {    
    if(io_context) {    
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
    }

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void dear_imgui_wrapper::toggle_state() {

    switch(state) {
    case UserDesiredState::play : {
        audio_handle.perform(
            [](audio_abstraction &handle) {
                handle.pause();
            }
        );
        state = UserDesiredState::pause;
    } break;
    case UserDesiredState::pause: {
        audio_handle.perform(
            [](audio_abstraction &handle) {
                handle.play();
            }
        );
        state = UserDesiredState::play;
    } break;
    default: {
        debug_print("Error, state is not play or pause.\n");
    }
    }
}

void dear_imgui_wrapper::hotkey_handler() {
    if(ImGui::IsKeyPressed(SDL_SCANCODE_F4) && ImGui::IsKeyPressed(SDL_SCANCODE_LALT))
        should_continue = false;
    if(ImGui::IsKeyPressed(SDL_SCANCODE_LCTRL) && ImGui::IsKeyPressed(SDL_SCANCODE_Q))
        should_continue = false;
    if (ImGui::IsKeyPressed(SDL_SCANCODE_LCTRL) && ImGui::IsKeyPressed(SDL_SCANCODE_O))
        fetch_rss_dialog();
    if(ImGui::IsKeyPressed(SDL_SCANCODE_F5)){
        save_subscriptions();
    }
    if(ImGui::IsKeyPressed(SDL_SCANCODE_F9)) {
        load_subscriptions();
    }
    if(ImGui::IsKeyPressed(SDL_SCANCODE_SPACE)) {
        toggle_state();
    }
}


void dear_imgui_wrapper::sync_audio_with_library_state() {
            
}

void dear_imgui_wrapper::sync_ui_with_audio_state() {

}

void dear_imgui_wrapper::sync_ui_with_download_state() {

}

void dear_imgui_wrapper::sync_ui_with_library_state() {

}

void dear_imgui_wrapper::run() {

    // Our state
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    episode const * cur_ep = nullptr;


    load_subscriptions();

    // Main loop
    while (should_continue)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if ((event.type == SDL_QUIT)
                || (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))) {
                should_continue = false;
            }
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();
        {
            ImGui::SetNextWindowSize(ImVec2(640,480));

            if(ImGui::BeginMainMenuBar())
            {
                if (ImGui::BeginMenu("Program"))
                {
                    if(ImGui::MenuItem("Load subscriptions")) {
                        load_subscriptions();
                    }
                    if(ImGui::MenuItem("Save subscriptions")) {
                        save_subscriptions();
                    }
                    if (ImGui::MenuItem("Close")) {
                        should_continue = false;
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Podcasts"))
                {

                    if (ImGui::MenuItem("Add podcast via remote RSS")) {
                        open_window = true;
                    }

                    ImGui::EndMenu();
                }

                

                menu_size = ImGui::GetWindowSize();

                ImGui::EndMainMenuBar();
            }

            hotkey_handler();

            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(ImVec2(0, menu_size.y));
            ImGui::SetNextWindowSize(viewport->WorkSize);

            //ImGui::SetNextWindowPos(ImGui::GetCursorPos(), ImGuiCond_FirstUseEver);
            //ImGui::SetNextWindowSize(ImVec2(ImGui::GetScrollMaxX(), ImGui::GetScrollMaxY()), ImGuiCond_FirstUseEver);


            ImGui::Begin("Dynamick Cast",
                        nullptr,
                        ImGuiWindowFlags_NoMove |
                        ImGuiWindowFlags_NoResize |
                        ImGuiWindowFlags_NoCollapse |
                        ImGuiWindowFlags_NoTitleBar);


            ImGui::BeginChild("sliders", ImVec2(-1,60), true);
            ImGui::BeginChild("volume slider", ImVec2(200, 20), false);

            ImGui::SliderFloat("Volume", &vol, 0.0f, 100.0f);
            if(ImGui::IsItemActive()) {
                user_holds_volume_slider = true;
            }
            else {
                user_holds_volume_slider = false;
            }
            ImGui::EndChild();

            ImGui::SameLine();

            if(ImGui::Button("Play/Pause")) {
                toggle_state();
            }

            ImGui::SameLine();

            ImGui::Text("%s", cur_ep ? cur_ep->get_title().c_str() : "");

            ImGui::BeginChild("position slider", ImVec2(-1, 20), false);
            ImGui::SliderFloat("Track Position", &track_position, 0.0f, 100.0f);
            if(ImGui::IsItemActive()) {
                user_holds_track_slider = true;
            }
            else {
                user_holds_track_slider = false;
            }

            ImGui::EndChild();


            ImGui::EndChild();

            ImGui::BeginGroup();
            ImGui::BeginChild("podcast view", ImVec2(300,-1), true);
            if(ImGui::BeginTable("podcasts", 1)) {
                ImGui::TableNextColumn();
                ImGui::Text("Podcasts");

                ImGui::Separator();

                library_handle.perform([this](library & channels){
                    for(auto & [name, podcast] : channels) {
                        ImGui::Selectable(name.c_str());
                        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                            this->open_channel = name;
                        }

                        if(ImGui::BeginPopupContextItem()) {
                            if(ImGui::Button("View episodes")) {
                                this->open_channel = name;
                                ImGui::CloseCurrentPopup();
                            }
                            if(ImGui::Button("Update feed")) {
                                this->fetch_rss(podcast.rss_url());
                                ImGui::CloseCurrentPopup();
                            }
                            if(ImGui::Button("Remove local files")) {
                                ImGui::CloseCurrentPopup();
                            }
                            if(ImGui::Button("Unsubscribe and remove local files")) {
                                ImGui::CloseCurrentPopup();
                            }
                            ImGui::EndPopup();
                        }
                    }
                });

                ImGui::EndTable();
            }
            ImGui::EndChild();

            ImGui::SameLine();

            ImGui::BeginChild("episode view", ImVec2(-1,-1), true);
            if(ImGui::BeginTable("episodes", 1)) {
                ImGui::TableNextColumn();
                ImGui::Text("Episodes");

                ImGui::Separator();
                library_handle.perform([this, &cur_ep](library & channels) mutable -> auto {
                    if(!(this->open_channel.empty())) {
                        auto result_itr = channels.find(open_channel);
                        if(result_itr != nullptr) {
                            auto &pod = *result_itr;
                            for(auto &ep : pod.peek_items()) {
                                auto dlr = ep.get_download_rights();

                                if(!dlr->is_downloading()){
                                    ImGui::Text("   ");
                                }
                                else {
                                    std::string quantity_read = fmt::format("{} kB",dlr->get_bytes_completed()/1000);
                                    ImGui::Text("%s", quantity_read.c_str());
                                }

                                ImGui::SameLine();
                                ImGui::Selectable(ep.get_title().c_str());

                                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                                    cur_ep = std::addressof(ep);
                                    this->download_or_play(ep);
                                }


                                if(ImGui::BeginPopupContextItem()) {
                                    if(ImGui::Button("Play / Download")) {
                                        cur_ep = std::addressof(ep);
                                        download_or_play(ep);
                                        ImGui::CloseCurrentPopup();
                                    }
                                    
                                    if(ImGui::Button("Delete file")) {

                                        std::filesystem::path file(project_directory + open_channel + native_separator + ep.get_sanitized_file_name());
                                        std::error_code ec;
                                        std::filesystem::remove(file,ec);

                                        ImGui::CloseCurrentPopup();
                                    }
                                    ImGui::EndPopup();
                                }
                            }
                        }


                    }
                });
                
                ImGui::EndTable();
            }
            ImGui::EndChild();
            ImGui::EndGroup();

            audio_handle.perform(
                [this](audio_abstraction &handle) {
                    
                    if(this->user_holds_volume_slider) {
                        handle.set_volume(static_cast<int>(this->vol));
                    }

                    if(this->user_holds_track_slider) {
                        handle.seek_by_percent(this->track_position/100.f);
                    }
                    else {
                        this->track_position = handle.get_percent_played()*100.f;
                    }
                }
            );

            ImGui::End();
        }

        fetch_rss_dialog();

        // Rendering
        ImGui::Render();
        glViewport(0, 0, int(io_context->DisplaySize.x), int(io_context->DisplaySize.y));
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }
}

void dear_imgui_wrapper::load_subscriptions() noexcept {
    debug_print("Loading subscriptions.\n");

    std::string file_path =  project_directory + "subscriptions.json"s;
    std::ifstream save_file(file_path, std::ios::in);

    if(! save_file.is_open()) {
        fmt::print("Error opening file: {}\nError code: {}\n", file_path, errno);
        return;
    }

    try {
        library_handle.perform([&save_file](library & channels) mutable -> void {
            channels.fill_from_json(save_file);
        });
        //sync_ui_with_library_state();
    }
    catch(const std::exception &e) {
        // Leave map in empty state.
        
        notify_and_ignore(e);
    }

    save_file.close();
}

void dear_imgui_wrapper::save_subscriptions() {
    debug_print("Saving subscriptions.\n");

    std::string file_path = project_directory + "subscriptions.json"s;
    std::ofstream save_file(file_path, std::ios::out);

    if(! save_file.is_open()) {
        fmt::print("Error opening file: {}\nError code: {}\n", file_path, errno);
        return;
    }

    try{
        library_handle.perform([&save_file](library & channels) mutable -> void {
            if(channels.empty()) {
                save_file.close();
                return;
            }

            channels.serialize_into(save_file);
        });

        save_file.close();
    }
    catch(std::runtime_error const &e) {
        notify_and_ignore(e);
    }

    return;
}

void dear_imgui_wrapper::download(episode const & ep, std::string file_path, std::string url) {


    auto download_rights = ep.get_download_rights();

    auto token = download_rights->should_start_downloading();

    if(!token.should_start_downloading) {
        return;
    }

    // Since the completion handler holds ownership of the shared_state, we can just use a raw pointer here.
    auto prog = [shared_state=download_rights]([[maybe_unused]] size_t completed, [[maybe_unused]] size_t total) -> void {
        shared_state->set_bytes_completed(completed);
    };

    auto cmpl = [file_path, download_rights=download_rights, token=token] (
            boost::beast::error_code const &ec,
            size_t bytes_read,
            beastly_connection &resources) mutable -> void {

        // RAII wrapper to clear the atomic<bool> download_rights->currently_downloading on destruction
        debug_print("Beginning completion handler\n");
        std::fflush(stdout);

        if(ec || bytes_read == 0) {
            notify_and_ignore(ec);

            download_rights->request_gui_update();
            return;
        }

        std::ofstream output_file(file_path, std::ios::binary);

        debug_print("File saved to: {}\n", file_path);

        std::string const &body = resources.parser().body();

        output_file.write(body.c_str(), body.size());
        output_file.close();

    };

    auto f = get.get(url, std::move(prog), std::move(cmpl));
}

void dear_imgui_wrapper::fetch_rss(std::string url) {
        auto f = get.get(url,
                       []([[maybe_unused]] size_t read, [[maybe_unused]] size_t total){ return;},
                       [this, url](boost::beast::error_code const & ec, [[maybe_unused]] size_t bytes_read, beastly_connection &res){
            if(ec) {
                notify_and_ignore(ec);
                return;
            }

            auto rss = rss_parser(res.take_body());

            this->library_handle.perform([podcast = rss.parse(url)](library & channels) mutable -> auto {
                std::string t = podcast.title();
                auto [itr, was_inserted] = channels.insert_or_assign(t, std::move(podcast));

                if( !was_inserted ) {
                    debug_print("Assigned {}\n", t);
                }
            });

        }
    );
}

void dear_imgui_wrapper::fetch_rss_dialog() {
    if(open_window) {
        ImGui::SetNextWindowSize(ImVec2(512,128), ImGuiCond_FirstUseEver);
        ImGui::Begin("Fetch remote RSS", &open_window, ImGuiWindowFlags_MenuBar);
                        
        ImGui::InputText("URL input", url_input_buffer.get(), buffer_size, ImGuiInputTextFlags_CharsNoBlank);
        if(ImGui::Button("Fetch")) {
            auto url = std::string(url_input_buffer.get());
            fmt::print("{}\n",url);
            fetch_rss(url);
            open_window = false;
            for(auto i = 0; i < buffer_size; ++i) {
                url_input_buffer[i] = '\0';
            }
        }
                        
        ImGui::End();
    }
}


void dear_imgui_wrapper::download_or_play(episode const & ep) {
    std::string title = ep.get_title();
    std::string url = ep.url();
    std::string file_name = ep.get_sanitized_file_name();

    std::error_code ec;
    std::string folder_for_this_podcast = project_directory + open_channel + native_separator;
    if (!std::filesystem::create_directory(folder_for_this_podcast, ec) && ec) {
        debug_print("Failed to create directory.\n");
        notify_and_ignore(ec);
        return;
    }

    std::string file_path = folder_for_this_podcast + file_name;

    if( audio_handle.perform(
        [file_path](audio_abstraction &handle) -> bool {
            if(! handle.open_from_file(file_path))
                return false;
            handle.play();
            return true;
        }
        )) {
        // interior of if
        state = UserDesiredState::play;
    }
    else {
        download(ep, file_path, url);
    }
}

thread_safe_interface<gui_abstraction> make_gui(int argc, char ** argv, thread_safe_interface<audio_abstraction> audio) {
    return thread_safe_interface<gui_abstraction>::make<dear_imgui_wrapper>(argc, argv, std::move(audio));
}
