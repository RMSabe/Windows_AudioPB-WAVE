"C:\MinGW64\bin\g++.exe" globldef.c -c -m64 -std=c++11 -o globldef_64.o
"C:\MinGW64\bin\g++.exe" cstrdef.c -c -m64 -std=c++11 -o cstrdef_64.o
"C:\MinGW64\bin\g++.exe" thread.c -c -m64 -std=c++11 -o thread_64.o
"C:\MinGW64\bin\g++.exe" strdef.cpp -c -m64 -std=c++11 -o strdef_64.o
"C:\MinGW64\bin\g++.exe" AudioPB.cpp -c -m64 -std=c++11 -o AudioPB_64.o
"C:\MinGW64\bin\g++.exe" AudioPB_i16_1ch.cpp -c -m64 -std=c++11 -o AudioPB_i16_1ch_64.o
"C:\MinGW64\bin\g++.exe" AudioPB_i16_2ch.cpp -c -m64 -std=c++11 -o AudioPB_i16_2ch_64.o
"C:\MinGW64\bin\g++.exe" AudioPB_i24_1ch.cpp -c -m64 -std=c++11 -o AudioPB_i24_1ch_64.o
"C:\MinGW64\bin\g++.exe" AudioPB_i24_2ch.cpp -c -m64 -std=c++11 -o AudioPB_i24_2ch_64.o
"C:\MinGW64\bin\g++.exe" main.cpp -c -m64 -std=c++11 -o main_64.o

"C:\MinGW64\bin\g++.exe" main_64.o globldef_64.o cstrdef_64.o thread_64.o strdef_64.o AudioPB_64.o AudioPB_i16_1ch_64.o AudioPB_i16_2ch_64.o AudioPB_i24_1ch_64.o AudioPB_i24_2ch_64.o -lole32 -mwindows -m64 -o pb64.exe

del globldef_64.o
del cstrdef_64.o
del thread_64.o
del strdef_64.o
del AudioPB_64.o
del AudioPB_i16_1ch_64.o
del AudioPB_i16_2ch_64.o
del AudioPB_i24_1ch_64.o
del AudioPB_i24_2ch_64.o
del main_64.o
