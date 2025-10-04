"C:\MinGW64\bin\g++.exe" globldef.c -c -std=c++11 -m64 -o globldef_64.o
"C:\MinGW64\bin\g++.exe" cstrdef.c -c -std=c++11 -m64 -o cstrdef_64.o
"C:\MinGW64\bin\g++.exe" thread.c -c -std=c++11 -m64 -o thread_64.o
"C:\MinGW64\bin\g++.exe" strdef.cpp -c -std=c++11 -m64 -o strdef_64.o
"C:\MinGW64\bin\g++.exe" main.cpp -c -std=c++11 -m64 -o main_64.o
"C:\MinGW64\bin\g++.exe" AudioPB.cpp -c -std=c++11 -m64 -o AudioPB_64.o
"C:\MinGW64\bin\g++.exe" AudioPB_i16.cpp -c -std=c++11 -m64 -o AudioPB_i16_64.o
"C:\MinGW64\bin\g++.exe" AudioPB_i24.cpp -c -std=c++11 -m64 -o AudioPB_i24_64.o

"C:\MinGW64\bin\g++.exe" main_64.o globldef_64.o cstrdef_64.o thread_64.o strdef_64.o AudioPB_64.o AudioPB_i16_64.o AudioPB_i24_64.o -lole32 -lksuser -mwindows -m64 -o pb64.exe

del globldef_64.o
del cstrdef_64.o
del thread_64.o
del strdef_64.o
del main_64.o
del AudioPB_64.o
del AudioPB_i16_64.o
del AudioPB_i24_64.o
