:: Script to build SDL3 from source, requires cmake
:: haven't tested properly from scratch just made this while figuring everything out

cd third_party/SDL
md build
cmake -S . -B build
cmake --build build
cd ../..

copy third_party\SDL\build\Debug bin

:: WSL:
:: cd third_party/SDL
:: mkdir build
:: cd build
:: cmake ..
:: make -j<N>
:: mv libSDL3*.so* ../../../bin/