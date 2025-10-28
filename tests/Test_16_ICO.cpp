#include "Test_16_ICO.h"

#include <algorithm>

#include "../ImageWindow.h"
#include "../ImageData.h"

void Test_16_ICO( Context &context )
{
    auto &text = context.output();
    auto s = context.scope( __FUNCTION__ );

    auto &info = context.information;
    bool readDisk = info( L"readDisk" ).as<bool>();
    bool writeDisk = info( L"writeDisk" ).as<bool>();
    bool showImages = info( L"showImages" ).as<bool>();

    if( !readDisk || !writeDisk )
        return;

    std::vector<ImageData> images;

    std::ostringstream debug;
    if( !ImageData::readICO( L"input\\icon.ico", images ) )
    {
        text << L"Failed to open icon.ico\n";
        return;
    }

    auto isNotEmpty = []( const auto & object )
    {
        return !object.empty();
    };

    text << L"Showing " << std::count_if( images.begin(), images.end(), isNotEmpty ) << L" image(s)\n";
    for( unsigned i = 0; i < images.size(); ++i )
    {
        if( showImages )
        {
            ImageWindow image( images[i], nullptr );
            image.run();
        }
        images[i].output( context.Output() / ( std::to_wstring( i ) + L".png" ) );
    }

    if( !ImageData::writeICO( context.Output() / L"icon.ico", images ) )
    {
        text << L"Failed to save icon.ico\n";
    }
}
