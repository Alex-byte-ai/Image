#include "Test_26_Force_image_size.h"

#include "../ImageData.h"
#include "../GetImage.h"

void Test_26_Force_image_size( Context &context )
{
    auto s = context.scope( __FUNCTION__ );

    auto &info = context.information;
    bool readDisk = info( L"readDisk" ).as<bool>();
    bool writeDisk = info( L"writeDisk" ).as<bool>();

    if( !readDisk || !writeDisk )
        return;

    ImageData input, scaled, output;
    for( const auto &file : std::filesystem::recursive_directory_iterator( context.Input() ) )
    {
        std::filesystem::path f = file;
        if( f.extension() == L".png" )
        {
            ImageConvert::Reference in, sc;

            input.input( f );
            makeReference( input, in );

            int side = 512;
            double size = Max( Max( input.w(), input.h() ), side );

            scaled.reset( Round( input.w() * side / size ), Round( input.h() * side / size ) );
            makeReference( scaled, sc );

            makeException( scaled.w() <= side && scaled.h() <= side );

            translate( in, sc, true );

            int x = ( side - scaled.w() ) / 2;
            int y = ( side - scaled.h() ) / 2;

            output.reset( side, side, Pixel( 0, 0, 0, 0 ) );
            scaled.place( output, x, y );

            output.output( context.Output() / f.filename() );
        }
    }
}
