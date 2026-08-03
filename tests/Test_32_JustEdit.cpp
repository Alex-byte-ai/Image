#include "Test_32_JustEdit.h"

#include "../ImageWindow.h"
#include "../ImageData.h"
#include "../JustEdit.h"

#include "Information.h"

void Test_32_JustEdit( Context &context )
{
    auto &text = context.output();
    auto s = context.scope( __FUNCTION__ );

    auto &info = context.information;
    bool readDisk = info( L"readDisk" ).as<bool>();
    bool writeDisk = info( L"writeDisk" ).as<bool>();
    bool showImages = info( L"showImages" ).as<bool>();

    if( !readDisk || !writeDisk || !showImages )
        return;

    auto rootObject = std::make_shared<JustEdit::Raster>( L"root", 512, 512 );
    auto& root = *rootObject;
    root.fill = Color( 0, 0, 1, 0.5 );

    auto& raster = *dynamic_cast<JustEdit::Raster*>( root.add( std::make_shared<JustEdit::Raster>( L"raster0", 128, 128, JustEdit::Position( {}, 1.5, 2, 1.1 * Pi() / 4, 1.3 ) ) ) );
    raster.fill = Color( 0, 1, 0 );

    raster.add( std::make_shared<JustEdit::Circle>( L"circle0", Vector2D( 64, 64 ), 32 ) );

    auto& line = *dynamic_cast<JustEdit::Line*>( raster.add( std::make_shared<JustEdit::Line>( L"line0", Vector2D( 64, 0 ), Vector2D( 128, 64 ) ) ) );
    line.contour = Color( 1, 0, 0 );
    line.thickness = 4;

    auto& rectangle = *dynamic_cast<JustEdit::Rectangle*>( raster.add( std::make_shared<JustEdit::Rectangle>( L"rectangle0", 16, 16, JustEdit::Position( Vector2D( 16, 96 ) ) ) ) );
    rectangle.contour = Color( 0.5, 0.5, 0.5 );
    rectangle.fill = Color( 1, 0.5, 0 );
    rectangle.thickness = 1;

    Overlap::Frame frame0( {8.0, 8.0}, {120, -8.0}, {136.0, 128.0}, {-8.0, 120.0} );
    Overlap::Frame frame1( {16.0, 0.0}, {112.0, 0.0}, {128.0, 128.0}, {0.0, 128.0} );

    auto isConner = []( int i, int j, int size )
    {
        if( i == 0 && j == 0 )
            return true;
        if( i == 0 && j == size )
            return true;
        if( i == size && j == 0 )
            return true;
        if( i == size && j == size )
            return true;
        return false;
    };

    auto isInside = []( const Vector2D & uv )
    {
        return 0 <= uv.x && uv.x <= 1 && 0 <= uv.y && uv.y <= 1;
    };

    for( int i = -40; i <= 40; ++i )
    {
        for( int j = -40; j <= 40; ++j )
        {
            Vector2D orig( i / 10.0, j / 10.0 );
            auto p = frame0.p( orig );
            auto uv0 = frame0.uv( p, -1 );
            auto uv1 = frame0.uv( p, +1 );

            // auto& circle = *dynamic_cast<JustEdit::Circle*>( root.add( std::make_shared<JustEdit::Circle>( L"dot", p, isConner( i, j, 10 ) ? 8 : 2 ) ) );
            // circle.fill = Color( isInside( orig ), isInside( uv0 ), isInside( uv1 ) );
        }
    }

    {
        ImageData image;
        image.reset( 256, 512 );
        ImageWindow window( image, nullptr, rootObject, ImageWindow::Data( Popup( Popup::Type::Info, L"Test", L"Sample Text." ) ) );
        window.run();
    }

    makeException( root.save( context.Output() / L"save.jei" ) );

    auto nextRoot = root.load( context.Input() / L"load.jei" );
    makeException( nextRoot );

    {
        ImageData image;
        image.reset( 512, 256 );
        ImageWindow window( image, nullptr, nextRoot, ImageWindow::Data( Popup( Popup::Type::Info, L"Test", L"Sample Text." ) ) );
        window.run();
    }
}
