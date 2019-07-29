#ifndef MP3_H_INCLUDED
#define MP3_H_INCLUDED

#include <SFML/Audio.hpp>
#include "mpg123.h"

namespace contrib
{
class Mp3 : public sf::SoundStream
{
public :
    Mp3();
    ~Mp3();

    sf::Time getDuration() const;

    bool openFromFile(const std::string& filename);
    bool OpenFromMemory(void* data, size_t sizeInBytes);

protected :
    bool onGetData(Chunk& data) override;
    void onSeek(sf::Time timeOffset) override;

private :
    mpg123_handle*      myHandle;
    size_t              myBufferSize;
    unsigned char*      myBuffer;
    sf::Mutex           myMutex;
    long                mySamplingRate;
};

} // namespace contrib

#endif // MP3_H_INCLUDED
