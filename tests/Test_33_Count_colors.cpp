#include "Test_33_Count_colors.h"

#include "../ImageData.h"

#include "Information.h"

#include "Basic.h"
#include <fstream>

void Test_33_Count_colors( Context &context )
{
    auto &text = context.output();
    auto s = context.scope( __FUNCTION__ );

    auto &info = context.information;
    bool readDisk = info( L"readDisk" ).as<bool>();
    bool writeDisk = info( L"writeDisk" ).as<bool>();

    if( !readDisk || !writeDisk )
        return;

    {
        std::vector<double> x, y;
        double value;

        ImageData result;
        result.reset( 512, 512, Pixel( 255, 255, 255 ) );
        Pixel colors[2] = {{0, 0, 0}, {0, 0, 255}};

        for( int i = 0; i < 2; ++i )
        {
            Interval<double> xr, yr;
            std::ifstream f;

            f.open( std::filesystem::path( L"input\\data" + std::to_wstring( i ) + L".txt" ) );
            while( f >> value )
            {
                x.push_back( value );
                xr.add( value );

                if( f >> value )
                {
                    y.push_back( value );
                    yr.add( value );
                }
                else
                {
                    break;
                }
            }
            f.close();

            // xr.add( 0 );
            // yr.add( 0 );

            for( auto& v : x )
                v = 511 * xr.normalize( v );

            for( auto& v : y )
                v = 511 * ( 1 - yr.normalize( v ) );

            auto size = x.size();
            if( size == y.size() && size > 1 )
            {
                for( size_t j = 0; j < size; ++j )
                {
                    result.line( x[j], 511, x[j], 500, colors[i] );
                }

                --size;
                for( size_t j = 0; j < size; ++j )
                {
                    result.line( x[j], y[j], x[j + 1], y[j + 1], colors[i] );
                    result.line( x[j], y[j], x[j + 1], y[j + 1], colors[i] );
                }

            }
        }

        result.output( context.Output() / ( L"graph.png" ) );
    }

    ImageData input;

    if( !input.input( L"input\\test.png" ) )
    {
        text << L"Can't open an image.\n";
        return;
    }

    int width = input.w();
    int height = input.h();
    std::map<Pixel, unsigned> colors;

    for( int j = 0; j < width; ++j )
    {
        for( int i = 0; i < height; ++i )
        {
            auto& pixel = *input( j, i );

            auto k = colors.find( pixel );
            if( k != colors.end() )
                ++k->second;
            else
                colors.emplace( pixel, 1 );
        }
    }

    unsigned limit = 0;
    for( auto& [pixel, count] : colors )
    {
        text << count << L" × ( " << pixel.r << L", " << pixel.g << L", " << pixel.b << L", " << pixel.a << L")\n";
        if( limit > 32 )
        {
            text << L"...\n";
            break;
        }
        ++limit;
    }
}
