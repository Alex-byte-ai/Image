#include "Test_04_image_processing.h"

#include "../ApplyKernel.h"
#include "../ImageWindow.h"
#include "../ImageData.h"
#include "../Outline.h"
#include "../Filters.h"

void Test_04_image_processing( Context &context )
{
    auto &text = context.output();
    auto s = context.scope( __FUNCTION__ );

    auto &info = context.information;
    bool readDisk = info( L"readDisk" ).as<bool>();
    bool writeDisk = info( L"writeDisk" ).as<bool>();
    bool additional = info( L"additional" ).as<bool>();
    bool showImages = info( L"showImages" ).as<bool>();

    if( !readDisk )
        return;

    ImageData input, output, ico;

    if( !ico.input( L"input\\ico1.png" ) )
        return;

    std::vector<std::filesystem::path> paths
    {
        L"input\\street.png",
        L"input\\tree.png"
    };

    Filters::Convolution params;

    std::vector<std::function<const wchar_t *( const ImageData &, ImageData & )>> processors
    {
        []( const ImageData & in, ImageData & out )
        {
            in.copy( out );
            return L"copy";
        },
        []( const ImageData & in, ImageData & out )
        {
            in.function( out, [&]( double, double, Color u, Color & v )
            {
                v.r = Sqrt( u.r );
                v.g = Sqrt( u.g );
                v.b = Sqrt( u.b );
                v.a = u.a;
            } );
            return L"sqrt";
        },
        []( const ImageData & in, ImageData & out )
        {
            in.function( out, [&]( double, double, Color u, Color & v )
            {
                v.r = Sqr( u.r );
                v.g = Sqr( u.g );
                v.b = Sqr( u.b );
                v.a = u.a;
            } );
            return L"sqr";
        },
        []( const ImageData & in, ImageData & out )
        {
            in.invert( out );
            return L"invert";
        },
        []( const ImageData & in, ImageData & out )
        {
            in.sub( out, 32, 32, 64, 64 );
            return L"sub0";
        },
        []( const ImageData & in, ImageData & out )
        {
            in.sub( out, 100, 100, 1000, 1000 );
            return L"sub1";
        },
        []( const ImageData & in, ImageData & out )
        {
            in.sub( out, 64, 32, 32, 64 );
            return L"sub2_incorrect_rectangle";
        },
        []( const ImageData & in, ImageData & out )
        {
            in.copy( out );
            out.sub( out, 64, 64, 128, 128 );
            return L"sub3_applying_to_self";
        },
        []( const ImageData & in, ImageData & out )
        {
            in.shiftRGB( out, 0, 0, 10, 10, 20, 20 );
            return L"shiftRGB";
        },
        []( const ImageData & in, ImageData & out )
        {
            in.flipX( out );
            return L"flipX";
        },
        []( const ImageData & in, ImageData & out )
        {
            in.flipY( out );
            return L"flipY";
        },
        []( const ImageData & in, ImageData & out )
        {
            in.transpose( out );
            return L"transpose";
        },
        []( const ImageData & in, ImageData & out )
        {
            in.function( out, Filters::fiveSectors );
            return L"fiveSectors";
        },
        []( const ImageData & in, ImageData & out )
        {
            in.function( out, Filters::function0 );
            return L"function0";
        },
        []( const ImageData & in, ImageData & out )
        {
            in.function( out, Filters::function1 );
            return L"function1";
        },
        []( const ImageData & in, ImageData & out )
        {
            in.function( out, Filters::function2 );
            return L"function2";
        },
        []( const ImageData & in, ImageData & out )
        {
            in.function( out, Filters::function3 );
            return L"function3";
        },
        []( const ImageData & in, ImageData & out )
        {
            in.function( out, Filters::gray );
            return L"gray";
        },
        []( const ImageData & in, ImageData & out )
        {
            in.function( out, Filters::noize );
            return L"noize";
        },
        []( const ImageData & in, ImageData & out )
        {
            in.function( out, Filters::grayOut );
            return L"grayOut";
        },
        []( const ImageData & in, ImageData & out )
        {
            in.function( out, Filters::rainbowPie );
            return L"rainbowPie";
        },
        [&params]( const ImageData & in, ImageData & out )
        {
            params.blurGaussian( 3, 0.849322 );
            Filters::convolution( params, in, out );
            return L"blurGaussian3";
        },
        []( const ImageData & in, ImageData & out )
        {
            ApplyKernel( in, out );
            return L"ApplyKernel";
        },
        []( const ImageData & in, ImageData & out )
        {
            Outline( in, out );
            return L"Outline";
        },
        [&params]( const ImageData & in, ImageData & out )
        {
            params.outline( true );
            Filters::convolution( params, in, out );
            return L"outlineHorizontal";
        },
        [&params]( const ImageData & in, ImageData & out )
        {
            params.outline( false );
            Filters::convolution( params, in, out );
            return L"outlineVertical";
        },
        [&params]( const ImageData & in, ImageData & out )
        {
            params.operatorSobel( true );
            Filters::convolution( params, in, out );
            return L"operatorSobelHorizontal";
        },
        [&params]( const ImageData & in, ImageData & out )
        {
            params.operatorSobel( false );
            Filters::convolution( params, in, out );
            return L"operatorSobelVertical";
        },
        [&params]( const ImageData & in, ImageData & out )
        {
            params.sharpening();
            Filters::convolution( params, in, out );
            return L"sharpening";
        },
        [&params]( const ImageData & in, ImageData & out )
        {
            params.emboss();
            Filters::convolution( params, in, out );
            return L"emboss";
        },
        [&params]( const ImageData & in, ImageData & out )
        {
            params.motionBlur();
            Filters::convolution( params, in, out );
            return L"motionBlur";
        },
        [&params]( const ImageData & in, ImageData & out )
        {
            params.swirl();
            Filters::convolution( params, in, out );
            return L"swirl";
        },
        [&params]( const ImageData & in, ImageData & out )
        {
            params.highPass();
            Filters::convolution( params, in, out );
            return L"highPass";
        },
        [&params]( const ImageData & in, ImageData & out )
        {
            params.checkerboard();
            Filters::convolution( params, in, out );
            return L"checkerboard";
        },
        [&ico]( const ImageData & in, ImageData & out )
        {
            in.copy( out );
            ico.place( out, -8, -8 );
            return L"place-8-8";
        },
        [&ico]( const ImageData & in, ImageData & out )
        {
            in.copy( out );
            ico.place( out, 3, 4 );
            return L"place+3+4";
        },
        [&ico]( const ImageData & in, ImageData & out )
        {
            in.copy( out );
            ico.place( out, out.w() - 24, out.h() - 24 );
            return L"place_w-24h-24";
        },
        [&ico]( const ImageData & in, ImageData & out )
        {
            in.copy( out );
            ico.place( out, -8, out.h() - 24 );
            return L"place-8h-24";
        },
        [&ico]( const ImageData & in, ImageData & out )
        {
            in.copy( out );
            ico.place( out, out.w() - 24, -8 );
            return L"place_w-24h-8";
        }
    };

    auto demonstrateAndSave = [&]( const std::wstring & name )
    {
        text << name << L"\n";
        if( showImages )
        {
            ImageWindow window( output, nullptr );
            window.run();
        }
        if( writeDisk )
            output.output( context.Output() / ( name + L".png" ) );
    };

    for( const auto &path : paths )
    {
        if( !input.input( path ) )
            continue;

        for( const auto &processor : processors )
            demonstrateAndSave( processor( input, output ) );

        if( !additional )
            break;
    }
}
