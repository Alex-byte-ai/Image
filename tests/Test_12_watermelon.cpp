#include "Test_12_watermelon.h"

#include "RandomNumber.h"

#include "../ProceduralTextures.h"
#include "../ImageWindow.h"
#include "../ImageData.h"

void Test_12_watermelon( Context &context )
{
    auto s = context.scope( __FUNCTION__ );

    auto &info = context.information;
    bool writeDisk = info( L"writeDisk" ).as<bool>();
    bool showImages = info( L"showImages" ).as<bool>();

    std::array<bool, 2> options = {false, true};
    ImageData image, peel, pulp;
    RandomNumber randomNumber;

    for( bool open : options )
    {
        watermelonPeel( peel, randomNumber );

        int w = peel.w();
        int h = peel.h();

        image.reset( w, h );

        peel.sub( peel, 0, 0, open ? w / 2 : w, h );
        peel.place( image, 0, 0 );

        if( open )
        {
            watermelonPulp( pulp, randomNumber );
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
