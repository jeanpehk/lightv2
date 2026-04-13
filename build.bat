@echo off
cls

set SDL_LIB_PATH="bin/SDL3.lib"
set SDL_INCLUDE_DIR="third_party/SDL/include"

set SRCS=src/main.cpp src/obj_parser.cpp src/core.cpp

cl %SDL_LIB_PATH% %SRCS% /std:c++14 /W3 /ZI /I %SDL_INCLUDE_DIR% /link /out:bin/main.exe

:: ======================================
:: linux messing around saved for later.. 
:: ======================================

:: g++ -Wall -Wextra -pedantic -std=c++11 -I$SDL_INCLUDE_DIR src/main.cpp -L$SDL_LIB_DIR -lSDL3 -o $OUT

::export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SDL_LIB_DIR

::./$OUT