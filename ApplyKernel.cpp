#include "ApplyKernel.h"

#include "Basic.h"

void ApplyKernel( const ImageData &in, ImageData &out )
{
    if( in.empty() )
        return;

    Pixel pi0, pi1, pi2,
          pi3, pi4, pi5,
          pi6, pi7, pi8,
          *po;

    double r, g, b, rx, ry, gx, gy, bx, by;
    Interval<double> rInterval, gInterval, bInterval;

    out.reset( in.w(), in.h() );

    auto calculate = [&]( unsigned i, unsigned j )
    {
        pi0 = *in( j - 1, i - 1 );
        pi1 = *in( j + 0, i - 1 );
        pi2 = *in( j + 1, i - 1 );
        pi3 = *in( j - 1, i + 0 );
        pi4 = *in( j + 0, i + 0 );
        pi5 = *in( j + 1, i + 0 );
        pi6 = *in( j - 1, i + 1 );
        pi7 = *in( j + 0, i + 1 );
        pi8 = *in( j + 1, i + 1 );

        rx = ( pi6.r + 2 * pi7.r + pi8.r ) - ( pi0.r + 2 * pi1.r + pi2.r );
        ry = ( pi2.r + 2 * pi5.r + pi8.r ) - ( pi0.r + 2 * pi3.r + pi6.r );
        r = Sqrt( Sqr( rx ) + Sqr( ry ) );

        gx = ( pi6.g + 2 * pi7.g + pi8.g ) - ( pi0.g + 2 * pi1.g + pi2.g );
        gy = ( pi2.g + 2 * pi5.g + pi8.g ) - ( pi0.g + 2 * pi3.g + pi6.g );
        g = Sqrt( Sqr( gx ) + Sqr( gy ) );

        bx = ( pi6.b + 2 * pi7.b + pi8.b ) - ( pi0.b + 2 * pi1.b + pi2.b );
        by = ( pi2.b + 2 * pi5.b + pi8.b ) - ( pi0.b + 2 * pi3.b + pi6.b );
        b = Sqrt( Sqr( bx ) + Sqr( by ) );
    };

    int i, j, w, h;

    w = out.w() - 1;
    h = out.h() - 1;

    for( j = 1; j < w; ++j )
    {
        for( i = 1; i < h; ++i )
        {
            calculate( i, j );
            rInterval.add( r );
            gInterval.add( g );
            bInterval.add( b );
        }
    }

    for( j = 1; j < out.w() - 1; ++j )
    {
        for( i = 1; i < out.h() - 1; ++i )
        {
            calculate( i, j );
            r = rInterval.normalize( r );
            g = gInterval.normalize( g );
            b = bInterval.normalize( b );

            po = out( j, i );
            po->r = r * 255;
            po->g = g * 255;
            po->b = b * 255;
            po->a = 255;

            double x, y, l;

            x = rx + gx + bx;
            y = ry + gy + by;
            l = Sqrt( x * x + y * y );
            x /= l;
            y /= l;
            po->r = x * 255;
            po->g = y * 255;
            po->b = 128;

            if( ( rx > 10 ) || ( ry > 10 ) || ( gx > 10 ) || ( gy > 10 ) || ( bx > 10 ) || ( by > 10 ) )
            {
                po->r = 255;
                po->g = 255;
                po->b = 255;
            }
            else
            {
                po->r = 0;
                po->g = 0;
                po->b = 0;
            }
        }
    }
}
