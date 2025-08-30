#include "ImageWindow.h"

#include <windowsx.h>
#include <windows.h>

#include "RandomString.h"
#include "Exception.h"
#include "Lambda.h"
#include "Basic.h"

#include "ImageData.h"

#include "resource.h"

namespace GraphicInterface
{
static uint32_t makeColor( uint8_t r, uint8_t g, uint8_t b, uint8_t a )
{
    return ( a << 24 ) | ( r << 16 ) | ( g << 8 ) | b;
}

struct Box
{
    int x = 0, y = 0, w = 0, h = 0;

    bool inside( int x0, int y0 )
    {
        return x <= x0 && x0 < x + w && y <= y0 && y0 < y + h;
    }
};

struct Image : public Box
{
    std::vector<uint32_t> pixels;
    int bufferW = 0, bufferH = 0;

    void prepare( const void *data, int stride, int height )
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
};

struct Description
{
    Description( int h, int sz, int bh, int tgw, int b )
        : titlebarHeight( h ), buttonSize( sz ), buttonSpacingH( bh ), triggerWidth( tgw ), borderWidth( b )
    {
        buttonSpacingV = ( titlebarHeight - buttonSize ) / 2;
    }

    Description( const Description &other ) = default;

    int titlebarHeight, buttonSize, buttonSpacingH, buttonSpacingV, triggerWidth, borderWidth;

    Box window, title, minimizeButton, maximizeButton, closeButton, leftBorder, rightBorder, topBorder, bottomBorder, client;
    Box topTrigger, bottomTrigger, leftTrigger, rightTrigger;
    Image icon, text, content;

    int getMinX() const
    {
        auto titleBarMinWidth = 3 * buttonSize + buttonSpacingV + 3 * buttonSpacingH + borderWidth + icon.x + icon.w;
        auto minWidth = content.w + 2 * borderWidth;

        if( titleBarMinWidth > minWidth )
            return titleBarMinWidth;

        return minWidth;
    }

    int getMinY() const
    {
        return titlebarHeight + content.h + 2 * borderWidth;
    }

    void update()
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
};

static void drawRect( uint32_t *pixels, int width, int height, const Box &b, uint32_t color )
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

static void drawGradient( uint32_t *pixels, int width, int height, const Box &b )
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

static void drawImage( uint32_t *pixels, int width, int height, const Image &b )
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

static void drawLineR( uint32_t *pixels, int width, int, int x, int y, int size, uint32_t color )
{
    while( size > 0 )
    {
        pixels[y * width + x] = color;
        ++x;
        --size;
    }
}

static void drawLineD( uint32_t *pixels, int width, int, int x, int y, int size, uint32_t color )
{
    while( size > 0 )
    {
        pixels[y * width + x] = color;
        ++y;
        --size;
    }
}

static void drawLineRD( uint32_t *pixels, int width, int, int x, int y, int size, uint32_t color )
{
    while( size > 0 )
    {
        pixels[y * width + x] = color;
        ++x;
        ++y;
        --size;
    }
}

static void drawLineRU( uint32_t *pixels, int width, int, int x, int y, int size, uint32_t color )
{
    while( size > 0 )
    {
        pixels[y * width + x] = color;
        ++x;
        --y;
        --size;
    }
}

static void cross( uint32_t *pixels, int width, int height, const Box &b, bool selected )
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

static void square( uint32_t *pixels, int width, int height, const Box &b, bool selected )
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

static void line( uint32_t *pixels, int width, int height, const Box &b, bool selected )
{
    auto color = selected ? makeColor( 235, 235, 235, 255 ) : makeColor( 255, 255, 255, 255 );
    drawRect( pixels, width, height, b, color );

    auto black = makeColor( 0, 0, 0, 255 );

    drawLineR( pixels, width, height, b.x + 3, b.y + b.h / 2 - 1, b.w - 6, black );
}

static void drawInterface( GraphicInterface::Description &g, uint32_t *pixels, int x, int y )
{
    int width = g.window.w;
    int height = g.window.h;

    // --- Title Bar ---
    auto &tib = g.title;
    auto titleBarColor = makeColor( 255, 255, 255, 255 );
    drawRect( pixels, width, height, tib, titleBarColor );

    // Icon
    drawImage( pixels, g.window.w, g.window.h, g.icon );

    // Title
    drawImage( pixels, g.window.w, g.window.h, g.text );

    // Close button
    auto &clb = g.closeButton;
    cross( pixels, width, height, clb, clb.inside( x, y ) );

    // Maximize button
    auto &mab = g.maximizeButton;
    square( pixels, width, height, mab, mab.inside( x, y ) );

    // Minimize button
    auto &mib = g.minimizeButton;
    line( pixels, width, height, mib, mib.inside( x, y ) );

    // --- Borders ---
    auto borderColor = makeColor( 85, 85, 85, 255 );
    drawRect( pixels, width, height, g.leftBorder, borderColor );
    drawRect( pixels, width, height, g.rightBorder, borderColor );
    drawRect( pixels, width, height, g.topBorder, borderColor );
    drawRect( pixels, width, height, g.bottomBorder, borderColor );
}
}

static void updateWindowContent( GraphicInterface::Description &g, HWND hwnd, int x, int y )
{
    RECT rect;
    GetWindowRect( hwnd, &rect );

    g.window.x = 0;
    g.window.y = 0;
    g.window.w = rect.right - rect.left;
    g.window.h = rect.bottom - rect.top;
    g.update();

    BITMAPINFO bmi;
    clear( &bmi, sizeof( bmi ) );
    bmi.bmiHeader.biSize        = sizeof( BITMAPINFOHEADER );
    bmi.bmiHeader.biWidth       = g.window.w;
    bmi.bmiHeader.biHeight      = -g.window.h;
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
    if( g.content.w > 0 && g.content.h > 0 )
    {
        drawRect( pixels, g.window.w, g.window.h, g.client, GraphicInterface::makeColor( 60, 70, 200, 255 ) );
        drawImage( pixels, g.window.w, g.window.h, g.content );
    }
    else
    {
        drawGradient( pixels, g.window.w, g.window.h, g.client );
    }

    GraphicInterface::drawInterface( g, pixels, x, y );

    BLENDFUNCTION blend;
    clear( &blend, sizeof( blend ) );
    blend.BlendOp             = AC_SRC_OVER;
    blend.SourceConstantAlpha = 255; // Use per-pixel alpha.
    blend.AlphaFormat         = AC_SRC_ALPHA;
    POINT ptZero = {0, 0};
    SIZE sizeWindow = {g.window.w, g.window.h};
    UpdateLayeredWindow( hwnd, hdcScreen, nullptr, &sizeWindow, hdcMem, &ptZero, 0, &blend, ULW_ALPHA );

    SelectObject( hdcMem, hOldBmp );
    DeleteObject( hBitmap );
    DeleteDC( hdcMem );
    ReleaseDC( nullptr, hdcScreen );
}

class ImageWindow::DefaultSettings
{
public:
    GraphicInterface::Description g;

    DefaultSettings( const GraphicInterface::Description &description ) : g( description )
    {}
};

class ImageWindow::WindowData
{
public:
    GraphicInterface::Description g;
    std::wstring className, name;
    ImageWindow *window;
    ATOM windowClass;

    WindowData( WNDPROC windowProc, ImageWindow *w )
        : g( 24, 16, 24, 8, 1 ), window( w )
    {
        static RandomNumber randomNumber;
        RandomWString( randomNumber, L"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", className, 16, true );

        name = L"Canvas";

        WNDCLASSEXW wc;
        memset( &wc, 0, sizeof( wc ) );
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

                if( realHeight < g.titlebarHeight && realHeight > maxHeight )
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
                g.icon.prepare( icon( 0, 0 ), icon.s(), icon.h() );
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
            font.set( L"height", std::to_wstring( g.buttonSize ) );
            font.measure( width, height );
            text.reset( width, height, Pixel( 255, 255, 255 ) );
            text.text( font );
            g.text.prepare( text( 0, 0 ), text.s(), text.h() );
        }
        else
        {
            g = window->defaultSettings->g;
            g.window.x = 0;
            g.window.y = 0;
            g.update();
        }
    }

    ~WindowData()
    {
        g.window.x = *window->data.x;
        g.window.y = *window->data.y;
        g.update();

        if( !window->defaultSettings )
        {
            window->defaultSettings = std::make_unique<ImageWindow::DefaultSettings>( g );
        }
        else
        {
            window->defaultSettings->g = g;
        }

        if( windowClass )
            UnregisterClassW( className.c_str(), GetModuleHandleW( nullptr ) );
    }
};

std::unique_ptr<ImageWindow::DefaultSettings> ImageWindow::defaultSettings;

ImageWindow::ImageWindow( ImageDataBase &image_, HandleMsg handleMsg_, Data initData )
    : outputData( image_ ), handleMsg( handleMsg_ ), image( image_ ), data( initData )
{
    windowData = nullptr;
    if( image.empty() )
        return;

    // This code is positioned in lambda to accesses private members of Window
    auto windowProc = []( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam ) -> LRESULT
    {
        auto d = ( WindowData * )GetWindowLongPtr( hwnd, GWLP_USERDATA );

        auto inputReset = [d]()
        {
            if( d )
                d->window->inputReset();
        };

        auto inputRelease = [d]()
        {
            if( d )
                d->window->inputRelease();
        };

        auto quit = [&]()
        {
            DestroyWindow( hwnd );
        };

        auto getPos = [&]( int &x, int &y )
        {
            POINT point;
            GetCursorPos( &point );

            RECT rect;
            GetWindowRect( hwnd, &rect );

            x = point.x - rect.left;
            y = point.y - rect.top;
        };

        auto handle = [&]()
        {
            if( !d || !d->window )
                return;

            try
            {
                if( d->window->handleMsg )
                    d->window->handleMsg( d->window->inputData, d->window->outputData );
            }
            catch( ... )
            {
                quit();
                return;
            }

            inputReset();

            auto &img = d->window->outputData.image;
            if( img.changed() )
            {
                int x = 0, y = 0;
                d->g.content.prepare( ( *img )( 0, 0 ), img->s(), img->h() );
                updateWindowContent( d->g, hwnd, x, y );
                img.reset();
            }

            auto &popup = d->window->outputData.popup;
            if( popup.changed() )
            {
                popup.get().run();
                popup.reset();
            }

            if( d->window->outputData.quit )
            {
                quit();
            }
        };

        switch( message )
        {
        case WM_NCCREATE:
        case WM_CREATE:
            SetWindowLongPtr( hwnd, GWLP_USERDATA, ( LONG_PTR )( ( LPCREATESTRUCT )lParam )->lpCreateParams );
            break;
        case WM_DESTROY:
            SetWindowLongPtr( hwnd, GWLP_USERDATA, 0 );
            PostQuitMessage( 0 );
            return 0;
        case WM_APP:
            break;
        case WM_NCHITTEST:
            {
                RECT rect;
                GetWindowRect( hwnd, &rect );
                int x = LOWORD( lParam ) - rect.left;
                int y = HIWORD( lParam ) - rect.top;

                bool left   = d->g.leftTrigger.inside( x, y );
                bool right  = d->g.rightTrigger.inside( x, y );
                bool top    = d->g.topTrigger.inside( x, y );
                bool bottom = d->g.bottomTrigger.inside( x, y );

                if( top && left ) return HTTOPLEFT;
                if( top && right ) return HTTOPRIGHT;
                if( bottom && left ) return HTBOTTOMLEFT;
                if( bottom && right ) return HTBOTTOMRIGHT;

                if( left ) return HTLEFT;
                if( right ) return HTRIGHT;
                if( top ) return HTTOP;
                if( bottom ) return HTBOTTOM;

                if( d->g.closeButton.inside( x, y ) || d->g.minimizeButton.inside( x, y ) || d->g.maximizeButton.inside( x, y ) )
                    return HTCLIENT;

                if( d->g.title.inside( x, y ) )
                    return HTCAPTION;

                return HTCLIENT;
            }
        case WM_GETMINMAXINFO:
            {
                if( d )
                {
                    MINMAXINFO *p = ( MINMAXINFO * )lParam;
                    p->ptMinTrackSize.x = d->g.getMinX();
                    p->ptMinTrackSize.y = d->g.getMinY();
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
                updateWindowContent( d->g, hwnd, 0, 0 );
                break;
            }
        case WM_MOVE:
            {
                RECT rect;
                GetWindowRect( hwnd, &rect );
                d->window->data.x = rect.left;
                d->window->data.y = rect.top;
                break;
            }
        case WM_PAINT:
            break;
        case WM_NCMOUSEMOVE:
        case WM_MOUSEMOVE:
            {
                int x, y;
                getPos( x, y );
                updateWindowContent( d->g, hwnd, x, y );
                if( d->g.content.inside( x, y ) )
                {
                    d->window->inputData.mouseX = x - d->g.content.x;
                    d->window->inputData.mouseY = y - d->g.content.y;
                    handle();
                }
                return 0;
            }
        case WM_LBUTTONDOWN:
            {
                int x, y;
                getPos( x, y );
                if( d->g.content.inside( x, y ) )
                {
                    d->window->inputData.leftMouse = true;
                    handle();
                }
            }
            return 0;
        case WM_LBUTTONUP:
            {
                int x, y;
                getPos( x, y );

                if( d->g.closeButton.inside( x, y ) )
                {
                    quit();
                    return 0;
                }

                if( d->g.maximizeButton.inside( x, y ) )
                {
                    if( IsZoomed( hwnd ) )
                        ShowWindow( hwnd, SW_RESTORE );
                    else
                        ShowWindow( hwnd, SW_MAXIMIZE );
                    return 0;
                }

                if( d->g.minimizeButton.inside( x, y ) )
                {
                    ShowWindow( hwnd, SW_MINIMIZE );
                    return 0;
                }

                if( d->g.content.inside( x, y ) )
                {
                    d->window->inputData.leftMouse = false;
                    handle();
                }
                return 0;
            }
        case WM_RBUTTONDOWN:
            {
                int x, y;
                getPos( x, y );
                if( d->g.content.inside( x, y ) )
                {
                    d->window->inputData.rightMouse = true;
                    handle();
                }
            }
            return 0;
        case WM_RBUTTONUP:
            {
                int x, y;
                getPos( x, y );
                if( d->g.content.inside( x, y ) )
                {
                    d->window->inputData.rightMouse = false;
                    handle();
                }
            }
            return 0;
        case WM_MBUTTONDOWN:
            {
                int x, y;
                getPos( x, y );
                if( d->g.content.inside( x, y ) )
                {
                    d->window->inputData.middleMouse = true;
                    handle();
                }
            }
            return 0;
        case WM_MBUTTONUP:
            {
                int x, y;
                getPos( x, y );
                if( d->g.content.inside( x, y ) )
                {
                    d->window->inputData.middleMouse = false;
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
            auto &input = d->window->inputData;
            switch( wParam )
            {
            case VK_ESCAPE:
                if( !system )
                {
                    if( *( input.escape = pressed ) )
                        quit();
                    return 0;
                }
                break;
            case VK_SHIFT:
                if( !system )
                {
                    if( *( input.shift = pressed ) )
                    {
                        POINT point;
                        RECT rect;
                        GetWindowRect( hwnd, &rect );
                        GetCursorPos( &point );

                        d->window->data.x = point.x;
                        d->window->data.y = point.y;
                        SetWindowPos( hwnd, HWND_TOP, point.x, point.y, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER );
                    }
                    handle();
                    return 0;
                }
                break;
            case VK_SPACE:
                if( !system )
                {
                    if( *( input.space = pressed ) )
                    {
                        d->window->image.output();
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
                        if( d->window->data.help )
                            d->window->data.help->run();
                    }
                    handle();
                    return 0;
                }
            default:
                break;
            }

            if( !system && ( 'A' <= wParam && wParam <= 'Z' ) )
            {
                auto &key = d->window->inputData.keys.letter( wParam );
                key = pressed;
                handle();
                return 0;
            }
        }

        return DefWindowProc( hwnd, message, wParam, lParam );
    };

    windowData = new WindowData( windowProc, this );

    auto &x = data.x;
    auto &y = data.y;

    float shift;
    if( !x || !y )
    {
        if( defaultSettings )
        {
            x = defaultSettings->g.window.x;
            y = defaultSettings->g.window.y;
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
    delete windowData;
}

void ImageWindow::run()
{
    if( image.empty() )
    {
        Popup( Popup::Type::Error, L"Problem", L"Can't show empty image." ).run();
        return;
    }

    windowData->g.content.prepare( image( 0, 0 ), image.s(), image.h() );

    auto hwnd = CreateWindowExW( WS_EX_LAYERED,
                                 windowData->className.c_str(),
                                 windowData->name.c_str(),
                                 WS_POPUP | WS_THICKFRAME | WS_VISIBLE,
                                 *data.x, *data.y, windowData->g.getMinX(), windowData->g.getMinY(),
                                 nullptr, nullptr, GetModuleHandleW( nullptr ), windowData );

    makeException( hwnd );

    SetFocus( hwnd );
    SetForegroundWindow( hwnd );

    MSG msg;
    while( GetMessage( &msg, nullptr, 0, 0 ) )
    {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }
}

void ImageWindow::inputReset()
{
    inputData.escape.reset();
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
    inputData.escape = false;
    inputData.shift = false;
    inputData.space = false;
    inputData.enter = false;
    inputData.leftMouse = false;
    inputData.rightMouse = false;
    inputData.middleMouse = false;
    inputData.keys.release();
}
