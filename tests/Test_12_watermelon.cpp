#include "Test_12_watermelon.h"

#include "RandomNumber.h"

#include "../ImageWindow.h"
#include "../ImageData.h"

void Test_12_watermelon( Context &context )
{
    auto s = context.scope( __FUNCTION__ );

    auto &info = context.information;
    bool writeDisk = info( L"writeDisk" ).as<bool>();
    bool showImages = info( L"showImages" ).as<bool>();

    ImageData image, peel, pulp;
    RandomNumber randomNumber;
    int w = 1024, h = 1024;

    auto getSize = []( double x, double /*totalWidth*/ )
    {
        double lineWidth = 64;

        int lineId = RoundDown( x / lineWidth );

        double lineMiddle = lineId * lineWidth + lineWidth * 0.5;

        double deviationFromTheMiddle = x - lineMiddle;

        double size = 5.5 - 0.8 * Sqrt( Abs( deviationFromTheMiddle ) );
        if( size < 0 )
            size = 0;

        return size;
    };

    std::array<bool, 2> options = {false, true};

    for( bool open : options )
    {
        peel.reset( w, h, Pixel( 0, 255, 0 ) );

        if( open )
            pulp.reset( w, h, Pixel( 255, 0, 0 ) );

        for( int i = 0; i < 1024 * 50; ++i )
        {
            int x = randomNumber.getInteger( 0, w );
            int y = randomNumber.getInteger( 0, h );
            int size = Round( getSize( x, w ) );
            peel.circle( x, y, size, {}, Pixel( 0, 128, 0 ) );
        }

        if( open )
        {
            for( int i = 0; i < 256; ++i )
            {
                int x = randomNumber.getInteger( 0, w );
                int y = randomNumber.getInteger( 0, h );
                int size = randomNumber.getInteger( 5, 9 );
                int d = 6 * ( 9 - size );
                pulp.circle( x, y, size, {}, Pixel( 40 + 3 * d / 2, 40 - d, 40 - d ) );
            }
        }

        image.reset( w, h );

        peel.sub( peel, 0, 0, open ? w / 2 : w, h );
        peel.place( image, 0, 0 );

        if( open )
        {
            pulp.sub( pulp, w / 2, 0, w, h );
            pulp.place( image, w / 2, 0 );
        }

        if( showImages )
        {
            ImageWindow window( image, nullptr );
            window.run();
        }
        if( writeDisk )
            image.output( context.Output() / ( open ? L"WatermelonOpen.png" : L"WatermelonClosed.png" ) );
    }
}
