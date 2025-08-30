#include "Test_X3.h"

#include "GetPathToFile.h"

#include "../ApplyKernel.h"
#include "../ImageWindow.h"
#include "../ImageData.h"

#include "Common.h"

void Test_X3( Context &context )
{
    auto &text = context.output();
    auto s = context.scope( __FUNCTION__ );

    std::filesystem::path path0, path1;
    ImageData in, mask, ou0, out;

    auto p = OpenPath();
    if( !p.has_value() )
        return;

    path0 = *p;
    path1 = path0.parent_path().parent_path();
    path1.append( "output" );
    path1.append( "test13.png" );

    in.input( path0.wstring() );
    ApplyKernel( in, ou0 );

    getCores().image = &ou0;
    ImageWindow window( ou0, nullptr );
    window.run();

    if( !LimitPalette( ou0, out, Sqrt( 3.0 ) / 10 ) )
    {
        text << L"Wasn't able to finish test13.\n";
        return;
    }

    out.output( path1.wstring() );

    system( "pause" );

    mask.input( path1.wstring() );

    if( !ReplacePinkWithTransparent( in, mask, out ) )
    {
        text << L"Wasn't able to finish test13.\n";
        return;
    }

    out.output( path1.wstring() );
}
