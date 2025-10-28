#include "Test_28_Tree.h"

#include "RandomNumber.h"

#include "../ImageData.h"
#include "../GetImage.h"
#include "../Line.h"

class Painter
{
private:
    float direction, x, y; // Current drawing direction ( angle in degrees ) and coordinates
    Pixel penColor;
    int thickness;
    bool penIsDown;
    int scale;

    ImageData &canvas;
public:
    Painter( ImageData &c, int s = 1 )
        : direction( 0.0f ), x( 0 ), y( 0 ), penColor( 0, 0, 0 ), thickness( 1 ),
          penIsDown( true ), scale( s ), canvas( c )
    {
        if( scale > 1 )
        {
            ImageData output;
            output.reset( canvas.w() * scale, canvas.h() * scale );

            ImageConvert::Reference in, out;
            makeReference( canvas, in );
            makeReference( output, out );

            translate( in, out, true );

            output.copy( canvas );
        }
        else
        {
            scale = 1;
        }
    }

    void release()
    {
        if( scale > 1 )
        {
            ImageData output;
            output.reset( canvas.w() / scale, canvas.h() / scale );

            ImageConvert::Reference in, out;
            makeReference( canvas, in );
            makeReference( output, out );

            translate( in, out, true );

            output.copy( canvas );

            scale = 1;
        }
    }

    void pen( int t, int r, int g, int b )
    {
        thickness = t;
        penColor = Pixel( r, g, b );
    }

    void setHeading( float angle )
    {
        direction = angle;
    }

    // Moves to ( x, y )
    void setPosition( float xCoordinate, float yCoordinate )
    {
        int x0, y0, x1, y1;

        x0 = Round( x * scale );
        y0 = Round( y * scale );

        x = xCoordinate;
        y = yCoordinate;

        x1 = Round( x * scale );
        y1 = Round( y * scale );

        if( penIsDown )
        {
            DrawLine l( x0, y0, x1, y1 );
            while( !l.isFinished() )
            {
                canvas.circle( l.x(), l.y(), thickness * scale / 2, penColor );
                l.nextPixel();
            }
        }
    }

    void penUp()
    {
        penIsDown = false;
    }

    void penDown()
    {
        penIsDown = true;
    }

    // Moves forward in the current heading direction while drawing ( if pen is down )
    void forward( int distance )
    {
        auto angle = direction * Pi() / 180;
        setPosition( x + distance * Cos( angle ), y + distance * Sin( angle ) );
    }

    // Rotates drawing direction to the right by 'angle'
    void right( int angle )
    {
        direction += angle;
    }

    // Rotates drawing direction to the left by 'angle'
    void left( int angle )
    {
        direction -= angle;
    }

    float heading() const
    {
        return direction;
    }

    void position( float &xCoordinate, float &yCoordinate )
    {
        xCoordinate = x;
        yCoordinate = y;
    }
};

static void branch( int x, Painter &painter, RandomNumber &random )
{
    float px, py, h;
    int n, nn;

    if( x <= 0 )
        return;

    h = painter.heading();
    painter.position( px, py );

    nn = ( x > 6 ) ? 8 : 75;

    for( n = 1; n <= nn; ++n )
    {
        if( x > 3 && n > 22 )
        {
            // (255, 146, 52) - (255, 250, 0)
            // painter.pen( x, 255, 100 + n * 2, 75 - n );

            // (219, 0, 173) - (240, 10, 40)
            painter.pen( x, Round( 218.72 + 0.28 * n ), Round( 0.13 * n ), Round( 174.77 - 1.77 * n ) );
        }
        else
        {
            // (100, 255, 0) - (100, 21, 0)
            // painter.pen( x, 100, 255 - x * 18, 0 );

            // (100, 255, 0) - (10, 50, 10)
            painter.pen( x, Round( 100 - 6.92 * x ), Round( 255 - 15.76 * x ), Round( 0.76 * x ) );
        }

        painter.forward( x );

        if( x % 2 == 0 )
            painter.right( n + random.getInteger( 0, n / 2 ) + 8 );
        else
            painter.left( n + random.getInteger( 0, n / 2 ) + 7 );

        if( n == 6 )
            branch( x - 2, painter, random );
        if( n == 8 )
            branch( x - 1, painter, random );
    }

    painter.setHeading( h );
    painter.penUp();
    painter.setPosition( px, py );
    painter.penDown();
}

void Test_28_Tree( Context &context )
{
    auto s = context.scope( __FUNCTION__ );

    auto &info = context.information;
    bool writeDisk = info( L"writeDisk" ).as<bool>();

    if( !writeDisk )
        return;

    for( int i = 0; i < 10; ++i )
    {
        ImageData output( 1024, 1024 );
        RandomNumber random( 10873402375 + 143 * i );
        Painter painter( output, i == 6 ? 8 : 1 );

        painter.penUp();
        painter.setPosition( 400, 700 );
        painter.penDown();
        branch( 13, painter, random );

        painter.release();
        output.output( context.Output() / ( L"branch" + std::to_wstring( i ) + L".png" ) );
    }
}
