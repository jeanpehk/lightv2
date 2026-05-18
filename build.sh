SDL_LIB_DIR=bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:bin

SDL_INCLUDE_DIR="third_party/SDL/include"

SRCS="src/main.cpp src/obj_parser.cpp src/core.cpp src/os/os.cpp"

#FLAGS="-Wall -Wextra -pedantic"
FLAGS=

g++ $SRCS -o bin/a.out $FLAGS -DBUILD_LINUX -I$SDL_INCLUDE_DIR -L$SDL_LIB_DIR -lSDL3 -Wl,-rpath,'$ORIGIN'