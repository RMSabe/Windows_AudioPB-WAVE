WAVE Audio Playback for Windows
Version 1.3 (mono-stereo)

This version is a variation of v1.3, for allowing playback of mono/stereo audio files on stereo playback devices.

This application supports .wav files, 16bit and 24bit, mono and stereo.

Unfortunately, I still couldn't figure out completely how Windows MMDeviceAPI works, so there are still a lot of limitations to this application:

1. It will only access the playback audio device in exclusive mode.
(I tried using shared mode, but that doesn't use either WAVE or IEEE FLOAT streaming format, instead it uses another format that I don't know how works).
This means that if any other application is using the intended audio device, this application will not work.
If this application is running, no other application will have access to the audio device.

2. Since this program uses exclusive audio device access, the audio file sampling rate must be compatible with the audio device for it to work.

Latest Update:
Native support for 24bit audio
Some bug fixes

Author: Rafael Sabe
Email: rafaelmsabe@gmail.com
