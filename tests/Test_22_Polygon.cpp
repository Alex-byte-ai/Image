#include "Test_22_Polygon.h"

#include "RandomNumber.h"
#include "Affine2D.h"
#include "Polygon.h"

#include "../CheckProgress.h"
#include "../ImageWindow.h"
#include "../ImageData.h"
#include "../Overlap.h"

static void prepare( ConvexPolygon &polygon, ConvexPolygon &otherPolygon, ConvexPolygon &/*intersection*/, bool invert, bool otherInvert )
{
    double x = invert ? 0 : 256;
    double y = otherInvert ? 0 : 256;

    polygon =
    {
        {
            { x + 28, y + 115 },
            { x + 192, y + 220 },
            { x + 233, y + 82 },
            { x + 62, y + 34 }
        }
    };

    otherPolygon =
    {
        {
            { x + 115, y + 80 },
            { x + 154, y + 104 },
            { x + 166, y + 81 },
            { x + 108, y + 26 }
        }
    };

    if( invert )
        polygon = polygon.inverse();
    if( otherInvert )
        otherPolygon = otherPolygon.inverse();

    // intersection = polygon.intersect( otherPolygon );
};

static void prepareQ( Quadrangle &polygon, Quadrangle &otherPolygon, Quadrangle &, bool invert, bool otherInvert )
{
    double x = invert ? 0 : 256;
    double y = otherInvert ? 0 : 256;

    polygon =
    {
        {
            { x + 28, y + 115 },
            { x + 192, y + 220 },
            { x + 233, y + 82 },
            { x + 62, y + 34 }
        }
    };

    otherPolygon =
    {
        {
            { x + 115, y + 80 },
            { x + 154, y + 104 },
            { x + 166, y + 81 },
            { x + 108, y + 26 }
        }
    };

    if( invert )
        polygon.flip();
    if( otherInvert )
        otherPolygon.flip();
};

static void demonstrate( ConsoleOutput &text, Context &context, bool writeDisk, bool showImages )
{
    std::array<bool, 2> options{false, true};
    Color red( 1, 0, 0 ), green( 0, 1, 0 ), blue( 0, 0, 1 ),
          cyan( 0, 1, 1 ), magenta( 1, 0, 1 ), yellow( 1, 1, 0 ),
          result, color, otherColor, lastColor;

    ImageData image;
    image.reset( 512, 512, Pixel( 0, 0, 0 ) );

    CheckProgress timer( nullptr, 1000 );

    for( auto invert : options )
    {
        for( auto otherInvert : options )
        {
            double x, y, area;
            ConvexPolygon polygon, otherPolygon, intersection, pixel;
            prepare( polygon, otherPolygon, intersection, invert, otherInvert );
            image.function( image, [&]( int, int, int j, int i, const Pixel & input, Pixel & output )
            {
                x = j;
                y = i;
                pixel =
                {
                    {
                        { x, y },
                        { x, y + 1 },
                        { x + 1, y + 1 },
                        { x + 1, y }
                    }, true
                };

                result = ( Color )input;

                area = polygon.intersect( pixel ).area();
                color = area > 0 ? yellow : blue;
                area = Abs( area );
                result = result * ( 1 - area ) + color * area;

                area = otherPolygon.intersect( pixel ).area();
                otherColor = area > 0 ? green : magenta;
                area = Abs( area );
                result = result * ( 1 - area ) + otherColor * area;

                /*area = intersection.intersect( pixel ).area();
                lastColor = ( color * 0.5 + otherColor * 0.5 ).invert();
                // lastColor = area > 0 ? red : cyan;
                area = Abs( area );
                result = result * ( 1 - area ) + lastColor * area;*/

                output = ( Pixel )result;
            } );
        }
    }

    text << L"ConvexPolygon time: " << timer.check() << L"\n";

    if( showImages )
    {
        ImageWindow window( image, nullptr );
        window.run();
    }
    if( writeDisk )
        image.output( context.Output() / L"convex_polygon.png" );
}

static void demonstrateQ( ConsoleOutput &text, Context &context, bool writeDisk, bool showImages )
{
    std::array<bool, 2> options{false, true};
    Color red( 1, 0, 0 ), green( 0, 1, 0 ), blue( 0, 0, 1 ),
          cyan( 0, 1, 1 ), magenta( 1, 0, 1 ), yellow( 1, 1, 0 ),
          result, color, otherColor, lastColor;

    ImageData image;
    image.reset( 512, 512, Pixel( 0, 0, 0 ) );

    CheckProgress timer( nullptr, 1000 );

    for( auto invert : options )
    {
        for( auto otherInvert : options )
        {
            double x, y, area;
            Quadrangle polygon, otherPolygon, intersection, pixel;
            prepareQ( polygon, otherPolygon, intersection, invert, otherInvert );
            image.function( image, [&]( int, int, int j, int i, const Pixel & input, Pixel & output )
            {
                x = j;
                y = i;
                pixel =
                {
                    {
                        { x, y },
                        { x, y + 1 },
                        { x + 1, y + 1 },
                        { x + 1, y }
                    }
                };

                result = ( Color )input;

                area = Quadrangle::commonArea( pixel, polygon );
                color = area > 0 ? yellow : blue;
                area = Abs( area );
                result = result * ( 1 - area ) + color * area;

                area = Quadrangle::commonArea( pixel, otherPolygon );
                otherColor = area > 0 ? green : magenta;
                area = Abs( area );
                result = result * ( 1 - area ) + otherColor * area;

                output = ( Pixel )result;
            } );
        }
    }

    text << L"Quadrangle time: " << timer.check() << L"\n";

    if( showImages )
    {
        ImageWindow window( image, nullptr );
        window.run();
    }
    if( writeDisk )
        image.output( context.Output() / L"quadrangle.png" );
}

static void demonstrateNull( ConsoleOutput &text, Context &, bool, bool showImages )
{
    std::array<bool, 2> options{false, true};
    Color red( 1, 0, 0 ), green( 0, 1, 0 ), blue( 0, 0, 1 ),
          cyan( 0, 1, 1 ), magenta( 1, 0, 1 ), yellow( 1, 1, 0 ),
          result, color, otherColor, lastColor;

    ImageData image;
    image.reset( 512, 512, Pixel( 0, 0, 0 ) );

    CheckProgress timer( nullptr, 1000 );
    for( auto invert : options )
    {
        for( auto otherInvert : options )
        {
            ( void )invert;
            ( void )otherInvert;

            double x, y, area;
            image.function( image, [&]( int, int, int j, int i, const Pixel & input, Pixel & output )
            {
                x = j;
                y = i;

                result = ( Color )input;

                area = 0.5;
                color = area > 0 ? yellow : blue;
                area = Abs( area );
                result = result * ( 1 - area ) + color * area;

                area = 0.5;
                otherColor = area > 0 ? green : magenta;
                area = Abs( area );
                result = result * ( 1 - area ) + otherColor * area;

                output = ( Pixel )result;
            } );
        }
    }

    text << L"Null time: " << timer.check() << L"\n";

    if( showImages )
    {
        ImageWindow window( image, nullptr );
        window.run();
    }
}

static void experiment0( ConsoleOutput &text, Context &, bool, bool showImages )
{
    Color red( 1, 0, 0 ), green( 0, 1, 0 ), blue( 0, 0, 1 ),
          cyan( 0, 1, 1 ), magenta( 1, 0, 1 ), yellow( 1, 1, 0 ),
          result, color, otherColor, lastColor;

    ImageData image;
    image.reset( 512, 512, Pixel( 0, 0, 0 ) );

    std::optional<ComplexPolygon> polygon0, polygon1, polygon2;
    std::vector<Vector2D> points;
    int id = 0;

    polygon0.emplace( Vector2D( -0.25, -0.25 ), Vector2D( -0.25, 0.25 ), Vector2D( 0.25, 0.25 ) );

    auto draw = [&]( const ImageWindow::InputData & inputData, ImageWindow::OutputData & outputData )
    {
        auto update = [&]()
        {
            try
            {
                CheckProgress timer( nullptr, 1000 );

                if( ( id == 1 || id == 2 ) && polygon0 && polygon1 )
                    polygon2.emplace( *polygon0 && *polygon1 );

                outputData.image.get().function( image, [&]( double x, double y, const Color &, Color & output )
                {
                    Vector2D point( x, y );

                    bool f = false;
                    if( id == 0 && polygon0 && polygon1 )
                    {
                        f = polygon0->carcass( point ) || polygon1->carcass( point );
                    }
                    else if( id == 1 && polygon2 )
                    {
                        f = polygon2->inside( point );
                    }
                    else if( id == 2 && polygon2 )
                    {
                        f = polygon2->carcass( point );
                    }
                    output = f ? Color( 1, 1, 1 ) : Color( 0, 0, 0 );
                } );
                text << L"experiment0 time: " << timer.check() << L"\n";
            }
            catch( ... )
            {}
        };

        auto &leftMouse = inputData.leftMouse;
        if( leftMouse.changed() && *leftMouse )
        {
            double mouseX = ( *inputData.mouseX + 0.5 ) / 512 - 0.5;
            double mouseY = ( *inputData.mouseY + 0.5 ) / 512 - 0.5;
            points.emplace_back( mouseX, mouseY );
            if( points.size() == 3 )
            {
                polygon1.emplace( points[0], points[1], points[2] );
                points.clear();
            }
            update();
        }

        auto &rightMouse = inputData.rightMouse;
        if( rightMouse.changed() && *rightMouse )
        {
            id = ( id + 1 ) % 3;
            update();
        }
    };

    if( showImages )
    {
        ImageWindow window( image, draw );
        window.run();
    }
}

static void experiment1( ConsoleOutput &text, Context &, bool, bool showImages )
{
    double ang = 0, xs = 1, ys = 1;
    ImageData background, spot;
    Vector2D shift;

    background.input( L"input\\tree.png" );
    spot.input( L"input\\64x32.png" );

    Canvas canvas( background );
    Picture picture( spot );

    auto draw = [&]()
    {
        Vector2D c = Vector2D( spot.w() * 0.5, spot.h() * 0.5 );
        Affine2D pos = Affine2D( shift ) * Affine2D( Matrix2D::Rotation( ang ) ) * Affine2D( Matrix2D::Scale( xs, ys ) ) * Affine2D( -c );
        picture.set( pos );
        canvas.draw( picture );
        canvas.bake();
    };

    RandomNumber a;
    CheckProgress timer( nullptr, 1000 );
    for( size_t i = 0; i < 250; ++i )
    {
        shift.x = a.getReal( 0, 1 ) * background.w();
        shift.y = a.getReal( 0, 1 ) * background.h();
        ang = a.getReal( -Pi(), Pi() );
        xs = a.getReal( -2, 2 );
        ys = a.getReal( -2, 2 );
        draw();
    }
    text << L"experiment1 time: " << timer.check() << L"\n";

    if( showImages )
    {
        canvas.render( background );
        ImageWindow window( background, nullptr );
        window.run();
    }
}

void Test_22_Polygon( Context &context )
{
    auto &text = context.output();
    auto s = context.scope( __FUNCTION__ );

    auto &info = context.information;
    bool writeDisk = info( L"writeDisk" ).as<bool>();
    bool showImages = info( L"showImages" ).as<bool>();

    experiment0( text, context, writeDisk, showImages );
    experiment1( text, context, writeDisk, showImages );
    demonstrate( text, context, writeDisk, showImages );
    demonstrateQ( text, context, writeDisk, showImages );
    demonstrateNull( text, context, writeDisk, showImages );
}
