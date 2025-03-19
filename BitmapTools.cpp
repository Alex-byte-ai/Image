#include "BitmapTools.h"

#include "Lambda.h"

void BitmapTools::init( BITMAPINFOHEADER &bmiHeader, int w, int h )
{
    memset( &bmiHeader, 0, sizeof( bmiHeader ) );
    bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
    bmiHeader.biWidth = w;
    bmiHeader.biHeight = -h; // Top-down DIB
    bmiHeader.biPlanes = 1;
    bmiHeader.biBitCount = 32;
    bmiHeader.biCompression = BI_RGB;
    bmiHeader.biSizeImage = w * h * sizeof( RGBQUAD );
}

void BitmapTools::init( BITMAPINFO &bmpInfo, int w, int h )
{
    memset( &bmpInfo, 0, sizeof( bmpInfo ) );
    init( bmpInfo.bmiHeader, w, h );
}

BitmapTools::BitmapTools( ImageDataBase &i ) : image( i )
{}

void BitmapTools::setCropping( int x0, int y0, int x1, int y1 )
{
    cropping.reset();
    cropping.emplace();
    cropping->x0 = x0;
    cropping->y0 = y0;
    cropping->x1 = x1;
    cropping->y1 = y1;
}

void BitmapTools::setCropping( int x0, int y0, int x1, int y1, const Pixel &background )
{
    setCropping( x0, y0, x1, y1 );
    cropping->background = background;
}

BitmapTools::~BitmapTools()
{}

static bool getSize( HDC hdc, int &w, int &h )
{
    BITMAP bitmap;
    memset( &bitmap, 0, sizeof( bitmap ) );

    HGDIOBJ hBitmap = GetCurrentObject( hdc, OBJ_BITMAP );
    if( !hBitmap )
        return false;

    if( !GetObject( hBitmap, sizeof( bitmap ), &bitmap ) )
        return false;

    w = bitmap.bmWidth;
    h = bitmap.bmHeight;
    return true;
}

bool BitmapTools::operator<<( HDC hdc )
{
    Finalizer _;

    _.push( { [this]()
    {
        crop();
    }, true } );

    if( hdc == nullptr )
        return _( false );

    int w, h;
    if( !getSize( hdc, w, h ) )
        return _( false );

    if( w <= 0 || h <= 0 )
        return _( false );

    HDC hdcMem = CreateCompatibleDC( hdc );
    if( !hdcMem )
        return _( false );
    _.push( [hdcMem]()
    {
        DeleteDC( hdcMem );
    } );

    // Create a compatible bitmap
    HBITMAP hBitmap = CreateCompatibleBitmap( hdc, w, h );
    if( !hBitmap )
        return _( false );

    _.push( [hBitmap]()
    {
        DeleteObject( hBitmap );
    } );

    HGDIOBJ oldBitmap = SelectObject( hdcMem, hBitmap );
    if( !oldBitmap )
        return _( false );

    _.push( [hdcMem, oldBitmap]()
    {
        SelectObject( hdcMem, oldBitmap );
    } );

    // Copy the data from the source DC to the memory DC
    if( !BitBlt( hdcMem, 0, 0, w, h, hdc, 0, 0, SRCCOPY ) )
        return _( false );

    BITMAPINFOHEADER bi;
    init( bi, w, h );

    if( !fits( w, h ) )
        image.reset( w, h );

    return _( GetDIBits( hdcMem, hBitmap, 0, h, image.rawData()[0], reinterpret_cast<BITMAPINFO *>( &bi ), DIB_RGB_COLORS ) );
}

bool BitmapTools::operator>>( HDC hdc ) const
{
    if( hdc == nullptr || image.empty() )
        return false;

    BITMAPINFO bmpInfo;
    init( bmpInfo, image.w(), image.h() );

    return SetDIBitsToDevice( hdc, 0, 0, image.w(), image.h(), 0, 0, 0, image.h(), image.rawData()[0], &bmpInfo, DIB_RGB_COLORS ) != 0;
}

bool BitmapTools::operator<<( HBITMAP hBitmap )
{
    Finalizer _;

    _.push( { [this]()
    {
        crop();
    }, true } );

    if( hBitmap == nullptr )
        return _( false );

    BITMAP bmp = {};
    if( GetObject( hBitmap, sizeof( BITMAP ), &bmp ) == 0 )
        return _( false );

    auto w = bmp.bmWidth;
    auto h = bmp.bmHeight;
    if( w <= 0 || h <= 0 )
        return _( false );

    if( !fits( w, h ) )
        image.reset( w, h );

    BITMAPINFO bmpInfo = {};
    init( bmpInfo, w, h );

    HDC hdc = GetDC( nullptr );
    _.push( [hdc]()
    {
        ReleaseDC( nullptr, hdc );
    } );

    return _( GetDIBits( hdc, hBitmap, 0, h, image.rawData()[0], &bmpInfo, DIB_RGB_COLORS ) );
}

bool BitmapTools::operator>>( HBITMAP hBitmap ) const
{
    Finalizer _;

    if( hBitmap == nullptr )
        return false;

    BITMAPINFO bmpInfo = {};
    init( bmpInfo, image.w(), image.h() );

    HDC hdc = GetDC( nullptr );
    _.push( [hdc]()
    {
        ReleaseDC( nullptr, hdc );
    } );

    HDC memDC = CreateCompatibleDC( hdc );
    if( !memDC )
        return false;

    _.push( [memDC]()
    {
        DeleteDC( memDC );
    } );

    HGDIOBJ oldBitmap = SelectObject( memDC, hBitmap );
    _.push( [memDC, oldBitmap]()
    {
        SelectObject( memDC, oldBitmap );
    } );

    return SetDIBits( memDC, hBitmap, 0, image.h(), image.rawData()[0], &bmpInfo, DIB_RGB_COLORS ) != 0;
}

bool BitmapTools::operator<<( const ImageDataBase &other )
{
    Finalizer _;

    _.push( { [this]()
    {
        crop();
    }, true } );

    if( !fits( other.w(), other.h() ) )
        image.reset( other.w(), other.h() );
    image.rawData() = other.rawData();
    return _( true );
}

bool BitmapTools::operator>>( ImageDataBase &other ) const
{
    if( other.w() != image.w() || other.h() != image.h() )
        other.reset( image.w(), image.h() );
    other.rawData() = image.rawData();
    return true;
}

void BitmapTools::crop()
{
    if( cropping )
    {
        if( cropping->background )
            image.crop( image, cropping->x0, cropping->y0, cropping->x1, cropping->y1, *cropping->background );
        else
            image.sub( image, cropping->x0, cropping->y0, cropping->x1, cropping->y1 );
    }
}
