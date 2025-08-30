#include "Tasks.h"

#include "Library.h"

extern "C"
{
    __attribute__( ( section( ".funcs" ), used ) ) Library::Function exportedFunctions[] =
    {
        {"Task0", Task0},
        {"Task1", Task1},
        {"Task2", Task2},
        {nullptr, nullptr} // Null-terminator
    };
}
