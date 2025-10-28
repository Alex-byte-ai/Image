#include "Test_05_interactive_window.h"

#include <memory>

#include "Vector2D.h"
#include "Affine2D.h"

#include "../ImageWindow.h"
#include "../ImageData.h"
#include "../Filters.h"
#include "../Overlap.h"

static ConsoleOutput *globalText = nullptr;

static void MouseDrawing( const ImageWindow::InputData &inputData, ImageWindow::OutputData &outputData )
{
    if( inputData.mouseX.changed() || inputData.mouseY.changed() )
    {
        int x = *inputData.mouseX, y = *inputData.mouseY;
        if( ( 0 <= x ) && ( x < outputData.image->w() ) && ( 0 <= y ) && ( y < outputData.image->h() ) )
        {
            if( *inputData.leftMouse || *inputData.rightMouse || *inputData.middleMouse )
            {
                *( outputData.image.get() )( x, y ) = Pixel( *inputData.leftMouse * 255, *inputData.middleMouse * 255, *inputData.rightMouse * 255 );
            }
        }
    }

    auto &key = inputData.keys.letter( 'R' );
    if( key.changed() && *key )
    {
        outputData.image.get().reset( 512, 512, Pixel( 128, 128, 128 ) );
    }
}

static void DrawTextOnBackground( const ImageWindow::InputData &inputData, ImageWindow::OutputData &outputData )
{
    static ImageData backgroundImage, cursorPositionMarker;
    static bool textBackground;
    static std::wstring string = L"initial";
    static int textId = -1;

    if( inputData.mouseX.changed() || inputData.mouseY.changed() )
    {
        int x = *inputData.mouseX, y = *inputData.mouseY;
        if( ( 0 <= x ) && ( x < outputData.image->w() ) && ( 0 <= y ) && ( y < outputData.image->h() ) )
        {
            if( backgroundImage.empty() )
            {
                backgroundImage.input( L"input\\passage.png" );
            }

            if( cursorPositionMarker.empty() )
            {
                cursorPositionMarker.reset( 64, 64 );
                cursorPositionMarker.function( cursorPositionMarker, Filters::rainbowPie );
            }

            backgroundImage.copy( outputData.image.get() );
            cursorPositionMarker.placeTransperent( outputData.image.get(), x - 32, y - 32 );

            TextGraphics text;
            text.set( L"text", string );
            text.set( L"x", std::to_wstring( x ) );
            text.set( L"y", std::to_wstring( y ) );
            if( textBackground )
            {
                text.set( L"background.red", L"5" );
                text.set( L"background.green", L"55" );
                text.set( L"background.blue", L"5" );
            }
            outputData.image.get().text( text );
        }
    }

    if( inputData.leftMouse.changed() && *inputData.leftMouse )
    {
        textId = ( textId + 1 ) % 4;
        switch( textId )
        {
        case 0:
            string = L"ABC";
            break;
        case 1:
            string = L"テキスト";
            break;
        case 2:
            string = L"XYZ\tWUV";
            break;
        case 3:
            string = L"\tXYZ\tWUV";
            break;
        default:
            makeException( false );
        }
    }

    if( inputData.rightMouse.changed() && *inputData.rightMouse )
    {
        textBackground = !textBackground;
    }
}

static void DrawTransformedImage( const ImageWindow::InputData &inputData, ImageWindow::OutputData &outputData )
{
    static std::unique_ptr<Overlap::Picture> picture;
    static std::unique_ptr<Overlap::Canvas> canvas;
    static double ang = 0, xs = 1, ys = 1;
    static ImageData background, spot;
    static int transformId = 0;
    static int parameterId = 0;
    static Vector2D shift;

    auto draw = [&]()
    {
        if( background.empty() )
        {
            background.input( L"input\\tree.png" );
            canvas = std::make_unique<Overlap::Canvas>( background );
        }

        if( spot.empty() )
        {
            spot.input( L"input\\64x32.png" );
            picture = std::make_unique<Overlap::Picture>( spot );
        }

        if( transformId == 0 )
        {
            Vector2D c = Vector2D( spot.w() * 0.5, spot.h() * 0.5 );
            Affine2D pos = Affine2D( shift ) * Affine2D( Matrix2D::Rotation( ang ) ) * Affine2D( Matrix2D::Scale( xs, ys ) ) * Affine2D( -c );
            canvas->clear();
            picture->set( pos );
            canvas->draw( *picture );
            canvas->render( outputData.image.get() );
        }
        else
        {
            background.copy( outputData.image.get() );
            spot.place( outputData.image.get(), shift.x, shift.y );
        }
    };

    if( inputData.mouseX.changed() || inputData.mouseY.changed() )
    {
        shift.x = *inputData.mouseX;
        shift.y = *inputData.mouseY;
        draw();
    }

    if( inputData.leftMouse.changed() && *inputData.leftMouse )
    {
        //spot.Input();
        transformId = ( transformId + 1 ) % 2;

        if( transformId == 0 )
        {
            *globalText << L"Affine2D\n";
        }
        else
        {
            *globalText << L"Place\n";
        }

        draw();
    }

    if( inputData.rightMouse.changed() && *inputData.rightMouse )
    {
        parameterId = ( parameterId + 1 ) % 3;

        if( parameterId == 0 )
        {
            *globalText << L"Angle\n";
        }
        else if( parameterId == 1 )
        {
            *globalText << L"Horizontal scale\n";
        }
        else if( parameterId == 2 )
        {
            *globalText << L"Vertical scale\n";
        }
    }

    auto &a = inputData.keys.letter( 'A' );
    if( a.changed() && *a )
    {
        if( parameterId == 0 )
        {
            ang -= 5 * Pi() / 180;
        }
        else if( parameterId == 1 )
        {
            xs -= 0.25;
        }
        else if( parameterId == 2 )
        {
            ys -= 0.25;
        }

        draw();
    }

    auto &d = inputData.keys.letter( 'D' );
    if( d.changed() && *d )
    {
        if( parameterId == 0 )
        {
            ang += 5 * Pi() / 180;
        }
        else if( parameterId == 1 )
        {
            xs += 0.25;
        }
        else if( parameterId == 2 )
        {
            ys += 0.25;
        }

        draw();
    }
}

void Test_05_interactive_window( Context &context )
{
    auto &text = context.output();
    auto s = context.scope( __FUNCTION__ );
    globalText = &text;

    auto &info = context.information;
    bool readDisk = info( L"readDisk" ).as<bool>();
    bool writeDisk = info( L"writeDisk" ).as<bool>();
    bool showImages = info( L"showImages" ).as<bool>();
    bool outputVariableData = info( L"outputVariableData" ).as<bool>();

    ImageData img;
    img.reset( 32, 32, Pixel( 255, 0, 0 ) );

    if( showImages && readDisk )
    {
        ImageWindow window( img, DrawTextOnBackground );
        window.run();
        if( outputVariableData && writeDisk )
            img.output( context.Output() / L"canvas.png" );
    }

    img.errorMsg( L"Error #0.\nError #1." );
    img.errorMsg( L"Error #2." );

    if( showImages )
    {
        ImageWindow window( img, MouseDrawing );
        window.run();
        if( outputVariableData && writeDisk )
            img.output( context.Output() / L"canvasWithErrors.png" );
    }

    if( showImages && readDisk )
    {
        ImageWindow window( img, DrawTransformedImage );
        window.run();
        if( outputVariableData && writeDisk )
            img.output( context.Output() / L"canvasWithTransformedImage.png" );
    }
}
