WAVE Audio Playback for Windows
Version 2.0 (Interop C++ C# version 1.0)

This application supports .wav files, 16bit and 24bit.

This application uses C# and dotnet WPF resources to handle GUI, while everything else is done in C/C++.
I could implement the GUI in C++ using the legacy Win32 resources, but it makes the codebase very messy and I don't think they work very well on recent versions of Windows.

Unfortunately, I still couldn't figure out completely how Windows MMDeviceAPI works, so there are still a lot of limitations to this application:

1. It will only access the playback audio device in exclusive mode.
(I tried using shared mode, but that doesn't use either WAVE or IEEE FLOAT streaming format, instead it uses another format that I don't know how works).
This means that if any other application is using the intended audio device, this application will not work.
If this application is running, no other application will have access to the audio device.

2. Since the application accesses the audio device in exclusive mode, bit-depth, sample-rate and number of channels must be compatible with the audio device for it to work.

3. For this application, I'm focusing more on mono and stereo audio files. Files with more channels might work, but channels might be misplaced.
I do not recommend using this application for audio files with more than 2 channels.

Compilation Notes:

This project currently supports 32-bit and 64-bit x86 based CPUs.
The C/C++ code is compiled into a DLL file named core.dll. The output file "core.dll" must be placed in the same folder as the C# WPF .exe file.
Both the C# and C++ output files must have the same target CPU (32-bit x86 or 64-bit x86).

Latest Updates:
Some bug fixes.
Code Optimization.
New Features.

Author: Rafael Sabe
Email: rafaelmsabe@gmail.com
