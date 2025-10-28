#include "Test_30_Flower.h"

#include "RandomNumber.h"
#include "Affine2D.h"

#include "../ImageData.h"
#include "../Overlap.h"

class GrowthParameters
{
public:
    double x, y, angle, size;
};

static void chunk( ImageData& image, GrowthParameters& p )
{
    ImageData sample;

    int rx = Round( p.size ) * 2;
    int ry = Round( 256 / p.size ) * 2;

    auto k = 256 / ( p.size * p.size );
    if( k > 1 )
        k = 1;

    Color color = k >= 1 ? Color( 1, 0, 0 ) : Color( 0, 0.5, 0 ) * ( 1 - k ) + Color( 0, 1, 0 ) * k;

    sample.reset( 2 * rx, 2 * ry, Pixel( 0, 0, 0, 0 ) );
    sample.ellipse( rx, ry, rx, ry, {}, ( Pixel )color );

    auto position =
        Affine2D( Vector2D( p.x, p.y ) ) *
        Affine2D( Matrix2D::Rotation( p.angle ) ) *
        Affine2D( Matrix2D::Scale( 0.25, 0.25 ) ) *
        Affine2D( Vector2D( 0, -ry ) );

    Overlap::Picture oval( sample );
    oval.set( position );

    Overlap::Canvas canvas( image );
    canvas.draw( oval );
    canvas.render( image );

    double s = p.size * 0.9375;
    p.x += s * Cos( p.angle );
    p.y += s * Sin( p.angle );
}

static void cluster( ImageData& image, GrowthParameters p, int n, int m, bool flower );

static void branch( ImageData& image, GrowthParameters& p, int n, bool flower )
{
    GrowthParameters p0;

    for( int i = 0; i < n; ++i )
    {
        if( i == 10 )
        {
            p0 = p;
            p0.angle += Pi() / 4;
            branch( image, p0, 7, true );

            p0 = p;
            p0.angle -= Pi() / 4;
            branch( image, p0, 7, true );
        }
        else if( i == 15 )
        {
            p0 = p;
            p0.angle += Pi() / 4;
            branch( image, p0, 3, true );

            p0 = p;
            p0.angle -= Pi() / 4;
            branch( image, p0, 3, true );

            p0 = p;
            p0.angle += 3 * Pi() / 4;
            branch( image, p0, 3, true );

            p0 = p;
            p0.angle -= 3 * Pi() / 4;
            branch( image, p0, 3, true );
        }

        chunk( image, p );
        p.size *= 0.96593632892484555106514431292046;
        p.angle -= Pi() / 16;
    }

    if( flower )
    {
        p.size = 16;
        cluster( image, p, 1, 5, false );
    }
}

static void cluster( ImageData& image, GrowthParameters p, int n, int m, bool flower )
{
    GrowthParameters p0;
    for( int i = 0; i < m; ++i )
    {
        p0 = p;
        p0.angle += 2 * Pi() * i / m;
        branch( image, p0, n, flower );
    }
}

void Test_30_Flower( Context &context )
{
    auto s = context.scope( __FUNCTION__ );

    auto &info = context.information;
    bool writeDisk = info( L"writeDisk" ).as<bool>();

    if( !writeDisk )
        return;

    RandomNumber random( 4567584 );

    ImageData output;
    output.reset( 1024, 1024, Pixel( 0, 0, 0, 0 ) );

    cluster( output, { 512, 512, Pi() / 4, 32 }, 20, 5, true );

    output.output( context.Output() / L"flower.png" );
}
