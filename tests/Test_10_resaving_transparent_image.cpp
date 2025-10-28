#include "Test_10_resaving_transparent_image.h"

#include "../ImageData.h"

void Test_10_resaving_transparent_image( Context &context )
{
    auto s = context.scope( __FUNCTION__ );

    auto &info = context.information;
    bool readDisk = info( L"readDisk" ).as<bool>();
    bool writeDisk = info( L"writeDisk" ).as<bool>();

    ImageData image;
    if( readDisk && writeDisk && image.input( L"input\\Png\\cubes.png" ) )
        image.output( context.Output() / L"cubes.bmp" );
}
