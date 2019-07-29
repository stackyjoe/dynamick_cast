#ifndef MUSIC_ADAPTER_HPP
#define MUSIC_ADAPTER_HPP

#include <SFML/Audio.hpp>

#ifdef USE_SFML_MP3_EXPERIMENTAL
#include "contrib/sfml_mp3/mp3.hpp"
#endif

class music_adapter
{
public:
    music_adapter();

    enum class player{
        neither,
        generic,
        mp3
    };

    sf::Time getDuration() const;
    sf::Time getPlayingOffset() const;
    sf::Music::Status getStatus() const;
    bool openFromFile(std::string const &filename);
    void setPlayingOffset(sf::Time offset);
    void setVolume(float volume);

    void play();
    void pause();
    void stop();

protected:
    sf::Music generic_handler;
#ifdef USE_SFML_MP3_EXPERIMENTAL
    contrib::Mp3 mp3_handler;

    player state;
#endif
};

#endif // MUSIC_ADAPTER_HPP
