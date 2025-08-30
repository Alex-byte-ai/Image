#include "Test_01_screenshot_and_clipboard.h"

#include "Clipboard.h"

#include "../ImageWindow.h"
#include "../ImageData.h"
#include "../GetImage.h"

void Test_01_screenshot_and_clipboard( Context &context )
{
    auto &pause = context.pause;
    auto &text = context.output();
    auto s = context.scope( __FUNCTION__ );

    auto &info = context.information;
    bool writeDisk = info( L"writeDisk" ).as<bool>();
    bool pauseFlag = info( L"pause" ).as<bool>();
    bool showImages = info( L"showImages" ).as<bool>();
    bool inputVariableData = info( L"inputVariableData" ).as<bool>();
    bool outputVariableData = info( L"outputVariableData" ).as<bool>();

    if( !inputVariableData || !pauseFlag )
        return;

    ImageData in;

    text << L"Taking a screenshot.\n";
    pause.wait( Pause::InputType::shift );

    if( screenCapture( in ) )
    {
        text << L"Screenshot.\n";
        if( showImages )
        {
            ImageWindow window( in, nullptr );
            window.run();
        }

        if( outputVariableData && writeDisk )
            in.output( context.Output() / L"screenshot.png" );
    }
    else
    {
        text << L"Screenshot failed.\n";
    }

    text << L"Select window.\n";
    pause.wait( Pause::InputType::shift );

    if( windowCapture( in ) )
    {
        text << L"Image of a window.\n";
        if( showImages )
        {
            ImageWindow window( in, nullptr );
            window.run();
        }

        if( outputVariableData && writeDisk )
            in.output( context.Output() / L"window.png" );
    }
    else
    {
        text << L"Can't get an image of a window.\n";
    }

    text << L"Copy something.\n";
    pause.wait( Pause::InputType::shift );

    Clipboard::Item item;
    Clipboard::input( item );
    if( auto image = std::get_if<Clipboard::Image>( &item ) )
    {
        ImageConvert::Reference destination;
        makeReference( in, destination );
        translate( *image, destination, false );

        text << L"Copied image.\n";
        if( showImages )
        {
            ImageWindow window( in, nullptr );
            window.run();
        }

        if( outputVariableData && writeDisk )
            in.output( context.Output() / L"copy.png" );
    }
    else
    {
        text << L"Can't copy an image.\n";
    }
}
