#include "Test_09_colourful_rectangle_trace.h"

#include "../ImageWindow.h"
#include "../ImageData.h"

void Test_09_colourful_rectangle_trace( Context &context )
{
    auto s = context.scope( __FUNCTION__ );

    auto &info = context.information;
    bool writeDisk = info( L"writeDisk" ).as<bool>();
    bool showImages = info( L"showImages" ).as<bool>();
    bool outputVariableData = info( L"outputVariableData" ).as<bool>();

    ImageData image;
    image.reset( 512, 512 );

    auto action = []( const ImageWindow::InputData & inputData, ImageWindow::OutputData & outputData )
    {
        struct point
        {
            int x, y;
            point( int xCoordinate, int yCoordinate ): x( xCoordinate ), y( yCoordinate )
            {}
        };

        static std::vector<point> points;
        static std::optional<point> center;

        auto makeRect = [&outputData]()
        {
            auto w = points[1].x - points[0].x;
            auto h = points[1].y - points[0].y;

            double r = double( w ) / h;

            Color baseColor, resultColor;
            if( Abs( r ) > 1 )
            {
                if( r > 0 )
                {
                    baseColor = { 1.0, 0.0, 0.0 };
                }
                else
                {
                    baseColor = { 0.0, 1.0, 1.0 };
                }
                r = 1 / r;
            }
            else
            {
                if( r > 0 )
                {
                    baseColor = { 0.0, 0.0, 1.0 };
                }
                else
                {
                    baseColor = { 1.0, 1.0, 0.0 };
                }
            }
            r = Abs( r );

            resultColor = Color( 0.0, 1.0, 0.0 ) * r + baseColor * ( 1.0 - r );

            outputData.image.get().rectangle( Min( points[0].x, points[1].x ), Min( points[0].y, points[1].y ), Abs( w ), Abs( h ), {}, ( Pixel )resultColor );
        };

        if( inputData.leftMouse.changed() && *inputData.leftMouse )
        {
            if( points.size() < 2 )
                points.push_back( { *inputData.mouseX, *inputData.mouseY } );
            if( points.size() == 2 )
            {
                center.emplace( ( points[0].x + points[1].x ) / 2, ( points[0].y + points[1].y ) / 2 );
                makeRect();
            }
        }

        if( ( points.size() == 2 ) && ( inputData.mouseX.changed() || inputData.mouseY.changed() ) )
        {
            auto dx = *inputData.mouseX - points[1].x;
            auto dy = *inputData.mouseY - points[1].y;

            points[0].x += -dx;
            points[0].y += -dy;

            points[1].x = *inputData.mouseX;
            points[1].y = *inputData.mouseY;

            makeRect();
        }

        return false;
    };

    if( showImages )
    {
        ImageWindow window( image, action );
        window.run();
        if( writeDisk && outputVariableData )
            image.output( context.Output() / L"canvas.png" );
    }
}
