#ifndef RTAUDIO_WRAPPER_HPP
#define RTAUDIO_WRAPPER_HPP

#include <audiere.h>

#include "audio_interface.hpp"

class audiere_wrapper : public audio_wrapper
{
friend std::unique_ptr<audiere_wrapper> std::make_unique<audiere_wrapper>();
public:
    ~audiere_wrapper() override = default;

    audiere_wrapper (const audiere_wrapper &) = delete;
    audiere_wrapper &operator=(const audiere_wrapper &)= delete;

    audiere_wrapper (audiere_wrapper &&) = default;
    audiere_wrapper &operator=(audiere_wrapper &&) = default;

    int estimate_duration() const noexcept override;
    float get_percent_played() const noexcept override;
    PlayerStatus get_status() const noexcept override;
    const std::vector<std::string> & supported_file_formats() const noexcept override;

    void pause() override;
    void play() override;
    void stop() override;

    bool open_from_file(const std::string &path) override;
    void seek_by_percent(float percent) override;
    void set_volume(int new_vol) override;

protected:
    audiere_wrapper();

private:
    std::vector<std::string> supported_formats;
    audiere::AudioDevicePtr device;
    audiere::OutputStreamPtr decoder;
    int estimated_duration;
};

extern audio_interface audio_handle;

#endif // RTAUDIO_WRAPPER_HPP
