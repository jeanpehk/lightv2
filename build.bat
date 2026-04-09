@echo off
cls

set SDL_LIB_PATH="third_party/SDL/build/Debug/SDL3.lib"
set SDL_INCLUDE_DIR="third_party/SDL/include"

set SRCS=src/main.cpp

cl %SDL_LIB_PATH% %SRCS% /std:c++14 /W3 /I %SDL_INCLUDE_DIR% /link /out:build/main.exe

:: ======================================
:: linux messing around saved for later.. 
:: ======================================

:: g++ -Wall -Wextra -pedantic -std=c++11 -I$SDL_INCLUDE_DIR src/main.cpp -L$SDL_LIB_DIR -lSDL3 -o $OUT

::export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SDL_LIB_DIR

::./$OUT