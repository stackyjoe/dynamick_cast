#ifndef AUDIO_INTERFACE_HPP
#define AUDIO_INTERFACE_HPP

#include <memory>
#include <type_traits>

#include "audio_wrapper.hpp"

/// \brief This is a thread safe interface for the abstract class music_player.
/// The class audio_interface is an interface in the sense of the pimpl idiom.
/// Unfortunately, the implementation is an abstract class, which are also called
/// interfaces. This is the one which kept the title. This class only directly
/// exposes a few functions of the wrapper class, but everything can be called
/// through the "perform" or "try_perform" template function, which enforces
/// proper mutex safety.
class  audio_interface {
private:
    std::unique_ptr<audio_wrapper> implementation;

    template<class audio_implementation>
    explicit audio_interface(std::unique_ptr<audio_implementation> _implementation)
        : implementation(std::move(_implementation)) {
        static_assert (std::is_base_of<audio_wrapper, audio_implementation>::value,
                       "audio_interface template constructor must be called with a std::unique_ptr to a class derived from audio_player.");
    }

public:

    const std::vector<std::string> & supported_file_formats() const noexcept {
        return implementation->supported_file_formats();
    }

    PlayerStatus get_status() const noexcept {
        return implementation->get_status();
    }

    float get_percent_played() const noexcept {
        return implementation->get_percent_played();
    }

    audio_interface() = default;
    // Copy constructor and copy assignment are implicitly forbidden by virtue of owning a unique_ptr and mutex.
    audio_interface(audio_interface &&) = default;
    audio_interface &operator=(audio_interface &&) = default;

    audio_interface(audio_interface const &) = delete;
    audio_interface &operator=(audio_interface const &) = delete;

    ~audio_interface() = default;


    template<class audio_implementation>
    static audio_interface make() {
        static_assert (std::is_base_of<audio_wrapper,audio_implementation>::value,
                       "audio_interface::make must be called with an implementation of the audio_wrapper class.");
        return audio_interface(std::make_unique<audio_implementation>());
    }

    // Trick to avoid sharing a mutex for the music_player directly, originally due to Herb Sutter,
    // at least as far as I know. Checking with a toy example in M. Godbolt's Compiler Explorer
    // suggests that on -O0, -Og, and -O1 you cannot assume the lambda calls will be optimized out,
    // but that they likely are on higher optimization settings, at least for GCC and clang.
    template<class operation>
    auto perform(operation o) const -> decltype (o(*implementation)) {
        static_assert (std::is_invocable<decltype(o), audio_wrapper &>::value,
                       "audio_interface::perform should be called on lambda invocable with a audio_wrapper & argument.");
        std::scoped_lock access(implementation->access_lock);
        return o(*implementation);
    }

    template<class operation>
    auto try_perform(operation o) const -> std::optional<decltype(o(implementation))> {
        static_assert (std::is_invocable<decltype(o), audio_wrapper &>::value,
                       "audio_interface::try_perform should be called on lambda invocable with a audio_wrapper & argument.");

        if(std::unique_lock access(implementation->access_lock, std::try_to_lock); not access.owns_lock())
            return std::nullopt;
        return o(implementation);
    }
};

#endif // AUDIO_INTERFACE_HPP
