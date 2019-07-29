#ifndef MP3_H_INCLUDED
#define MP3_H_INCLUDED

#include <SFML/Audio.hpp>
#include "mpg123.h"

namespace sfe
{
class Mp3 : public sf::SoundStream
{
public :
    Mp3();
    ~Mp3();

    bool OpenFromFile(const std::string& filename);
    bool OpenFromMemory(void* data, size_t sizeInBytes);

protected :
    bool OnGetData(Chunk& data);
    void OnSeek(sf::Time timeOffset);

private :
    mpg123_handle*      myHandle;
    size_t              myBufferSize;
    unsigned char*      myBuffer;
    sf::Mutex           myMutex;
    long                mySamplingRate;
};

} // namespace sfe

#endif // MP3_H_INCLUDED