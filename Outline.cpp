#include "Outline.h"

#include "Basic.h"

void Outline( const ImageData &in, ImageData &out )
{
    Pixel pi, pix, piy, *po;
    int rx, gx, bx,
        ry, gy, by;
    Interval<long double> xInterval;
    long double x;

    out.reset( in.w(), in.h() );

    auto calculate = [&]( unsigned i, unsigned j )
    {
        pi = *in( j, i );
        pix = *in( ( j + 1 ) % out.w(), i );
        piy = *in( j, ( i + 1 ) % out.h() );
        po = out( j, i );

        rx = pix.r - pi.r;
        gx = pix.g - pi.g;
        bx = pix.b - pi.b;

        ry = piy.r - pi.r;
        gy = piy.g - pi.g;
        by = piy.b - pi.b;

        x = Sqrt( ( long double )( Sqr( rx ) + Sqr( ry ) ) ) + Sqrt( ( long double )( Sqr( gx ) + Sqr( gy ) ) ) + Sqrt( ( long double )( Sqr( bx ) + Sqr( by ) ) );
    };

    for( int j = 0; j < out.w(); ++j )
    {
        for( int i = 0; i < out.h(); ++i )
        {
            calculate( i, j );
            xInterval.add( x );
        }
    }

    for( int j = 0; j < out.w(); ++j )
    {
        for( int i = 0; i < out.h(); ++i )
        {
            calculate( i, j );
            x = xInterval.normalize( x );
            po->r = Round( x * 255 );
            po->g = po->r;
            po->b = po->r;
            po->a = 255;
        }
    }
}
