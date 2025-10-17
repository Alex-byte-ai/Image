#include "Overlap.h"

#include <vector>

#include "Basic.h"

Picture::Picture( const ImageDataBase &picture )
    : colors( picture.w(), picture.h() ), mesh( picture.w() + 1, picture.h() + 1 )
{
    colors.apply( [this, &picture]( int j, int i, Color & color )
    {
        color = ( Color ) * picture( j, i );
    } );
    mesh.apply( []( int j, int i, Vector2D & v )
    {
        v = Vector2D( j, i );
    } );
}

void Picture::set( const Affine2D &transformation )
{
    mesh.apply( [&transformation]( int j, int i, Vector2D & v )
    {
        v = transformation( Vector2D( j, i ) );
    } );
}

void Picture::apply( const Affine2D &transformation )
{
    mesh.apply( [&transformation]( int, int, Vector2D & v )
    {
        v = transformation( v );
    } );
}

void Picture::apply( const Picture::FunctionConst &f ) const
{
    colors.apply( [this, &f]( int j, int i, const Color & color )
    {
        Quadrangle q;
        q.a[3] = mesh( j, i );
        q.a[2] = mesh( j + 1, i );
        q.a[1] = mesh( j + 1, i + 1 );
        q.a[0] = mesh( j, i + 1 );
        f( j, i, color, q );
    } );
}

void PixelObject::draw( const Color &c, double area )
{
    overlaping.emplace_back( Overlap{c, area} );
}

void PixelObject::bake()
{
    color = calculate();
    clear();
}

Color PixelObject::calculate()
{
    Color result( 0, 0, 0, 0 );
    for( const auto &o : overlaping )
    {
        result.r += o.color.r * o.color.a * o.area;
        result.g += o.color.g * o.color.a * o.area;
        result.b += o.color.b * o.color.a * o.area;
        result.a += o.color.a * o.area;
    }
    if( result.a > 0 )
    {
        result.r /= result.a;
        result.g /= result.a;
        result.b /= result.a;
    }
    return result = color.layer( result );
}

void PixelObject::clear()
{
    overlaping.clear();
}

Canvas::Canvas( const ImageDataBase &canvas ) :
    Parent( canvas.w(), canvas.h() )
{
    apply( [this, &canvas]( int j, int i, PixelObject & pixel )
    {
        pixel.color = ( Color ) * canvas( j, i );
    } );
}

void Canvas::draw( const Quadrangle &q, const Color &c )
{
    int i, i0, i1, j, j0, j1;
    Vector2D p0, p1;
    Quadrangle p;

    q.boundingBox( p0, p1 );

    i0 = RoundDown( p0.y );
    if( i0 < 0 )
    {
        i0 = 0;
    }
    else if( i0 >= h )
    {
        i0 = h - 1;
    }

    j0 = RoundDown( p0.x );
    if( j0 < 0 )
    {
        j0 = 0;
    }
    else if( j0 >= w )
    {
        j0 = w - 1;
    }

    i1 = RoundUp( p1.y );
    if( i1 < 0 )
    {
        i1 = 0;
    }
    else if( i1 >= h )
    {
        i1 = h - 1;
    }

    j1 = RoundUp( p1.x );
    if( j1 < 0 )
    {
        j1 = 0;
    }
    else if( j1 >= w )
    {
        j1 = w - 1;
    }

    for( j = j0; j <= j1; ++j )
    {
        for( i = i0; i <= i1; ++i )
        {
            p.a[3] = Vector2D( j,   i );
            p.a[2] = Vector2D( j + 1, i );
            p.a[1] = Vector2D( j + 1, i + 1 );
            p.a[0] = Vector2D( j, i + 1 );

            double area = Quadrangle::commonArea( p, q );

            if( area >= 0 )
                operator()( j, i ).draw( c, area );
            else
                operator()( j, i ).draw( c.invert(), -area );
        }
    }
}

void Canvas::draw( const Picture &picture )
{
    picture.apply( [this]( int, int, const Color & color, const Quadrangle & quadrangle )
    {
        draw( quadrangle, color );
    } );
}

void Canvas::bake()
{
    apply( []( int, int, PixelObject & pixel )
    {
        pixel.bake();
    } );
}

void Canvas::render( ImageDataBase &out )
{
    if( out.w() != w || out.h() != h )
        out.reset( w, h );

    apply( [&out]( int j, int i, PixelObject & pixel )
    {
        *out( j, i ) = ( Pixel )pixel.calculate();
    } );
}

void Canvas::clear()
{
    apply( []( int, int, PixelObject & pixel )
    {
        pixel.clear();
    } );
}
