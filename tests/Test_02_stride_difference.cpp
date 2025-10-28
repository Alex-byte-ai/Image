#include "Test_02_stride_difference.h"

#include "../ImageWindow.h"
#include "../ImageData.h"

// Those versions of the same image are shown differently in a window!!!
void Test_02_stride_difference( Context &context )
{
    auto &text = context.output();
    auto s = context.scope( __FUNCTION__ );

    auto &info = context.information;
    bool writeDisk = info( L"writeDisk" ).as<bool>();
    bool showImages = info( L"showImages" ).as<bool>();

    unsigned size, i, j;
    ImageData in;
    Pixel *p;

    size = 256;

    for( int k = 0; k < 2; ++k )
    {
        in.reset( size, size * ( 2 * k - 1 ) );

        text << L"arrow" << k << L"\n";
        text << L"Stride: " << in.s() << L"\n";

        for( i = 0; i < size; ++i )
        {
            for( j = 0; j < size; ++j )
            {
                p = in( i, j );
                p->r = p->g = p->b = 255;
                p->a = 255;
            }
        }

        for( i = 0; i < size; ++i )
        {
            p = in( i, i );
            p->r = p->g = p->b = 0;
        }
        for( i = 0; i < size; ++i )
        {
            p = in( i, 0 );
            p->r = p->g = p->b = 0;
        }
        for( i = 0; i < size; ++i )
        {
            p = in( 0, i );
            p->r = p->g = p->b = 0;
        }

        if( showImages )
        {
            ImageWindow window( in, nullptr );
            window.run();
        }

        if( writeDisk )
        {
            if( k == 0 )
                in.output( context.Output() / L"arrow0.bmp" );
            else
                in.output( context.Output() / L"arrow1.bmp" );
        }
    }
}
