#include <iostream>

#include "audio_backends/audiere_wrapper.hpp"

audio_wrapper::~audio_wrapper() = default;

using std::string_literals::operator""s;

audiere_wrapper::audiere_wrapper() :
    supported_formats({"*.wav"s,"*.mp3"s,"*.ogg"s, "*.flac"s, "*.aiff"s}),
    device(audiere::OpenDevice()),
    estimated_duration(0)
{
    if(device.get() == nullptr) {
        std::vector<audiere::AudioDeviceDesc> device_descriptions;
        audiere::GetSupportedAudioDevices(device_descriptions);

        std::cout << "Found " << device_descriptions.size() << " devices.\n";
        for(auto const &desc : device_descriptions)
            std::cout << "audio device: " << desc.name << "\n";

        auto * ptr = audiere::OpenDevice(device_descriptions[0].name.c_str());
        device = ptr;
        if(device.get() == nullptr)
            throw std::runtime_error("unable to get a working audio device.");
    }
}

int audiere_wrapper::estimate_duration() const noexcept {
    return estimated_duration;
}

float audiere_wrapper::get_percent_played() const noexcept {
    if(decoder.get() == nullptr)
        return 0;

    return static_cast<float>(decoder->getPosition())/static_cast<float>(std::max(1,decoder->getLength()));
}

PlayerStatus audiere_wrapper::get_status() const noexcept {
    if(decoder.get() == nullptr)
        return PlayerStatus::stopped;

    if(decoder->isPlaying())
        return PlayerStatus::playing;
    if(decoder->isSeekable()) {
        // quick and dirty way to check to see if the track finished.
        // If the user is able to pause before any chunks are played
        // it will be incorrectly reported as stopped.
        if(decoder->getPosition() != 0)
            return PlayerStatus::paused;

        return PlayerStatus::stopped;
    }

    return PlayerStatus::stopped;
}

const std::vector<std::string> & audiere_wrapper::supported_file_formats() const noexcept {
    return supported_formats;
}

void audiere_wrapper::pause() {
    if(decoder.get() == nullptr)
        return;

    decoder->stop();
}

void audiere_wrapper::play() {
    if(decoder.get() == nullptr)
        return;

    fmt::print("Audiere backend attempting to play.\n");

    decoder->setRepeat(false);
    decoder->play();
}

void audiere_wrapper::stop() {
    if(decoder.get() == nullptr)
        return;

    decoder->stop();
    decoder->reset();
}

bool audiere_wrapper::open_from_file(const std::string &path) {
    audiere::OutputStream * os = audiere::OpenSound(device, path.c_str(), true);
    if(os == nullptr) {
        if(device.get() == nullptr)
            fmt::print("Null audio device.\n");
        fmt::"Unable to open sound file: {}\n", path);
        return false;
    }

    if(audiere::SampleSourcePtr a { audiere::OpenSampleSource(path.c_str()) }; a.get() != nullptr) {
        int channel_count, sample_rate;
        audiere::SampleFormat sample_format;
        a->getFormat(channel_count, sample_rate, sample_format);
        estimated_duration = os->getLength()/std::max(1,sample_rate);
    }
    else {
        estimated_duration = 0;
    }

    decoder = audiere::OutputStreamPtr(os);
    return true;
}

void audiere_wrapper::seek_by_percent(float percent) {
    if(decoder.get() == nullptr)
        return;

    float pos = percent*decoder->getLength();
    decoder->setPosition(static_cast<int>(pos));
}

void audiere_wrapper::set_volume(int new_vol) {
    if(decoder.get() == nullptr)
        return;

    decoder->setVolume(new_vol*0.01f);
}

thread_safe_interface<audio_abstraction> make_audio() {
    return thread_safe_interface<audio_abstraction>::make<audiere_wrapper>();
}
