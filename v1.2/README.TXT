WAVE Audio Playback for Windows
Version 1.2

This application supports .wav files, 16bit and 24bit, mono and stereo.

Unfortunately, I still couldn't figure out completely how Windows MMDeviceAPI works, so there are still a lot of limitations to this application:

1. It will only access the playback audio device in exclusive mode.
(I tried using shared mode, but that doesn't use either WAVE or IEEE FLOAT streaming format, instead it uses another format that I don't know how works).
This means that if any other application is using the intended audio device, this application will not work.
If this application is running, no other application will have access to the audio device.

2. Even running in WAVE format exclusive mode, I couldn't figure out how to make it work for 24bit audio.
This means that every playback will use the WAVE 16bit stereo stream format.
This application supports 24bit files, however the audio resolution will be reduced to 16bit to match the stream format.
The reason I'm sticking to 16bit 2 channel stream format is that most audio devices (especially motherboard audio codecs) are expected to work fine with this format.

Lastest Update: Some bug fixes.

Author: Rafael Sabe
Email: rafaelmsabe@gmail.com
