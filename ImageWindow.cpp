#include "ImageWindow.h"

#include <windowsx.h>
#include <windows.h>

#include <algorithm>
#include <cstdlib>

#include "Exception.h"
#include "Lambda.h"
#include "Basic.h"

#include "ImageData.h"

#include "resource.h"

namespace GraphicInterface
{
bool Box::inside( int x0, int y0 )
{
    return x <= x0 && x0 < x + w && y <= y0 && y0 < y + h;
}

void Image::prepare( const void *data, int stride, int height )
{
    bufferW = w = Abs( stride );
    bufferH = h = height;

    pixels.resize( w * h );

    auto output = ( RGBQUAD * )pixels.data();
    auto input = ( const RGBQUAD * )data;

    while( height > 0 )
    {
        for( int j = 0; j < w; ++j )
        {
            auto &o = *( output + j );
            auto &i = *( input + j );
            o.rgbBlue = i.rgbBlue * i.rgbReserved / 255;
            o.rgbGreen = i.rgbGreen * i.rgbReserved / 255;
            o.rgbRed = i.rgbRed * i.rgbReserved / 255;

            // It seems, fully transparent parts of window are not interactable, and there is no way to disable that
            o.rgbReserved = i.rgbReserved <= 0 ? 1 : i.rgbReserved;
        }
        output += bufferW;
        input += stride;
        --height;
    }
}

Description::Description( int h, int sz, int bh, int tgw, int b )
    : titlebarHeight( h ), buttonSize( sz ), buttonSpacingH( bh ), triggerWidth( tgw ), borderWidth( b )
{
    buttonSpacingV = ( titlebarHeight - buttonSize ) / 2;
}

int Description::getMinX() const
{
    auto titleBarMinWidth = 3 * buttonSize + buttonSpacingV + 3 * buttonSpacingH + borderWidth + icon.x + icon.w;
    auto minWidth = content.w + 2 * borderWidth;

    if( titleBarMinWidth > minWidth )
        return titleBarMinWidth;

    return minWidth;
}

int Description::getMinY() const
{
    return titlebarHeight + content.h + 2 * borderWidth;
}

void Description::update()
{
    title.x = window.x;
    title.y = window.y;
    title.w = window.w;
    title.h = titlebarHeight + borderWidth;

    rightTrigger.x = window.x + window.w - triggerWidth;
    rightTrigger.y = window.y;
    rightTrigger.w = triggerWidth;
    rightTrigger.h = window.h;

    bottomTrigger.x = window.x;
    bottomTrigger.y = window.y + window.h - triggerWidth;
    bottomTrigger.w = window.w;
    bottomTrigger.h = triggerWidth;

    leftBorder.x = window.x;
    leftBorder.y = window.y;
    leftBorder.w = borderWidth;
    leftBorder.h = window.h;

    rightBorder.x = window.x + window.w - borderWidth;
    rightBorder.y = window.y;
    rightBorder.w = borderWidth;
    rightBorder.h = window.h;

    topBorder.x = window.x;
    topBorder.y = window.y;
    topBorder.w = window.w;
    topBorder.h = borderWidth;

    bottomBorder.x = window.x;
    bottomBorder.y = window.y + window.h - borderWidth;
    bottomBorder.w = window.w;
    bottomBorder.h = borderWidth;

    closeButton.x = window.x + window.w - buttonSpacingV - buttonSize - borderWidth;
    closeButton.y = window.y + buttonSpacingV + borderWidth;
    closeButton.w = buttonSize;
    closeButton.h = buttonSize;

    maximizeButton = closeButton;
    maximizeButton.x -= buttonSpacingH + buttonSize;

    minimizeButton = maximizeButton;
    minimizeButton.x -= buttonSpacingH + buttonSize;

    icon.y = borderWidth + ( titlebarHeight - icon.h ) / 2;
    icon.x = icon.y;

    text.x = icon.x + icon.w + buttonSpacingV;
    text.y = closeButton.y;

    text.w = window.w - borderWidth - 2 * buttonSpacingV - 3 * buttonSize - 2 * buttonSpacingH - text.x;
    if( text.w < text.bufferW )
        text.w = 0;

    client.x = window.x + borderWidth;
    client.y = window.y + borderWidth + titlebarHeight;
    client.w = window.w - 2 * borderWidth;
    client.h = window.h - titlebarHeight - 2 * borderWidth;

    content.w = content.bufferW;
    content.h = content.bufferH;

    content.x = ( client.w - content.w ) / 2;
    content.y = ( client.h - content.h ) / 2;

    if( content.x <= 0 )
    {
        content.x = 0;
        content.w = client.w;
    }

    if( content.y <= 0 )
    {
        content.y = 0;
        content.h = client.h;
    }

    content.x += borderWidth;
    content.y += borderWidth + titlebarHeight;
}

void Description::draw( uint32_t *pixels, int x, int y )
{
    int width = window.w;
    int height = window.h;

    // --- Title Bar ---
    auto &tib = title;
    auto titleBarColor = makeColor( 255, 255, 255, 255 );
    drawRect( pixels, width, height, tib, titleBarColor );

    // Icon
    drawImage( pixels, window.w, window.h, icon );

    // Title
    drawImage( pixels, window.w, window.h, text );

    // Close button
    auto &clb = closeButton;
    cross( pixels, width, height, clb, clb.inside( x, y ) );

    // Maximize button
    auto &mab = maximizeButton;
    square( pixels, width, height, mab, mab.inside( x, y ) );

    // Minimize button
    auto &mib = minimizeButton;
    line( pixels, width, height, mib, mib.inside( x, y ) );

    // --- Borders ---
    auto borderColor = makeColor( 85, 85, 85, 255 );
    drawRect( pixels, width, height, leftBorder, borderColor );
    drawRect( pixels, width, height, rightBorder, borderColor );
    drawRect( pixels, width, height, topBorder, borderColor );
    drawRect( pixels, width, height, bottomBorder, borderColor );
}

uint32_t makeColor( uint8_t r, uint8_t g, uint8_t b, uint8_t a )
{
    return ( a << 24 ) | ( r << 16 ) | ( g << 8 ) | b;
}

void drawRect( uint32_t *pixels, int width, int height, const Box &b, uint32_t color )
{
    int x0 = b.x;
    int y0 = b.y;
    int x1 = b.x + b.w;
    int y1 = b.y + b.h;

    if( x0 < 0 )
        x0 = 0;
    if( y0 < 0 )
        y0 = 0;

    if( x1 > width )
        x1 = width;
    if( y1 > height )
        y1 = height;

    for( int j = y0; j < y1; ++j )
    {
        for( int i = x0; i < x1; ++i )
        {
            pixels[j * width + i] = color;
        }
    }
}

void drawGradient( uint32_t *pixels, int width, int height, const Box &b )
{
    int x0 = b.x;
    int y0 = b.y;
    int x1 = b.x + b.w;
    int y1 = b.y + b.h;

    if( x0 < 0 )
        x0 = 0;
    if( y0 < 0 )
        y0 = 0;

    if( x1 > width )
        x1 = width;
    if( y1 > height )
        y1 = height;

    int w = x1 - x0;

    for( int j = y0; j < y1; ++j )
    {
        for( int i = x0; i < x1; ++i )
        {
            int x = i - x0;
            uint8_t alpha = x * 255 / w;
            uint8_t red   = ( ( w - x ) * 255 ) / w;
            uint8_t green = 0;
            uint8_t blue  = x * 255 / w;

            pixels[j * width + i] = GraphicInterface::makeColor( red, green, blue, alpha );
        }
    }
}

void drawImage( uint32_t *pixels, int width, int height, const Image &b )
{
    int x0 = b.x;
    int y0 = b.y;
    int x1 = b.x + Min( b.w, b.bufferW );
    int y1 = b.y + Min( b.h, b.bufferH );

    if( x0 < 0 )
        x0 = 0;
    if( y0 < 0 )
        y0 = 0;

    if( x1 > width )
        x1 = width;
    if( y1 > height )
        y1 = height;

    int bx0 = x0 - b.x;

    int w = x1 - x0;
    auto line = w * sizeof( uint32_t );
    auto pointer = b.pixels.data() + bx0;
    for( int j = y0; j < y1; ++j )
    {
        copy( &pixels[j * width + x0], pointer, line );
        pointer += b.bufferW;
    }
}

void drawLineR( uint32_t *pixels, int width, int, int x, int y, int size, uint32_t color )
{
    while( size > 0 )
    {
        pixels[y * width + x] = color;
        ++x;
        --size;
    }
}

void drawLineD( uint32_t *pixels, int width, int, int x, int y, int size, uint32_t color )
{
    while( size > 0 )
    {
        pixels[y * width + x] = color;
        ++y;
        --size;
    }
}

void drawLineRD( uint32_t *pixels, int width, int, int x, int y, int size, uint32_t color )
{
    while( size > 0 )
    {
        pixels[y * width + x] = color;
        ++x;
        ++y;
        --size;
    }
}

void drawLineRU( uint32_t *pixels, int width, int, int x, int y, int size, uint32_t color )
{
    while( size > 0 )
    {
        pixels[y * width + x] = color;
        ++x;
        --y;
        --size;
    }
}

void cross( uint32_t *pixels, int width, int height, const Box &b, bool selected )
{
    auto color = selected ? makeColor( 245, 10, 10, 255 ) : makeColor( 255, 255, 255, 255 );
    drawRect( pixels, width, height, b, color );

    auto black = makeColor( 0, 0, 0, 255 );
    auto half = selected ? makeColor( 196, 8, 8, 255 ) : makeColor( 204, 204, 204, 255 );

    drawLineRD( pixels, width, height, b.x + 4, b.y + 3, b.w - 7, half );
    drawLineRU( pixels, width, height, b.x + 4, b.y + b.h - 4, b.w - 7, half );

    drawLineRD( pixels, width, height, b.x + 3, b.y + 4, b.w - 7, half );
    drawLineRU( pixels, width, height, b.x + 3, b.y + b.h - 5, b.w - 7, half );

    drawLineRD( pixels, width, height, b.x + 3, b.y + 3, b.w - 6, black );
    drawLineRU( pixels, width, height, b.x + 3, b.y + b.h - 4, b.w - 6, black );
}

void square( uint32_t *pixels, int width, int height, const Box &b, bool selected )
{
    auto color = selected ? makeColor( 235, 235, 235, 255 ) : makeColor( 255, 255, 255, 255 );
    drawRect( pixels, width, height, b, color );

    auto black = makeColor( 0, 0, 0, 255 );
    auto half = selected ? makeColor( 188, 188, 188, 255 ) : makeColor( 204, 204, 204, 255 );

    drawLineR( pixels, width, height, b.x + 4, b.y + 4, b.w - 8, half );
    drawLineD( pixels, width, height, b.x + 4, b.y + 4, b.w - 8, half );
    drawLineR( pixels, width, height, b.x + 4, b.y + b.h - 5, b.w - 8, half );
    drawLineD( pixels, width, height, b.x + b.w - 5, b.y + 4, b.w - 8, half );

    drawLineR( pixels, width, height, b.x + 3, b.y + 3, b.w - 6, black );
    drawLineD( pixels, width, height, b.x + 3, b.y + 3, b.w - 6, black );
    drawLineR( pixels, width, height, b.x + 3, b.y + b.h - 4, b.w - 6, black );
    drawLineD( pixels, width, height, b.x + b.w - 4, b.y + 3, b.w - 6, black );
}

void line( uint32_t *pixels, int width, int height, const Box &b, bool selected )
{
    auto color = selected ? makeColor( 235, 235, 235, 255 ) : makeColor( 255, 255, 255, 255 );
    drawRect( pixels, width, height, b, color );

    auto black = makeColor( 0, 0, 0, 255 );

    drawLineR( pixels, width, height, b.x + 3, b.y + b.h / 2 - 1, b.w - 6, black );
}
}

static void updateWindowContent( GraphicInterface::Description &gid, HWND hwnd, int x, int y )
{
    RECT rect;
    GetWindowRect( hwnd, &rect );

    gid.window.x = 0;
    gid.window.y = 0;
    gid.window.w = rect.right - rect.left;
    gid.window.h = rect.bottom - rect.top;
    gid.update();

    BITMAPINFO bmi;
    clear( &bmi, sizeof( bmi ) );
    bmi.bmiHeader.biSize        = sizeof( BITMAPINFOHEADER );
    bmi.bmiHeader.biWidth       = gid.window.w;
    bmi.bmiHeader.biHeight      = -gid.window.h;
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    HDC hdcScreen = GetDC( nullptr );
    HDC hdcMem    = CreateCompatibleDC( hdcScreen );
    void *pBits  = nullptr;
    HBITMAP hBitmap = CreateDIBSection( hdcScreen, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0 );
    if( !hBitmap )
    {
        DeleteDC( hdcMem );
        ReleaseDC( nullptr, hdcScreen );
        return;
    }
    HBITMAP hOldBmp = ( HBITMAP )SelectObject( hdcMem, hBitmap );

    auto *pixels = ( uint32_t * )pBits;

    // Draw the client area
    if( gid.content.w > 0 && gid.content.h > 0 )
    {
        drawRect( pixels, gid.window.w, gid.window.h, gid.client, GraphicInterface::makeColor( 60, 70, 200, 255 ) );
        drawImage( pixels, gid.window.w, gid.window.h, gid.content );
    }
    else
    {
        drawGradient( pixels, gid.window.w, gid.window.h, gid.client );
    }

    gid.draw( pixels, x, y );

    BLENDFUNCTION blend;
    clear( &blend, sizeof( blend ) );
    blend.BlendOp = AC_SRC_OVER;
    blend.SourceConstantAlpha = 255; // Use per-pixel alpha.
    blend.AlphaFormat = AC_SRC_ALPHA;
    POINT ptZero = {0, 0};
    SIZE sizeWindow = {gid.window.w, gid.window.h};
    UpdateLayeredWindow( hwnd, hdcScreen, nullptr, &sizeWindow, hdcMem, &ptZero, 0, &blend, ULW_ALPHA );

    SelectObject( hdcMem, hOldBmp );
    DeleteObject( hBitmap );
    DeleteDC( hdcMem );
    ReleaseDC( nullptr, hdcScreen );
}

class ImageWindow::Implementation
{
public:
    GraphicInterface::Description gid;
    std::wstring className, name;
    ImageWindow *window;
    ATOM windowClass;
    HWND hwnd;

    JustEdit::Selection *selection;
    JustEdit::Entity *layer;
    Affine2D camera;

    void updateRoot()
    {
        auto find = []( JustEdit::Entity * root )
        {
            return dynamic_cast<JustEdit::Selection*>( ( *root )( []( JustEdit::Entity * s ) -> bool
            {
                return !dynamic_cast<JustEdit::Selection*>( s );
            } ) );
        };

        layer = window->rootObject.get();
        if( layer )
        {
            if( selection )
            {
                auto sel = selection->extract();
                selection = nullptr;
                ( void )sel;
            }

            selection = find( layer );
            makeException( !selection );

            layer->add( std::make_shared<JustEdit::Selection>() );
            selection = find( layer );
            makeException( selection );

            selection->select( nullptr, false );

            if( layer->getNodes().empty() )
                layer->add( std::make_shared<JustEdit::Text>( L"hint", L"Press right mouse button to begin.", JustEdit::Position( Vector2D( 32, 32 ) ) ) );
        }
    }

    Implementation( WNDPROC windowProc, ImageWindow *w )
        : gid( 24, 16, 24, 8, 1 ), window( w )
    {
        static long long unsigned index = 0;

        selection = nullptr;
        updateRoot();

        className = L"ImageWindowWinApiImplementation" + std::to_wstring( index++ );
        name = layer ? L"JustEdit" : L"Canvas";

        WNDCLASSEXW wc;
        clear( &wc, sizeof( wc ) );
        wc.cbSize = sizeof( wc );
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = windowProc;
        wc.hInstance = GetModuleHandleW( nullptr );
        wc.hIcon = LoadIcon( wc.hInstance, MAKEINTRESOURCE( IDI_WINDOW_ICON ) );
        wc.hCursor = LoadCursorW( nullptr, IDC_ARROW );
        wc.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
        wc.lpszClassName = className.c_str();

        windowClass = RegisterClassExW( &wc );
        makeException( windowClass );

        hwnd = nullptr;

        if( !window->defaultSettings )
        {
            std::vector<ImageData> samples;
            ImageData::readICO( L"image.ico", samples );

            auto isEmpty = []( int i, const auto & img )
            {
                int j = 0;
                bool empty = true;
                while( empty && j < img.w() )
                {
                    empty = img( j, i )->a == 0;
                    ++j;
                }
                return empty;
            };

            int maxHeight = 0;
            size_t idMax = 0, id = 0;
            int lowerMax = 0, upperMax = 0;
            for( const auto &sample : samples )
            {
                int realHeight = sample.h();

                int upper = 0;
                bool empty = true;
                while( empty && upper < sample.h() )
                {
                    if( ( empty = isEmpty( upper, sample ) ) )
                        ++upper;
                }

                realHeight -= upper;

                int lower = 0;
                empty = true;
                while( empty && lower < sample.h() )
                {
                    if( ( empty = isEmpty( sample.h() - lower - 1, sample ) ) )
                        ++lower;
                }

                realHeight -= lower;

                if( realHeight < gid.titlebarHeight && realHeight > maxHeight )
                {
                    maxHeight = realHeight;
                    lowerMax = lower;
                    upperMax = upper;
                    idMax = id;
                }

                ++id;
            }

            if( maxHeight > 0 )
            {
                ImageData icon;
                samples[idMax].sub( icon, 0, upperMax, samples[idMax].w(), samples[idMax].h() - lowerMax );
                gid.icon.prepare( icon( 0, 0 ), icon.s(), icon.h() );
            }

            int width, height;
            ImageData text;
            TextGraphics font;
            font.set( L"text", name );
            font.set( L"x", L"0" );
            font.set( L"y", L"0" );
            font.set( L"text.red", L"0" );
            font.set( L"text.green", L"0" );
            font.set( L"text.blue", L"0" );
            font.set( L"height", std::to_wstring( gid.buttonSize ) );
            font.measure( width, height );
            text.reset( width, height, Pixel( 255, 255, 255 ) );
            text.text( font );
            gid.text.prepare( text( 0, 0 ), text.s(), text.h() );
        }
        else
        {
            gid = *window->defaultSettings;
            gid.window.x = 0;
            gid.window.y = 0;
            gid.update();
        }
    }

    ~Implementation()
    {
        gid.window.x = *window->data.x;
        gid.window.y = *window->data.y;
        gid.update();

        if( !window->defaultSettings )
        {
            window->defaultSettings = std::make_unique<GraphicInterface::Description>( gid );
        }
        else
        {
            *window->defaultSettings = gid;
        }

        if( windowClass )
            UnregisterClassW( className.c_str(), GetModuleHandleW( nullptr ) );

        if( selection )
        {
            auto sel = selection->extract();
            selection = nullptr;
            ( void )sel;
        }
    }
};

std::unique_ptr<GraphicInterface::Description> ImageWindow::defaultSettings;

ImageWindow::ImageWindow( ImageDataBase &image_, HandleMsg handleMsg_, std::shared_ptr<JustEdit::Entity> object, Data initData )
    : outputData( image_ ), rootObject( std::move( object ) ), handleMsg( handleMsg_ ), image( image_ ), data( initData )
{
    implementation = nullptr;
    if( image.empty() )
        return;

    // This code is positioned in lambda to accesses private members of ImageWindow
    auto windowProc = []( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam ) -> LRESULT
    {
        auto impl = ( Implementation * )GetWindowLongPtr( hwnd, GWLP_USERDATA );

        auto inputReset = [&impl]()
        {
            if( impl )
                impl->window->inputReset();
        };

        auto inputRelease = [&impl]()
        {
            if( impl )
                impl->window->inputRelease();
        };

        auto quit = [hwnd]()
        {
            DestroyWindow( hwnd );
        };

        auto getPos = [hwnd]( int &x, int &y )
        {
            POINT point;
            GetCursorPos( &point );

            RECT rect;
            GetWindowRect( hwnd, &rect );

            x = point.x - rect.left;
            y = point.y - rect.top;
        };

        auto justEditMsg = [&impl]( const InputData & input, OutputData & output )
        {
            if( !impl )
                return false;

            auto& root = impl->layer;

            auto getFreeName = [&impl]( const std::wstring & prefix )
            {
                unsigned freeId = 0, length = prefix.length() + 1;
                std::wstring freeIdString = L"0";

                ( *impl->window->rootObject )( [&]( JustEdit::Entity * s )
                {
                    auto& name = s->name;

                    if( name.length() == length && name.substr( 0, prefix.length() ) == prefix )
                    {
                        if( s->name.substr( 0, prefix.length() ) == freeIdString )
                        {
                            ++freeId;
                            freeIdString = std::to_wstring( freeId );
                            length = prefix.length() + freeIdString.length();
                        }
                    }

                    return true;
                } );

                return prefix + freeIdString;
            };

            auto update = [&]()
            {
                auto& img = output.image.get();
                img.reset( img.w(), img.h() );

                Overlap::Canvas canavs( img );
                root->draw( impl->camera, canavs );
                canavs.render( img );
            };

            auto fitCamera = [&]()
            {
                Vector2D topLeft, bottomRight;
                if( root->size( Affine2D( Vector2D() ), topLeft, bottomRight ) )
                {
                    int w = output.image.get().w();
                    int h = output.image.get().h();
                    double imageW = Round( 0.65 * w );
                    double imageH = Round( 0.65 * h );

                    auto sx = imageW / ( bottomRight.x - topLeft.x );
                    auto sy = imageH / ( bottomRight.y - topLeft.y );

                    if( sx > 1 )
                        sx = RoundDown( sx );

                    if( sy > 1 )
                        sy = RoundDown( sy );

                    auto scale = Min( sx, sy );

                    imageW = scale * ( bottomRight.x - topLeft.x );
                    imageH = scale * ( bottomRight.y - topLeft.y );

                    impl->camera = Affine2D( Vector2D( w * 0.5, h * 0.5 ) ) * Affine2D( Matrix2D::Scale( scale ) ) * Affine2D( -( topLeft + bottomRight ) * 0.5 );

                    update();
                }
            };

            auto select = [&]( JustEdit::Entity * target, bool add = false )
            {
                impl->selection->select( target, add );
            };

            auto edit = [&]( JustEdit::Entity * target )
            {
                Settings::Parameters parameters;

                auto list = target->editData();
                for( auto& [name, set, get] : list )
                    parameters.emplace_back( name, set, get );

                Settings test( target->description(), parameters );
                test.run();
                update();
            };

            auto create = [&]( const Vector2D & p, int itemId )
            {
                using namespace JustEdit;

                if( itemId == 0 )
                {
                    select( root->add( std::make_shared<JustEdit::Raster>( getFreeName( L"raster" ), 64, 64, Position( p ) ) ) );
                }
                else if( itemId == 1 )
                {
                    select( root->add( std::make_shared<JustEdit::Line>( getFreeName( L"line" ), p, p + Vector2D( 32, 32 ) ) ) );
                }
                else if( itemId == 2 )
                {
                    select( root->add( std::make_shared<JustEdit::Rectangle>( getFreeName( L"rectangle" ), 32, 16, Position( p ) ) ) );
                }
                else if( itemId == 3 )
                {
                    select( root->add( std::make_shared<JustEdit::Circle>( getFreeName( L"circle" ), p, 24 ) ) );
                }
                else if( itemId == 4 )
                {
                    select( root->add( std::make_shared<JustEdit::Text>( getFreeName( L"text" ), L"Lorem ipsum", Position( p ) ) ) );
                }
                else if( itemId == 5 )
                {
                    select( root->add( std::make_shared<JustEdit::Polygon>( getFreeName( L"polygon" ), Position( p ) ) ) );
                }
                else if( itemId == 6 )
                {
                    select( root->add( std::make_shared<JustEdit::Point>( getFreeName( L"point" ), 0, p ) ) );
                }

                update();
            };

            static uint16_t toolId = 0;
            static std::optional<Vector2D> initialCanvasGrab;
            auto pickTool = [&]( uint16_t id )
            {
                toolId = id;
            };

            auto modify = [&]( int modificationId )
            {
                Popup( Popup::Type::Info, L"Modification", L"Apply modification #" + std::to_wstring( modificationId ) ).run();
            };

            auto deletef = [&]( const std::vector<JustEdit::Entity*>& targets, bool test )
            {
                if( targets.empty() )
                    return false;

                for( auto target : targets )
                {
                    if( !target )
                        return false;

                    auto rootContainer = root;
                    while( rootContainer )
                    {
                        if( rootContainer == target )
                            return false;
                        rootContainer = rootContainer->getRoot();
                    }
                }

                if( test )
                    return true;

                select( nullptr );

                for( auto target : targets )
                    target->detach();

                update();
                return true;
            };

            auto group = [&]( bool f, bool test )
            {
                if( f )
                {
                    auto targets = impl->selection->getTargets();
                    if( targets.empty() )
                        return false;

                    if( test )
                        return true;

                    select( nullptr );

                    auto nodes = targets[0]->getRoot()->getNodes();
                    auto g = std::make_shared<JustEdit::Group>( L"group" );
                    for( auto i = nodes.rbegin(); i != nodes.rend(); ++i )
                    {
                        auto target = *i;
                        if( std::find( targets.begin(), targets.end(), target ) != targets.end() )
                            g->add( target->detach() );
                    }

                    root->add( g );
                    update();
                    return true;
                }

                auto g = impl->selection->getTarget();
                if( !g || g->type() != L"Group" )
                    return false;

                auto nodes = g->getNodes();
                if( nodes.empty() )
                    return false;

                if( test )
                    return true;

                select( nullptr );

                for( auto i = nodes.rbegin(); i != nodes.rend(); ++i )
                {
                    auto node = *i;
                    node->position( g->position() * node->position() );
                    root->add( node->detach() );
                }

                g->detach();
                update();
                return true;
            };

            auto open = [&]()
            {
                impl->window->rootObject = JustEdit::Entity::load();
                impl->updateRoot();
                update();
            };

            auto save = [&]()
            {
                auto mainRoot = root;
                auto next = mainRoot->getRoot();
                while( next )
                {
                    mainRoot = next;
                    next = next->getRoot();
                }
                mainRoot->save();
            };

            auto import = [&]( const Vector2D & p )
            {
                auto raster = std::make_shared<JustEdit::Raster>( getFreeName( L"import" ), 64, 64, JustEdit::Position( p ) );

                raster->image->input();
                raster->w = raster->image->w();
                raster->h = raster->image->h();

                raster->position.scaleX = 64.0 / raster->w;
                raster->position.scaleY = 64.0 / raster->h;

                root->add( raster );
                select( raster.get() );
                update();
            };

            auto exportf = [&]()
            {
                select( nullptr );

                Vector2D topLeft, bottomRight;
                if( root->size( Affine2D( Vector2D() ), topLeft, bottomRight ) )
                {
                    int w = RoundUp( bottomRight.x - topLeft.x );
                    int h = RoundUp( bottomRight.y - topLeft.y );

                    ImageData image( w, h );

                    Overlap::Canvas canavs( image );
                    root->draw( Affine2D( -topLeft ), canavs );
                    canavs.render( image );

                    image.output();
                }
            };

            auto undo = [&]( bool f )
            {
                Popup( Popup::Type::Info, L"Change buffer", f ? L"Undone is not implemented" : L"Redone is not implemented" ).run();
            };

            auto view = [&]( JustEdit::Entity * target, bool test )
            {
                if( target )
                {
                    Vector2D a, b;
                    if( target && target->size( Affine2D( Vector2D() ), a, b ) )
                    {
                        if( test )
                            return true;
                        select( nullptr );
                        root = target;
                        target->establishVirtualStructure();
                        fitCamera();
                        return true;
                    }

                    return false;
                }

                auto newRoot = root->getRoot();
                if( newRoot )
                {
                    if( test )
                        return true;
                    select( newRoot );
                    root->dumpVirtualStructure();
                    root = newRoot;
                    fitCamera();
                    return true;
                }

                return false;
            };

            auto help = [&]()
            {
                Popup( Popup::Type::Info, L"Help", L"Some information..." ).run();
            };

            auto exit = [&]()
            {
                output.quit = true;
            };

            auto keyDown = [&]( char symbol )
            {
                auto &key = input.keys.letter( symbol );
                return key.changed() && *key;
            };

            if( input.init )
            {
                int w = RoundUp( GetSystemMetrics( SM_CXSCREEN ) * 0.65 );
                int h = RoundUp( GetSystemMetrics( SM_CYSCREEN ) * 0.65 );

                if( w % 2 == 0 )
                    --w;

                if( h % 2 == 0 )
                    --h;

                output.image.get().reset( w, h );

                fitCamera();
            }

            if( *input.ctrl && keyDown( 'G' ) )
                group( true, false );

            if( *input.ctrl && keyDown( 'U' ) )
                group( false, false );

            if( *input.ctrl && keyDown( 'O' ) )
                open();

            if( *input.ctrl && keyDown( 'S' ) )
                save();

            if( *input.ctrl && *input.up && input.up.changed() )
            {
                if( auto target = impl->selection->getTarget() )
                    view( target, false );
            }

            if( *input.ctrl && *input.down && input.down.changed() )
            {
                view( nullptr, false );
            }

            if( *input.del && input.del.changed() )
            {
                deletef( impl->selection->getTargets(), false );
            }

            static uint16_t forcedOutTool = 0;
            if( input.space.changed() && *input.space )
            {
                forcedOutTool = toolId;
                toolId = 2;
            }

            if( input.space.changed() && !*input.space )
                toolId = forcedOutTool;

            bool lmb = input.leftMouse.changed() && *input.leftMouse;
            bool rmb = input.rightMouse.changed() && *input.rightMouse;

            if( lmb || rmb )
            {
                Vector2D point( *input.mouseX, *input.mouseY );
                auto target = root->pointsTo( impl->camera, point, JustEdit::SelectionMode::Part );

                point = impl->camera.inv()( point );

                bool isSelection = target == impl->selection;

                if( lmb )
                {
                    if( toolId == 0 )
                    {
                        if( isSelection )
                        {
                            impl->selection->grab( point );
                        }
                        else
                        {
                            select( target, *input.ctrl );
                            update();
                        }
                    }
                    else if( toolId == 1 && dynamic_cast<JustEdit::Raster*>( root ) )
                    {
                    }
                    else if( toolId == 2 )
                    {
                        initialCanvasGrab = impl->camera.s - Vector2D( *input.mouseX, *input.mouseY );
                    }
                    else if( toolId == 3 )
                    {
                    }
                }

                if( rmb )
                {
                    auto make = []( const auto & f, const auto & arg )
                    {
                        std::function<void()> function = [f, arg]()
                        {
                            f( arg );
                        };
                        return function;
                    };

                    auto make2 = []( const auto & f, const auto & arg0, const auto & arg1 )
                    {
                        std::function<void()> function = [f, arg0, arg1]()
                        {
                            f( arg0, arg1 );
                        };
                        return function;
                    };

                    ContextMenu menu(
                    {
                        {L"Edit", !isSelection && target, make( edit, target )},
                        {},
                        {
                            L"Tools", true, {},
                            {
                                {L"Select", true, make( pickTool, 0 )},
                                {L"Pixel drawing", true, make( pickTool, 1 )},
                                {L"Move canvas (Hold Space)", true, make( pickTool, 2 )},
                                {L"Zoom (Mouse wheel up/Mouse wheel down/Ctrl+(+)/Ctrl+(-))", true, make( pickTool, 3 )},
                            }
                        },
                        {},
                        {
                            L"Create", true, {},
                            {
                                {L"Raster", true, make2( create, point, 0 )},
                                {L"Line", true, make2( create, point, 1 )},
                                {L"Rectangle", true, make2( create, point, 2 )},
                                {L"Circle", true, make2( create, point, 3 )},
                                {L"Text", true, make2( create, point, 4 )},
                                {L"Polygon", true, make2( create, point, 5 )},
                                {L"Point", dynamic_cast<JustEdit::Polygon*>( root ) != nullptr, make2( create, point, 6 )}
                            }
                        },
                        {
                            L"Modify", true, {},
                            {
                                {L"Do something", true, make( modify, 0 )},
                                {L"Do something else", true, make( modify, 1 )}
                            }
                        },
                        { L"Delete", !isSelection && deletef( { target }, true ), make2( deletef, std::vector<JustEdit::Entity*>{ target }, false ) },
                        {},
                        { L"Group (ctrl+G)", group( true, true ), make2( group, true, false ) },
                        { L"Ungroup (ctrl+U)", group( false, true ), make2( group, false, false ) },
                        {},
                        { L"Copy (ctrl+C)" },
                        { L"Cut (ctrl+X)" },
                        { L"Paste (ctrl+V)" },
                        { L"Place (ctrl+alt+V)" },
                        {},
                        {L"Open (ctrl+O)", true, open},
                        {L"Save (ctrl+S)", true, save},
                        {},
                        {L"Import", true, make( import, point )},
                        {L"Export", true, exportf},
                        {},
                        {L"Undo last change (ctrl+Z)", true, make( undo, true )},
                        {L"Redo last change (ctrl+Y)", true, make( undo, false )},
                        {},
                        {L"View structure (ctrl+↑)", target && view( target, true ), make2( view, target, false )},
                        {L"Return (ctrl+↓)", view( nullptr, true ), make2( view, nullptr, false )},
                        {},
                        {L"Help", true, help},
                        {},
                        {L"Exit", true, exit}
                    } );
                    menu.run();
                }
            }

            if( input.mouseX.changed() || input.mouseY.changed() )
            {
                if( toolId == 0 )
                {
                    if( impl->selection->move( impl->camera.inv()( Vector2D( *input.mouseX, *input.mouseY ) ) ) )
                        update();
                }
                else if( toolId == 2 && initialCanvasGrab )
                {
                    impl->camera.s = *initialCanvasGrab + Vector2D( *input.mouseX, *input.mouseY );
                    update();
                }
            }

            if( input.leftMouse.changed() && !*input.leftMouse )
            {
                impl->selection->release();
                initialCanvasGrab.reset();
            }

            return false;
        };

        auto handle = [&]()
        {
            if( !impl || !impl->window )
                return;

            try
            {
                if( impl->window->handleMsg )
                {
                    impl->window->handleMsg( impl->window->inputData, impl->window->outputData );
                }
                else if( impl->window->rootObject.get() )
                {
                    justEditMsg( impl->window->inputData, impl->window->outputData );
                }
            }
            catch( ... )
            {
                quit();
                return;
            }

            inputReset();

            auto &img = impl->window->outputData.image;
            if( img.changed() )
            {
                int x = 0, y = 0;
                impl->gid.content.prepare( ( *img )( 0, 0 ), img->s(), img->h() );
                updateWindowContent( impl->gid, hwnd, x, y );
                img.reset();
            }

            auto &popup = impl->window->outputData.popup;
            if( popup.changed() )
            {
                popup.get().run();
                popup.reset();
            }

            if( impl->window->outputData.quit )
            {
                quit();
            }

            impl->window->inputData.init = false;
        };

        switch( message )
        {
        case WM_CREATE:
            impl = ( Implementation * )( ( LPCREATESTRUCT )lParam )->lpCreateParams;
            SetWindowLongPtr( hwnd, GWLP_USERDATA, ( LONG_PTR )impl );
            impl->window->inputData.init = true;
            handle();
            break;
        case WM_DESTROY:
            impl->hwnd = nullptr;
            impl = nullptr;
            SetWindowLongPtr( hwnd, GWLP_USERDATA, ( LONG_PTR )impl );
            // PostQuitMessage( 0 );
            return 0;
        case WM_APP:
            break;
        case WM_NCHITTEST:
            {
                RECT rect;
                GetWindowRect( hwnd, &rect );
                int x = LOWORD( lParam ) - rect.left;
                int y = HIWORD( lParam ) - rect.top;

                bool left   = impl->gid.leftTrigger.inside( x, y );
                bool right  = impl->gid.rightTrigger.inside( x, y );
                bool top    = impl->gid.topTrigger.inside( x, y );
                bool bottom = impl->gid.bottomTrigger.inside( x, y );

                if( top && left ) return HTTOPLEFT;
                if( top && right ) return HTTOPRIGHT;
                if( bottom && left ) return HTBOTTOMLEFT;
                if( bottom && right ) return HTBOTTOMRIGHT;

                if( left ) return HTLEFT;
                if( right ) return HTRIGHT;
                if( top ) return HTTOP;
                if( bottom ) return HTBOTTOM;

                if( impl->gid.closeButton.inside( x, y ) || impl->gid.minimizeButton.inside( x, y ) || impl->gid.maximizeButton.inside( x, y ) )
                    return HTCLIENT;

                if( impl->gid.title.inside( x, y ) )
                    return HTCAPTION;

                return HTCLIENT;
            }
        case WM_GETMINMAXINFO:
            {
                if( impl )
                {
                    MINMAXINFO *p = ( MINMAXINFO * )lParam;
                    p->ptMinTrackSize.x = impl->gid.getMinX();
                    p->ptMinTrackSize.y = impl->gid.getMinY();
                    return 0;
                }
            }
            break;
        case WM_KILLFOCUS:
            inputRelease();
            inputReset();
            break;
        case WM_SETFOCUS:
            inputRelease();
            inputReset();
            break;
        case WM_SETCURSOR:
            // Set the cursor to the default arrow
            SetCursor( LoadCursorW( nullptr, IDC_ARROW ) );
            break;
        case WM_SIZE:
            {
                updateWindowContent( impl->gid, hwnd, 0, 0 );
                break;
            }
        case WM_MOVE:
            {
                RECT rect;
                GetWindowRect( hwnd, &rect );
                impl->window->data.x = rect.left;
                impl->window->data.y = rect.top;
                break;
            }
        case WM_PAINT:
            break;
        case WM_NCMOUSEMOVE:
        case WM_MOUSEMOVE:
            {
                int x, y;
                getPos( x, y );
                updateWindowContent( impl->gid, hwnd, x, y );
                if( impl->gid.content.inside( x, y ) )
                {
                    impl->window->inputData.mouseX = x - impl->gid.content.x;
                    impl->window->inputData.mouseY = y - impl->gid.content.y;
                    handle();
                }
                return 0;
            }
        case WM_LBUTTONDOWN:
            {
                int x, y;
                getPos( x, y );
                if( impl->gid.content.inside( x, y ) )
                {
                    impl->window->inputData.leftMouse = true;
                    handle();
                }
            }
            return 0;
        case WM_LBUTTONUP:
            {
                int x, y;
                getPos( x, y );

                if( impl->gid.closeButton.inside( x, y ) )
                {
                    quit();
                    return 0;
                }

                if( impl->gid.maximizeButton.inside( x, y ) )
                {
                    if( IsZoomed( hwnd ) )
                        ShowWindow( hwnd, SW_RESTORE );
                    else
                        ShowWindow( hwnd, SW_MAXIMIZE );
                    return 0;
                }

                if( impl->gid.minimizeButton.inside( x, y ) )
                {
                    ShowWindow( hwnd, SW_MINIMIZE );
                    return 0;
                }

                if( impl->gid.content.inside( x, y ) )
                {
                    impl->window->inputData.leftMouse = false;
                    handle();
                }
                return 0;
            }
        case WM_RBUTTONDOWN:
            {
                int x, y;
                getPos( x, y );
                if( impl->gid.content.inside( x, y ) )
                {
                    impl->window->inputData.rightMouse = true;
                    handle();
                }
            }
            return 0;
        case WM_RBUTTONUP:
            {
                int x, y;
                getPos( x, y );
                if( impl->gid.content.inside( x, y ) )
                {
                    impl->window->inputData.rightMouse = false;
                    handle();
                }
            }
            return 0;
        case WM_MBUTTONDOWN:
            {
                int x, y;
                getPos( x, y );
                if( impl->gid.content.inside( x, y ) )
                {
                    impl->window->inputData.middleMouse = true;
                    handle();
                }
            }
            return 0;
        case WM_MBUTTONUP:
            {
                int x, y;
                getPos( x, y );
                if( impl->gid.content.inside( x, y ) )
                {
                    impl->window->inputData.middleMouse = false;
                    handle();
                }
            }
            return 0;
        default:
            break;
        }

        bool pressed = message == WM_KEYDOWN;
        bool pressedSystem = message == WM_SYSKEYDOWN;
        pressed = pressed || pressedSystem;

        bool released = message == WM_KEYUP;
        bool releasedSystem = message == WM_SYSKEYUP;
        released = released || releasedSystem;

        bool system = pressedSystem || releasedSystem;

        if( pressed || released )
        {
            auto &input = impl->window->inputData;
            switch( wParam )
            {
            case VK_UP:
                if( !system )
                {
                    input.up = pressed;
                    handle();
                    return 0;
                }
                break;
            case VK_DOWN:
                if( !system )
                {
                    input.down = pressed;
                    handle();
                    return 0;
                }
                break;
            case VK_LEFT:
                if( !system )
                {
                    input.left = pressed;
                    handle();
                    return 0;
                }
                break;
            case VK_RIGHT:
                if( !system )
                {
                    input.right = pressed;
                    handle();
                    return 0;
                }
                break;
            case VK_ESCAPE:
                if( !system )
                {
                    if( *( input.escape = pressed ) )
                        quit();
                    return 0;
                }
                break;
            case VK_DELETE:
                if( !system )
                {
                    input.del = pressed;
                    handle();
                    return 0;
                }
                break;
            case VK_CONTROL:
                if( !system )
                {
                    input.ctrl = pressed;
                    handle();
                    return 0;
                }
                break;
            case VK_SHIFT:
                if( !system )
                {
                    input.shift = pressed;
                    handle();
                    return 0;
                }
                break;
            case VK_SPACE:
                if( !system )
                {
                    if( *( input.space = pressed ) && !impl->window->rootObject.get() )
                    {
                        impl->window->image.output();
                    }
                    handle();
                    return 0;
                }
                break;
            case VK_RETURN:
                if( !system )
                {
                    input.enter = pressed;
                    handle();
                    return 0;
                }
                break;
            case VK_LWIN:
                return 0;
            case VK_F1:
                if( !system )
                {
                    if( *( input.f1 = pressed ) )
                    {
                        if( impl->window->data.help )
                            impl->window->data.help->run();
                    }
                    handle();
                    return 0;
                }
            default:
                break;
            }

            if( !system && ( 'A' <= wParam && wParam <= 'Z' ) )
            {
                auto &key = impl->window->inputData.keys.letter( wParam );
                key = pressed;
                handle();
                return 0;
            }
        }

        return DefWindowProc( hwnd, message, wParam, lParam );
    };

    implementation = new Implementation( windowProc, this );

    auto &x = data.x;
    auto &y = data.y;

    float shift;
    if( !x || !y )
    {
        if( defaultSettings )
        {
            x = defaultSettings->window.x;
            y = defaultSettings->window.y;
        }
        else
        {
            RECT screenRect;
            GetClientRect( GetDesktopWindow(), &screenRect );

            float scalar = 0.0625f;
            int width = image.w();
            int height = image.h();
            int screenWidth = screenRect.right - screenRect.left;
            int screenHeight = screenRect.bottom - screenRect.top;
            float shiftX = ( screenWidth - width ) * scalar;
            float shiftY = ( screenHeight - height ) * scalar;
            shift = Min( shiftX, shiftY );
        }
    }

    if( !x )
        x = shift;

    if( !y )
        y = shift;
}

ImageWindow::~ImageWindow()
{
    delete implementation;
}

void ImageWindow::run()
{
    if( image.empty() )
    {
        Popup( Popup::Type::Error, L"Problem", L"Can't show empty image." ).run();
        return;
    }

    implementation->gid.content.prepare( image( 0, 0 ), image.s(), image.h() );
    auto hwnd = implementation->hwnd = CreateWindowExW( WS_EX_LAYERED,
                                       implementation->className.c_str(),
                                       implementation->name.c_str(),
                                       WS_POPUP | WS_THICKFRAME | WS_VISIBLE,
                                       *data.x, *data.y, implementation->gid.getMinX(), implementation->gid.getMinY(),
                                       nullptr, nullptr, GetModuleHandleW( nullptr ), implementation );
    makeException( hwnd );

    MSG msg;
    BOOL result;
    while( implementation->hwnd && ( result = GetMessageW( &msg, hwnd, 0, 0 ) ) != 0 )
    {
        makeException( result != -1 );
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }
}

void ImageWindow::inputReset()
{
    inputData.up.reset();
    inputData.down.reset();
    inputData.left.reset();
    inputData.right.reset();
    inputData.escape.reset();
    inputData.del.reset();
    inputData.ctrl.reset();
    inputData.shift.reset();
    inputData.space.reset();
    inputData.enter.reset();
    inputData.leftMouse.reset();
    inputData.rightMouse.reset();
    inputData.middleMouse.reset();
    inputData.mouseX.reset();
    inputData.mouseY.reset();
    inputData.keys.reset();
}

void ImageWindow::inputRelease()
{
    inputData.up = false;
    inputData.down = false;
    inputData.left = false;
    inputData.right = false;
    inputData.escape = false;
    inputData.del = false;
    inputData.ctrl = false;
    inputData.shift = false;
    inputData.space = false;
    inputData.enter = false;
    inputData.leftMouse = false;
    inputData.rightMouse = false;
    inputData.middleMouse = false;
    inputData.keys.release();
}
