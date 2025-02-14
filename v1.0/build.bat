"C:\MinGW64\bin\g++.exe" globldef.c -c -std=c++11 -o globldef.o
"C:\MinGW64\bin\g++.exe" cstrdef.c -c -std=c++11 -o cstrdef.o
"C:\MinGW64\bin\g++.exe" console.c -c -std=c++11 -o console.o
"C:\MinGW64\bin\g++.exe" strdef.cpp -c -std=c++11 -o strdef.o
"C:\MinGW64\bin\g++.exe" main.cpp -c -std=c++11 -o main.o
"C:\MinGW64\bin\g++.exe" AudioBaseClass.cpp -c -std=c++11 -o AudioBaseClass.o
"C:\MinGW64\bin\g++.exe" AudioPB_i16_1ch.cpp -c -std=c++11 -o AudioPB_i16_1ch.o
"C:\MinGW64\bin\g++.exe" AudioPB_i16_2ch.cpp -c -std=c++11 -o AudioPB_i16_2ch.o
"C:\MinGW64\bin\g++.exe" AudioPB_i24_1ch.cpp -c -std=c++11 -o AudioPB_i24_1ch.o
"C:\MinGW64\bin\g++.exe" AudioPB_i24_2ch.cpp -c -std=c++11 -o AudioPB_i24_2ch.o

"C:\MinGW64\bin\g++.exe" main.o globldef.o cstrdef.o console.o strdef.o AudioBaseClass.o AudioPB_i16_1ch.o AudioPB_i16_2ch.o AudioPB_i24_1ch.o AudioPB_i24_2ch.o -lole32 -o playback.exe

del globldef.o
del cstrdef.o
del console.o
del strdef.o
del main.o
del AudioBaseClass.o
del AudioPB_i16_1ch.o
del AudioPB_i16_2ch.o
del AudioPB_i24_1ch.o
del AudioPB_i24_2ch.o
