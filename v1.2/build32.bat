"C:\MinGW64\bin\g++.exe" globldef.c -c -m32 -std=c++11 -o globldef_32.o
"C:\MinGW64\bin\g++.exe" cstrdef.c -c -m32 -std=c++11 -o cstrdef_32.o
"C:\MinGW64\bin\g++.exe" thread.c -c -m32 -std=c++11 -o thread_32.o
"C:\MinGW64\bin\g++.exe" strdef.cpp -c -m32 -std=c++11 -o strdef_32.o
"C:\MinGW64\bin\g++.exe" AudioPB.cpp -c -m32 -std=c++11 -o AudioPB_32.o
"C:\MinGW64\bin\g++.exe" AudioPB_i16_1ch.cpp -c -m32 -std=c++11 -o AudioPB_i16_1ch_32.o
"C:\MinGW64\bin\g++.exe" AudioPB_i16_2ch.cpp -c -m32 -std=c++11 -o AudioPB_i16_2ch_32.o
"C:\MinGW64\bin\g++.exe" AudioPB_i24_1ch.cpp -c -m32 -std=c++11 -o AudioPB_i24_1ch_32.o
"C:\MinGW64\bin\g++.exe" AudioPB_i24_2ch.cpp -c -m32 -std=c++11 -o AudioPB_i24_2ch_32.o
"C:\MinGW64\bin\g++.exe" main.cpp -c -m32 -std=c++11 -o main_32.o

"C:\MinGW64\bin\g++.exe" main_32.o globldef_32.o cstrdef_32.o thread_32.o strdef_32.o AudioPB_32.o AudioPB_i16_1ch_32.o AudioPB_i16_2ch_32.o AudioPB_i24_1ch_32.o AudioPB_i24_2ch_32.o -lole32 -mwindows -m32 -o pb32.exe

del globldef_32.o
del cstrdef_32.o
del thread_32.o
del strdef_32.o
del AudioPB_32.o
del AudioPB_i16_1ch_32.o
del AudioPB_i16_2ch_32.o
del AudioPB_i24_1ch_32.o
del AudioPB_i24_2ch_32.o
del main_32.o
