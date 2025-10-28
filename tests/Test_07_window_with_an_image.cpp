#include "Test_07_window_with_an_image.h"

#include "../ImageWindow.h"
#include "../ImageData.h"

void Test_07_window_with_an_image( Context &context )
{
    auto s = context.scope( __FUNCTION__ );

    auto &info = context.information;
    bool readDisk = info( L"readDisk" ).as<bool>();
    bool writeDisk = info( L"writeDisk" ).as<bool>();
    bool showImages = info( L"showImages" ).as<bool>();

    ImageData in;
    if( readDisk && in.input( L"input\\image.png" ) )
    {
        if( showImages )
        {
            ImageWindow window( in, nullptr );
            window.run();
            window.run(); // Testing, if you can run the same window second time

            ImageData h;
            if( h.input( L"input\\in1.png" ) )
            {
                auto manage = [&]( const ImageWindow::InputData & inputData, ImageWindow::OutputData & )
                {
                    auto keyDown = [&]( char symbol )
                    {
                        auto &key = inputData.keys.letter( symbol );
                        return key.changed() && *key;
                    };

                    if( keyDown( 'A' ) )
                    {
                        window.run();
                    }
                };

                ImageWindow host( h, manage );
                host.run(); // Testing, if you can run window within window another window even manager
            }

            window.run(); // Testing, if it's fine after that operation
        }
        if( writeDisk )
            in.output( context.Output() / L"image.png" );
    }
}
