#include "Test_19_icon_for_console.h"

#include <algorithm>

#include "../ImageData.h"
#include "../Palette.h"

static ConsoleOutput *globalText = nullptr;

static void drawCross( ImageData &image, int x, int y, int size )
{
    if( size <= 0 )
        return;

    image.line( x, y, x + size, y + size, Pixel( 0, 0, 0 ) );
    image.line( x, y + size - 1, x + size, y - 1, Pixel( 0, 0, 0 ) );
}

static void drawSquare( ImageData &image, int x, int y, int size )
{
    if( size <= 0 )
        return;

    image.rectangle( x, y, size, size, Pixel( 0, 0, 0 ) );
    image.rectangle( x + 1, y + 1, size - 2, size - 2, Pixel( 170, 170, 170 ) );
}

static void drawLine( ImageData &image, int x, int y, int size )
{
    if( size <= 0 )
        return;

    int half = size / 2;
    image.line( x, y + half, x + size, y + half, Pixel( 0, 0, 0 ) );
}

static void drawIcon( ImageData &image, int x, std::optional<int> yOpt, int size )
{
    if( size <= 0 )
        return;

    int y = yOpt ? *yOpt : RoundDown( size * 0.125 );

    int w, h;
    if( yOpt ) // It's better to interpret size as hight for recursive icons.
    {
        h = size;
        w = RoundUp( h * 4.0 / 3 );
    }
    else
    {
        w = size; // Width of the drawing
        h = RoundDown( w * 3.0 / 4 ); // Height of the drawing
    }

    if( w <= 0 || h <= 0 )
        return;

    double unit = w * 0.00390625; // w/256

    int wBorder = RoundDown( unit ); // Window's border. Width and height include this.
    int tBorder = RoundDown( 2 * unit ); // Title's border.
    int item = RoundDown( 12 * unit ); // Title's space left, items use all of it.
    int distance = RoundDown( 6 * unit ); // Distance between window's items.

    if( tBorder <= 0 && item > 0 )
        tBorder = 1;
    if( wBorder <= 0 && item > 0 )
        wBorder = 1;
    if( distance <= 0 && item > 0 )
        distance = 1;

    *globalText << w << " ";

    // Window with its border.
    image.rectangle( x, y, w, h, Pixel( 85, 85, 85 ) );
    image.rectangle( x + wBorder, y + wBorder, w - 2 * wBorder, h - 2 * wBorder, Pixel( 0, 0, 255 ) );

    // Tittle bacground
    image.rectangle( x + wBorder, y + wBorder, w - 2 * wBorder, item + 2 * tBorder, Pixel( 170, 170, 170 ) );

    // This next call will use its size argument as its hight.
    drawIcon( image, x + wBorder + tBorder, y + wBorder + tBorder, item );

    int position = x + w - wBorder - tBorder - item ;

    drawCross( image, position, y + wBorder + tBorder, item );
    position -= item + distance;

    drawLine( image, position, y + wBorder + tBorder, item );
    position -= item + distance;

    drawSquare( image, position, y + wBorder + tBorder, item );
    position -= item + distance;

    // Text, its distances from corner are the same as for items in title.
    TextGraphics text;
    text.set( L"text", L"[Console] Running..." );
    text.set( L"x", std::to_wstring( x + wBorder + tBorder ) );
    text.set( L"y", std::to_wstring( y + wBorder + 3 * tBorder + item ) );
    text.set( L"x'", std::to_wstring( x + w ) );
    text.set( L"y'", std::to_wstring( y + h ) );
    text.set( L"height", std::to_wstring( RoundDown( 16 * unit ) ) );
    text.set( L"faceName", L"DejaVuSansMono" );
    text.set( L"text.red", L"255" );
    text.set( L"text.green", L"255" );
    text.set( L"text.blue", L"0" );
    image.text( text );
}

static void fixColors( ImageData &image )
{
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

void Test_19_icon_for_console( Context &context )
{
    auto &text = context.output();
    auto s = context.scope( __FUNCTION__ );
    globalText = &text;

    auto &info = context.information;
    bool writeDisk = info( L"writeDisk" ).as<bool>();

    if( !writeDisk )
        return;

    std::vector<int> ids { 4, 8, 16, 24, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 256 };
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
        drawIcon( image, 0, {}, id );
        text << L"\n";
        fixColors( image );
        //image.output( context.Output() / ( std::to_wstring( id ) + L".png" ) );
    }
    ImageData::writeICO( context.Output() / L"console.ico", icon );
}
