#include "Test_X1.h"

#include "Window.h"

#include "../ImageWindow.h"
#include "../ImageData.h"
#include "../Outline.h"

#include "Common.h"

void Test_X1( Context &context )
{
    auto &text = context.output();
    auto s = context.scope( __FUNCTION__ );

    std::filesystem::path path0, path1;
    ImageData in, out;

    text << L"Open output\\test10.png\n";

    auto p = openPath();
    if( !p.has_value() )
        return;

    path0 = *p;
    path1 = path0.parent_path().parent_path();
    path1.append( "output" );
    path1.append( "test11.png" );

    if( in.input( path0.wstring() ) )
    {
        Outline( in, out );
        out.output( path1.wstring() );
    }
}
