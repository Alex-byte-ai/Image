#include "Test_03_clipboard_and_text.h"

#include "Clipboard.h"

#include "../ImageWindow.h"
#include "../ImageData.h"
#include "../GetImage.h"

#include "../Text.h"

void Test_03_clipboard_and_text( Context &context )
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

    if( !pauseFlag )
        return;

    std::wstring string;
    ImageData in;

    if( inputVariableData )
    {
        text << L"Copy some text.\n";
        pause.wait();

        Clipboard::Item clipboard;
        Clipboard::input( clipboard );
        if( auto data = std::get_if<Clipboard::Text>( &clipboard ) )
        {
            string = L"Text from clipboard: \"" + *data + L"\".";
        }
        else
        {
            string = L"There is no text in clipboard.\n";
        }
    }
    else
    {
        string = L"Some text.";
    }

    in.reset( 256, 256, Pixel( 100, 100, 255 ) );

    TextGraphics textImage;
    textImage.set( L"text", string );
    textImage.set( L"x", L"32" );
    textImage.set( L"y", L"32" );
    textImage.set( L"text.red", L"255" );
    textImage.set( L"text.green", L"255" );
    textImage.set( L"text.blue", L"0" );
    in.text( textImage );

    if( showImages )
    {
        ImageWindow window( in, nullptr );
        window.run();
    }

    if( ( outputVariableData || !inputVariableData ) && writeDisk )
        in.output( context.Output() / L"SomeText.png" );

    if( inputVariableData && showImages )
    {
        text << L"Copy anything, cycle will continue while you copy images.\n";
        text << L"Window that will open isn't interactable.\nTo close it, select it and press 'esc'\n";

        Clipboard::Item clipboard;

        pause.wait();
        Clipboard::input( clipboard );
        while( auto image = std::get_if<Clipboard::Image>( &clipboard ) )
        {
            ImageConvert::Reference destination;
            makeReference( in, destination );
            translate( *image, destination, false );

            ImageWindow window( in, nullptr );
            window.run();

            pause.wait();
            Clipboard::input( clipboard );
        }
    }
}
