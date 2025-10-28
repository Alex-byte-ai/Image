#pragma once

#include "RandomNumber.h"

#include "ImageDataBase.h"

class B4
{
public:
    long double x, y, z, w;

    B4() : x( 0 ), y( 0 ), z( 0 ), w( 0 )
    {}

    B4( long double s ) : x( s ), y( s ), z( s ), w( s )
    {}

    B4( long double xCoordinate, long double yCoordinate, long double zCoordinate, long double wCoordinate )
        : x( xCoordinate ), y( yCoordinate ), z( zCoordinate ), w( wCoordinate )
    {}

    B4( const B4 &other ) : B4( other.x, other.y, other.z, other.w )
    {}

    B4 operator+( const B4 &other ) const
    {
        return B4( x + other.x, y + other.y, z + other.z, w + other.w );
    }

    B4 operator*( const B4 &other ) const
    {
        return B4( x * other.x, y * other.y, z * other.z, w * other.w );
    }

    B4 &operator=( const B4 &other )
    {
        x = other.x;
        y = other.y;
        z = other.z;
        w = other.w;
        return *this;
    }

    B4 &operator+=( const B4 &other )
    {
        return *this = *this + other;
    }

    B4 &operator*=( const B4 &other )
    {
        return *this = *this * other;
    }
};

class Filters
{
private:
    template<typename T>
    static void basic( MatrixBase<T> &m, int &w, int &h, T v )
    {
        MatrixBase<T> r;
        r.reset( m.w() + w, m.h() + h, v );
        w = Round( w * 0.5 );
        h = Round( h * 0.5 );
        m.place( r, w, h );
        r.copy( m );
    }
public:
    enum class Border
    {
        extend,
        wrap,
        mirror,
        crop,
        cropKernel,
        constant,
    };

    static RandomNumber randomNumber;

    static void fiveSectors( double x, double y, Color w, Color &z );
    static void function0( double x, double y, Color w, Color &z );
    static void function1( double x, double y, Color w, Color &z );
    static void function2( int w, int h, int j, int i, Pixel x, Pixel &y );
    static void function3( int w, int h, int j, int i, Pixel x, Pixel &y );
    static void gray( double x, double y, Color w, Color &z );
    static void noize( double x, double y, Color w, Color &z );
    static void grayOut( double x, double y, Color w, Color &z );
    static void rainbowPie( double x, double y, Color w, Color &z );

    template<typename T>
    static void constant( MatrixBase<T> &m, int w, int h, T v )
    {
        basic( m, w, h, v );
    }

    template<typename T>
    static void extend( MatrixBase<T> &m, int padW, int padH )
    {
        int w = m.w();
        int h = m.h();

        basic( m, padW, padH, T() );

        int j0, j1, i0, i1;
        for( j0 = 0; j0 < m.w(); ++j0 )
        {
            for( i0 = 0; i0 < m.h(); ++i0 )
            {
                j1 = j0 - padW;
                if( j1 < 0 ) j1 = 0;
                if( j1 >= w ) j1 = w - 1;
                j1 += padW;

                i1 = i0 - padH;
                if( i1 < 0 ) i1 = 0;
                if( i1 >= h ) i1 = h - 1;
                i1 += padH;

                *m( j0, i0 ) = *m( j1, i1 );
            }
        }
    }

    template<typename T>
    static void wrap( MatrixBase<T> &m, int padW, int padH )
    {
        int w = m.w();
        int h = m.h();

        basic( m, padW, padH, T() );

        int j0, j1, i0, i1;
        for( j0 = 0; j0 < m.w(); ++j0 )
        {
            for( i0 = 0; i0 < m.h(); ++i0 )
            {
                j1 = j0 - padW;
                j1 = ( j1 % w + w ) % w;
                j1 += padW;

                i1 = i0 - padH;
                i1 = ( i1 % h + h ) % h;
                i1 += padH;

                *m( j0, i0 ) = *m( j1, i1 );
            }
        }
    }

    template<typename T>
    static void mirror( MatrixBase<T> &m, int padW, int padH )
    {
        int w = m.w();
        int h = m.h();

        basic( m, padW, padH, T() );

        int j0, j1, i0, i1, value;
        for( j0 = 0; j0 < m.w(); ++j0 )
        {
            for( i0 = 0; i0 < m.h(); ++i0 )
            {
                j1 = j0 - padW;
                value = j1;
                j1 = ( j1 % w + w ) % w;
                if( ( ( value - j1 ) / w ) % 2 ) j1 = w - j1 - 1;
                j1 += padW;

                i1 = i0 - padH;
                value = i1;
                i1 = ( i1 % h + h ) % h;
                if( ( ( value - i1 ) / h ) % 2 ) i1 = h - i1 - 1;
                i1 += padH;

                *m( j0, i0 ) = *m( j1, i1 );
            }
        }
    }

    class Convolution
    {
    public:
        MatrixBase<B4> kernel;
        Border border;
        B4 constant;
        bool alpha;

        Convolution();

        void outline( bool horizontal );
        void operatorSobel( bool horizontal );
        void blurGaussian( int size, long double sigma = 0.849322 );
        void sharpening();
        void emboss();
        void motionBlur();
        void swirl();
        void highPass();
        void checkerboard();
    };

    static void convolution( const Convolution &params, const ImageDataBase &in, ImageDataBase &out );
};
