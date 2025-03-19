#include "Test_13_rotating_image.h"

#include "Vector2D.h"
#include "Affine2D.h"
#include "Window.h"

#include "../ImageWindow.h"
#include "../ImageData.h"
#include "../Overlap.h"

class RotatingImage
{
public:
    std::unique_ptr<Picture> picture;
    Vector2D center;
    double k;
};

void Test_13_rotating_image( Context &context )
{
    auto &text = context.output();
    auto s = context.scope( __FUNCTION__ );

    auto &info = context.information;
    bool readDisk = info( "readDisk" ).as<bool>();
    bool writeDisk = info( "writeDisk" ).as<bool>();
    bool showImages = info( "showImages" ).as<bool>();
    bool outputVariableData = info( "outputVariableData" ).as<bool>();

    ImageData base, image, circle;
    if( !readDisk || !base.input( L"input\\Rotate.png" ) )
        return;

    base.copy( image );

    Canvas canvas( base );
    std::vector<RotatingImage> pictures;

    unsigned actionId = 0;
    double rx = 128, ry = 128, ang = 0;
    bool canRotate = false;
    Vector2D shift;

    auto spin = [&]( const ImageWindow::InputData & inputData, ImageWindow::OutputData & outputData )
    {
        auto drawCircle = [&]()
        {
            shift.x = *inputData.mouseX;
            shift.y = *inputData.mouseY;
            base.copy( outputData.image.get() );
            outputData.image.get().ellipse( *inputData.mouseX, *inputData.mouseY, rx, ry, Pixel( 0, 0, 0, 0 ) );
            canRotate = false;
        };

        auto invertCircle = [&]()
        {
            drawCircle();

            base.copy( circle );

            auto &mask = *outputData.image;
            auto applyMask = [&]( int, int, int j, int i, const Pixel & input, Pixel & output )
            {
                auto maskColor = mask( j, i );
                if( !maskColor || maskColor->a == 0 )
                {
                    output = input;
                }
                else
                {
                    output = Pixel( 0, 0, 0, 0 );
                }
            };

            circle.function( circle, applyMask );
            circle.copy( outputData.image.get() );
            pictures.push_back( {std::make_unique<Picture>( circle ), shift, rx / ry} );

            canRotate = true;
        };

        auto turn = [&]()
        {
            if( !canRotate )
                return false;

            canvas.clear();
            for( auto &r : pictures )
            {
                Affine2D pos =
                    Affine2D( +r.center ) *
                    Affine2D( Matrix2D::Scale( 1, 1 / r.k ) ) *
                    Affine2D( Matrix2D::Rotation( ang ) ) *
                    Affine2D( Matrix2D::Scale( 1, r.k ) ) *
                    Affine2D( -r.center );

                r.picture->set( pos );
                canvas.draw( *r.picture );
            }
            canvas.render( outputData.image.get() );

            ang += 10 * Pi() / 180;
            if( ang > 2 * Pi() )
            {
                ang -= 2 * Pi();
                return false;
            }
            return true;
        };

        auto keyDown = [&]( char symbol )
        {
            auto &key = inputData.keys.letter( symbol );
            return key.changed() && *key;
        };

        if( ( actionId == 1 ) && ( inputData.mouseX.changed() || inputData.mouseY.changed() ) )
        {
            drawCircle();
        }

        if( keyDown( 'A' ) )
        {
            if( actionId == 0 )
            {
                text << L"Setup position and size of an ellipse\n";
                text << L"Use left/right mouse button to increase/decrease size\n";
                text << L"Hold Z to use smaller step\n";
                text << L"Hold X to only change horizontal size\n";
                text << L"Hold Y to only change vertical size\n";
                text << L"Press A to continue\n\n";
                drawCircle();
                ++actionId;
            }
            else if( actionId == 1 )
            {
                text << L"Observe resulting hole in an image, press A to continue\n\n";
                invertCircle();
                ++actionId;
            }
            else if( actionId == 2 )
            {
                text << L"Press A to rotate an image, press S to start over\n\n";
                turn();
            }
        }

        if( keyDown( 'S' ) )
        {
            Popup question( Popup::Type::Question, L"Editor", L"Do you want to remove created circles?" );
            question.run();
            if( question.answer && *question.answer )
                pictures.clear();

            actionId = 0;
            base.copy( outputData.image.get() );
        }

        if( writeDisk && outputVariableData && ( actionId == 2 ) &&  keyDown( 'L' ) )
        {
            auto spn = context.scope( "spin" );

            unsigned j = 0;
            ang = 0;

            while( turn() )
            {
                outputData.image->output( context.Output() / ( std::to_wstring( j++ ) + L".png" ) );
            }
        }

        auto &keyZ = inputData.keys.letter( 'Z' );
        auto &keyX = inputData.keys.letter( 'X' );
        auto &keyC = inputData.keys.letter( 'C' );

        if( ( actionId == 1 ) && !canRotate && inputData.leftMouse.changed() && *inputData.leftMouse )
        {
            bool both = !*keyX && !*keyC;
            bool x = *keyX || both;
            bool y = *keyC || both;
            int delta = *keyZ ? 1 : 20;
            if( x )
                rx += delta;
            if( y )
                ry += delta;
            drawCircle();
        }

        if( ( actionId == 1 ) && !canRotate && inputData.rightMouse.changed() && *inputData.rightMouse )
        {
            bool both = !*keyX && !*keyC;
            bool x = *keyX || both;
            bool y = *keyC || both;
            int delta = *keyZ ? 1 : 20;
            if( x )
                rx -= delta;
            if( y )
                ry -= delta;
            if( rx < 0 )
                rx = 0;
            if( ry < 0 )
                ry = 0;
            drawCircle();
        }
    };

    if( showImages )
    {
        ImageWindow window( image, spin );
        text << L"Observe an image, press A to continue\n\n";
        window.run();
    }

    if( writeDisk && outputVariableData )
        image.output( context.Output() / L"rotated.png" );
}
