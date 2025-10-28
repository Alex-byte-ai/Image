#include "Filters.h"

#include "MatrixArithmetic.h"
#include "Exception.h"
#include "Basic.h"

// https://en.wikipedia.org/wiki/Kernel_(image_processing)

static double randMult()
{
    return Filters::randomNumber.getReal( 0.8, 1 );
}

static double fiveSectorsBase( double x )
{
    if( x < 0.1 )
        return 0.0;
    if( x < 0.3 )
        return 0.2;
    if( x < 0.5 )
        return 0.4;
    if( x < 0.7 )
        return 0.6;
    if( x < 0.9 )
        return 0.8;
    return 1.0;
}

void Filters::fiveSectors( double, double, Color w, Color &z )
{
    z.r = fiveSectorsBase( w.r );
    z.g = fiveSectorsBase( w.g );
    z.b = fiveSectorsBase( w.b );
    z.a = w.a;
}

void Filters::function0( double, double, Color w, Color &z )
{
    z.r = ( w.r + w.g ) / 2;
    z.g = ( w.g + w.b ) / 2;
    z.b = ( w.b + w.r ) / 2;
    z.a = 1;
}

void Filters::function1( double x, double y, Color, Color &z )
{
    z.r = Sqr( 0.2 * x + 0.8 * y );
    z.g = Sqr( 0.4 * x + 0.6 * y );
    z.b = Sqr( 0.7 * x + 0.3 * y );
    z.a = 1;
}

void Filters::function2( int, int, int, int, Pixel x, Pixel &y )
{
    y.r = x.r + x.g;
    y.g = x.g + x.b;
    y.b = x.b + x.r;
    y.a = 255;
}

void Filters::function3( int, int, int j, int i, Pixel, Pixel &y )
{
    y.r = 0.2 * j + 0.8 * i;
    y.g = 0.4 * j + 0.6 * i;
    y.b = 0.7 * j + 0.3 * i;
    y.a = 255;
}

void Filters::gray( double, double, Color w, Color &z )
{
    double v = ( w.r + w.g + w.b ) / 3;
    z.r = v;
    z.g = v;
    z.b = v;
    z.a = w.a;
}

void Filters::noize( double, double, Color w, Color &z )
{
    z.r = w.r * randMult();
    z.g = w.g * randMult();
    z.b = w.b * randMult();
    z.a = w.a;
}

void Filters::grayOut( double, double, Color w, Color &z )
{
    z = w.invert();
    z.r *= 0.25;
    z.g *= 0.25;
    z.b *= 0.25;
    z = z.invert();
    z.a = w.a;
}

void Filters::rainbowPie( double x, double y, Color, Color &z )
{
    if( ( x >= 0 ) && ( y >= 0 ) )
    {
        z.r = 1;
        z.g = 1;
        z.b = 0;
    }
    else if( ( x >= 0 ) && ( y < 0 ) )
    {
        z.r = 0;
        z.g = 1;
        z.b = 0;
    }
    else if( ( x < 0 ) && ( y >= 0 ) )
    {
        z.r = 1;
        z.g = 0;
        z.b = 0;
    }
    else if( ( x < 0 ) && ( y < 0 ) )
    {
        z.r = 0;
        z.g = 0;
        z.b = 1;
    }

    z.a = 0.5 - Sqrt( Sqr( x ) + Sqr( y ) );
    if( z.a < 0 ) z.a = 0;
}

Filters::Convolution::Convolution()
{
    auto &k = kernel;
    k.reset( 1, 1 );
    *k( 0, 0 ) = B4( 1 );
    border = Border::extend;
    alpha = true;
}

void Filters::Convolution::outline( bool horizontal )
{
    auto &k = kernel;

    k.reset( 2, 2 );

    *k( 0, 0 ) = B4( -1 );
    *k( 1, 0 ) = B4( horizontal );

    *k( 0, 1 ) = B4( !horizontal );
    *k( 1, 1 ) = B4( 0 );

    border = Border::extend;
    alpha = false;
}

void Filters::Convolution::operatorSobel( bool horizontal )
{
    auto &k = kernel;

    k.reset( 3, 3 );

    *k( 0, 0 ) = B4( -1 );
    *k( 1, 0 ) = B4( 0 );
    *k( 2, 0 ) = B4( 1 );

    *k( 0, 1 ) = B4( -2 );
    *k( 1, 1 ) = B4( 0 );
    *k( 2, 1 ) = B4( 2 );

    *k( 0, 2 ) = B4( -1 );
    *k( 1, 2 ) = B4( 0 );
    *k( 2, 2 ) = B4( 1 );

    if( horizontal )
        k.transpose( k );

    border = Border::extend;
    alpha = false;
}

void Filters::Convolution::blurGaussian( int size, long double sigma )
{
    auto &k = kernel;

    k.reset( size, size );

    int j, i;
    long double sum = 0;
    int halfSize = size / 2;

    for( j = 0; j < size; ++j )
    {
        for( i = 0; i < size; ++i )
        {
            int x = i - halfSize;
            int y = j - halfSize;
            auto result = Exp( -( x * x + y * y ) / ( 2 * sigma * sigma ) );
            *k( j, i ) = result;
            sum += result;
        }
    }

    for( j = 0; j < size; ++j )
    {
        for( i = 0; i < size; ++i )
        {
            *k( j, i ) *= B4( 1 / sum );
        }
    }

    border = Border::mirror;
    alpha = true;
}

void Filters::Convolution::sharpening()
{
    auto &k = kernel;

    k.reset( 3, 3 );

    *k( 0, 0 ) = B4( 0 );
    *k( 1, 0 ) = B4( -1 );
    *k( 2, 0 ) = B4( 0 );

    *k( 0, 1 ) = B4( -1 );
    *k( 1, 1 ) = B4( 5 );
    *k( 2, 1 ) = B4( -1 );

    *k( 0, 2 ) = B4( 0 );
    *k( 1, 2 ) = B4( -1 );
    *k( 2, 2 ) = B4( 0 );

    border = Border::extend;
    alpha = true;
}

void Filters::Convolution::emboss()
{
    auto &k = kernel;

    k.reset( 3, 3 );

    *k( 0, 0 ) = B4( -2 );
    *k( 1, 0 ) = B4( -1 );
    *k( 2, 0 ) = B4( 0 );

    *k( 0, 1 ) = B4( -1 );
    *k( 1, 1 ) = B4( 1 );
    *k( 2, 1 ) = B4( 1 );

    *k( 0, 2 ) = B4( 0 );
    *k( 1, 2 ) = B4( 1 );
    *k( 2, 2 ) = B4( 2 );

    border = Border::extend;
    alpha = true;
}

void Filters::Convolution::motionBlur()
{
    auto &k = kernel;

    k.reset( 5, 5, B4() );

    B4 m( 0.2 );

    *k( 0, 0 ) = B4( 1 ) * m;
    *k( 1, 0 ) = B4( 1 ) * m;
    *k( 2, 0 ) = B4( 1 ) * m;
    *k( 3, 0 ) = B4( 1 ) * m;
    *k( 4, 0 ) = B4( 1 ) * m;

    border = Border::extend;
    alpha = true;
}

void Filters::Convolution::swirl()
{
    auto &k = kernel;

    k.reset( 3, 3 );

    *k( 0, 0 ) = B4( 1 );
    *k( 1, 0 ) = B4( 0 );
    *k( 2, 0 ) = B4( -1 );

    *k( 0, 1 ) = B4( 0 );
    *k( 1, 1 ) = B4( 0 );
    *k( 2, 1 ) = B4( 0 );

    *k( 0, 2 ) = B4( -1 );
    *k( 1, 2 ) = B4( 0 );
    *k( 2, 2 ) = B4( 1 );

    border = Border::extend;
    alpha = false;
}

void Filters::Convolution::highPass()
{
    auto &k = kernel;

    k.reset( 3, 3 );

    *k( 0, 0 ) = B4( -1 );
    *k( 1, 0 ) = B4( -1 );
    *k( 2, 0 ) = B4( -1 );

    *k( 0, 1 ) = B4( -1 );
    *k( 1, 1 ) = B4( 8 );
    *k( 2, 1 ) = B4( -1 );

    *k( 0, 2 ) = B4( -1 );
    *k( 1, 2 ) = B4( -1 );
    *k( 2, 2 ) = B4( -1 );

    border = Border::extend;
    alpha = false;
}

void Filters::Convolution::checkerboard()
{
    auto &k = kernel;

    k.reset( 3, 3 );

    *k( 0, 0 ) = B4( 1 );
    *k( 1, 0 ) = B4( -1 );
    *k( 2, 0 ) = B4( 1 );

    *k( 0, 1 ) = B4( -1 );
    *k( 1, 1 ) = B4( 1 );
    *k( 2, 1 ) = B4( -1 );

    *k( 0, 2 ) = B4( 1 );
    *k( 1, 2 ) = B4( -1 );
    *k( 2, 2 ) = B4( 1 );

    border = Border::extend;
    alpha = true;
}

void Filters::convolution( const Convolution &params, const ImageDataBase &in, ImageDataBase &out )
{
    using TransformIn = std::function<void( int, int, int, int, const Pixel &, B4 & )>;
    using TransformOut = std::function<void( int, int, int, int, const B4 &, Pixel & )>;

    MatrixArithmetic<B4> input, output;

    in.transform( input, ( TransformIn )[]( int, int, int, int, const Pixel & p, B4 & b )
    {
        b = B4( ( long double )p.r / 255, ( long double )p.g / 255, ( long double )p.b / 255, ( long double )p.a / 255 );
    } );

    switch( params.border )
    {
    case Border::extend:
        extend( input, params.kernel.w() - 1, params.kernel.h() - 1 );
        break;
    case Border::wrap:
        wrap( input, params.kernel.w() - 1, params.kernel.h() - 1 );
        break;
    case Border::mirror:
        mirror( input, params.kernel.w() - 1, params.kernel.h() - 1 );
        break;
    case Border::cropKernel:
        constant( input, params.kernel.w() - 1, params.kernel.h() - 1, B4() );
        break;
    case Border::constant:
        constant( input, params.kernel.w() - 1, params.kernel.h() - 1, params.constant );
        break;
    case Border::crop:
        break;
    default:
        makeException( false );
        break;
    }

    output = input.convolution( MatrixArithmetic<B4>( params.kernel ) );

    Interval<long double> x, y, z, w;
    output.transform( out, ( TransformOut )[&]( int, int, int, int, const B4 & b, Pixel & )
    {
        x.add( b.x );
        y.add( b.y );
        z.add( b.z );
        w.add( b.w );
    } );
    output.transform( out, ( TransformOut )[&]( int, int, int, int, const B4 & b, Pixel & p )
    {
        Color c;
        c.r = x.normalize( b.x );
        c.g = y.normalize( b.y );
        c.b = z.normalize( b.z );
        c.a = params.alpha ? w.normalize( b.w ) : 1;
        p = ( Pixel )c;
    } );
}

RandomNumber Filters::randomNumber( 926374 );
