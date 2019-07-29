#ifndef MP3_H_INCLUDED
#define MP3_H_INCLUDED

#include <mpg123.h>
#include <SFML/Audio.hpp>

#include <cstdint>
#include <string>

namespace contrib {

class SoundFileReaderMp3 : public sf::SoundFileReader
{
public:
    SoundFileReaderMp3();

    ~SoundFileReaderMp3();

    // \brief Check if this reader can handle a file given by an input stream
    // \param stream Source stream to check
    // \return True if the file is supported by this reader
    static bool check(sf::InputStream& stream);

    // \brief Open a sound file for reading
    // \param stream Source stream to read from
    // \param info   Structure to fill with the properties of the loaded sound
    // \return True if the file was successfully opened
    bool open(sf::InputStream& stream, sf::SoundFileReader::Info& info) override;

    // \brief Change the current read position to the given sample offset
    // If the given offset exceeds to total number of samples,
    // this function must jump to the end of the file.
    // \param sampleOffset Index of the sample to jump to, relative to the beginning
    void seek(sf::Uint64 sampleOffset) override;

    // \brief Read audio samples from the open file
    // \param samples  Pointer to the sample array to fill
    // \param maxCount Maximum number of samples to read
    // \return Number of samples actually read (may be less than \a maxCount)
    sf::Uint64 read(sf::Int16* samples, sf::Uint64 maxCount) override;


protected:

    // Encapsulates mp3 output format
    class OutputFormat {
    private:
        long m_rate = 0;
        int m_channels = 0;
        int m_encoding = 0;
    public:
        bool update (mpg123_handle* handle ){
            return mpg123_getformat(handle, &m_rate, &m_channels, &m_encoding) == MPG123_OK;
        }
        std::string toString() const {
            std::string result;
            result += "format: sample rate in Hz: " + std::to_string(m_rate);
            result += "\nformat: channels: " + std::to_string(m_channels);
            result += "\nformat: encoding: " + std::to_string(m_encoding);
            return result;
        }
    };

    // Easy access to mpg123_frameinfo struct
    class FrameInfo {
    private:
        mpg123_frameinfo  m_info;
        const std::size_t m_headerSize = 4;
    public:
        bool update (mpg123_handle* handle ){
            return mpg123_info(handle, &m_info) == MPG123_OK;
        }
        std::size_t getFrameSizeInBytesIncludingHeader() const {
            return m_info.framesize;
        }
        std::size_t getFrameSizeInBytesNoHeader() const {
            return m_info.framesize - m_headerSize;
        }
        std::size_t getExpectedNumberOfSamples() const {
            return getFrameSizeInBytesNoHeader() / sizeof(short);
        }
        std::uint32_t getSamplingRateInHz() const {
            return m_info.rate;
        }
        std::uint32_t getNumberOfChannels() const {
            switch (m_info.mode)
            {
                case mpg123_mode::MPG123_M_MONO:
                    return 1;
                case mpg123_mode::MPG123_M_STEREO:
                case mpg123_mode::MPG123_M_JOINT:
                case mpg123_mode::MPG123_M_DUAL:
                    return 2;
                default:
                    break;
            }
            return 0;
        }
        std::string toString() const {
            std::string result;
            result += "header: MPEG version: ";
            switch (m_info.version)
            {
                case mpg123_version::MPG123_1_0:
                    result += "1.0";
                    break;
                case mpg123_version::MPG123_2_0:
                    result += "2.0";
                    break;
                case mpg123_version::MPG123_2_5:
                    result += "2.5";
                    break;
                default:
                    result += "unknown version";
                    break;
            }
            result += "\nheader: MPEG Audio mode: ";
            switch (m_info.mode)  // Only the mono mode has 1 channel, the others have 2 channels.
            {
                case mpg123_mode::MPG123_M_STEREO:
                    result += "Standard Stereo (2 channels)";
                    break;
                case mpg123_mode::MPG123_M_JOINT:
                    result += "Joint Stereo (2 channels)";
                    break;
                case mpg123_mode::MPG123_M_DUAL:
                    result += "Dual Channel (2 channels)";
                    break;
                case mpg123_mode::MPG123_M_MONO:
                    result += "Mono (single channel)";
                    break;
                default:
                    result += "unknown mode";
                    break;
            }
        //	enum mpg123_flags 	    flags
            result += "\nheader: Mode type of variable bitrate (vbr): ";
            switch (m_info.vbr)
            {
                case mpg123_vbr::MPG123_CBR:
                    result += "Constant Bitrate Mode (default)";
                    break;
                case mpg123_vbr::MPG123_VBR:
                    result += "Variable Bitrate Mode";
                    break;
                case mpg123_vbr::MPG123_ABR:
                    result += "Average Bitrate Mode";
                    break;
                default:
                    result += "unknown vbr mode";
                    break;
            }
            result += "\nheader: MPEG Audio Layer (MP1/MP2/MP3): " + std::to_string(m_info.layer);
            result += "\nheader: Mode extension bit flag: " + std::to_string(m_info.mode_ext);
            result += "\nheader: Emphasis type (?): " + std::to_string(m_info.emphasis);
            result += "\nheader: Target average bitrate: " + std::to_string(m_info.abr_rate);
            result += "\nheader: Bitrate of the frame (kbps): " + std::to_string(m_info.bitrate);
            result += "\nheader: Sampling rate in Hz: " + std::to_string(m_info.rate);
            result += "\nheader: Size of the frame (in bytes, including header): " + std::to_string(m_info.framesize);

            return result;
        }
    };

    // library objects
    mpg123_handle*      m_handle;
    sf::InputStream*    m_stream;
    FrameInfo           m_frameInfo;
    OutputFormat        m_outputFormat;

    // buffers
    std::size_t         m_streambufferSize;
    unsigned char*      m_streambuffer;

    // Creates the handle to the mp3 library
    bool initializeLibrary();
    // Used by open() to consume just enough data to retrieve frame info
    // and output format so we can set parameters in sf::SoundFileReader::Info
    bool probeFirstFrame(sf::InputStream& stream);
    void fillSfmlInfo(sf::SoundFileReader::Info& info) const;
    // Called by destructor to close the library and delete the buffers
    void close();
};

}
#endif // MP3_H_INCLUDED
