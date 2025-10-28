#include "Test_X2.h"

#include "GetPathToFile.h"

#include "../ApplyKernel.h"
#include "../ImageWindow.h"
#include "../ImageData.h"

#include "Common.h"

void Test_X2( Context &context )
{
    auto s = context.scope( __FUNCTION__ );

    std::filesystem::path path0, path1;
    ImageData in, out;

    auto p = OpenPath();
    if( !p.has_value() )
        return;

    path0 = *p;
    path1 = path0.parent_path().parent_path();
    path1.append( "output" );
    path1.append( "test12.png" );

    if( in.input( path0.wstring() ) )
    {
        ApplyKernel( in, out );
        out.output( path1.wstring() );
    }
}
