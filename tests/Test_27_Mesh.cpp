#include "Test_27_Mesh.h"

#include <set>

#include "Vector3D.h"
#include "Basic.h"
#include "Mesh.h"

#include "../ImageWindow.h"
#include "../ImageData.h"
#include "../GetImage.h"
#include "../Overlap.h"
#include "../Filters.h"

void Test_27_Mesh( Context &context )
{
    auto s = context.scope( __FUNCTION__ );

    auto &info = context.information;
    bool readDisk = info( L"readDisk" ).as<bool>();
    bool writeDisk = info( L"writeDisk" ).as<bool>();
    bool showImages = info( L"showImages" ).as<bool>();
    bool outputVariableData = info( L"outputVariableData" ).as<bool>();

    if( !readDisk || !writeDisk )
        return;

    ImageData texture, image;

    if( !texture.input( L"input\\texture.png" ) )
        return;

    image.reset( 256, 256, Pixel( 0, 0, 0, 0 ) );

    Mesh mesh, original, cube, cubeObj, field, prism;

    {
        Affine3D pos =
            Affine3D( Matrix3D::Scale( 0.5 ) ) *
            Affine3D( Vector3D( -0.5, -0.5, -0.5 ) );

        cube.cube();
        cube.transform( pos );
    }

    {
        Affine3D pos =
            Affine3D( Matrix3D::Scale( 0.5 ) ) *
            Affine3D( Vector3D( -0.5, -0.5, -0.5 ) );

        makeException( cubeObj.input( L"input\\models\\cube.obj" ) );
        cubeObj.transform( pos );
    }

    {
        field.plane( 16, 16 );

        Vector3D zero = Vector3D( 0.75, 0.75, 0 );
        double sigma = 0.2;

        field.transform( [&]( Vector3D & point )
        {
            point.z = 0.3 * Exp( -0.5 * ( point - zero ).Sqr() / ( sigma * sigma ) );
        } );

        Affine3D pos =
            Affine3D( Matrix3D::Scale( 0.5 ) ) *
            Affine3D( Vector3D( -0.5, -0.5, 0 ) );
        field.transform( pos );
    }

    {
        std::vector<Vector2D> base
        {
            {0.0, 0.0},
            {4.0, 0.0},
            {4.0, 4.0},
            {3.0, 4.0},
            {3.0, 1.0},
            {1.0, 1.0},
            {1.0, 4.0},
            {0.0, 4.0}
        };

        prism.prism( base );
    }

    Vector3D light( 3, -3, -3 );
    double intensity = 37.5;

    Affine3D camera;
    double angleX = 0, angleY = 0;

    auto draw = [&]( ImageDataBase & img )
    {
        camera = Affine3D( Matrix3D::Identity(), Vector3D( 0, 0, -10 ) );
        camera = Affine3D( Matrix3D::Rotation( Vector3D( 1, 0, 0 ), angleX ) ) * camera;
        camera = Affine3D( Matrix3D::Rotation( Vector3D( 0, 1, 0 ), angleY ) ) * camera;

        img.function( img, [&]( double x, double y, const Color &, Color & out )
        {
            Vector3D p0( x, y, 0 ), p1( x, y, 1 );
            double u, v, t;

            p0 = camera( p0 );
            p1 = camera( p1 );

            if( auto index = mesh.intersectSegment( p0, p1, u, v, t ) )
            {
                auto triangle = mesh[*index];

                auto dp = p1 - p0;
                auto p = p0 + t * dp;

                auto normal = triangle.n( u, v );
                double k = -intensity * ( dp * normal ) / ( p - light ).Sqr();

                auto uv = triangle.uv( u, v );
                u = uv.x;
                v = uv.y;

                if( k < 0 )
                    k = 0;
                if( k > 1 )
                    k = 1;

                int width = texture.w();
                int height = texture.h();

                int j = Mod( RoundDown( u * width ), width );
                int i = Mod( RoundDown( v * height ), height );

                out = ( Color ) * texture( j, i );
                out.r *= k;
                out.g *= k;
                out.b *= k;
            }
            else
            {
                out = Color( 0, 0, 0, 0 );
            }
        } );
    };

    auto scene = [&]( const ImageWindow::InputData & inputData, ImageWindow::OutputData & outputData )
    {
        auto keyDown = [&]( char symbol )
        {
            auto &key = inputData.keys.letter( symbol );
            return key.changed() && *key;
        };

        if( keyDown( 'R' ) )
        {
            angleX = 0;
            angleY = 0;
            draw( outputData.image.get() );
        }

        if( keyDown( 'W' ) )
        {
            angleX -= Pi() / 8;
            draw( outputData.image.get() );
        }

        if( keyDown( 'A' ) )
        {
            angleY += Pi() / 8;
            draw( outputData.image.get() );
        }

        if( keyDown( 'S' ) )
        {
            angleX += Pi() / 8;
            draw( outputData.image.get() );
        }

        if( keyDown( 'D' ) )
        {
            angleY -= Pi() / 8;
            draw( outputData.image.get() );
        }
    };

    mesh = original = cube;
    draw( image );
    if( showImages )
    {
        ImageWindow window( image, scene );
        window.run();
        if( outputVariableData )
            image.output( context.Output() / L"cube.png" );
    }

    mesh = original = cubeObj;
    draw( image );
    if( showImages )
    {
        ImageWindow window( image, scene );
        window.run();
        if( outputVariableData )
            image.output( context.Output() / L"cube.obj.png" );
    }

    mesh = original = field;
    draw( image );
    if( showImages )
    {
        ImageWindow window( image, scene );
        window.run();
        if( outputVariableData )
            image.output( context.Output() / L"field.png" );
    }

    mesh = original = prism;
    draw( image );
    if( showImages )
    {
        ImageWindow window( image, scene );
        window.run();
        if( outputVariableData )
            image.output( context.Output() / L"prism.png" );
    }
}
