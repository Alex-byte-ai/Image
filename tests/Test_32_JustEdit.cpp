#include "Test_32_JustEdit.h"

#include "../ImageWindow.h"
#include "../ImageData.h"
#include "../JustEdit.h"

#include <fstream>
#include "Scanner.h"

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

    JustEdit::Raster root( L"root", 512, 512 );
    root.fill = Color( 0, 0, 1, 0.5 );

    auto& raster = *dynamic_cast<JustEdit::Raster*>( root.add( std::make_shared<JustEdit::Raster>( L"raster0", 128, 128, JustEdit::Position( {}, 1.5, 2, 1.1 * Pi() / 4, 1.3 ) ) ) );
    raster.fill = Color( 0, 1, 0 );

    auto& circle = *dynamic_cast<JustEdit::Circle*>( raster.add( std::make_shared<JustEdit::Circle>( L"circle0", Vector2D( 64, 64 ), 32 ) ) );
    circle.contour = Color( 0, 0, 1 );
    circle.fill = Color( 1, 1, 0 );

    auto& line = *dynamic_cast<JustEdit::Line*>( raster.add( std::make_shared<JustEdit::Line>( L"line0", Vector2D( 64, 0 ), Vector2D( 128, 64 ) ) ) );
    line.contour = Color( 1, 0, 0 );

    text << root.position.shear << "\n";
    text << root.position.rotation << "\n";
    text << root.position.scaleX << "\n";
    text << root.position.scaleY << "\n";

    auto m = root.position().t;
    text << m.a00 << ", " << m.a01 << ", " << m.a10 << ", " << m.a11 << "\n";

    {
        ImageData image;
        image.reset( 512, 512 );
        ImageWindow window( image, nullptr, &root );
        window.run();
    }

    root.position( root.position() );
    text << root.position.shear << "\n";
    text << root.position.rotation << "\n";
    text << root.position.scaleX << "\n";
    text << root.position.scaleY << "\n";
}
