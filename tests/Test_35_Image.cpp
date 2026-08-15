#include "Test_35_Image.h"

#include "../ImageData.h"

void Test_35_Image( Context &context )
{
    auto s = context.scope( __FUNCTION__ );

    auto &info = context.information;
    bool writeDisk = info( L"writeDisk" ).as<bool>();

    if( !writeDisk )
        return;

    int w = 32, h = 32, k = 0;
    auto get = [&k]()
    {
        return "Image"[k++ % 5];
    };

    // Same if 'Image' is repeatedly written into raw bytes of 24 bits bmp
    ImageData image( w, h );
    for( int i = 0; i < h; ++i )
    {
        for( int j = 0; j < w; ++j )
        {
            auto& out = *image( j, h - i - 1 );
            out.b = get();
            out.g = get();
            out.r = get();
            out.a = 255;
        }
    }
    image.output( context.Output() / L"像.png" );
}
