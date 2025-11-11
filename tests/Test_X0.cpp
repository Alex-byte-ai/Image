#include "Test_X0.h"

#include "Window.h"

#include "../ImageWindow.h"
#include "../ImageData.h"

#include "Common.h"

void Test_X0( Context &context )
{
    auto &text = context.output();
    auto s = context.scope( __FUNCTION__ );

    std::filesystem::path path0, path1;
    FILE *manualPicks = nullptr;
    ImageData in, mask, out;

    text << L"Open input\\in.png\n";

    auto p = openPath();
    if( !p.has_value() )
        return;

    path0 = *p;
    path1 = path0.parent_path().parent_path();
    path1.append( "output" );
    path1.append( "test10.png" );

    in.input( path0.wstring() );

    getCores().image = &in;

    manualPicks = _wfopen( L"output\\manualPicks", L"rb" );
    if( manualPicks )
    {
        unsigned n;
        Core next;

        fread( &n, sizeof( n ), 1, manualPicks );
        while( n > 0 )
        {
            next.image = getCores().image;
            fread( &next.j, sizeof( next.j ), 1, manualPicks );
            fread( &next.i, sizeof( next.i ), 1, manualPicks );
            next.c = *( *getCores().image )( next.j, next.i );
            getCores().cores.push_back( next );
            --n;
        }
        fclose( manualPicks );
    }
    else
    {
        ImageWindow window( in, ManualPicks );
        window.run();

        manualPicks = _wfopen( L"output\\manualPicks", L"wb" );
        if( manualPicks )
        {
            unsigned i = 0, n = getCores().cores.size();

            fwrite( &n, sizeof( n ), 1, manualPicks );
            while( i < n )
            {
                auto &next = getCores().cores[i];
                fwrite( &next.j, sizeof( next.j ), 1, manualPicks );
                fwrite( &next.i, sizeof( next.i ), 1, manualPicks );
                ++i;
            }
            fclose( manualPicks );
        }
    }

    if( !LimitPalette( in, out, Sqrt( 3.0 ) / 20 ) )
    {
        text << L"Wasn't able to finish test10.\n";
        return;
    }

    out.output( path1.wstring() );

    system( "pause" );

    mask.input( path1.wstring() );

    if( !ReplacePinkWithTransparent( in, mask, out ) )
    {
        text << L"Wasn't able to finish test10.\n";
        return;
    }

    out.output( path1.wstring() );
}
