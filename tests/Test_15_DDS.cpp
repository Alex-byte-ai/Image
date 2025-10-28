#include "Test_15_DDS.h"

#include "../ImageData.h"

void Test_15_DDS( Context &context )
{
    auto &text = context.output();
    auto s = context.scope( __FUNCTION__ );

    auto &info = context.information;
    bool readDisk = info( L"readDisk" ).as<bool>();
    bool writeDisk = info( L"writeDisk" ).as<bool>();

    if( !readDisk || !writeDisk )
        return;

    std::vector<ImageData> img( 3 );

    img[0].input( L"input\\frame0.png" );
    img[1].input( L"input\\frame1.png" );
    img[2].input( L"input\\frame2.png" );
    ImageData::writeDDS( context.Output() / L"animation.dds", img );

    img.clear();

    if( !ImageData::readDDS( context.Output() / L"animation.dds", img ) || img.size() != 3 )
    {
        text << L"Failed to input.\n";
        return;
    }

    text << img.size() << L"\n";
    img[0].output( context.Output() / L"frame0.png" );
    img[1].output( context.Output() / L"frame1.png" );
    img[2].output( context.Output() / L"frame2.png" );
}
