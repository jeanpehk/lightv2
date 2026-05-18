@echo off
cls

set SDL_LIB_PATH="bin/SDL3.lib"
set SDL_INCLUDE_DIR="third_party/SDL/include"

set SRCS=src/main.cpp src/obj_parser.cpp src/core.cpp src/os/os.cpp

cl %SDL_LIB_PATH% %SRCS% /D_CRT_SECURE_NO_WARNINGS /std:c++14 /W3 /ZI /I %SDL_INCLUDE_DIR% /link /out:bin/main.exe