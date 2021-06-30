#include <exception>

#include <SDL.h>
#include <fmt/core.h>

#include <boost/iostreams/stream.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_sdl.h"
#include "imgui/backends/imgui_impl_opengl3.h"


#include "dear_imgui_wrapper.hpp"

#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
//  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
//  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>            // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>            // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>          // Initialize with gladLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
#include <glad/gl.h>            // Initialize with gladLoadGL(...) or gladLoaderLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/Binding.h>  // Initialize with glbinding::Binding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/glbinding.h>// Initialize with glbinding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

using namespace std::string_literals;

dear_imgui_wrapper::dear_imgui_wrapper(int &argc, char **argv, thread_safe_interface<audio_abstraction> &&audio_handle)
    : audio_handle(std::move(audio_handle)),
    state(UserDesiredState::stop),
    home_path("/usr/home/joe"s),
    native_separator("/"s),
    project_directory(home_path + "/.local/share/applications/dynamick-cast/"s)
    {

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
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    bool err = gladLoadGL() == 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
    bool err = gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress) == 0; // glad2 recommend using the windowing library loader instead of the (optionally) bundled one.
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
    bool err = false;
    glbinding::Binding::initialize();
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
    bool err = false;
    glbinding::initialize([](const char* name) { return (glbinding::ProcAddress)SDL_GL_GetProcAddress(name); });
#else
    bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
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
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;
    while (!done)
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
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {

            static float vol = 100.0f;
            static float track_position = 0.0f;
            static episode const * cur_ep = nullptr;
            static int counter = 0;
            static ImVec2 menu_size = ImVec2(0,20);
            ImGui::SetNextWindowSize(ImVec2(640,480));

            if(ImGui::BeginMainMenuBar())
            {
                if (ImGui::BeginMenu("Program"))
                {
                    if(ImGui::MenuItem("Load subscriptions")) {
                        load_subscriptions();
                    }
                    if(ImGui::MenuItem("Save subscriptions")) {

                    }
                    if (ImGui::MenuItem("Close"))
                        done = true;
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Podcasts"))
                {
                    if (ImGui::MenuItem("Add podcast via remote RSS")) {

                    }
                    ImGui::EndMenu();
                }

                menu_size = ImGui::GetWindowSize();

                ImGui::EndMainMenuBar();
            }


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
            ImGui::EndChild();

            ImGui::SameLine();

            ImGui::Text("%s", cur_ep ? cur_ep->get_title().c_str() : "");

            ImGui::BeginChild("position slider", ImVec2(-1, 20), false);
            ImGui::SliderFloat("Track Position", &track_position, 0.0f, 100.0f);
            ImGui::EndChild();


            ImGui::EndChild();

            ImGui::BeginGroup();
            ImGui::BeginChild("podcast view", ImVec2(300,-1), true);
            if(ImGui::BeginTable("podcasts", 1)) {
                ImGui::TableNextColumn();
                ImGui::Text("Podcasts");

                for(auto & [name, podcast] : channels) {
                    ImGui::TableNextColumn();
                    if(ImGui::Button(name.c_str())) {
                        open_channel = name;
                    }
                }

                ImGui::EndTable();
            }
            ImGui::EndChild();

            ImGui::SameLine();

            ImGui::BeginChild("episode view", ImVec2(-1,-1), true);
            if(ImGui::BeginTable("episodes", 1)) {
                ImGui::TableNextColumn();
                ImGui::Text("Episodes");

                if(!open_channel.empty()) {
                    auto result_itr = channels.find(open_channel);
                    if(result_itr != channels.end()) {
                        auto &pod = result_itr->second;
                        for(auto &ep : pod.peek_items()) {
                            if(ImGui::Button(ep.get_title().c_str())) {
                                cur_ep = std::addressof(ep);
                                download_or_play(ep);
                            }
                        }

                    }


                }
                ImGui::EndTable();
            }
            ImGui::EndChild();
            ImGui::EndGroup();

            audio_handle.perform(
                [vol=vol, track_position=track_position](audio_abstraction &handle) {
                    handle.set_volume(vol);
                });
            ImGui::End();
        }

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
    fmt::print("Loading subscriptions.\n");
    std::string file_path = project_directory + "subscriptions.json"s;
    std::ifstream save_file(file_path, std::ios::in);

    if(not save_file.is_open()) {
        fmt::print("Error opening file: {}\n", file_path);
        return;
    }

    try {
        boost::property_tree::ptree tree;
        boost::property_tree::read_json(save_file, tree);

        std::map<std::string, podcast> new_channels;

        auto end = tree.end();
        for(auto root = tree.begin(); root != end; ++root) {
            podcast channel(root->first);
            channel.fill_from_xml(root->second);
            new_channels.insert_or_assign(channel.title(), std::move(channel));
        }

        channels = std::move(new_channels);

        for(auto &[name, channel] : channels)
            std::cout << "Loaded channel: " << name << " with " << channel.episode_count() << " episodes." << std::endl;


        sync_ui_with_library_state();
    }
    catch(const std::exception &e) {
        // Leave map in empty state.
        channels.clear();
        std::cout << "An exception occurred: " << e.what() << std::endl;
    }    
}

void dear_imgui_wrapper::download_or_play(episode const & ep) {
    std::string title = ep.get_title();
    std::string url = ep.url();
    std::string file_name = ep.get_sanitized_file_name();

    std::string file_path = project_directory + native_separator + open_channel + native_separator + file_name;

    if( audio_handle.perform(
        [file_path](audio_abstraction &handle) -> bool {
            if(not handle.open_from_file(file_path))
                return false;
            handle.play();
            return true;
        }
        )) {
        // interior of if
        state = UserDesiredState::play;
    }
    else {
        fmt::print("Error opening file: {}\n", file_path);
    }
}