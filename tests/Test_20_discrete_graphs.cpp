#include "Test_20_discrete_graphs.h"

#include "Basic.h"

#include "../ImageWindow.h"
#include "../ImageData.h"

void Test_20_discrete_graphs( Context &context )
{
    auto &text = context.output();
    auto s = context.scope( __FUNCTION__ );

    auto &info = context.information;
    bool additional = info( L"additional" ).as<bool>();
    bool showImages = info( L"showImages" ).as<bool>();

    // q = Div( a, b )
    // r = Mod( a, b )
    // a = b * q + r

    ImageData image;

    int b = 7;

    auto half = Abs( b * 6 );
    auto scalingFactor = 5;
    auto coreSize = 2 * half + 1;
    auto size = scalingFactor * coreSize;

    auto squish = []( double x )
    {
        makeException( isNumber( x ) );

        auto u = Exp( x );
        auto v = Exp( -x );

        if( isInfinite( u ) )
            return 1.0;
        if( isInfinite( v ) )
            return -1.0;

        return ( u - v ) / ( u + v );
    };

    std::vector<std::pair<std::function<std::wstring()>, std::function<double( int, int )>>> functions =
    {
        {
            [&]()
            {
                std::wstring result = L"y = x % ";
                if( b < 0 )
                    result += L"(";
                result += std::to_wstring( b );
                if( b < 0 )
                    result += L")";
                return result;
            },
            [&]( int x, int y )
            {
                bool result = y == x % b;
                return result ? 10 : 0;
            }
        },
        {
            [&]()
            {
                std::wstring result = L"y = x / ";
                if( b < 0 )
                    result += L"(";
                result += std::to_wstring( b );
                if( b < 0 )
                    result += L")";
                return result;
            },
            [&]( int x, int y )
            {
                bool result = y == x / b;
                return result ? 10 : 0;
            }
        },
        {
            [&]()
            {
                std::wstring result = L"y = Mod( x, ";
                result += std::to_wstring( b );
                result += L" )";
                return result;
            },
            [&]( int x, int y )
            {
                bool result = y == Mod( x, b );
                return result ? 10 : 0;
            }
        },
        {
            [&]()
            {
                std::wstring result = L"y = Div( x, ";
                result += std::to_wstring( b );
                result += L" )";
                return result;
            },
            [&]( int x, int y )
            {
                bool result = y == Div( x, b );
                return result ? 10 : 0;
            }
        }
    };

    if( additional )
    {
        functions.push_back(
        {
            [&]()
            {
                return L"...";
            },
            [&]( int x, int y )
            {
                auto u = 2 * y * y + 3 * x;
                auto v = y - x * x;
                return ( v != 0 ) ? ( u % v ) * Sign( u ) * Sign( v ) : 0;
            }
        } );
    }

    int id = 0;

    auto drawGraph = [&]( ImageDataBase & img )
    {
        std::vector<std::vector<Pixel>> results( coreSize, std::vector<Pixel>( coreSize ) );
        for( auto x = -half; x <= half; ++x )
        {
            for( auto y = -half; y <= half; ++y )
            {
                auto &output = results[x + half][y + half];

                auto result = squish( functions[id].second( x, y ) );
                if( result >= 0 )
                    output = Pixel( 0, Round( 255 * result ), 0 );
                else
                    output = Pixel( Round( -255 * result ), 0, 0 );

                if( x == 0 || y == 0 )
                {
                    Color v = ( Color )output, u;
                    u = v.invert();
                    u.r *= 0.25;
                    u.g *= 0.25;
                    u.b *= 0.25;
                    u = u.invert();
                    u.a = v.a;
                    output = ( Pixel )u;
                }
            }
        }

        img.reset( size, size, Pixel( 0, 0, 0 ) );
        img.function( img, [&]( int, int, int j, int i, const Pixel &, Pixel & output )
        {
            int x = j / scalingFactor - half;
            int y = half - i / scalingFactor;
            output = results[x + half][y + half];
        } );

        std::wstring dsc = functions[id].first();
        text << dsc << L"\n";

        TextGraphics description;
        description.set( L"text", dsc );
        description.set( L"height", L"32" );
        img.text( description );
    };

    drawGraph( image );

    auto graph = [&]( const ImageWindow::InputData & inputData, ImageWindow::OutputData & outputData )
    {
        auto keyDown = [&]( char symbol )
        {
            auto &key = inputData.keys.letter( symbol );
            return key.changed() && *key;
        };

        if( keyDown( 'A' ) )
        {
            id = ( id + 1 ) % functions.size();
            drawGraph( outputData.image.get() );
        }

        if( keyDown( 'S' ) )
        {
            b = -b;
            drawGraph( outputData.image.get() );
        }
    };

    if( showImages )
    {
        ImageWindow window( image, graph );
        window.run();
    }
}
