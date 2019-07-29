#ifndef SFML_WRAPPER_HPP
#define SFML_WRAPPER_HPP

#include <SFML/Audio.hpp>

#include "contrib/sfml_mp3/music_adapter.hpp"

#include "audio_interface.hpp"

class sfml_wrapper : public audio_wrapper
{
friend std::unique_ptr<sfml_wrapper> std::make_unique<sfml_wrapper>();
public:
    ~sfml_wrapper() override = default;

    sfml_wrapper(const sfml_wrapper &other) = delete;
    sfml_wrapper &operator=(const sfml_wrapper &other) = delete;

    sfml_wrapper(sfml_wrapper &&other) = default;
    sfml_wrapper &operator=(sfml_wrapper &&other) = default;

    int estimate_duration() const noexcept override;
    float get_percent_played() const noexcept override;
    PlayerStatus get_status() const noexcept override;
    std::vector<std::string> const & supported_file_formats() const noexcept override;


    void pause() override;
    void play() override;
    void stop() override;

    bool open_from_file(const std::string &path) override;
    void seek_by_percent(float percent) override;
    void set_volume(int new_vol) override;

protected:
    sfml_wrapper();

private:
    std::vector<std::string> const supported_formats;
    music_adapter interface;
};

extern audio_interface audio_handle;

#endif // SFML_WRAPPER_HPP
