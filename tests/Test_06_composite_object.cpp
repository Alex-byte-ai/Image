#include "Test_06_composite_object.h"

#include "Vector2D.h"
#include "Matrix2D.h"
#include "Affine2D.h"

#include "../CompositeObject.h"
#include "../ImageWindow.h"
#include "../ImageData.h"

void Test_06_composite_object( Context &context )
{
    auto s = context.scope( __FUNCTION__ );

    auto &info = context.information;
    bool writeDisk = info( L"writeDisk" ).as<bool>();
    bool showImages = info( L"showImages" ).as<bool>();

    Matrix2D a( 1, 2, 3, 4 ), b( 5, 6, 7, 8 ), c;
    Vector2D v0( 3, 4 ), v1;
    ImageData image;
    int x, y;

    x = y = 0;
    image.reset( 256, 256 );
    Draw( image, L"A", a, x, y );
    Draw( image, L"B", b, x, y );
    c = a;
    c *= b;
    Draw( image, L"A*B", c, x, y );
    c = a * b;
    Draw( image, L"A*B", c, x, y );
    if( showImages )
    {
        ImageWindow window( image, nullptr );
        window.run();
    }
    if( writeDisk )
        image.output( context.Output() / L"AmultiplyB.png" );

    x = y = 0;
    image.reset( 256, 256 );
    Draw( image, L"A", a, x, y );
    Draw( image, L"B", b, x, y );
    c = a;
    c += b;
    Draw( image, L"A+B", c, x, y );
    c = a + b;
    Draw( image, L"A+B", c, x, y );
    if( showImages )
    {
        ImageWindow window( image, nullptr );
        window.run();
    }
    if( writeDisk )
        image.output( context.Output() / L"AplusB.png" );

    x = y = 0;
    image.reset( 256, 256 );
    Draw( image, L"A", a, x, y );
    Draw( image, L"B", b, x, y );
    c = a;
    c -= b;
    Draw( image, L"A-B", c, x, y );
    c = a - b;
    Draw( image, L"A-B", c, x, y );
    if( showImages )
    {
        ImageWindow window( image, nullptr );
        window.run();
    }
    if( writeDisk )
        image.output( context.Output() / L"AminusB.png" );

    x = y = 0;
    image.reset( 256, 256 );
    Draw( image, L"A", a, x, y );
    c = +a;
    Draw( image, L"+A", c, x, y );
    if( showImages )
    {
        ImageWindow window( image, nullptr );
        window.run();
    }
    if( writeDisk )
        image.output( context.Output() / L"plusA.png" );

    x = y = 0;
    image.reset( 256, 256 );
    Draw( image, L"A", a, x, y );
    c = -a;
    Draw( image, L"-A", c, x, y );
    if( showImages )
    {
        ImageWindow window( image, nullptr );
        window.run();
    }
    if( writeDisk )
        image.output( context.Output() / L"minusB.png" );

    x = y = 0;
    image.reset( 256, 256 );
    Draw( image, L"A", a, x, y );
    Draw( image, L"v", v0, x, y );
    v1 = a * v0;
    Draw( image, L"A*v", v1, x, y );
    if( showImages )
    {
        ImageWindow window( image, nullptr );
        window.run();
    }
    if( writeDisk )
        image.output( context.Output() / L"AmultiplyV.png" );
}
