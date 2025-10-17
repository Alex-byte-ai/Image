#pragma once

#include <functional>
#include <optional>
#include <array>

#include "ChangedValue.h"
#include "Exception.h"
#include "Window.h"

#include "ImageDataBase.h"
#include "JustEdit.h"

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
        ChangedValue<bool> escape, shift, space, enter, leftMouse, rightMouse, middleMouse, f1;
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

    ImageWindow( ImageDataBase &image, HandleMsg handleMsg, JustEdit::Entity* root = nullptr, Data initData = Data() );
    ~ImageWindow();

    void run();
private:
    void inputReset();
    void inputRelease();

    class Implementation;
    Implementation *implementation;

    InputData inputData;
    OutputData outputData;

    JustEdit::Entity* root;
    HandleMsg handleMsg;

    ImageDataBase &image;
    Data data;

    class DefaultSettings;
    static std::unique_ptr<DefaultSettings> defaultSettings;
};
