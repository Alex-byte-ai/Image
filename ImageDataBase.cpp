#include "ImageDataBase.h"

#include "Exception.h"
#include "Basic.h"

Pixel::Pixel()
{
    r = g = b = 0;
    a = 255;
}

Pixel::Pixel( unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha ) : b( blue ), g( green ), r( red ), a( alpha )
{}

bool Pixel::operator==( Pixel other ) const
{
    return ( r == other.r ) && ( g == other.g ) && ( b == other.b ) && ( a == other.a );
}

bool Pixel::operator!=( Pixel other ) const
{
    return ( r != other.r ) || ( g != other.g ) || ( b != other.b ) || ( a != other.a );
}

Pixel Pixel::invert() const
{
    return Pixel( 255 - r, 255 - g, 255 - b, a );
}

Pixel::operator const Color() const
{
    return Color( ( ( double )r ) / 255, ( ( double )g ) / 255, ( ( double )b ) / 255, ( ( double )a ) / 255 );
}

double Color::epsilon = 1e-6;

Color::Color()
{
    r = g = b = 0;
    a = 1;
}

Color::Color( double red, double green, double blue, double alpha ) : r( red ), g( green ), b( blue ), a( alpha )
{}

bool Color::operator==( Color other ) const
{
    return ( Abs( r - other.r ) <= epsilon ) &&
           ( Abs( g - other.g ) <= epsilon ) &&
           ( Abs( b - other.b ) <= epsilon ) &&
           ( Abs( a - other.a ) <= epsilon );
}

bool Color::operator!=( Color other ) const
{
    return ( Abs( r - other.r ) > epsilon ) ||
           ( Abs( g - other.g ) > epsilon ) ||
           ( Abs( b - other.b ) > epsilon ) ||
           ( Abs( a - other.a ) > epsilon );
}

Color Color::invert() const
{
    return Color( 1 - r, 1 - g, 1 - b, a );
}

Color::operator const Pixel() const
{
    makeException( valid() );
    Color c = limit();
    return Pixel( Round( c.r * 255 ), Round( c.g * 255 ), Round( c.b * 255 ), Round( c.a * 255 ) );
}

Color Color::operator+( Color other ) const
{
    Color result;
    auto s = a + other.a;
    result.r = s > 0 ? ( r * a + other.r * other.a ) / s : 0;
    result.g = s > 0 ? ( g * a + other.g * other.a ) / s : 0;
    result.b = s > 0 ? ( b * a + other.b * other.a ) / s : 0;
    result.a = s;
    return result;
}

Color Color::operator-( Color other ) const
{
    Color result;
    auto s = a - other.a;
    result.r = ( r * a - other.r * other.a ) / s;
    result.g = ( g * a - other.g * other.a ) / s;
    result.b = ( b * a - other.b * other.a ) / s;
    result.a = s;
    return result;
}

Color Color::operator*( double k ) const
{
    Color result;
    result.r = r;
    result.g = g;
    result.b = b;
    result.a = a * k;
    return result;
}

Color Color::operator/( double k ) const
{
    Color result;
    result.r = r;
    result.g = g;
    result.b = b;
    result.a = a / k;
    return result;
}

Color Color::layer( Color other, int count ) const
{
    auto result = *this;
    if( count >= 0 )
    {
        while( count > 0 )
        {
            result = result * ( 1 - other.a ) + other;
            --count;
        }
    }
    else
    {
        while( count < 0 )
        {
            result = ( result - other ) / ( 1 - other.a );
            ++count;
        }
    }
    return result;
}

Color Color::pad( Color other, int count ) const
{
    Color result;
    if( count >= 0 )
    {
        result = Color( 0, 0, 0, 0 );
        while( count > 0 )
        {
            result = result.layer( other );
            --count;
        }
        result = result.layer( *this );
    }
    else
    {
        result = *this;
        while( count < 0 )
        {
            result = ( result * ( 1 - other.a ) - other * ( 1 - result.a ) ) / ( 1 - other.a );
            ++count;
        }
    }
    return result;
}

Color Color::limit() const
{
    Color result = *this;
    if( result.r > 1 ) result.r = 1;
    if( result.g > 1 ) result.g = 1;
    if( result.b > 1 ) result.b = 1;
    if( result.r < 0 ) result.r = 0;
    if( result.g < 0 ) result.g = 0;
    if( result.b < 0 ) result.b = 0;
    if( result.a > 1 ) result.a = 1;
    if( result.a < 0 ) result.a = 0;
    return result;
}

bool Color::valid() const
{
    return -epsilon < r && r < 1 + epsilon &&
           -epsilon < g && g < 1 + epsilon &&
           -epsilon < b && b < 1 + epsilon &&
           -epsilon < a && a < 1 + epsilon;
}
