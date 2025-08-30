#include "Tasks.h"

#include "Window.h"

DLL_API void Task0( const void *, void * )
{
    Popup( Popup::Type::Info, L"Test", L"Sample Text." ).run();
}
