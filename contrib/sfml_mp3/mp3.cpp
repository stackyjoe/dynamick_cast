#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>

extern "C" {
#define __STC_LIB_EXT1__
#define __STDC_WANT_LIB_EXT1__ 1
#include <string.h>
}

#include "contrib/sfml_mp3/mp3.hpp"

namespace {
    constexpr std::byte zero_byte{0x00};
    int pseudo_memcpy_s( std::byte * __restrict destination,
                         ptrdiff_t destination_size,
                         std::byte * __restrict source,
                         ptrdiff_t count) {
        if(destination == nullptr)
            return 1;
        if(/* ranges overlap */ (source <= destination and destination <= source+count) or (destination <= source and source <= destination + destination_size) or
        /* source is nullptr */ source == nullptr or
        /* would overflow buffer */ count > destination_size)
            std::fill(destination, destination+destination_size, zero_byte);

        std::copy(destination, destination+destination_size, source);
        return 0;
    }
}


namespace contrib
{
// Used for OpenFromMemory
struct Mp3MemoryData
{
    void* data;
    size_t size;
    off_t offset;
};

// Custom reader I/O for OpenFromMemory
ssize_t MemoryDataRead(void* rawMp3Data, void* buffer, size_t nbyte);
off_t MemoryDataLSeek(void* rawMp3Data, off_t offset, int whence);
void MemoryDataCleanup(void* rawMp3Data);

Mp3::Mp3() :
myHandle    (NULL),
myBufferSize(0),
myBuffer    (NULL),
mySamplingRate(0)
{
    int  err = MPG123_OK;
    if ((err = mpg123_init()) != MPG123_OK)
    {
        std::cerr << mpg123_plain_strerror(err) << std::endl;
        return;
    }

    myHandle = mpg123_new(NULL, &err);
    if (!myHandle)
    {
        std::cerr << "Unable to create mpg123 handle: " << mpg123_plain_strerror(err) << std::endl;
        return;
    }

    mpg123_replace_reader_handle(myHandle, &MemoryDataRead, &MemoryDataLSeek, &MemoryDataCleanup);
}

Mp3::~Mp3()
{
    stop();

    if (myBuffer)
    {
        delete [] myBuffer;
        myBuffer = NULL;
    }

    mpg123_close(myHandle);
    mpg123_delete(myHandle);
    mpg123_exit();
}

sf::Time Mp3::getDuration() const {
    constexpr float magic_number = 26.122*1.0065681445/1.16049382716;

    // mpg123_length returns the number of frames. Were the mp3 encoded with
    // a fixed bitrate, it would be trivial to compute the play length based on this.
    // however in practice podcasts (the purpose of this app) use variable bit rate
    // encodings to reduce filesizes. This has the downside of making it a bit tricky
    // to accurately report duration, there's no real way around scanning the entire file
    // which is intrinsically at odds with the method of streaming it.

    return sf::microseconds(mpg123_length(myHandle)*magic_number);
}

bool Mp3::openFromFile(const std::string& filename)
{
    stop();

    if (myBuffer)
    {
        delete [] myBuffer;
        myBuffer = nullptr;
    }

    if(myHandle)
      mpg123_close(myHandle);

    if (mpg123_open(myHandle, filename.c_str()) != MPG123_OK)
    {
        std::cout << "Error: " << mpg123_strerror(myHandle) << std::endl;
        return false;
    }

    mpg123_scan(myHandle);

    long rate = 0;
    int  channels = 0, encoding = 0;
    if (mpg123_getformat(myHandle, &rate, &channels, &encoding) != MPG123_OK)
    {
        std::cout << "Failed to get format information for \"" << filename << "\"" << std::endl;
        return false;
    }
    mySamplingRate = rate;

    myBufferSize = mpg123_outblock(myHandle);
    myBuffer = new unsigned char[myBufferSize];
    if (!myBuffer)
    {
        std::cout << "Failed to reserve memory for decoding one frame for \"" << filename << "\"" << std::endl;
        return false;
    }

    initialize(channels, rate);

    return true;
}

bool Mp3::OpenFromMemory(void* data, size_t sizeInBytes)
{
    stop();

    if (myBuffer)
    {
        delete [] myBuffer;
        myBuffer = nullptr;
    }

    if(myHandle)
      mpg123_close(myHandle);

    Mp3MemoryData* mp3Data = new Mp3MemoryData{ data, sizeInBytes, 0 };
    if (!mp3Data)
    {
        std::cout << "Failed to reserve memory for keeping track of Memory Object" << std::endl;
    }

    if (mpg123_open_handle(myHandle, mp3Data) != MPG123_OK)
    {
        std::cout << "Error: " << mpg123_strerror(myHandle) << std::endl;
        delete mp3Data;
        mp3Data = NULL;
        return false;
    }

    long rate = 0;
    int  channels = 0, encoding = 0;
    if (mpg123_getformat(myHandle, &rate, &channels, &encoding) != MPG123_OK)
    {
        std::cout << "Failed to get format information for Memory Object" << std::endl;
        return false;
    }
    mySamplingRate = rate;

    myBufferSize = mpg123_outblock(myHandle);
    myBuffer = new unsigned char[myBufferSize];
    if (!myBuffer)
    {
        std::cerr << "Failed to reserve memory for decoding one frame for Memory Object" << std::endl;
        return false;
    }

    initialize(channels, rate);

    return true;
}

bool Mp3::onGetData(Chunk& data)
{
    sf::Lock lock(myMutex);

    if (myHandle)
    {
        size_t done;
        mpg123_read(myHandle, myBuffer, myBufferSize, &done);

        data.samples   = (short*)myBuffer;
        data.sampleCount = done/sizeof(short);

        return (data.sampleCount > 0);
    }
    else
        return false;
}

void Mp3::onSeek(sf::Time timeOffset)
{
    sf::Lock lock(myMutex);

    // tschumacher: sampleoff must be (seconds * samplingRate) to make this working correctly
    if (myHandle)
        mpg123_seek(myHandle, static_cast<off_t>(timeOffset.asSeconds() * mySamplingRate), SEEK_SET);
}

ssize_t MemoryDataRead(void* rawMp3Data, void* buffer, size_t nbyte)
{
    Mp3MemoryData* mp3Data = (Mp3MemoryData*)rawMp3Data;
    if (mp3Data->offset >= (ssize_t)mp3Data->size)
    {
        memset(buffer, 0, nbyte);
        return (ssize_t)0;
    }
    else if (mp3Data->offset + (ssize_t)nbyte > (ssize_t)mp3Data->size)
    {
        size_t readSize = mp3Data->size - mp3Data->offset;
        size_t memSetSize = mp3Data->offset + nbyte - (ssize_t)mp3Data->size;
        pseudo_memcpy_s(static_cast<std::byte*>(buffer), readSize, static_cast<std::byte*>(mp3Data->data) + mp3Data->offset, readSize);
        memset(buffer, 0, memSetSize);
        mp3Data->offset += readSize;
        return (ssize_t)readSize;
    }
    else
    {
        pseudo_memcpy_s(static_cast<std::byte*>(buffer), nbyte, static_cast<std::byte*>(mp3Data->data) + mp3Data->offset, nbyte);
        mp3Data->offset += nbyte;
        return (ssize_t)nbyte;
    }
}

off_t MemoryDataLSeek(void* rawMp3Data, off_t offset, int whence)
{
    Mp3MemoryData* mp3Data = (Mp3MemoryData*)rawMp3Data;
    switch (whence)
    {
    case SEEK_SET: mp3Data->offset = offset; break;
    case SEEK_CUR: mp3Data->offset += offset; break;
    case SEEK_END: mp3Data->offset = mp3Data->size + offset; break;
    }
    return mp3Data->offset;
}

void MemoryDataCleanup(void* rawMp3Data)
{
    delete[] static_cast<std::byte*>(rawMp3Data);
}

} // namespace contrib
