#include "Test_34_Digits.h"

#include "../ImageData.h"

#include "Information.h"

#include "Basic.h"
#include <fstream>

void Test_34_Digits( Context &context )
{
    auto &text = context.output();
    auto s = context.scope( __FUNCTION__ );

    auto &info = context.information;
    bool readDisk = info( L"readDisk" ).as<bool>();
    bool writeDisk = info( L"writeDisk" ).as<bool>();

    if( !readDisk || !writeDisk )
        return;

    ImageData input, output;
    input.reset( 3, 5 );
    output.reset( 41, 7, Pixel( 0, 0, 255 ) );

    auto n = []( bool a )
    {
        if( a )
            return 1;
        return 0;
    };

    bool x, y, z, w, r[15];
    for( int d = 0; d < 10; ++d )
    {
        x = d / 8 % 2;
        y = d / 4 % 2;
        z = d / 2 % 2;
        w = d / 1 % 2;

        text << d << " = " << n( x ) << n( y ) << n( z ) << n( w ) << "\n";

        r[0] = x || y || z || !w;
        r[1] = x || z || ( y && w ) || ( !y && !w );
        r[2] = true;
        r[3] = ( !x && !z && !w ) || ( !x && y && !z && w ) || ( !x && y && z && !w ) || ( x && !y && !z && !w ) || ( x && !y && !z && w );
        r[4] = false;
        r[5] = x || !y || ( z && w ) || ( !z && !w );
        r[6] = x || !w || ( y && !z ) || ( !y && z );
        r[7] = x || ( y && !z ) || ( !y && z ) || ( y && !w ) || ( z && !w );
        r[8] = true;
        r[9] = ( !x && !y && !w ) || ( !x && y && z && !w ) || ( x && !y && !z && !w );
        r[10] = false;
        r[11] = x || y || !z || w;
        r[12] = x || ( y && !z && w ) || ( !y && z ) || ( z && !w ) || ( !y && !w );
        r[13] = r[12];
        r[14] = true;

        int k = 0;
        for( int j = 0; j < 5; ++j )
        {
            for( int i = 0; i < 3; ++i )
            {
                *input( i, j ) = r[k] ? Pixel( 255, 255, 0 ) : Pixel( 0, 0, 0 );
                ++k;
            }
        }

        input.place( output, 1 + d * 4, 1 );
    }

    output.output( context.Output() / ( L"digits.png" ) );
}
