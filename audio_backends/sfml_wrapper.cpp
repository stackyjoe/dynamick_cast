#include "audio_backends/sfml_wrapper.hpp"

#if USE_SFML_MP3_EXPERIMENTAL
#include "contrib/sfml_mp3/mp3.hpp"
#endif

#include <iostream>

audio_wrapper::~audio_wrapper() = default;

using std::string_literals::operator""s;

sfml_wrapper::sfml_wrapper()
    : supported_formats(
          #ifdef USE_SFML_MP3_EXPERIMENTAL
          {"*.wav"s, "*.ogg"s, "*.flac"s, "*.mp3"s}
          #else
          {"*.wav"s, "*.ogg"s, "*.flac"s}
          #endif
          ) {

}

int sfml_wrapper::estimate_duration() const noexcept {
    return static_cast<int>(interface.getDuration().asSeconds());
}

float sfml_wrapper::get_percent_played() const noexcept {
    return static_cast<float>(interface.getPlayingOffset().asMilliseconds())/static_cast<float>(std::max(interface.getDuration().asMilliseconds(),1));
}

PlayerStatus sfml_wrapper::get_status() const noexcept {
    PlayerStatus st = PlayerStatus::error;

    switch(interface.getStatus()) {
    case sf::Music::Status::Paused:
        st = PlayerStatus::paused;
        break;

    case sf::Music::Status::Playing:
        st = PlayerStatus::playing;
        break;

    case sf::Music::Status::Stopped:
        st = PlayerStatus::stopped;
        break;

    }

    return st;
}

const std::vector<std::string> & sfml_wrapper::supported_file_formats() const noexcept {
    return supported_formats;
}


void sfml_wrapper::play() {
    interface.play();
    return;
}

void sfml_wrapper::pause() {
    interface.pause();
    return;
}

void sfml_wrapper::stop() {
    interface.stop();
    return;
}

bool sfml_wrapper::open_from_file(const std::string &path) {

    return interface.openFromFile(path);
}

void sfml_wrapper::seek_by_percent(float percent) {
    interface.setPlayingOffset(percent*interface.getDuration());
    return;
}

void sfml_wrapper::set_volume(int new_vol) {
    interface.setVolume(static_cast<float>(new_vol));
    return;
}


audio_interface audio_handle = audio_interface::make<sfml_wrapper>();
