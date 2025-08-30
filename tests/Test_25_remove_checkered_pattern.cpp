#include "Test_25_remove_checkered_pattern.h"

#include "../ImageWindow.h"
#include "../ImageData.h"

void Test_25_remove_checkered_pattern( Context &context )
{
    auto s = context.scope( __FUNCTION__ );

    auto &info = context.information;
    bool readDisk = info( L"readDisk" ).as<bool>();
    bool writeDisk = info( L"writeDisk" ).as<bool>();
    bool showImages = info( L"showImages" ).as<bool>();

    ImageData input, output;
    if( !readDisk || !input.input( L"input\\png\\sample.png" ) )
        return;

    input.function( output, [&]( int, int, int j, int i, const Pixel & in, Pixel & out )
    {
        Color v, pattern;
        auto u = ( Color )in;

        pattern = ( Color )( j / 15 % 2 == i / 15 % 2 ? Pixel( 255, 255, 255 ) : Pixel( 230, 230, 230 ) );

        // Formula in implementation seems to break, when at least one of images are fully opaque!!!
        v = u.pad( pattern, -1 );

        if( 0 <= v.r && v.r <= 1 && 0 <= v.g && v.g <= 1 && 0 <= v.b && v.b <= 1 && 0 <= v.a && v.a <= 1 )
        {
            out = ( Pixel )v;
        }
        else
        {
            out = Pixel( 255, 0, 255 );
        }
    } );

    if( showImages )
    {
        ImageWindow window( output, nullptr );
        window.run();
    }

    if( writeDisk )
        output.output( context.Output() / L"sample.png" );
}
