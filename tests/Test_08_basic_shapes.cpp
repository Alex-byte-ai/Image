#include "Test_08_basic_shapes.h"

#include "../ImageWindow.h"
#include "../ImageData.h"

void Test_08_basic_shapes( Context &context )
{
    class Canvas
    {
    public:
        std::optional<int> currentId;
        bool ignoreContour = false;
        int w, h;

        ImageData image;

        bool prepare( int id )
        {
            if( currentId && id != *currentId )
                return false;
            if( currentId )
                image.reset( 256, 256 );
            return true;
        };

        std::optional<Pixel> c( const std::optional<Pixel> &p ) const
        {
            if( ignoreContour )
                return {};
            return p;
        }
    };

    struct Point
    {
        int x, y;
    };

    class AnyShape
    {
    public:
        int id;

        AnyShape( int i ) : id( i )
        {}

        virtual bool draw( Canvas &canvas, const std::optional<Point> &origin ) const = 0;
        virtual std::wstring name() const = 0;

        virtual ~AnyShape()
        {}
    };

    class Line : public AnyShape
    {
    public:
        int x0, y0, x1, y1;
        std::optional<Pixel> contour;

        Line( int i, int x0_, int y0_, int x1_, int y1_, const std::optional<Pixel> &contour_ )
            : AnyShape( i ), x0( x0_ ), y0( y0_ ), x1( x1_ ), y1( y1_ ), contour( contour_ )
        {}

        bool draw( Canvas &canvas, const std::optional<Point> &origin ) const override
        {
            if( !canvas.prepare( id ) )
                return false;

            if( origin )
                canvas.image.line( origin->x, origin->y, x1 - x0 + origin->x, y1 - y0 + origin->y, canvas.c( contour ) );
            else
                canvas.image.line( x0, y0, x1, y1, canvas.c( contour ) );
            return true;
        };

        std::wstring name() const override
        {
            return L"Line";
        }
    };

    class Rectangle : public AnyShape
    {
    public:
        int x0, y0, w, h;
        std::optional<Pixel> contour, fill;

        Rectangle( int i, int x0_, int y0_, int w_, int h_, const std::optional<Pixel> &contour_, const std::optional<Pixel> &fill_ )
            : AnyShape( i ), x0( x0_ ), y0( y0_ ), w( w_ ), h( h_ ), contour( contour_ ), fill( fill_ )
        {}

        bool draw( Canvas &canvas, const std::optional<Point> &origin ) const override
        {
            if( !canvas.prepare( id ) )
                return false;

            canvas.image.rectangle( origin ? origin->x : x0, origin ? origin->y : y0, w, h, canvas.c( contour ), fill );
            return true;
        };

        std::wstring name() const override
        {
            return L"Rectangle";
        }
    };

    class Circle : public AnyShape
    {
    public:
        int x0, y0, r;
        std::optional<Pixel> contour, fill;

        Circle( int i, int x0_, int y0_, int r_, const std::optional<Pixel> &contour_, const std::optional<Pixel> &fill_ )
            : AnyShape( i ), x0( x0_ ), y0( y0_ ), r( r_ ), contour( contour_ ), fill( fill_ )
        {}

        bool draw( Canvas &canvas, const std::optional<Point> &origin ) const override
        {
            if( !canvas.prepare( id ) )
                return false;

            canvas.image.circle( origin ? origin->x : x0, origin ? origin->y : y0, r, canvas.c( contour ), fill );
            return true;
        };

        std::wstring name() const override
        {
            return L"Circle";
        }
    };

    class Ellipse : public AnyShape
    {
    public:
        int x0, y0, rx, ry;
        std::optional<Pixel> contour, fill;
        double angle0, angle1;

        Ellipse( int i, int x0_, int y0_, int rx_, int ry_, const std::optional<Pixel> &contour_, const std::optional<Pixel> &fill_, double angle0_, double angle1_ )
            : AnyShape( i ), x0( x0_ ), y0( y0_ ), rx( rx_ ), ry( ry_ ), contour( contour_ ), fill( fill_ ), angle0( angle0_ ), angle1( angle1_ )
        {}

        bool draw( Canvas &canvas, const std::optional<Point> &origin ) const override
        {
            if( !canvas.prepare( id ) )
                return false;

            canvas.image.ellipse( origin ? origin->x : x0, origin ? origin->y : y0, rx, ry, canvas.c( contour ), fill, angle0, angle1 );
            return true;
        };

        std::wstring name() const override
        {
            return L"Ellipse";
        }
    };

    class Text : public AnyShape
    {
    public:
        std::wstring text;
        int x0, y0, height;
        Pixel c;

        Text( int i, std::wstring t, int x, int y, int h, Pixel color )
            : AnyShape( i ), text( std::move( t ) ), x0( x ), y0( y ), height( h ), c( color )
        {}

        bool draw( Canvas &canvas, const std::optional<Point> &origin ) const override
        {
            if( !canvas.prepare( id ) )
                return false;

            TextGraphics t;
            t.set( L"text", text );
            t.set( L"height", std::to_wstring( height ) );
            t.set( L"x", std::to_wstring( origin ? origin->x : x0 ) );
            t.set( L"y", std::to_wstring( origin ? origin->y : y0 ) );
            t.set( L"text.red", std::to_wstring( c.r ) );
            t.set( L"text.green", std::to_wstring( c.g ) );
            t.set( L"text.blue", std::to_wstring( c.b ) );
            canvas.image.text( t );
            return true;
        };

        std::wstring name() const override
        {
            return L"Text";
        }
    };

    auto &text = context.output();
    auto s = context.scope( __FUNCTION__ );

    auto &info = context.information;
    bool writeDisk = info( L"writeDisk" ).as<bool>();
    bool showImages = info( L"showImages" ).as<bool>();

    Canvas canvas;

    std::vector<std::shared_ptr<AnyShape>> shapes0
    {
        std::make_shared<Line>( 0, 102, 123, 61, 143, Pixel( 0, 255, 0 ) ),
        std::make_shared<Rectangle>( 1, 20, 41, 102, 123, Pixel( 255, 255, 0 ), Pixel( 0, 0, 255 ) ),
        std::make_shared<Circle>( 2, 184, 225, 66, Pixel( 0, 0, 255 ), Pixel( 255, 255, 0 ) ),
        std::make_shared<Text>( 3, L"Sample text.", 16, 230, 16, Pixel( 0, 255, 255 ) ),
        std::make_shared<Ellipse>( 4, 164, 123, 31, 51, Pixel( 0, 255, 255 ), Pixel( 255, 0, 0 ), -0.25 * Pi(), 0.75 * Pi() )
    };

    std::vector<std::shared_ptr<AnyShape>> shapes1
    {
        std::make_shared<Line>( 1, 0, 0, 64, 32, Pixel( 0, 0, 255 ) ),
        std::make_shared<Rectangle>( 0, 0, 0, 64, 32, Pixel( 0, 255, 255 ), Pixel( 255, 0, 0 ) ),
        std::make_shared<Circle>( 2, 0, 0, 16, Pixel( 0, 0, 255 ), Pixel( 255, 255, 0 ) ),
        std::make_shared<Text>( 3, L"Test!", 0, 0, 32, Pixel( 0, 255, 0 ) ),
        std::make_shared<Ellipse>( 4, 0, 0, 32, 16, Pixel( 0, 65, 255 ), Pixel( 255, 190, 0 ), 0.75 * Pi(), -0.25 * Pi() )
    };

    std::wstring description;

    auto add = [&description]( const std::wstring & item )
    {
        if( description.empty() )
            description = item;
        else
            description += L", " + item;
    };

    canvas.currentId.reset();
    canvas.image.reset( 256, 256, Pixel( 255, 255, 255 ) );
    for( auto &shape : shapes0 )
    {
        if( shape->draw( canvas, {} ) )
            add( shape->name() );
    }
    text << description;

    if( showImages )
    {
        ImageWindow window( canvas.image, nullptr );
        window.run();
    }
    if( writeDisk )
        canvas.image.output( context.Output() / L"shapes.png" );

    canvas.currentId = 0;
    auto action = [&]( const ImageWindow::InputData & inputData, ImageWindow::OutputData & outputData )
    {
        auto redraw = [&]()
        {
            description.clear();
            for( auto &shape : shapes1 )
            {
                if( shape->draw( canvas, {{*inputData.mouseX, *inputData.mouseY}} ) )
                {
                    add( shape->name() );
                }
            }
            outputData.image.get();
        };

        if( inputData.leftMouse.changed() && *inputData.leftMouse )
        {
            *canvas.currentId = ( *canvas.currentId + 1 ) % shapes1.size();
            redraw();
            text << description << "\n";
        }

        if( inputData.rightMouse.changed() && *inputData.rightMouse )
        {
            canvas.ignoreContour = !canvas.ignoreContour;
            redraw();
        }

        if( inputData.mouseX.changed() || inputData.mouseY.changed() )
            redraw();
    };

    if( showImages )
    {
        ImageWindow window( canvas.image, action );
        window.run();
    }
}
