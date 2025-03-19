#include "GetImage.h"

#include <windows.h>

#include "BitmapTools.h"
#include "Lambda.h"

bool screenCapture( ImageDataBase &image )
{
    return BitmapTools( image ) << GetDC( nullptr );
}

bool windowCapture( ImageDataBase &image )
{
    auto window = GetForegroundWindow();
    if( window == nullptr )
        return false;

    RECT rect;
    if( !GetClientRect( window, &rect ) )
        return false;

    POINT topLeft = { rect.left, rect.top };
    POINT bottomRight = { rect.right, rect.bottom };

    if( !ClientToScreen( window, &topLeft ) )
        return false;
    if( !ClientToScreen( window, &bottomRight ) )
        return false;

    rect.left = topLeft.x;
    rect.top = topLeft.y;
    rect.right = bottomRight.x;
    rect.bottom = bottomRight.y;

    BitmapTools tool( image );
    tool.setCropping( rect.left, rect.top, rect.right, rect.bottom );
    return tool << GetDC( nullptr );
}

void makeReference( ImageDataBase &image, ImageConvert::Reference &reference )
{
    reference.reset = [&image]( ImageConvert::Reference & ref )
    {
        image.reset( Abs( ref.w ), ref.h );
        ref.link = image.rawData()[0];
        return image.w() * image.h() * sizeof( Pixel ) >= ref.bytes;
    };
    reference.format = "B8G8R8A8*REPBG*REPRG*REPA255";
    reference.bytes = image.w() * image.h() * sizeof( Pixel );
    reference.link = image.rawData()[0];
    reference.w = image.w();
    reference.h = image.h() * Sign( image.s() );
}
