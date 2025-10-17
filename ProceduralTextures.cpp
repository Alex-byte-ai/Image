#include "ProceduralTextures.h"

#include <algorithm>
#include <sstream>
#include <fstream>
#include <set>

#include "Exception.h"
#include "Vector2D.h"
#include "Affine2D.h"
#include "Matrix.h"

#include "ImageData.h"
#include "GetImage.h"
#include "Text.h"

RandomFunction::RandomFunction( RandomNumber& random, size_t intervalCount, size_t coefficientsCount, double min, double max )
{
    if( intervalCount < 1 )
        return;

    std::vector<double> results( intervalCount );
    for( size_t n = 0; n < intervalCount; ++n )
        results[n] = random.getReal( min, max );

    auto aF = []( size_t n, double x )
    {
        double c = 2 * Pi() * n;
        return -Sin( c * x ) / c;
    };

    auto bF = []( size_t n, double x )
    {
        double c = 2 * Pi() * n;
        return Cos( c * x ) / c;
    };

    auto delta = 1.0 / intervalCount;

    coefficients.resize( coefficientsCount );
    for( size_t n = 1; n < coefficientsCount; ++n )
    {
        double a = 0, b = 0;
        for( size_t i = 0; i < intervalCount; ++i )
        {
            double x0 = i * delta;
            double x1 = ( i + 1 ) * delta;
            double y = 2 * results[i];
            a += y * ( aF( n, x1 ) - aF( n, x0 ) );
            b += y * ( bF( n, x1 ) - bF( n, x0 ) );
        }

        coefficients[n][0] = a;
        coefficients[n][1] = b;
    }

    auto& a = coefficients[0][0] = 0;
    for( size_t i = 0; i < intervalCount; ++i )
        a += results[i] * delta;

    coefficients[0][1] = 0;
}

double RandomFunction::operator()( double x ) const
{
    double y = 0.0;
    size_t size = coefficients.size();
    x *= 2 * Pi();
    for( size_t n = 0; n < size; ++n )
    {
        auto& c = coefficients[n];
        y += c[0] * Cos( n * x ) + c[1] * Sin( n * x );
    }
    return y;
}

Stripe::Stripe( const Vector2D& s, const Vector2D& e, double w ) : start( s ), end( e ), width( w )
{
    dir = end - start;
    d2 = dir.Sqr();
    d = Sqrt( d2 );
    half = width * 0.5;
}

Vector2D Stripe::middle() const
{
    return ( end + start ) * 0.5;
}

void Stripe::relative( const Vector2D& point, double& h, double& v, bool& inside ) const
{
    auto offset = point - start;
    h = offset * dir / d2;
    v = offset.M( dir ) / ( d * half );
    inside = 0 <= h && h <= 1 && -1 <= v && v <= 1;
}

void Stripe::move( const Vector2D& shift )
{
    start += shift;
    end += shift;
}

Tissue::Tissue( int w, int h, int g, int ver, int hor, double tg, double th, const Color& l, const Color& d ) :
    width( w ), height( h ), granule( g ),
    vertical( ver ), horizontal( hor ),
    tangent( tg ), thickness( th ),
    light( l ), dark( d )
{}

Trunk::Trunk( int w, int l, double v, double c, double s, double d, double p, const Color& ii, const Color& io, const Color& oi, const Color& oo ) :
    diameter( w ), layerCount( l ),
    variation( v ), coreRatio( c ), monotoneSliverAreaRatio( s ), maximalDentRatio( d ), power( p ),
    innerInside( ii ), innerOutside( io ), outerInside( oi ), outerOutside( oo )
{}

Sphere::Sphere( const Vector2D& center, double radius, bool normalize ) : p( center ), r( radius )
{
    if( normalize )
    {
        p = uncopy( p );
        xCopy = p.x > 0 ? ( p.x + r > 0.5 ? -1 : 0 ) : ( p.x - r < -0.5 ? 1 : 0 );
        yCopy = p.y > 0 ? ( p.y + r > 0.5 ? -1 : 0 ) : ( p.y - r < -0.5 ? 1 : 0 );
    }
    else
    {
        xCopy = false;
        yCopy = false;
    }
}

Vector2D Sphere::position() const
{
    return p;
}

double Sphere::radius() const
{
    return r;
}

double Sphere::height( const Vector2D& param ) const
{
    auto nparam = uncopy( param );
    for( auto cp : copy() )
    {
        auto h = Sqr( r ) - ( nparam - cp ).Sqr();
        if( h >= 0 )
            return h;
    }
    return std::numeric_limits<double>::lowest();
}

bool Sphere::inside( const Sphere & other ) const
{
    for( auto cp : copy() )
    {
        for( auto ocp : other.copy() )
        {
            if( ( ocp - cp ).Abs() + r < other.r )
                return true;
        }
    }
    return false;
}

bool Sphere::contains( const Vector2D & point ) const
{
    auto npoint = uncopy( point );
    for( auto cp : copy() )
    {
        if( ( npoint - cp ).Abs() <= r )
            return true;
    }
    return false;
}

std::optional<Arch> Sphere::intersect( const Sphere & other, bool trimmer ) const
{
    for( auto s : shift() )
    {
        for( auto os : other.shift() )
        {
            auto direction = ( other.p + os ) - ( p + s );

            auto a = r;
            auto b = other.r;
            auto c = direction.Abs();

            auto area = ( a + b + c ) * ( -a + b + c ) * ( a - b + c ) * ( a + b - c ); // 16S², S - area of a triangle with sides a, b, c
            if( area <= 0 )
                continue;
            area = Sqrt( area ); // 4S

            auto normal = direction / c;

            auto center = ( ( ( p + s ) + ( other.p + os ) ) - normal * ( Sqr( other.r ) - Sqr( r ) ) / c ) * 0.5;
            auto radius = Vector2D( -normal.y, normal.x ) * ( area / ( 2 * c ) );

            return Arch( center - s, radius, -1.0, 1.0, !trimmer );
        }
    }
    return {};
}

std::vector<Vector2D> Sphere::shift() const
{
    std::vector<Vector2D> copies;

    copies.emplace_back( 0, 0 );

    if( xCopy != 0 )
    {
        copies.emplace_back( xCopy, 0 );
        if( yCopy != 0 )
        {
            copies.emplace_back( xCopy, yCopy );
        }
    }

    if( yCopy != 0 )
    {
        copies.emplace_back( 0, yCopy );
    }

    return copies;
}

std::vector<Vector2D> Sphere::copy() const
{
    std::vector<Vector2D> copies = shift();
    for( auto& s : copies )
        s += p;
    return copies;
}

Vector2D Sphere::uncopy( Vector2D p )
{
    p.x = Mod( p.x + 0.5, 1.0 ) - 0.5;
    p.y = Mod( p.y + 0.5, 1.0 ) - 0.5;
    return p;
}

Arch::Arch( const Vector2D& center, const Vector2D& radius, double startParameter, double finishParameter, bool normalize )
    : host( center, radius.Abs(), normalize ), a( startParameter ), b( finishParameter ), v( radius )
{}

Vector2D Arch::position( double param ) const
{
    return host.position() + v * param;
}

Vector2D Arch::start() const
{
    return position( a );
}

Vector2D Arch::finish() const
{
    return position( b );
}

double Arch::radius() const
{
    return host.radius();
}

Vector2D Arch::vector() const
{
    return v;
}

std::vector<Vector2D> Arch::shift() const
{
    return host.shift();
}

double Arch::startParam() const
{
    return a;
}

double Arch::finishParam() const
{
    return b;
}

double Arch::height( double param ) const
{
    return ( 1 - Sqr( param ) ) * v.Sqr();
}

bool Arch::valid() const
{
    return a < b;
}

void Arch::trimBy( const Sphere& other )
{
    if( host.inside( other ) )
    {
        a = b = 0;
        return;
    }

    auto trimerOpt = host.intersect( other, true );
    if( !trimerOpt )
        return;
    auto& trimer = *trimerOpt;

    Vector2D rotated( -trimer.v.y, trimer.v.x );
    double x = ( ( trimer.host.position() - host.position() ) * rotated ) / ( v * rotated );
    if( !isNumber( x ) || isInfinite( x ) )
        return;

    bool fa = other.contains( position( -1.0 ) );
    bool fb = other.contains( position( 1.0 ) );

    if( fa && !fb )
    {
        if( a < x )
            a = x;
    }
    else if( !fa && fb )
    {
        if( b > x )
            b = x;
    }
    else if( fa && fb )
    {
        a = b = 0;
    }
    else if( !fa && !fb )
    {}
}

void threadSegment( ImageDataBase& image, const Stripe& s, double side, double height, const Stripe& area )
{
    auto mid = s.middle();
    auto length2 = s.d * ( s.d + 2 * side ) * 0.5;

    Affine2D base =
        Affine2D( Matrix2D::Scale( 1 / side, 2 / s.width ) ) *
        Affine2D( Matrix2D::Rotation( -ArcTan2( s.dir.y, s.dir.x ) ) );

    image.function( image, [&]( double x, double y, const Color & in, Color & out )
    {
        Vector2D p( x, y );

        {
            bool inside;
            double h, v;
            area.relative( p, h, v, inside );
            if( !inside )
            {
                out = in;
                return;
            }
        }

        bool inside;
        double h, v;
        s.relative( p, h, v, inside );

        auto ellipse = [&]( const auto & z )
        {
            Affine2D f = base * Affine2D( -z );

            auto r2 = f( p ).Sqr();
            if( r2 < 1 )
            {
                out.r = Sqrt( 1 - r2 );
                inside = true;
            }
        };

        if( inside )
        {
            out.r = Sqrt( 1 - Sqr( v ) );
        }
        else
        {
            ellipse( s.start );
            ellipse( s.end );
        }

        if( inside )
        {
            auto k = ( p - mid ) * s.dir / length2;
            k = 1 - Sqr( k );

            if( k >= 0 )
            {
                k = Sqrt( k ) * out.r;
                out.r = out.g = out.b = k;
                out.a = 1;
            }
        }
        else
        {
            out = in;
        }
    } );
}

void ropeSegment( ImageDataBase& image, const Stripe& s, double tangent, double step )
{
    auto threadWidth = step / Sqrt( 1 + Sqr( tangent ) );
    auto threadRadius = threadWidth * 0.5;
    auto ropeCoreRadius = s.half - threadRadius;

    if( ropeCoreRadius < 0 )
        return;

    auto b = Vector2D( ropeCoreRadius, tangent * ropeCoreRadius );
    auto a = - b;

    double empty = Abs( b.y );
    a.y -= empty;
    b.y -= empty;

    Affine2D rotation =
        Affine2D( s.start ) *
        Affine2D( Matrix2D::Rotation( ArcTan2( -s.dir.x, s.dir.y ) ) );

    a = rotation( a );
    b = rotation( b );

    auto step2d = step * s.dir.Normal();
    int count = RoundUp( ( s.d + 2 * empty ) / step ) + 1;
    Stripe thread( a, b, threadWidth );

    for( int i = 0; i < count; ++i )
    {
        threadSegment( image, thread, threadRadius, s.half, s );
        thread.move( step2d );
    }
}

void tissueFragment( ImageDataBase& image, const Tissue& t )
{
    image.reset( 1024, 1024 );

    double widthH = t.horizontal ? 1.0 / t.horizontal : 0.0;
    double widthV = t.vertical ? 1.0 / t.vertical : 0.0;

    double stepH = 1 / Round( 1 / ( widthH * t.thickness ) );
    double stepV = 1 / Round( 1 / ( widthV * t.thickness ) );

    ImageData vertical;
    vertical.reset( image.w(), image.h() );
    auto crossV = [&]( double x )
    {
        ropeSegment( vertical, Stripe( Vector2D( x, -0.5 ), Vector2D( x, 0.5 ), widthV ), t.tangent, stepV );
    };

    ImageData horizontal;
    horizontal.reset( image.w(), image.h() );
    auto crossH = [&]( double y )
    {
        ropeSegment( horizontal, Stripe( Vector2D( -0.5, y ), Vector2D( 0.5, y ), widthH ), t.tangent, stepH );
    };

    for( int i = 0; i < t.horizontal; ++i )
    {
        // if( i % 2 == 1 )
        crossH( ( i + 0.5 ) * widthH - 0.5 );
    }

    for( int j = 0; j < t.vertical; ++j )
    {
        // if( j % 2 == 1 )
        crossV( ( j + 0.5 ) * widthV - 0.5 );
    }

    image.function( image, [&]( int w, int h, int j, int i, const Pixel &, Pixel & out )
    {
        auto option0 = vertical( j, i );
        auto option1 = horizontal( j, i );

        bool priority = ( t.vertical * j / w ) % 2 != ( t.horizontal * i / h ) % 2;
        // bool priority = option0->r > option1->r;

        if( ( !priority && option1->a > 0 ) || option0->a <= 0 )
            std::swap( option0, option1 );

        if( option0->a > 0 )
        {
            double k = option0->r / 255.0;
            out = ( Pixel )( t.light * k + t.dark * ( 1 - k ) );
        }
        else
        {
            out = Pixel( 0, 0, 0, 0 );
        }
    } );

    ImageData scaled;
    scaled.reset( t.granule, t.granule );

    ImageConvert::Reference in, sc;
    makeReference( image, in );
    makeReference( scaled, sc );
    translate( in, sc, true );

    image.reset( t.width, t.height );
    image.function( image, [&]( int, int, int j, int i, const Pixel &, Pixel & out )
    {
        out = *scaled( j % scaled.w(), i % scaled.h() );
        out.a = 255;
    } );
}

void cell( MatrixBase<double>& map, std::deque<Vector2D> shape, const std::vector<Vector2D>& positions )
{
    auto size = shape.size();
    makeException( size >= 3 );

    double s = 0;
    for( size_t i = 0; i < size; ++i )
        s += shape[i].M( shape[( i + 1 ) % size] );

    if( s < 0 )
        std::reverse( shape.begin(), shape.end() );

    for( auto& shift : positions )
    {
        auto function = [&]( double x, double y ) -> std::optional<double>
        {
            double distance = std::numeric_limits<double>::max();
            auto p = Vector2D( x, y );

            for( size_t i = 0;  i < size; ++i )
            {
                auto p0 = shape[i] + shift;
                auto p1 = shape[( i + 1 ) % size] + shift;

                auto d = ( p - p0 ).M( ( p1 - p0 ).Normal() );
                if( d > 0 )
                    return {};

                d = -d;
                if( d < distance )
                    distance = d;
            }

            return distance;
        };

        map.transform<double>( map, [&]( int w, int h, int j, int i, const double & in, double & out )
        {
            auto d = function( ( j + 0.5 ) / w - 0.5, ( i + 0.5 ) / h - 0.5 );
            out = d ? *d : in;
        } );
    }
}

void cellStructure( ImageDataBase& image, RandomNumber& random, int size, double minRadius, double maxRadius, size_t cellCount )
{
    makeException( minRadius > 0 && maxRadius > 0 && maxRadius >= minRadius && maxRadius <= 0.25 );

    std::vector<Sphere> spheres;
    size_t count = 0;

    while( count < cellCount )
    {
        Sphere s( Vector2D( random.getReal( -0.5, 0.5 ), random.getReal( -0.5, 0.5 ) ), random.getReal( minRadius, maxRadius ) );

        bool inside = false;
        for( const auto& sphere : spheres )
        {
            if( s.inside( sphere ) )
            {
                inside = true;
                break;
            }
        }

        if( !inside )
        {
            spheres.push_back( s );
            ++count;
        }
    }

    std::vector<std::vector<size_t>> polygons( count );
    std::vector<Arch> arches;

    for( size_t i = 0; i < count; ++i )
    {
        for( size_t j = i + 1; j < count; ++j )
        {
            auto arch = spheres[i].intersect( spheres[j] );
            if( arch )
            {
                polygons[i].push_back( arches.size() );
                polygons[j].push_back( arches.size() );

                for( size_t k = 0; k < count; ++k )
                {
                    if( k != i && k != j )
                        arch->trimBy( spheres[k] );
                }
                arches.push_back( *arch );
            }
        }
    }

    auto function = [&spheres]( double x, double y )
    {
        double z = 0;
        Vector2D p( x, y );
        for( auto& s : spheres )
        {
            auto h = s.height( p );
            if( h > z )
                z = h;
        }
        return Sqrt( z );
    };

    Interval<double> heights;

    MatrixBase<double> matrix( size, size );

    for( int j = 0; j < size; ++j )
    {
        for( int i = 0; i < size; ++i )
        {
            auto height = function( ( j + 0.5 ) / size - 0.5, ( i + 0.5 ) / size - 0.5 );
            *matrix( j, i ) = height;
            heights.add( height );
        }
    }

    matrix.transform<Pixel>( image, [&]( int, int, int, int, const double & in, Pixel & out )
    {
        auto value = heights.normalize( in );
        out = ( Pixel ) Color( value, value, value );
    } );

    auto draw = [&]( const auto & arch, const auto & pixel )
    {
        if( arch.valid() )
        {
            auto start = arch.start();
            auto finish = arch.finish();
            for( auto s : arch.shift() )
            {
                image.line( Round( ( start.x + s.x + 0.5 ) * size ), Round( ( start.y + s.y + 0.5 ) * size ),
                            Round( ( finish.x + s.x + 0.5 ) * size ), Round( ( finish.y + s.y + 0.5 ) * size ),
                            pixel );
            }
        }
    };

    double minArchLength = std::numeric_limits<double>::max();
    for( auto& arch : arches )
    {
        if( arch.valid() )
        {
            auto length = ( arch.finish() - arch.start() ).Abs();
            if( length < minArchLength )
                minArchLength = length;
        }
        draw( arch, Pixel( 0, 255, 0 ) );
    }

    matrix.reset( size, size );

    for( const auto& polygon : polygons )
    {
        auto n = polygon.size();
        if( n < 3 )
            continue;

        std::deque<Vector2D> shape;
        std::vector<bool> status( n, true );
        size_t validCount = 0;

        for( size_t i = 0; i < n; ++i )
        {
            const auto& arch = arches[polygon[i]];
            if( arch.valid() )
            {
                ++validCount;
            }
            else
            {
                status[i] = false;
            }
        }

        if( validCount < 3 )
            continue;

        double epsilon = 0.5 * minArchLength;
        std::vector<Vector2D> shift
        {
            {-1.0, -1.0},
            {0.0, -1.0},
            {1.0, -1.0},
            {-1.0, 0.0},
            {0.0, 0.0},
            {1.0, 0.0},
            {-1.0, 1.0},
            {0.0, 1.0},
            {1.0, 1.0},
        };

        auto same = [&]( auto a, auto b ) -> std::optional<Vector2D>
        {
            for( auto s : shift )
            {
                if( ( b + s - a ).Abs() < epsilon )
                    return s;
            }
            return {};
        };

        auto sewBack = [&shape, &same]( auto a, auto b )
        {
            if( shape.empty() )
            {
                shape.push_back( a );
                shape.push_back( b );
                return true;
            }
            if( auto s = same( shape.back(), a ) )
            {
                shape.push_back( b + *s );
                return true;
            }
            if( auto s = same( shape.back(), b ) )
            {
                shape.push_back( a + *s );
                return true;
            }
            return false;
        };

        auto sewFront = [&shape, &same]( auto a, auto b )
        {
            if( auto s = same( shape.front(), a ) )
            {
                shape.push_front( b + *s );
                return true;
            }
            if( auto s = same( shape.front(), b ) )
            {
                shape.push_front( a + *s );
                return true;
            }
            return false;
        };

        auto sew = [&sewBack, &sewFront]( auto a, auto b )
        {
            if( sewBack( a, b ) )
                return true;
            return sewFront( a, b );
        };

        bool addedSomething;
        size_t addedCount = 0;
        do
        {
            addedSomething = false;
            for( size_t i = 0; i < n; ++i )
            {
                if( status[i] )
                {
                    const auto& arch = arches[polygon[i]];
                    if( sew( arch.start(), arch.finish() ) )
                    {
                        addedSomething = true;
                        status[i] = false;
                        ++addedCount;
                    }
                }
            }
        }
        while( addedSomething );

        makeException( addedCount == validCount );

        if( !shape.empty() && same( shape.front(), shape.back() ) )
            shape.pop_back();

        if( shape.size() < 3 )
            continue;

        cell( matrix, shape, shift );
    }

    Interval<double> values;

    matrix.transform<Pixel>( image, [&]( int, int, int, int, const double & in, Pixel & )
    {
        values.add( in );
    } );

    matrix.transform<Pixel>( image, [&]( int, int, int, int, const double & in, Pixel & out )
    {
        auto value = values.normalize( in );
        out = ( Pixel ) Color( value, value, value );
    } );
}

void woodSlice( ImageDataBase& image, RandomNumber& random, const Trunk& t )
{
    std::vector<double> layerWidth;
    layerWidth.resize( t.layerCount );

    {
        double r = 0.0;
        for( auto& l : layerWidth )
        {
            l = random.getReal( 1.0, 1.0 + t.variation );
            r += l;
        }

        for( auto& l : layerWidth )
            l /= r;
    }

    auto coreRatio = Pow( t.coreRatio, 2 * t.power );

    int i = 0;
    double currentRatio = 0.0;
    double remainder = coreRatio;
    for( auto& l : layerWidth )
    {
        currentRatio += l;

        if( currentRatio > coreRatio )
        {
            l -= remainder;
            layerWidth.insert( layerWidth.begin() + i, remainder );
            break;
        }

        remainder -= l;
        ++i;
    }

    RandomFunction f( random, 64, 16, 0, 1 );

    auto function = [&]( double x, double y )
    {
        auto angle = ArcTan2( y, x ) / ( 2 * Pi() );
        if( angle < 0 )
            angle += 1;
        return f( angle );
    };

    image.reset( t.diameter, t.diameter );

    Interval<double> deformation;
    image.function( image, [&]( double x, double y, const Color &, Color & )
    {
        deformation.add( function( x, y ) );
    } );

    auto maximalBumpRatio = 1 / t.maximalDentRatio - 1;

    image.function( image, [&]( double x, double y, const Color &, Color & out )
    {
        auto r = ( deformation.normalize( function( x, y ) ) * maximalBumpRatio + 1 ) * Pow( 4 * ( x * x + y * y ), t.power );
        if( r >= 1 )
        {
            out = Color( 0, 0, 0, 0 );
            return;
        }

        bool inner = r < coreRatio;

        double p = 0.0;
        for( auto& l : layerWidth )
        {
            if( r > l )
            {
                r -= l;
            }
            else
            {
                p = r / l;
                break;
            }
        }

        if( p < t.monotoneSliverAreaRatio )
        {
            p = 0;
        }
        else
        {
            p = ( p - t.monotoneSliverAreaRatio ) / ( 1 - t.monotoneSliverAreaRatio );
        }

        out = inner ? t.innerInside * ( 1 - p ) + t.innerOutside * p : t.outerInside * ( 1 - p ) + t.outerOutside * p;
    } );
}

void watermelonPeel( ImageDataBase& image, RandomNumber& random )
{
    auto getSize = []( double x )
    {
        double lineWidth = 64;

        int lineId = RoundDown( x / lineWidth );

        double lineMiddle = lineId * lineWidth + lineWidth * 0.5;

        double deviationFromTheMiddle = x - lineMiddle;

        double size = 5.5 - 0.8 * Sqrt( Abs( deviationFromTheMiddle ) );
        if( size < 0 )
            size = 0;

        return size;
    };

    int w = 1024, h = 1024;
    image.reset( w, h, Pixel( 0, 255, 0 ) );

    for( int i = 0; i < 1024 * 50; ++i )
    {
        int x = random.getInteger( 0, w );
        int y = random.getInteger( 0, h );
        int size = Round( getSize( x ) );
        image.circle( x, y, size, {}, Pixel( 0, 128, 0 ) );
    }
}

void watermelonPulp( ImageDataBase& image, RandomNumber& random )
{
    int w = 1024, h = 1024;
    image.reset( w, h, Pixel( 255, 0, 0 ) );

    for( int i = 0; i < 256; ++i )
    {
        int x = random.getInteger( 0, w );
        int y = random.getInteger( 0, h );
        int size = random.getInteger( 5, 9 );
        int d = 6 * ( 9 - size );
        image.circle( x, y, size, {}, Pixel( 40 + 3 * d / 2, 40 - d, 40 - d ) );
    }
}

void randomImage( ImageDataBase& image, RandomNumber& random, int width, int height, int m, int n, int granulePower, double decayCoefficient )
{
    std::vector<double> coefficients;
    coefficients.resize( granulePower );

    int maxGranule = 1;
    double coefficient = 1, maxValue = 0;
    for( int k = 0; k < granulePower; ++k )
    {
        coefficients[k] = coefficient;
        maxValue += coefficient;
        coefficient *= decayCoefficient;
        maxGranule *= 2;
    }

    auto createGranule = [&]( ImageData & g )
    {
        std::vector<MatrixBase<double>> matrices;

        int granule = 1;
        for( int k = 0; k < granulePower; ++k )
        {
            auto& matrix = matrices.emplace_back( granule, granule );
            for( int j = 0; j < granule; ++j )
            {
                for( int i = 0; i < granule; ++i )
                {
                    // *matrix( j, i ) = random.getReal( 0.0, 1.0 );
                    *matrix( j, i ) = random.getInteger( 0, 1 );
                }
            }
            granule *= 2;
        }

        g.function( g, [&]( int, int, int j, int i, const Pixel &, Pixel & out )
        {
            double value = 0.0;
            for( int k = 0; k < granulePower; ++k )
            {
                auto& matrix = matrices[k];
                int latticeSize = maxGranule / matrix.w();
                value += *matrix( j / latticeSize, i  / latticeSize ) * coefficients[k];
            }
            value /= maxValue;

            out = ( Pixel )Color( value, value, value );
        } );
    };

    ImageData canvas( m * maxGranule, n * maxGranule ), granule( maxGranule, maxGranule );
    for( int j = 0; j < m; ++j )
    {
        for( int i = 0; i < n; ++i )
        {
            createGranule( granule );
            granule.place( canvas, j * maxGranule, i * maxGranule );
        }
    }

    image.reset( width, height );

    ImageConvert::Reference in, sc;
    makeReference( canvas, in );
    makeReference( image, sc );
    translate( in, sc, true );
}
