#include "Test_00_save_and_load.h"

#include "../ImageWindow.h"
#include "../ImageData.h"

void Test_00_save_and_load( Context &context )
{
    auto &text = context.output();
    auto s = context.scope( __FUNCTION__ );

    auto &info = context.information;
    bool readDisk = info( L"readDisk" ).as<bool>();
    bool writeDisk = info( L"writeDisk" ).as<bool>();
    bool showImages = info( L"showImages" ).as<bool>();
    bool inputVariableData = info( L"inputVariableData" ).as<bool>();
    bool outputVariableData = info( L"outputVariableData" ).as<bool>();

    if( !readDisk || !writeDisk )
        return;

    const std::vector<std::filesystem::path> inputs =
    {
        L"input\\p24.bmp",
        L"input\\n24.bmp",
        L"input\\p32.bmp",
        L"input\\n32.bmp",
        L"input\\p.png",
    };

    const std::vector<std::filesystem::path> outputs0 =
    {
        context.Output() / L"0_p24.bmp",
        context.Output() / L"0_n24.bmp",
        context.Output() / L"0_p32.bmp",
        context.Output() / L"0_n32.bmp",
        context.Output() / L"0_p.png",
    };

    const std::vector<std::filesystem::path> outputs1 =
    {
        context.Output() / L"1_p24.bmp",
        context.Output() / L"1_n24.bmp",
        context.Output() / L"1_p32.bmp",
        context.Output() / L"1_n32.bmp",
        context.Output() / L"1_p.png",
    };

    ImageData in;

    auto debugInfo = [&]( const std::filesystem::path & inputPath, const std::filesystem::path & outputPath )
    {
        text << inputPath;
        if( in.input( inputPath ) )
        {
            Pixel p;
            in.output( outputPath );
            text << L"\nStride: " << in.s() << "\n";
            p = *in( 0, 0 );
            text << L"GREEN: (" << p.r << L", " << p.g << L", " << p.b << L")\n";
            p = *in( 1, 0 );
            text << L"BLUE: (" << p.r << L", " << p.g << L", " << p.b << L")\n";
            p = *in( 0, 1 );
            text << L"RED: (" << p.r << L", " << p.g << L", " << p.b << L")\n";
            if( showImages )
            {
                ImageWindow window( in, nullptr );
                window.run();
            }
        }
        else
        {
            text << L" can't be oppened.\n";
        }
        text << L"\n";
    };

    for( unsigned i = 0; i < inputs.size(); ++i )
    {
        debugInfo( inputs[i], outputs0[i] );
        debugInfo( outputs0[i], outputs1[i] );
    }

    if( inputVariableData )
    {
        text << L"Open input\\load_this.png\n";
        if( in.input() )
        {
            if( showImages )
            {
                ImageWindow window( in, nullptr );
                window.run();
            }
            if( outputVariableData )
            {
                text << L"Save it as " << ( context.Output() / L"save_this.png" ) << L"\n";
                in.output();
            }
        }
        else
        {
            text << L"Can't open this file.\n";
        }
    }
}
