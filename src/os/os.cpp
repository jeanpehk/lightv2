#include "os.h"

#ifdef BUILD_LINUX
#include "linux.cpp"
#else
#include "win.cpp"
#endif