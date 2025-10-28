#include "Test_31_Procedural_textures.h"

#include "../ProceduralTextures.h"

#include "../ImageWindow.h"
#include "../ImageData.h"

void Test_31_Procedural_textures( Context &context )
{
    auto &text = context.output();
    auto s = context.scope( __FUNCTION__ );

    auto &info = context.information;
    bool writeDisk = info( L"writeDisk" ).as<bool>();

    if( !writeDisk )
        return;

    ImageData image;

    /*
    image.reset( 512, 512 );
    threadSegment( image, Stripe( Vector2D( -0.25, -0.25 ), Vector2D( 0.25, 0.25 ), 0.05 ), 0.0125, 3, Stripe( Vector2D( -1, 0 ), Vector2D( 1, 0 ), 2 ) );
    image.output( context.Output() / L"thread.png" );

    image.reset( 1024, 1024 );
    ropeSegment( image, Stripe( Vector2D( 0, -0.25 ), Vector2D( 0, 0.25 ), 0.125 ), 0.5, 0.015625 );
    image.output( context.Output() / L"rope.png" );

    tissueFragment( image, Tissue( 1024, 1024, 1024, 16, 9, 0.35, 0.2, Color( 1, 1, 1 ), Color( 0.875, 0.9375, 1 ) ) );
    image.output( context.Output() / L"tissue.png" );
    */

    // for( auto i : std::vector<size_t> { 15, 16, 23, 24, 47, 48, 69, 70, 83, 84 } )
    // for( auto i : std::vector<size_t> { 400 } )
    for( size_t i = 0; i < 400; ++i )
    {
        RandomNumber random;
        cellStructure( image, random, 1024, 0.05, 0.10, i ); // 375
        image.output( context.Output() / ( L"cell_structure" + std::to_wstring( i ) + L".png" ) );
    }

    /*
    {
        RandomNumber random;
        woodSlice( image, random, Trunk( 1024, 32, 1.75, 0.60, 0.8, 0.9, 0.7, Color( 0.77, 0.51, 0.34 ), Color( 0.54, 0.25, 0.11 ), Color( 0.85, 0.72, 0.55 ), Color( 0.65, 0.40, 0.24 ) ) );
        image.output( context.Output() / L"wood_slice.png" );
    }
    */

    /*
    image.reset( 1024, 1024 );
    for( int i = 1; i <= 16; ++i )
    {
        RandomNumber random;
        RandomFunction f( random, 16, 16 * i, 0, 1 );
        image.function( image, [&]( double x, double y, const Color &, Color & out )
        {
            if( 0.5 - y <= f( x + 0.5 ) * 0.8 + 0.1 )
            {
                out = Color( 1, 1, 1 );
            }
            else
            {
                out = Color( 0, 0, 0 );
            }
        } );
        image.output( context.Output() / ( L"random" + std::to_wstring( i - 1 ) + L".png" ) );
    }
    */

    /*
    {
        // std::vector<double> decays{0.5, 0.45, 0.333333, 0.25, 0.2, 0};
        RandomNumber random;
        randomImage( image, random, 16, 16, 16, 16, 10, 0.5 );
        image.output( context.Output() / ( L"random.png" ) );
    }
    */
}
