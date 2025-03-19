#include "Test_12_watermelon.h"

#include "RandomNumber.h"

#include "../ImageWindow.h"
#include "../ImageData.h"

void Test_12_watermelon( Context &context )
{
    auto s = context.scope( __FUNCTION__ );

    auto &info = context.information;
    bool writeDisk = info( "writeDisk" ).as<bool>();
    bool showImages = info( "showImages" ).as<bool>();

    ImageData image;

    int width = 64;

    RandomNumber randomNumber;

    auto getSize = []( double x, double /*totalWidth*/, double lineWidth )
    {
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
        image.reset( 1024, 1024, Pixel( 0, 255, 0 ) );

        if( open )
        {
            image.rectangle( image.w() / 2, 0, image.w() / 2,  image.h(), Pixel( 255, 0, 0 ) );
        }

        for( int i = 0; i < 1024 * 50; ++i )
        {
            int x = randomNumber.getInteger( 0, image.w() );
            int y = randomNumber.getInteger( 0, image.h() );

            int size = Round( getSize( x, image.w(), width ) );
            if( !open || ( x <  image.w() / 2 ) )
                image.circle( x, y, size, Pixel( 0, 128, 0 ) );
        }

        if( open )
        {
            for( int i = 0; i < 256; ++i )
            {
                int x = randomNumber.getInteger( 0, image.w() );
                int y = randomNumber.getInteger( 0, image.h() );
                int size = randomNumber.getInteger( 5, 9 );
                if( x >  image.w() / 2 )
                    image.circle( x, y, size, Pixel( 40, 40, 40 ) );
            }
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
