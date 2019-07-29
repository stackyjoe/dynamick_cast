#include <iostream>

#include "music_adapter.hpp"


music_adapter::music_adapter() : state(player::neither)
{

}


sf::Time music_adapter::getDuration() const{
#ifdef USE_SFML_MP3_EXPERIMENTAL
    switch(state) {
    case player::neither:
        return sf::Time();
    case player::generic:
        return generic_handler.getDuration();
    case player::mp3:
        return mp3_handler.getDuration();
    }
#elif
    return generic_handler.getDuration();
#endif
    return sf::Time();
}

sf::Time music_adapter::getPlayingOffset() const {
#ifdef USE_SFML_MP3_EXPERIMENTAL
    switch(state) {
    case player::neither:
        return sf::Time();
    case player::generic:
        return generic_handler.getPlayingOffset();
    case player::mp3:
        return mp3_handler.getPlayingOffset();
    }
#elif
    return generic_handler.getPlayingOffset();
#endif
    return sf::Time();
}

sf::Music::Status music_adapter::getStatus() const {
#ifdef USE_SFML_MP3_EXPERIMENTAL
    switch(state) {
    case player::neither:
        return sf::Music::Status::Stopped;
    case player::generic:
        return generic_handler.getStatus();
    case player::mp3:
        return mp3_handler.getStatus();
    }
#elif
    return generic_handler.getStatus();
#endif
    return sf::Music::Status::Stopped;
}

bool music_adapter::openFromFile(std::string const &filename) {
#ifdef USE_SFML_MP3_EXPERIMENTAL
    std::string extension = filename.substr(filename.find_last_of('.'), 4);
    bool is_mp3 = extension == ".mp3";

    switch(state) {
    case player::neither:
        if(is_mp3) {
            state = player::mp3;
            std::cout << "Attempting to play mp3\n";
            return mp3_handler.openFromFile(filename);
        }
        state = player::generic;
        return generic_handler.openFromFile(filename);
    case player::generic:
        if(is_mp3){
            generic_handler.stop();
            state = player::mp3;
            return mp3_handler.openFromFile(filename);
        }
        return generic_handler.openFromFile(filename);
    case player::mp3:
        if(is_mp3) {
            return mp3_handler.openFromFile(filename);
        }
        mp3_handler.stop();
        state = player::generic;
        generic_handler.openFromFile(filename);
    }
#elif
    return generic_handler.openFromFile(filename);
#endif
    return false;
}

void music_adapter::setPlayingOffset(sf::Time offset) {
#ifdef USE_SFML_MP3_EXPERIMENTAL
    switch(state) {
    case player::neither:
        return;
    case player::generic:
        generic_handler.setPlayingOffset(offset);
        return;
    case player::mp3:
        mp3_handler.setPlayingOffset(offset);
        return;
    }
#elif
    generic_handler.setPlayingOffset(offset);
#endif
}

void music_adapter::setVolume(float volume) {
#ifdef USE_SFML_MP3_EXPERIMENTAL
    switch(state) {
    case player::neither:
        return;
    case player::generic:
        generic_handler.setVolume(volume);
        return;
    case player::mp3:
        mp3_handler.setVolume(volume);
        return;
    }
#elif
    generic_handler.setVolume(volume);
#endif
}

void music_adapter::play() {
#ifdef USE_SFML_MP3_EXPERIMENTAL
    switch(state) {
    case player::neither:
        return;
    case player::generic:
        generic_handler.play();
        return;
    case player::mp3:
        mp3_handler.play();
        return;
    }
#elif
    return generic_handler.play();
#endif
}

void music_adapter::pause() {
#ifdef USE_SFML_MP3_EXPERIMENTAL
    switch(state) {
    case player::neither:
        return;
    case player::generic:
        generic_handler.pause();
        return;
    case player::mp3:
        mp3_handler.pause();
        return;
    }
#elif
    return generic_handler.pause();
#endif
}

void music_adapter::stop() {
#ifdef USE_SFML_MP3_EXPERIMENTAL
    switch(state) {
    case player::neither:
        return;
    case player::generic:
        generic_handler.stop();
        state = player::neither;
        return;
    case player::mp3:
        mp3_handler.stop();
        state = player::neither;
        return;
    }
#elif
    return generic_handler.stop();
#endif
}
