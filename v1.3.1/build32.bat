"C:\MinGW64\bin\g++.exe" globldef.c -c -std=c++11 -m32 -o globldef_32.o
"C:\MinGW64\bin\g++.exe" cstrdef.c -c -std=c++11 -m32 -o cstrdef_32.o
"C:\MinGW64\bin\g++.exe" thread.c -c -std=c++11 -m32 -o thread_32.o
"C:\MinGW64\bin\g++.exe" strdef.cpp -c -std=c++11 -m32 -o strdef_32.o
"C:\MinGW64\bin\g++.exe" main.cpp -c -std=c++11 -m32 -o main_32.o
"C:\MinGW64\bin\g++.exe" AudioPB.cpp -c -std=c++11 -m32 -o AudioPB_32.o
"C:\MinGW64\bin\g++.exe" AudioPB_i16.cpp -c -std=c++11 -m32 -o AudioPB_i16_32.o
"C:\MinGW64\bin\g++.exe" AudioPB_i24.cpp -c -std=c++11 -m32 -o AudioPB_i24_32.o

"C:\MinGW64\bin\g++.exe" main_32.o globldef_32.o cstrdef_32.o thread_32.o strdef_32.o AudioPB_32.o AudioPB_i16_32.o AudioPB_i24_32.o -lole32 -lksuser -mwindows -m32 -o pb32.exe

del globldef_32.o
del cstrdef_32.o
del thread_32.o
del strdef_32.o
del main_32.o
del AudioPB_32.o
del AudioPB_i16_32.o
del AudioPB_i24_32.o
