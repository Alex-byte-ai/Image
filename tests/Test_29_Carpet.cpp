#include "Test_29_Carpet.h"

#include "../ImageData.h"

static Pixel warp( ImageData& image, int j, int i, bool supress = false )
{
    if( j < 0 || j >= image.w() || i < 0 || i >= image.h() )
        return supress ? Pixel( 0, 0, 0 ) : *image( ( j + image.w() ) % image.w(), ( i + image.h() ) % image.h() );
    return *image( j, i );
};

static void growCloth( ImageData& image, bool f )
{
    Pixel p0, p1, p2, p3, p4, p5, p6, p7, pi, *po;

    static ImageData buffer;
    if( buffer.w() != image.w() || buffer.h() != image.h() )
        buffer.reset( image.w(), image.h() );

    for( int j = 0; j < image.w(); ++j )
    {
        for( int i = 0; i < image.h(); ++i )
        {
            pi = *image( j, i );
            po = buffer( j, i );

            p0 = warp( image, j - 1, i + 0 );
            p1 = warp( image, j - 1, i - 1 );
            p2 = warp( image, j + 0, i - 1 );
            p3 = warp( image, j + 1, i - 1 );
            p4 = warp( image, j + 1, i + 0 );
            p5 = warp( image, j + 1, i + 1 );
            p6 = warp( image, j + 0, i + 1 );
            p7 = warp( image, j - 1, i + 1 );

            po->r = ( f * pi.r + p0.r + p1.r + p2.r + p3.r + p4.r + p5.r + p6.r + p7.r ) % 256;
            po->g = ( f * pi.g + p0.g + p1.g + p2.g + p3.g + p4.g + p5.g + p6.g + p7.g ) % 256;
            po->b = ( f * pi.b + p0.b + p1.b + p2.b + p3.b + p4.b + p5.b + p6.b + p7.b ) % 256;
            po->a = pi.a;
        }
    }

    buffer.copy( image );
}

void Test_29_Carpet( Context &context )
{
    auto s = context.scope( __FUNCTION__ );

    auto &info = context.information;
    bool readDisk = info( L"readDisk" ).as<bool>();
    bool writeDisk = info( L"writeDisk" ).as<bool>();

    if( !readDisk || !writeDisk )
        return;

    unsigned n = 100;

    auto length = std::to_wstring( n - 1 ).length();
    auto pad = [length]( unsigned i )
    {
        auto id = std::to_wstring( i );
        auto l = id.length();
        while( l < length )
        {
            id = L"0" + id;
            ++l;
        }
        return id;
    };

    ImageData image;
    if( image.input( L"input\\checker.png" ) )
    {
        for( unsigned i = 0; i < n; ++i )
        {
            image.output( context.Output() / ( pad( i ) + L".png" ) );
            growCloth( image, false );
        }
    }
}
