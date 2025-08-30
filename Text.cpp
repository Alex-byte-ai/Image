#include "Text.h"

#undef min
#undef max

#include <optional>
#include <sstream>
#include <limits>

#include "Exception.h"
#include "Lambda.h"
#include "Basic.h"

#include "BitmapTools.h"
#include "ImageData.h"

template<typename T>
static T extract( const std::wstring &s )
{
    std::wistringstream stream( s );
    T t;
    stream >> t;
    makeException( !stream.fail() );
    return t;
}

template<typename T>
static T extractWithInfinity( const std::wstring &s )
{
    if( s == L"infinity" )
        return std::numeric_limits<T>::max();
    return extract<T>( s );
}

static bool extractBool( const std::wstring &s )
{
    if( s == L"true" )
        return true;
    makeException( s == L"false" );
    return false;
}

static unsigned char extractColorComponent( const std::wstring &s )
{
    auto result = extract<unsigned>( s );
    makeException( result <= std::numeric_limits<unsigned char>::max() );
    return result;
}

class TextGraphics::Data
{
public:
    unsigned char r, g, b, br, bg, bb;
    DRAWTEXTPARAMS params;
    std::wstring text;
    RECT rectangle;
    LOGFONTW font;

    bool changed = true;

    bool update( const TextGraphics &t )
    {
        if( !changed )
            return true;
        changed = false;

        auto get = [&t]( const std::wstring & key )
        {
            auto value = t.get( key );
            makeException( value.has_value() );
            return *value;
        };

        memset( &rectangle, 0, sizeof( rectangle ) );
        memset( &font, 0, sizeof( font ) );
        memset( &params, 0, sizeof( params ) );
        params.cbSize = sizeof( params );

        r = g = b = br = bg = bb = 0;
        text = L"";

        try
        {
            text = get( L"text" );

            rectangle.left = extract<LONG>( get( L"x" ) );
            rectangle.top = extract<LONG>( get( L"y" ) );

            rectangle.right = extractWithInfinity<LONG>( get( L"x'" ) ) - 1;
            rectangle.bottom = extractWithInfinity<LONG>( get( L"y'" ) ) - 1;

            r = extractColorComponent( get( L"text.red" ) );
            g = extractColorComponent( get( L"text.green" ) );
            b = extractColorComponent( get( L"text.blue" ) );

            br = extractColorComponent( get( L"background.red" ) );
            bg = extractColorComponent( get( L"background.green" ) );
            bb = extractColorComponent( get( L"background.blue" ) );

            font.lfQuality = extractBool( get( L"antialiased" ) ) ? ANTIALIASED_QUALITY : NONANTIALIASED_QUALITY;

            font.lfHeight = extract<unsigned>( get( L"height" ) );
            //font.lfHeight = -font.lfHeight;

            font.lfWidth = extract<LONG>( get( L"width" ) );
            font.lfEscapement = extract<LONG>( get( L"escapement" ) );
            font.lfOrientation = extract<LONG>( get( L"orientation" ) );
            font.lfWeight = extract<LONG>( get( L"weight" ) );
            font.lfItalic = extractBool( get( L"italic" ) );
            font.lfUnderline = extractBool( get( L"underline" ) );
            font.lfStrikeOut = extractBool( get( L"strikeOut" ) );

            auto faceName = get( L"faceName" );
            if( faceName.size() >= LF_FACESIZE )
                return false;
            wcscpy( font.lfFaceName, faceName.c_str() );

            params.iTabLength = extract<int>( get( L"tab.length" ) );
            params.iLeftMargin = extract<int>( get( L"margin.left" ) );
            params.iRightMargin = extract<int>( get( L"margin.right" ) );
        }
        catch( ... )
        {
            return false;
        }

        font.lfCharSet = DEFAULT_CHARSET;
        font.lfOutPrecision = OUT_RASTER_PRECIS;
        font.lfCharSet = CLIP_DEFAULT_PRECIS;
        font.lfPitchAndFamily = FF_SWISS | VARIABLE_PITCH;
        return true;
    }
};

void TextGraphics::set( const std::wstring &name, const std::wstring &value )
{
    auto i = parameters.find( name );
    if( i != parameters.end() )
    {
        if( i->second != value )
            data->changed = true;
        i->second = value;
        return;
    }
    parameters.insert( { name, value } );
    data->changed = true;
}

std::optional<std::wstring> TextGraphics::get( const std::wstring &parameterName ) const
{
    auto i = parameters.find( parameterName );
    if( i != parameters.end() )
        return i->second;
    return {};
}

TextGraphics::TextGraphics()
{
    data = new Data;
    set( L"text", L"Sample Text." );
    set( L"x", L"0" );
    set( L"y", L"0" );
    set( L"x'", L"infinity" );
    set( L"y'", L"infinity" );
    set( L"text.red", L"255" );
    set( L"text.green", L"255" );
    set( L"text.blue", L"255" );
    set( L"background.red", L"0" );
    set( L"background.green", L"0" );
    set( L"background.blue", L"0" );
    set( L"antialiased", L"true" );
    set( L"height", L"16" );
    set( L"width", L"0" );
    set( L"escapement", L"0" );
    set( L"orientation", L"0" );
    set( L"weight", L"400" );
    set( L"italic", L"false" );
    set( L"underline", L"false" );
    set( L"strikeOut", L"false" );
    set( L"faceName", L"DejaVuSansMono" );
    set( L"tab.length", L"0" );
    set( L"margin.left", L"0" );
    set( L"margin.right", L"0" );
}

TextGraphics::~TextGraphics()
{
    delete data;
}

static bool manageText( const TextGraphics &text, TextGraphics::Data *data, ImageDataBase *image, int *w, int *h )
{
    Finalizer _;

    if( !data->update( text ) )
        return false;

    if( data->font.lfHeight <= 0 )
    {
        if( w )
            *w = 1;
        if( h )
            *h = 1;
        return true;
    }

    HDC hdc = CreateCompatibleDC( nullptr );
    if( !hdc )
        return false;

    _.push( [hdc]()
    {
        DeleteDC( hdc );
    } );

    void *bits;
    std::shared_ptr<ImageData> area;
    size_t size, count;

    if( image )
    {
        int width, height;
        if( !manageText( text, data, nullptr, &width, &height ) )
            return false;

        area = std::make_shared<ImageData>();
        image->sub(
            *area,
            data->rectangle.left, data->rectangle.top,
            data->rectangle.left + width, data->rectangle.top + height );

        count = area->w() * area->h();
        size = count * sizeof( Pixel );

        BITMAPINFO bmpInfo;
        BitmapTools::init( bmpInfo, area->w(), area->h() );
        HBITMAP hBitmap = CreateDIBSection( hdc, &bmpInfo, DIB_RGB_COLORS, &bits, nullptr, 0 );
        if( !hBitmap )
            return false;

        _.push( [hBitmap]()
        {
            DeleteObject( hBitmap );
        } );

        HGDIOBJ oldBitmap = SelectObject( hdc, hBitmap );
        if( !oldBitmap )
            return false;

        _.push( [hdc, oldBitmap]()
        {
            SelectObject( hdc, oldBitmap );
        } );

        copy( bits, area->rawData()[0], size );
    }

    HFONT hFont = CreateFontIndirectW( &data->font );
    if( !hFont )
        return false;

    _.push( [hFont]()
    {
        DeleteObject( hFont );
    } );

    HGDIOBJ oldFont = SelectObject( hdc, hFont );
    if( !oldFont )
        return false;

    _.push( [hdc, oldFont]()
    {
        SelectObject( hdc, oldFont );
    } );

    auto oldBkMode = SetBkMode( hdc, TRANSPARENT );
    auto oldBkColor = SetBkColor( hdc, RGB( data->br, data->bg, data->bb ) );
    auto oldTextColor = SetTextColor( hdc, RGB( data->r, data->g, data->b ) );
    _.push( [hdc, oldBkMode, oldBkColor, oldTextColor]()
    {
        SetBkMode( hdc, oldBkMode );
        SetBkColor( hdc, oldBkColor );
        SetTextColor( hdc, oldTextColor );
    } );

    if( data->font.lfQuality == NONANTIALIASED_QUALITY )
    {
        auto oldGraphicsMode = SetGraphicsMode( hdc, GM_COMPATIBLE );
        auto oldMapMode = SetMapMode( hdc, MM_TEXT );
        _.push( [hdc, oldGraphicsMode, oldMapMode]()
        {
            SetMapMode( hdc, oldMapMode );
            SetGraphicsMode( hdc, oldGraphicsMode );
        } );
    }

    UINT format = DT_NOPREFIX | DT_EXPANDTABS | DT_LEFT | DT_TOP | ( image ? 0 : DT_CALCRECT );

    auto temporaryTextSize = data->text.length();
    std::vector<wchar_t> temporaryText( 2.5f * temporaryTextSize, 0 );
    copy( temporaryText.data(), data->text.c_str(), temporaryTextSize * sizeof( wchar_t ) );

    RECT rectangle;
    rectangle.left = 0;
    rectangle.top = 0;
    rectangle.right = data->rectangle.right - data->rectangle.left;
    rectangle.bottom = data->rectangle.bottom - data->rectangle.top;

    auto params = data->params;

    /*auto pixel = GetPixel( hdc, 32, 32 );

    SetPixel( hdc, 32, 32, RGB( 255, 0, 0 ) );
    pixel = GetPixel( hdc, 32, 32 );

    SetPixel( hdc, 32, 32, RGB( 0, 0, 255 ) );
    pixel = GetPixel( hdc, 32, 32 );

    for( unsigned i = 32; i < 64; ++i )
    {
        for( unsigned j = 32; j < 64; ++j )
        {
            SetPixel( hdc, j, i, RGB( 255, 0, 0 ) );
        }
    }

    for( unsigned i = 32; i < 42; ++i )
    {
        for( unsigned j = 32; j < 42; ++j )
        {
            SetPixel( hdc, j, i, RGB( data->r, data->g, data->b ) );
        }
    }*/

    // SafeDrawTextExW
    if( !DrawTextExW( hdc, temporaryText.data(), -1, &rectangle, format, &params ) )
        return false;

    if( w )
        *w = rectangle.right - rectangle.left + 1;
    if( h )
        *h = rectangle.bottom - rectangle.top + 1;
    if( image )
    {
        Pixel *dest = area->rawData()[0];
        Pixel *src = ( Pixel * )bits;
        for( size_t i = 0; i < count; ++i )
        {
            if( *dest != *src )
            {
                *dest = *src;
                dest->a = 255;
            }
            ++dest;
            ++src;
        }
        //copy( area->rawData()[0], bits, size );
        area->place( *image, data->rectangle.left, data->rectangle.top );
    }

    return true;
}

bool TextGraphics::measure( int &w, int &h ) const
{
    return manageText( *this, data, nullptr, &w, &h );
}

bool TextGraphics::operator()( ImageDataBase &image ) const
{
    return manageText( *this, data, &image, nullptr, nullptr );
}
