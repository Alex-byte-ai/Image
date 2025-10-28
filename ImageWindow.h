#pragma once

#include <functional>
#include <optional>
#include <array>

#include "ChangedValue.h"
#include "Exception.h"
#include "Window.h"

#include "ImageDataBase.h"
#include "JustEdit.h"

namespace GraphicInterface
{
struct Box
{
    int x = 0, y = 0, w = 0, h = 0;

    bool inside( int x0, int y0 );
};

struct Image : public Box
{
    std::vector<uint32_t> pixels;
    int bufferW = 0, bufferH = 0;

    void prepare( const void *data, int stride, int height );
};

struct Description
{
    Description( int h, int sz, int bh, int tgw, int b );
    Description( const Description &other ) = default;

    int titlebarHeight, buttonSize, buttonSpacingH, buttonSpacingV, triggerWidth, borderWidth;

    Box window, title, minimizeButton, maximizeButton, closeButton, leftBorder, rightBorder, topBorder, bottomBorder, client;
    Box topTrigger, bottomTrigger, leftTrigger, rightTrigger;
    Image icon, text, content;

    int getMinX() const;
    int getMinY() const;

    void update();
    void draw( uint32_t *pixels, int x, int y );
};

uint32_t makeColor( uint8_t r, uint8_t g, uint8_t b, uint8_t a );
void drawRect( uint32_t *pixels, int width, int height, const Box &b, uint32_t color );
void drawGradient( uint32_t *pixels, int width, int height, const Box &b );
void drawImage( uint32_t *pixels, int width, int height, const Image &b );
void drawLineR( uint32_t *pixels, int width, int, int x, int y, int size, uint32_t color );
void drawLineD( uint32_t *pixels, int width, int, int x, int y, int size, uint32_t color );
void drawLineRD( uint32_t *pixels, int width, int, int x, int y, int size, uint32_t color );
void drawLineRU( uint32_t *pixels, int width, int, int x, int y, int size, uint32_t color );
void cross( uint32_t *pixels, int width, int height, const Box &b, bool selected );
void square( uint32_t *pixels, int width, int height, const Box &b, bool selected );
void line( uint32_t *pixels, int width, int height, const Box &b, bool selected );
}

class ImageWindow
{
public:
    class Keys
    {
    private:
        std::array<ChangedValue<bool>, 26> letters;
    public:
        ChangedValue<bool> &letter( char symbol )
        {
            makeException( 'A' <= symbol && symbol <= 'Z' );
            return letters[symbol - 'A'];
        }
        const ChangedValue<bool> &letter( char symbol ) const
        {
            makeException( 'A' <= symbol && symbol <= 'Z' );
            return letters[symbol - 'A'];
        }
        void reset()
        {
            for( auto &letter : letters )
                letter.reset();
        }
        void release()
        {
            for( auto &letter : letters )
                letter = false;
        }
    };

    class InputData
    {
    public:
        ChangedValue<bool> up, down, left, right, escape, del, shift, ctrl, space, enter, leftMouse, rightMouse, middleMouse, f1;
        ChangedValue<int> mouseX{ -1 }, mouseY{ -1 };
        bool init;
        Keys keys;
    };

    class OutputData
    {
    public:
        ChangedValue<ImageDataBase &> image;
        ChangedValue<int> x{ 0 }, y{ 0 };
        ChangedValue<Popup> popup;
        bool quit = false;

        OutputData( ImageDataBase &image_ ) : image( image_ )
        {}
    };

    using HandleMsg = std::function<void( const InputData &, OutputData & )>;

    class Data
    {
    public:
        // Coordinates of left upper conner of the window
        std::optional<int> x, y;
        std::optional<Popup> help;

        Data( std::optional<Popup> h = {}, std::optional<int> xCoordinate = {}, std::optional<int> yCoordinate  = {} )
            : x( std::move( xCoordinate ) ), y( std::move( yCoordinate ) ), help( std::move( h ) )
        {}
    };

    ImageWindow( ImageDataBase &image, HandleMsg handleMsg, std::shared_ptr<JustEdit::Entity> root = nullptr, Data initData = Data() );
    ~ImageWindow();

    void run();
private:
    void inputReset();
    void inputRelease();

    class Implementation;
    Implementation *implementation;

    InputData inputData;
    OutputData outputData;

    std::shared_ptr<JustEdit::Entity> rootObject;
    HandleMsg handleMsg;

    ImageDataBase &image;
    Data data;

    static std::unique_ptr<GraphicInterface::Description> defaultSettings;
};
