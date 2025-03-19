#include "CheckProgress.h"

#include <windows.h>

CheckProgress::CheckProgress( const std::function<void()> &a, unsigned i ) : action( a ), interval( i )
{
    time = GetTickCount();
}

unsigned CheckProgress::check()
{
    unsigned result = GetTickCount() - time;
    if( result > interval )
    {
        if( action )
            action();
        time = GetTickCount();
    }
    return result;
}
