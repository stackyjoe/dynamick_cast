# dynamick_cast
Dynamick cast is a simple podcast subscription and playback application for the desktop. So far it compiles on FreeBSD 12 and Ubuntu 18.04. Uses the following libraries:

+ Audiere for audio handling.
    - This is quite old and will need to be replaced.
    - Likely I'll just wait until SFML adds mp3 support.
    - Bug: on Ubuntu the application needs to be run with padsp for audiere to find the OSS audio device.
+ Boost libraries
    - I've only successfully built with versions 1.68 and higher.
    - In particular, on Ubuntu 18.04 to build the project you will have to manually build Boost libraries and install them, as the apt package is 1.65 (and last I tried it the build will fail).
+ PugiXML
    - I did not remove the code to parse XML using Boost `property_tree`s. However they seem to work very inconsistently.
