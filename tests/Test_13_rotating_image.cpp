#include "Test_13_rotating_image.h"

#include <fstream>

#include "Vector2D.h"
#include "Affine2D.h"
#include "Window.h"

#include "../ImageWindow.h"
#include "../ImageData.h"
#include "../GetImage.h"
#include "../Overlap.h"

class Frame
{
private:
    Overlap::Canvas canvas;
    double speed, rx, ry;
    Vector2D center;

    std::vector<std::unique_ptr<Frame>> insides;
public:
    Frame( const ImageData &sample, const Vector2D &c, double s, double radiusX, double radiusY )
        : canvas( sample ), speed( s ), rx( radiusX ), ry( radiusY ), center( c )
    {};

    bool inside( const Vector2D &p ) const
    {
        double dx = ( p.x - center.x ) / rx;
        double dy = ( p.y - center.y ) / ry;
        return dx * dx + dy * dy < 1.0;
    }

    bool add( std::unique_ptr<Frame> item )
    {
        if( !inside( item->center ) )
            return false;

        for( auto &root : insides )
        {
            if( root->add( std::move( item ) ) )
                return true;
        }

        insides.emplace_back( std::move( item ) );
        return true;
    }

    void render( ImageDataBase &image, double time, int trick = 0 )
    {
        Affine2D position;
        draw( position, time );
        canvas.render( image );

        if( trick == 1 )
        {
            ImageData part;
            int x0 = Round( Mod( time * 2, 1 ) * image.w() );

            image.sub( part, 0, 0, x0, image.h() );
            image.crop( image, x0, 0, image.w() + x0, image.h(), Pixel( 0, 0, 0, 0 ) );
            part.place( image, image.w() - x0, 0 );
        }
        else if( trick == 2 )
        {
            ImageData part;
            int y0 = Round( Mod( time * 2, 1 ) * image.h() );

            image.sub( part, 0, 0, image.w(), y0 );
            image.crop( image, 0, y0, image.w(), image.h() + y0, Pixel( 0, 0, 0, 0 ) );
            part.place( image, 0, image.h() - y0 );
        }
        else if( trick == 3 )
        {
            ImageData part;
            int x0 = Round( Mod( time * 2, 1 ) * image.w() );

            image.sub( part, image.w() - x0, 0, image.w(), image.h() );
            image.crop( image, -x0, 0, image.w() - x0, image.h(), Pixel( 0, 0, 0, 0 ) );
            part.place( image, 0, 0 );
        }
        else if( trick == 4 )
        {
            ImageData part;
            int y0 = Round( Mod( time * 2, 1 ) * image.h() );

            image.sub( part, 0, image.h() - y0, image.w(), image.h() );
            image.crop( image, 0, -y0, image.w(), image.h() - y0, Pixel( 0, 0, 0, 0 ) );
            part.place( image, 0, 0 );
        }
        else if( trick == 5 || trick == 6 )
        {
            double t = time;

            t = time + 0.5;
            if( t > 1 )
                t -= 1;

            if( trick == 6 )
                t = 1 - t;

            t *= Pi() / 2;
            t = Tan( t );

            double x = Sqrt( 1 / t );
            double y = Sqrt( t );

            auto def = [&]()
            {
                image.reset( image.w(), image.h(), Pixel( 101, 210, 246 ) );
            };

            if( isNumber( x ) && isNumber( y ) && !isInfinite( x ) && !isInfinite( y ) )
            {
                int w = image.w() * x;
                int h = image.h() * y;

                if( w > 0 && h > 0 && w < 10000 && h < 10000 )
                {
                    ImageConvert::Reference in, out;

                    ImageData output( w, h );

                    makeReference( image, in );
                    makeReference( output, out );

                    translate( in, out, true );

                    def();
                    output.place( image, ( image.w() - w ) / 2, ( image.h() - h ) / 2 );
                }
                else
                {
                    def();
                }
            }
            else
            {
                def();
            }
        }
    }

    bool write( const std::filesystem::path &path ) const
    {
        std::ofstream file( path );
        return write( file, 0 );
    }

    bool read( const std::filesystem::path &path )
    {
        std::ifstream file( path );
        return read( file );
    }

private:
    bool write( std::ofstream &file, int tabs = 0 ) const
    {
        if( !file )
            return false;

        for( int i = 0; i < tabs; ++i )
        {
            file << "\t";
            if( !file )
                return false;
        }

        file << speed << "\n";
        if( !file )
            return false;

        ++tabs;
        for( auto &item : insides )
        {
            if( !item->write( file, tabs ) )
                return false;
        }

        return true;
    }

    bool read( std::ifstream &file )
    {
        if( !file )
            return false;

        file >> speed;

        if( !file )
            return false;

        for( auto &item : insides )
        {
            if( !item->read( file ) )
                return false;
        }

        return true;
    }

    void draw( Affine2D &position, double time )
    {
        canvas.clear();
        for( auto &frame : insides )
        {
            frame->draw( position, time );

            ImageData image;
            frame->canvas.render( image );

            Overlap::Picture picture( image );
            picture.set( position );
            canvas.draw( picture );
        }

        position =
            Affine2D( +center ) *
            Affine2D( Matrix2D::Scale( 1 / ry, 1 / rx ) ) *
            Affine2D( Matrix2D::Rotation( 2 * speed * time * Pi() ) ) *
            Affine2D( Matrix2D::Scale( ry, rx ) ) *
            Affine2D( -center );
    }
};

void Test_13_rotating_image( Context &context )
{
    auto &text = context.output();
    auto s = context.scope( __FUNCTION__ );

    auto &info = context.information;
    bool readDisk = info( L"readDisk" ).as<bool>();
    bool writeDisk = info( L"writeDisk" ).as<bool>();
    bool showImages = info( L"showImages" ).as<bool>();
    bool outputVariableData = info( L"outputVariableData" ).as<bool>();

    ImageData base, image;
    if( !readDisk || !base.input( L"input\\Rotate.png" ) )
        return;

    base.copy( image );

    std::unique_ptr<Frame> frame;
    auto newFrame = [&]()
    {
        frame = std::make_unique<Frame>( base, Vector2D(), 0, 500000, 500000 );
    };
    newFrame();

    unsigned actionId = 0, trick = 0;
    double rx = 128, ry = 128, time = 0;
    bool canRotate = false;
    Vector2D shift;

    auto spin = [&]( const ImageWindow::InputData & inputData, ImageWindow::OutputData & outputData )
    {
        auto drawCircle = [&]( bool mouseCoordinates )
        {
            if( mouseCoordinates )
            {
                shift.x = *inputData.mouseX;
                shift.y = *inputData.mouseY;
            }

            base.copy( outputData.image.get() );
            outputData.image.get().ellipse( Round( shift.x ), Round( shift.y ), Round( rx ), Round( ry ), {}, Pixel( 0, 0, 0, 0 ) );

            canRotate = false;
        };

        auto invertCircle = [&]()
        {
            drawCircle( false );

            ImageData ellipse;
            base.copy( ellipse );

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

            ellipse.function( ellipse, applyMask );
            ellipse.copy( outputData.image.get() );

            shift.x = Round( shift.x );
            shift.y = Round( shift.y );
            rx = Round( rx );
            ry = Round( ry );

            frame->add( std::make_unique<Frame>( ellipse, shift, 1, rx, ry ) );

            canRotate = true;
        };

        auto turn = [&]()
        {
            if( !canRotate )
                return false;

            frame->render( outputData.image.get(), time, trick );

            time += 0.5 * 0.5 * 0.5 * 0.5 * 0.5 * 0.5;
            if( time > 1 )
            {
                time -= 1;
                return false;
            }
            return true;
        };

        auto keyDown = [&]( char symbol )
        {
            auto &key = inputData.keys.letter( symbol );
            return key.changed() && *key;
        };

        if( keyDown( 'A' ) )
        {
            if( actionId == 0 )
            {
                text << L"Setup position and size of an ellipse\n";
                text << L"Use left/right mouse button to increase/decrease size\n";
                text << L"Hold Z to use smaller step\n";
                text << L"Hold X to only change horizontal size\n";
                text << L"Hold Y to only change vertical size\n";
                text << L"Or press R to create ellipse inscribed into image frame\n";
                text << L"Press A to continue\n\n";
                drawCircle( true );
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
                newFrame();

            actionId = 0;
            base.copy( outputData.image.get() );
            time = 0;
        }

        if( keyDown( 'T' ) )
        {
            auto path = context.Output() / L"config.txt";
            if( !frame->read( path ) )
                frame->write( path );
            time = 0;
        }

        if( keyDown( 'I' ) )
        {
            trick = ( trick + 1 ) % 7;
            time = 0;
        }

        if( ( actionId == 1 ) && ( inputData.mouseX.changed() || inputData.mouseY.changed() ) )
        {
            drawCircle( true );
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
            drawCircle( true );
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
            drawCircle( true );
        }

        if( writeDisk && outputVariableData && ( actionId == 2 ) &&  keyDown( 'L' ) )
        {
            auto spn = context.scope( "spin" );

            for( trick = 0; trick < 7; ++trick )
            {
                auto trk = context.scope( "trick" + std::to_string( trick ) );

                unsigned j = 0;
                time = 0;

                while( turn() )
                {
                    auto id = std::to_wstring( j++ );
                    while( id.size() < 2 )
                        id = L"0" + id;

                    outputData.image->output( context.Output() / ( id + L".png" ) );
                }
            }
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
