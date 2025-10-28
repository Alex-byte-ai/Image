#include "Test_24_icon_for_image.h"

#include <algorithm>

#include "RandomNumber.h"

#include "../ImageData.h"
#include "../Palette.h"

static ConsoleOutput *globalText = nullptr;

static void drawIcon( ImageData &image, int size )
{
    if( size <= 0 )
        return;

    double unit = 0.015625; // 1/64

    RandomNumber random;

    auto sky = ( Color )Pixel( 38, 100, 234 );

    auto grass = ( Color )Pixel( 50, 143, 19 );

    auto warmGrass = grass * 0.6 + Color( 1, 1, 0 ) * 0.4;
    auto moistGrass = grass * 0.9 + Color( 0, 0, 1 ) * 0.1;

    auto sunMiddle = ( Color )Pixel( 255, 255, 0 );
    auto sunEdge = ( Color )Pixel( 255, 55, 10 );

    auto cloudMiddle = ( Color )Pixel( 227, 227, 227 );

    auto sun = [&]( double x, double y )
    {
        return 1 - Sqr( ( x - 15.5 * unit ) / ( 4.5 * unit ) ) - Sqr( ( y + 21.5 * unit ) / ( 4.5 * unit ) );
    };

    auto field = [&]( double x, double y )
    {
        double r = 0;
        r = r * x - 18.907;
        r = r * x - 207.571;
        r = r * x + 8.07532;
        r = r * x + 125.568;
        r = r * x - 3.5423;
        r = r * x - 24.8576;
        r = r * x + 1.33276;
        r = r * x + 1.38594;
        r = r * x + 0.0465275;
        r = r * x + 0.140093;
        r = r * x + 0.0538756;

        double level = 0.5 - r;
        y = 0.5 - y;

        r = 1 - y / level;
        r /= Pow( level, 0.75 );

        return r;
    };

    auto cloud = [&]( double x, double y )
    {
        auto dx = x + 12.5 * unit;
        auto dy = y + 15.5 * unit;

        auto semiAngle = ArcTan2( dy, dx );

        dx = dx / ( 10.5 * unit );
        dy = dy / ( 4.5 * unit );

        auto semiRadius = Sqrt( Sqr( dx ) + Sqr( dy ) );

        semiRadius = semiRadius * ( 1 + 0.07 * Sin( 7 * semiAngle + 1.2 ) + 0.01 * Sin( 15 * semiAngle - 0.2 ) );
        return 1 - Sqr( semiRadius );
    };

    auto shake = [&]( double & x )
    {
        double left = x;
        double right = 1 - x;
        x = random.getReal( x - 0.08 * left, x + 0.08 * right );
    };

    auto shakeC = [&]( Color & x )
    {
        shake( x.r );
        shake( x.g );
        shake( x.b );
    };

    auto fix = [&]( Color & x )
    {
        if( x.r > 1 )
            x.r = 1;
        if( x.r < 0 )
            x.r = 0;
        if( x.g > 1 )
            x.g = 1;
        if( x.g < 0 )
            x.g = 0;
        if( x.b > 1 )
            x.b = 1;
        if( x.b < 0 )
            x.b = 0;
        if( x.a > 1 )
            x.a = 1;
        if( x.a < 0 )
            x.a = 0;
    };

    image.function( image, [&]( double x, double y, Color, Color & o )
    {
        o = sky;

        double f = sun( x, y );
        if( f >= 0 )
        {
            f = Sqrt( f );
            o = sunMiddle * f + sunEdge * ( 1 - f );
        }

        f = field( x, y );
        if( f >= 0 )
        {
            o = moistGrass * f + warmGrass * ( 1 - f );
        }

        f = cloud( x, y );
        if( f >= 0 )
        {
            f = Pow( f, 1.6 );
            o = cloudMiddle * f + sky * ( 1 - f );
        }

        fix( o );
        shakeC( o );
    } );
}

static void fixColors( ImageData &image )
{
    return;

    Palette palette;
    palette.add( Pixel( 255, 0, 255 ) );
    palette.add( Pixel( 0, 0, 0 ) );
    palette.add( Pixel( 85, 85, 85 ) );
    palette.add( Pixel( 170, 170, 170 ) );
    palette.add( Pixel( 0, 0, 255 ) );
    palette.add( Pixel( 255, 255, 0 ) );

    image.function( image, [&]( int, int, int, int, const Pixel & input, Pixel & output )
    {
        output = palette.contains( input ) ? input : Pixel( 255, 255, 0 );
        if( output == Pixel( 255, 0, 255 ) )
            output = Pixel( 0, 0, 0, 0 );
    } );
}

void Test_24_icon_for_image( Context &context )
{
    auto &text = context.output();
    auto s = context.scope( __FUNCTION__ );
    globalText = &text;

    auto &info = context.information;
    bool writeDisk = info( L"writeDisk" ).as<bool>();

    if( !writeDisk )
        return;

    std::vector<int> ids { 4, 8, 16, 24, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 256 };
    // std::vector<int> ids { 1024 };
    std::vector<ImageData> icon;

    for( const auto &id : ids )
        text << id % 16 << L" ";
    text << L"\n";
    for( const auto &id : ids )
        text << id / 16 << L" ";
    text << L"\n";

    std::reverse( ids.begin(), ids.end() );

    //for( int id = 1; id <= 256; ++id )
    for( const auto &id : ids )
    {
        auto &image = icon.emplace_back();
        image.reset( id, id, Pixel( 255, 0, 255 ) );
        drawIcon( image, id );
        fixColors( image );
        // image.output( context.Output() / ( std::to_wstring( id ) + L".png" ) );
    }
    ImageData::writeICO( context.Output() / L"image.ico", icon );
}
