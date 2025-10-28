#include "Test_21_Mandelbrot_set.h"

#include "Complex.h"
#include "Basic.h"

#include "../ImageWindow.h"
#include "../ImageData.h"
#include "../GetImage.h"

static void getRainbowColor( double x, Color &c )
{
    if( x < 0 )
        x = 0;
    if( x >= 1 )
        x = 0;

    double adjustedX = x * 6.0; // Map x to the range [0, 6]
    int segment = RoundDown( adjustedX ); // Integer part of the range
    double t = adjustedX - segment; // Fractional part for blending

    double &r = c.r;
    double &g = c.g;
    double &b = c.b;
    c.a = 1;

    if( segment == 0 )
    {
        r = 1;
        g = t;
        b = 0;
    }
    else if( segment == 1 )
    {
        r = 1 - t;
        g = 1;
        b = 0;
    }
    else if( segment == 2 )
    {
        r = 0;
        g = 1;
        b = t;
    }
    else if( segment == 3 )
    {
        r = 0;
        g = 1 - t;
        b = 1;
    }
    else if( segment == 4 )
    {
        r = t;
        g = 0;
        b = 1;
    }
    else if( segment == 5 )
    {
        r = 1;
        g = 0;
        b = 1 - t;
    }
}

void Test_21_Mandelbrot_set( Context &context )
{
    auto s = context.scope( __FUNCTION__ );

    auto &info = context.information;
    bool writeDisk = info( L"writeDisk" ).as<bool>();
    bool showImages = info( L"showImages" ).as<bool>();

    ImageData image, result;

    image.reset( 2048, 2048 );
    result.reset( 512, 512 );

    float left = -2.1f, right = 2.1f,
          top = 2.1f, bottom = -2.1f;

    int maxIterations = 1000;

    image.function( image, [&]( int width, int height, int j, int i, const Pixel &, Pixel & output )
    {
        double x = ( ( j + 0.5 ) / width ) * ( right - left ) + left;
        double y = ( ( i + 0.5 ) / height ) * ( bottom - top ) + top;

        Complex c( x, y ), z;

        int k = 0;
        double zabs = 0;
        do
        {
            z = z * z + c;
            ++k;

            zabs = z.Abs();
        }
        while( zabs < 2 && k < maxIterations );

        if( zabs < 2 )
        {
            output = Pixel( 0, 0, 0 );
        }
        else
        {
            Color color;
            getRainbowColor( Pow( 1.0 / k, 0.35 ), color );
            output = ( Pixel )color;
        }
    } );

    ImageConvert::Reference original, antialiased;
    makeReference( image, original );
    makeReference( result, antialiased );
    translate( original, antialiased, true );

    if( showImages )
    {
        ImageWindow window( result, nullptr );
        window.run();
    }
    if( writeDisk )
        result.output( context.Output() / L"image.png" );
}
