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

    {
        ImageData image;
        image.reset( 512, 512 );
        ImageWindow window( image, nullptr, rootObject );
        window.run();
    }

    makeException( root.save( context.Output() / L"save.jei" ) );

    auto nextRoot = root.load( context.Input() / L"load.jei" );
    makeException( nextRoot );

    {
        ImageData image;
        image.reset( 512, 512 );
        ImageWindow window( image, nullptr, nextRoot );
        window.run();
    }

    auto outputV = [&]( const Vector2D & v )
    {
        text << "[" << v.x << "]\n";
        text << "[" << v.y << "]\n\n";
    };

    auto outputM = [&]( const Matrix2D & m )
    {
        text << "[" << m.a00 << ", " << m.a01 << "]\n";
        text << "[" << m.a10 << ", " << m.a01 << "]\n\n";
    };

    double shear = 1.25;

    Matrix2D shearX( 1, shear, 0, 1 );
    Matrix2D shearY( 1, 0, shear, 1 );

    text << "x:\n\n";
    outputM( shearX );
    outputV( shearX * Vector2D( 0, 0 ) );
    outputV( shearX * Vector2D( 1, 0 ) );
    outputV( shearX * Vector2D( 1, 1 ) );
    outputV( shearX * Vector2D( 0, 1 ) );

    text << "y:\n\n";
    outputM( shearY );
    outputV( shearY * Vector2D( 0, 0 ) );
    outputV( shearY * Vector2D( 1, 0 ) );
    outputV( shearY * Vector2D( 1, 1 ) );
    outputV( shearY * Vector2D( 0, 1 ) );
}
