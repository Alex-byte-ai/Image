#include "Quadrangle.h"

#include <array>

#include "Basic.h"

template<typename T, size_t N>
class FastVector
{
public:
    T *elements;
    size_t &n;

    inline FastVector() : elements( globalElements ), n( globalN )
    {}

    inline FastVector( T *e, size_t &number ) : elements( e ), n( number )
    {}

    inline size_t size() const
    {
        return n;
    }

    inline void push_back( const T &e )
    {
        elements[n++] = e;
    }

    inline void clear()
    {
        n = 0;
    }

    inline FastVector &operator=( const FastVector &other )
    {
        n = other.n;
        for( size_t i = 0 ; i < n; ++i )
            elements[i] = other.elements[i];
        return *this;
    }

    inline T &operator[]( size_t i )
    {
        return elements[i];
    }

    const T &operator[]( size_t i ) const
    {
        return elements[i];
    }

    static T globalElements[N];
    static size_t globalN;
};

template<typename T, size_t N>
T FastVector<T, N>::globalElements[N];

template<typename T, size_t N>
size_t FastVector<T, N>::globalN = 0;

using Points = FastVector<Vector2D, 8>;

template<typename Container>
static inline void clip( Container &points, const Vector2D &a, const Vector2D &b )
{
    Container clippedPoints;

    double pointAB, nextAB;
    Vector2D c, point, next;

    nextAB = ( b.x - a.x ) * ( points[0].y - a.y ) - ( b.y - a.y ) * ( points[0].x - a.x );

    clippedPoints.clear();
    for( size_t i = 0; i < points.size(); ++i )
    {
        point = points[i];
        next = points[( i + 1 ) % points.size()];

        pointAB = nextAB;
        nextAB = ( b.x - a.x ) * ( next.y - a.y ) - ( b.y - a.y ) * ( next.x - a.x );

        if( ( pointAB > 0 ) && ( nextAB > 0 ) )
        {
            clippedPoints.push_back( next );
        }
        else if( ( pointAB <= 0 ) && ( nextAB > 0 ) )
        {
            c = ( next - point )  * ( pointAB / ( pointAB - nextAB ) ) + point;
            clippedPoints.push_back( c );
            clippedPoints.push_back( next );
        }
        else if( ( pointAB > 0 ) && ( nextAB <= 0 ) )
        {
            c = ( next - point )  * ( pointAB / ( pointAB - nextAB ) ) + point;
            clippedPoints.push_back( c );
        }
    }
    points = clippedPoints;
}

template<typename Container>
static inline void clip( Container &points, const Container &cutter )
{
    for( size_t i = 0; i < cutter.size(); ++i )
        clip( points, cutter[i], cutter[( i + 1 ) % cutter.size()] );
}

template<typename Container>
static inline double area( const Container &points )
{
    double s = 0;
    for( size_t i = 0; i < points.size(); ++i )
        s += points[i].M( points[( i + 1 ) % points.size()] );
    return s * 0.5;
}

void Quadrangle::boundingBox( Vector2D &p0, Vector2D &p1 ) const
{
    p0.x = p0.y = std::numeric_limits<double>::max();

    if( a[0].x < p0.x ) p0.x = a[0].x;
    if( a[1].x < p0.x ) p0.x = a[1].x;
    if( a[2].x < p0.x ) p0.x = a[2].x;
    if( a[3].x < p0.x ) p0.x = a[3].x;

    if( a[0].y < p0.y ) p0.y = a[0].y;
    if( a[1].y < p0.y ) p0.y = a[1].y;
    if( a[2].y < p0.y ) p0.y = a[2].y;
    if( a[3].y < p0.y ) p0.y = a[3].y;

    p1.x = p1.y = std::numeric_limits<double>::lowest();

    if( a[0].x > p1.x ) p1.x = a[0].x;
    if( a[1].x > p1.x ) p1.x = a[1].x;
    if( a[2].x > p1.x ) p1.x = a[2].x;
    if( a[3].x > p1.x ) p1.x = a[3].x;

    if( a[0].y > p1.y ) p1.y = a[0].y;
    if( a[1].y > p1.y ) p1.y = a[1].y;
    if( a[2].y > p1.y ) p1.y = a[2].y;
    if( a[3].y > p1.y ) p1.y = a[3].y;
}

void Quadrangle::flip()
{
    std::swap( a[0].x, a[2].x );
    std::swap( a[0].y, a[2].y );
}

double Quadrangle::area( size_t n ) const
{
    FastVector<const Vector2D, 8> q( a, n );
    return ::area( q );
}

double Quadrangle::commonArea( Quadrangle q0, Quadrangle q1 )
{
    size_t n0 = 4, n1 = 4;
    Points vq0( q0.a, n0 ), vq1( q1.a, n1 );

    auto sign = Sign( ( vq1[1] - vq1[0] ).M( vq1[2] - vq1[1] ) );
    if( sign < 0 )
        q1.flip();

    clip( vq0, vq1 );
    return sign * ::area( vq0 );
}
