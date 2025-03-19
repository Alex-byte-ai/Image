#include "Test_14_tiling.h"

#include "../ImageWindow.h"
#include "../ImageData.h"
#include "../Filters.h"

void Test_14_tiling( Context &context )
{
    auto &text = context.output();
    auto s = context.scope( __FUNCTION__ );

    auto &info = context.information;
    bool readDisk = info( "readDisk" ).as<bool>();
    bool writeDisk = info( "writeDisk" ).as<bool>();
    bool showImages = info( "showImages" ).as<bool>();

    ImageData base, out;
    if( readDisk && base.input( L"input\\64x32.png" ) )
    {
        base.copy( out );
        text << L"constant\n";
        Filters::constant( out, 160, 80, Pixel( 255, 190, 20 ) );
        if( showImages )
        {
            ImageWindow window( out, nullptr );
            window.run();
        }
        if( writeDisk )
            out.output( context.Output() / L"constant" );

        base.copy( out );
        text << L"extend\n";
        Filters::extend( out, 160, 80 );
        if( showImages )
        {
            ImageWindow window( out, nullptr );
            window.run();
        }
        if( writeDisk )
            out.output( context.Output() / L"extend" );

        base.copy( out );
        text << L"wrap\n";
        Filters::wrap( out, 160, 80 );
        if( showImages )
        {
            ImageWindow window( out, nullptr );
            window.run();
        }
        if( writeDisk )
            out.output( context.Output() / L"wrap" );

        base.copy( out );
        text << L"mirror\n";
        Filters::mirror( out, 160, 80 );
        if( showImages )
        {
            ImageWindow window( out, nullptr );
            window.run();
        }
        if( writeDisk )
            out.output( context.Output() / L"mirror" );
    }
}
