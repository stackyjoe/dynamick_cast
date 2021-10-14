#ifndef AUDIO_ABSTRACTION_HPP
#define AUDIO_ABSTRACTION_HPP

#include <mutex>
#include <optional>
#include <string>

#include "dynamick_cast/thread_safe_interface.hpp"

class audio_interface;
enum class PlayerStatus { error, playing, paused, stopped };

/// \class audio_abstraction
/// \brief The audio_abstraction class is an interface for different backends. Currently,
///  only an SFML audio backend is implemented, however it should be pretty easy to
/// implement others as needed. Implementations MUST declare
///     friend std::unique_ptr<impl_class_name> std::make_unique<impl_class_name>();
/// for the template make() function in audio_interface to compile.

class audio_abstraction
{
public:
    audio_abstraction() = default;
    virtual ~audio_abstraction() = default;

    /// \brief Get a list of accepted file formats for the audio backend.
    /// \return A const reference to a vector of strings. The implementation
    /// is expected to own the memory.
    virtual const std::vector<std::string> &supported_file_formats()  const noexcept = 0;
    virtual int estimate_duration() const noexcept = 0;
    virtual PlayerStatus get_status() const noexcept = 0;
    virtual float get_percent_played() const noexcept = 0;

    virtual void pause() = 0;
    virtual void play() = 0;
    virtual void stop() = 0;

    /// \brief openFromFile
    /// \param A string path giving the absolute path to the audio file.
    /// \return A bool indicating success or failure opening and streaming the
    /// audio file.
    virtual bool open_from_file(const std::string &path) = 0;
    virtual void seek_by_percent(float percent) = 0;
    virtual void set_volume( int new_vol) = 0;

    mutable std::mutex access_lock;

};

thread_safe_interface<audio_abstraction> make_audio();

#endif // AUDIO_ABSTRACTION_HPP
