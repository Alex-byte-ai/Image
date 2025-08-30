#include "Common.h"

#include "../Palette.h"
#include "../CheckProgress.h"

static Cores cores;
static ConsoleOutput *globalText = nullptr;

Cores &getCores()
{
    return cores;
}

ConsoleOutput *&getText()
{
    return globalText;
}

double Core::distance( const Core &other, const Interval<unsigned char> &r, const Interval<unsigned char> &g, const Interval<unsigned char> &b ) const
{
    return Sqrt( Sqr( r.normalize( other.c.r ) - r.normalize( c.r ) ) + Sqr( g.normalize( other.c.g ) - g.normalize( c.g ) ) + Sqr( b.normalize( other.c.b ) - b.normalize( c.b ) ) );
}

double Core::distance2( const Core &other ) const
{
    double x0, y0, x1, y1, dx, dy, dr, dg, db, mdr = 0, mdg = 0, mdb = 0, v;
    unsigned k, n;
    Pixel cur, prv;

    x0 = j + 0.5;
    y0 = i + 0.5;

    x1 = other.j + 0.5;
    y1 = other.i + 0.5;

    dx = x1 - x0;
    dy = y1 - y0;
    v = Sqrt( dx * dx + dy * dy );
    if( v <= 0 )
        return 0;

    n = v;

    dx /= v;
    dy /= v;

    prv = *( *image )( x0, y0 );
    for( k = 0; k < n; ++k )
    {
        x0 += dx;
        y0 += dy;
        cur = *( *image )( x0, y0 );

        dr = ( int )cur.r - ( int )prv.r;
        dg = ( int )cur.g - ( int )prv.g;
        db = ( int )cur.b - ( int )prv.b;

        if( Abs( dr ) > mdr )
            mdr = Abs( dr );
        if( Abs( dg ) > mdg )
            mdg = Abs( dg );
        if( Abs( db ) > mdb )
            mdb = Abs( db );

        prv = cur;
    }

    return Sqrt( mdr * mdr + mdg * mdg + mdb * mdb ) / 255;
}

void ManualPicks( const ImageWindow::InputData &inputData, ImageWindow::OutputData & )
{
    Core next;
    if( inputData.leftMouse.changed() && *inputData.leftMouse )
    {
        next.image = cores.image;
        next.j = *inputData.mouseX;
        next.i = *inputData.mouseY;
        next.c = *( *cores.image )( next.j, next.i );
        cores.cores.push_back( next );
    }
}

bool LimitPalette( const ImageData &in, ImageData &out, double size )
{
    Interval<unsigned char> r, g, b;

    Palette groups[256];
    Cores coresCopy;
    Core next;

    Pixel pi, *po;
    int i, j, k, m;

    out.reset( in.w(), in.h() );
    for( j = 0; j < out.w(); ++j )
    {
        for( i = 0; i < out.h(); ++i )
        {
            pi = *in( j, i );
            r.add( pi.r );
            g.add( pi.g );
            b.add( pi.b );
        }
    }

    // Manual picks
    k = cores.cores.size();
    if( k > 0 )
        coresCopy = cores;

    auto status = [&]()
    {
        *globalText << ( ( double )( j * out.h() + i ) ) * 100 / ( out.w() * out.h() ) << "\n";
    };

    CheckProgress checkProgress( status, 3000 );
    for( j = 0; j < out.w(); ++j )
    {
        for( i = 0; i < out.h(); ++i )
        {
            pi = *in( j, i );

            next.image = &in;
            next.i = i;
            next.j = j;
            next.c = pi;

            m = 0;
            while( ( m < k ) && ( next.distance( coresCopy.cores[m], r, g, b ) >= size ) )
            {
                ++m;
            }
            if( m < k )
            {
                groups[m].add( pi );
            }
            else
            {
                coresCopy.cores.push_back( next );
                groups[k].add( pi );
                ++k;
            }

            checkProgress.check();
        }
    }

    for( j = 0; j < out.w(); ++j )
    {
        for( i = 0; i < out.h(); ++i )
        {
            pi = *in( j, i );
            po = out( j, i );

            m = 0;
            while( ( m < k ) && !groups[m].contains( pi ) )
            {
                ++m;
            }
            if( m < k )
            {
                po->r = 255 * Round( r.normalize( coresCopy.cores[m].c.r ) );
                po->g = 255 * Round( g.normalize( coresCopy.cores[m].c.g ) );
                po->b = 255 * Round( b.normalize( coresCopy.cores[m].c.b ) );
                po->a = 255;
            }
            else
            {
                return false;
            }
        }
    }
    return true;
}

bool ReplacePinkWithTransparent( const ImageData &in, const ImageData &mask, ImageData &out )
{
    Pixel pi, p2, *po;
    int i, j;

    if( in.w() != mask.w() || in.h() != mask.h() )
        return false;

    out.reset( in.w(), in.h() );

    for( j = 0; j < out.w(); ++j )
    {
        for( i = 0; i < out.h(); ++i )
        {
            pi = *in( j, i );
            p2 = *mask( j, i );
            po = out( j, i );

            if( ( p2.r == 255 ) && ( p2.g == 0 ) && ( p2.b == 255 ) )
            {
                *po = p2;
                po->a = 0;
            }
            else
            {
                *po = pi;
                po->a = 255;
            }
        }
    }
    return true;
}
