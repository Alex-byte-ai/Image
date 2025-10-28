#include "Overlap.h"

#include <vector>

#include "Basic.h"
namespace Overlap
{
Picture::Picture( const Canvas &canvas ) :
    colors( canvas.width(), canvas.height() ), mesh( canvas.width() + 1, canvas.height() + 1 )
{
    colors.apply( [this, &canvas]( int j, int i, Color & color )
    {
        color = canvas( j, i ).color;
    } );
    mesh.apply( []( int j, int i, Vector2D & v )
    {
        v = Vector2D( j, i );
    } );
}

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
    Vector2D p0, p1;
    Quadrangle p;

    q.boundingBox( p0, p1 );

    int i0 = RoundDown( p0.y );
    if( i0 < 0 )
    {
        i0 = 0;
    }
    else if( i0 >= h )
    {
        i0 = h - 1;
    }

    int j0 = RoundDown( p0.x );
    if( j0 < 0 )
    {
        j0 = 0;
    }
    else if( j0 >= w )
    {
        j0 = w - 1;
    }

    int i1 = RoundUp( p1.y );
    if( i1 < 0 )
    {
        i1 = 0;
    }
    else if( i1 >= h )
    {
        i1 = h - 1;
    }

    int j1 = RoundUp( p1.x );
    if( j1 < 0 )
    {
        j1 = 0;
    }
    else if( j1 >= w )
    {
        j1 = w - 1;
    }

    for( int j = j0; j <= j1; ++j )
    {
        for( int i = i0; i <= i1; ++i )
        {
            p.a[3] = Vector2D( j, i );
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

static int solve( const Vector2D& p, const Vector2D& d, double r, Vector2D& p0, Vector2D& p1 )
{
    auto a = d.y;
    auto b = -d.x;
    auto c = d.M( p );

    auto s = Sqrt( a * a + b * b );
    if( s <= 0 )
        return 0;

    a /= s;
    b /= s;
    c /= s;

    s = r * r - c * c;
    if( s < 0 )
        return 0;

    s = Sqrt( s );
    auto ac = a * c;
    auto bc = b * c;
    auto bs = b * s;
    auto as = a * s;

    p0 = {-ac + bs, -bc - as};
    if( s <= 0 )
        return 1;

    p1 = {-ac - bs, -bc + as};
    return 2;
}

void Canvas::draw( const Affine2D& t, double r, double thickness, const Color &contour, const Color &fill )
{
    auto inv = t.inv();
    auto det = inv.t.det();

    auto s = r * Sqrt( Sqr( inv.t.a10 ) + Sqr( inv.t.a00 ) );
    auto b = inv.s.x * inv.t.a10 - inv.s.y * inv.t.a00;

    s /= det;
    b /= det;
    s = Abs( s );

    int i0 = RoundDown( b - s );
    if( i0 < 0 )
    {
        i0 = 0;
    }
    else if( i0 >= h )
    {
        i0 = h - 1;
    }

    int i1 = RoundUp( b + s );
    if( i1 < 0 )
    {
        i1 = 0;
    }
    else if( i1 >= h )
    {
        i1 = h - 1;
    }

    s = r * Sqrt( Sqr( inv.t.a01 ) + Sqr( inv.t.a11 ) );
    b = inv.s.y * inv.t.a01 - inv.s.x * inv.t.a11;

    s /= det;
    b /= det;
    s = Abs( s );

    int j0 = RoundDown( b - s );
    if( j0 < 0 )
    {
        j0 = 0;
    }
    else if( j0 >= w )
    {
        j0 = w - 1;
    }

    int j1 = RoundUp( b + s );
    if( j1 < 0 )
    {
        j1 = 0;
    }
    else if( j1 >= w )
    {
        j1 = w - 1;
    }

    Vector2D p0, p1, point, original;
    int count, total;
    Quadrangle q;
    bool inside;

    for( int j = j0; j <= j1; ++j )
    {
        for( int i = i0; i <= i1; ++i )
        {
            total = 0;
            point = inv( Vector2D( j, i ) );

            if( ( count = solve( point, inv.t * Vector2D( 1, 0 ), r, p0, p1 ) ) )
            {
                p0 = t( p0 );
                if( ( j < p0.x && p0.x < j + 1 ) )
                    q.a[total++] = p0;
                if( count > 1 )
                {
                    p1 = t( p1 );
                    if( j < p1.x && p1.x < j + 1 )
                        q.a[total++] = p1;
                }
            }
            original = Vector2D( j + 1, i );
            point = inv( original );
            inside = point.Abs() <= r;
            if( inside )
                q.a[total++] = original;

            if( ( count = solve( point, inv.t * Vector2D( 0, 1 ), r, p0, p1 ) ) )
            {
                p0 = t( p0 );
                if( ( i < p0.y && p0.y < i + 1 ) )
                    q.a[total++] = p0;
                if( count > 1 )
                {
                    p1 = t( p1 );
                    if( i < p1.y && p1.y < i + 1 )
                        q.a[total++] = p1;
                }
            }
            original = Vector2D( j + 1, i + 1 );
            point = inv( original );
            inside = point.Abs() <= r;
            if( inside )
                q.a[total++] = original;

            if( ( count = solve( point, inv.t * Vector2D( -1, 0 ), r, p0, p1 ) ) )
            {
                p0 = t( p0 );
                if( ( j < p0.x && p0.x < j + 1 ) )
                    q.a[total++] = p0;
                if( count > 1 )
                {
                    p1 = t( p1 );
                    if( j < p1.x && p1.x < j + 1 )
                        q.a[total++] = p1;
                }
            }
            original = Vector2D( j, i + 1 );
            point = inv( original );
            inside = point.Abs() <= r;
            if( inside )
                q.a[total++] = original;

            if( ( count = solve( point, inv.t * Vector2D( 0, -1 ), r, p0, p1 ) ) )
            {
                p0 = t( p0 );
                if( ( i < p0.y && p0.y < i + 1 ) )
                    q.a[total++] = p0;
                if( count > 1 )
                {
                    p1 = t( p1 );
                    if( i < p1.y && p1.y < i + 1 )
                        q.a[total++] = p1;
                }
            }
            original = Vector2D( j, i );
            point = inv( original );
            inside = point.Abs() <= r;
            if( inside )
                q.a[total++] = original;

            double area = q.area( total );
            if( area >= 0 )
                operator()( j, i ).draw( fill, area );
            else
                operator()( j, i ).draw( fill.invert(), -area );
        }
    }
}

void Canvas::draw( const Vector2D& a, const Vector2D& b, double t, const Color &c )
{
    auto n = t * 0.5 * ( b - a ).Normal();
    n = Vector2D( -n.y, n.x );

    Quadrangle line;
    line.a[0] = a - n;
    line.a[1] = a + n;
    line.a[2] = b + n;
    line.a[3] = b - n;
    draw( line, c );
}

void Canvas::draw( const Affine2D& transform, double width, double height, double t, const Color &contour, const Color &fill )
{
    Quadrangle q;

    if( 2 * t < width && 2 * t < height )
    {
        q.a[0] = transform( {t, t} );
        q.a[1] = transform( {t, height - t} );
        q.a[2] = transform( {width - t, height - t} );
        q.a[3] = transform( {width - t, t} );
        draw( q, fill );

        q.a[0] = transform( {0.0, 0.0} );
        q.a[1] = transform( {0.0, height} );
        q.a[2] = transform( {t, height} );
        q.a[3] = transform( {t, 0.0} );
        draw( q, contour );

        q.a[0] = transform( {width - t, 0.0} );
        q.a[1] = transform( {width - t, height} );
        q.a[2] = transform( {width, height} );
        q.a[3] = transform( {width, 0.0} );
        draw( q, contour );

        q.a[0] = transform( {t, 0.0} );
        q.a[1] = transform( {t, t} );
        q.a[2] = transform( {width - t, t} );
        q.a[3] = transform( {width - t, 0.0} );
        draw( q, contour );

        q.a[0] = transform( {t, height - t} );
        q.a[1] = transform( {t, height} );
        q.a[2] = transform( {width - t, height} );
        q.a[3] = transform( {width - t, height - t} );
        draw( q, contour );
    }
    else
    {
        q.a[0] = transform( {0.0, 0.0} );
        q.a[1] = transform( {0.0, height} );
        q.a[2] = transform( {width, height} );
        q.a[3] = transform( {width, 0.0} );
        draw( q, contour );
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
}
