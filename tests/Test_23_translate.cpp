#include "Test_23_Translate.h"

#include <algorithm>
#include <fstream>
#include <set>

#include "Image/Translate.h"
#include "GetPathToFile.h"
#include "Basic.h"

#include "../ImageWindow.h"
#include "../ImageData.h"

void Test_23_Translate( Context &context )
{
    auto &text = context.output();
    auto s = context.scope( __FUNCTION__ );

    auto &info = context.information;
    bool readDisk = info( L"readDisk" ).as<bool>();
    bool writeDisk = info( L"writeDisk" ).as<bool>();
    bool showImages = info( L"showImages" ).as<bool>();
    bool inputVariableData = info( L"inputVariableData" ).as<bool>();
    bool outputVariableData = info( L"outputVariableData" ).as<bool>();
    // ???

    if( !readDisk )
        return;

    ImageData image;

    std::vector<std::vector<uint8_t>> samples;

    auto get = [&samples]( const std::filesystem::path & path )
    {
        std::ifstream file( path, std::ios::binary );

        file.seekg( 0, std::ios::end );
        auto size = ( size_t )file.tellg();
        file.seekg( 0, std::ios::beg );

        auto &sample = samples.emplace_back();
        sample.resize( size );
        makeException( file.read( ( char * )sample.data(), size ) );
    };

    get( L"input/abstract_space.bmp" );
    get( L"input/abstract_space_2.bmp" );
    get( L"input/abstract_space_16.bmp" );
    get( L"input/abstract_space_256.bmp" );
    get( L"input/abstract_space.png" );

    ImageConvert::Reference input;
    input.format = ".ANYF";

    ImageConvert::Reference output;
    output.format = "B8G8R8A8*REPBG*REPRG*REPA255";

    auto reset = [&]( ImageConvert::Reference & ref )
    {
        image.reset( Abs( ref.w ), Abs( ref.h ) );
        ref.link = image.rawData()[0];
        return image.w() * image.h() * sizeof( Pixel ) >= ref.bytes;
    };

    output.reset = reset;

    std::vector<double> scales{0.33, 0.4, 0.5, 0.75, 1.0, 1.5, 2.0};
    size_t sId = 0, wId = 3, hId = 3, scount = scales.size() * 2;
    std::vector<std::filesystem::path> customSamples;
    bool scale = true, folder = false;
    int width = 128, height = 128;

    auto redrawBase = [&]()
    {
        if( folder )
        {
            samples.clear();
            get( customSamples[sId] );
        }

        input.link = samples[folder ? 0 : sId].data();
        input.bytes = samples[folder ? 0 : sId].size();

        output.w = Round( width * ( wId < scales.size() ? scales[wId] : -scales[scount - wId - 1] ) );
        output.h = Round( height * ( hId < scales.size() ? scales[hId] : -scales[scount - hId - 1] ) );

        reset( output );

        auto generateErrorImage = [&]( const Pixel & c )
        {
            image.reset( 256, 256 );
            auto checkerboard = [&c]( int, int, int j, int i, Pixel, Pixel & y )
            {
                if( j / 16 % 2 != i / 16 % 2 )
                {
                    y = c;
                }
                else
                {
                    y = Pixel( 0, 0, 0 );
                }
            };
            image.function( image, checkerboard );
        };

        try
        {
            translate( input, output, scale );
            if( folder )
                text << L"Currently displaying: " << customSamples[sId].native() << L"\n";
        }
        catch( const Exception &e )
        {
            generateErrorImage( Pixel( 0, 255, 0 ) );
            if( folder )
            {
                text << L"Can't display: " << customSamples[sId].native() << L"\n";
                ++text;
                text << L"Error: " << e.message() << L"\n";
                --text;
            }
        }
        catch( const std::exception &e )
        {
            generateErrorImage( Pixel( 255, 0, 0 ) );
            if( folder )
            {
                text << L"Can't display: " << customSamples[sId].native() << L"\n";
                ++text;
                text << L"Error: " << e.what() << L"\n";
                --text;
            }
        }
        catch( ... )
        {
            generateErrorImage( Pixel( 255, 0, 255 ) );
            if( folder )
            {
                text << L"Can't display: " << customSamples[sId].native() << L"\n";
                ++text;
                text << L"Unknown error.\n";
                --text;
            }
        }
    };

    auto resize = [&]( const ImageWindow::InputData & inputData, ImageWindow::OutputData & outputData )
    {
        auto redraw = [&]()
        {
            redrawBase();
            outputData.image.get();
        };

        auto keyDown = [&]( char symbol )
        {
            auto &key = inputData.keys.letter( symbol );
            return key.changed() && *key;
        };

        if( keyDown( 'A' ) )
        {
            wId = ( wId + scount - 1 ) % scount;
            redraw();
        }

        if( keyDown( 'D' ) )
        {
            wId = ( wId + 1 ) % scount;
            redraw();
        }

        if( keyDown( 'W' ) )
        {
            hId = ( hId + scount - 1 ) % scount;
            redraw();
        }

        if( keyDown( 'S' ) )
        {
            hId = ( hId + 1 ) % scount;
            redraw();
        }

        if( keyDown( 'E' ) )
        {
            if( folder )
                sId = ( sId + 1 ) % customSamples.size();
            else
                sId = ( sId + 1 ) % samples.size();
            redraw();
        }

        if( keyDown( 'Q' ) )
        {
            if( folder )
                sId = ( sId + customSamples.size() - 1 ) % customSamples.size();
            else
                sId = ( sId + samples.size() - 1 ) % samples.size();
            redraw();
        }

        if( keyDown( 'R' ) )
        {
            redraw();
        }

        if( inputVariableData && !folder && keyDown( 'L' ) )
        {
            auto path = OpenPath();
            if( path )
            {
                get( *path );
                customSamples.push_back( *path );
            }
        }

        if( folder && sId == 0 && keyDown( 'C' ) )
        {
            sId = 1;
            while( sId < customSamples.size() )
            {
                redrawBase();
                ++sId;
            }
            sId = customSamples.size() - 1;
        }

        if( keyDown( 'B' ) )
        {
            scale = !scale;
            redraw();
            width = output.w;
            height = output.h;
        }

        if( writeDisk && outputVariableData && keyDown( 'K' ) )
        {
            auto path = SavePath();
            if( path )
            {
                ImageConvert::Reference data;
                data.fill();

                auto ext = path->extension().string();
                std::transform( ext.begin(), ext.end(), ext.begin(), []( char c )
                {
                    return std::toupper( c );
                } );
                data.format = ext;

                output.w = image.w();
                output.h = image.h();

                translate( output, data, false );

                std::ofstream file( *path, std::ios::binary );
                makeException( file.write( ( char * )data.link, data.bytes ) );
            }
        }
    };

    if( showImages )
    {
        redrawBase();
        ImageWindow window( image, resize );
        window.run();
    }

    if( !customSamples.empty() )
    {
        std::set<std::filesystem::path> folders;
        for( const auto &customSample : customSamples )
            folders.insert( customSample.parent_path() );

        std::set<std::filesystem::path> files;
        for( const auto &path : folders )
        {
            for( const auto &file : std::filesystem::recursive_directory_iterator( path ) )
            {
                std::filesystem::path f = file;
                if( f.extension() == L".png" || f.extension() == L".bmp" || f.extension() == L".rle" || f.extension() == L".dib" )
                    files.emplace( std::move( f ) );
            }
        }

        customSamples.clear();
        for( auto &file : files )
            customSamples.emplace_back( std::move( file ) );

        sId = 0;
        folder = true;
        if( showImages )
        {
            redrawBase();
            ImageWindow window( image, resize );
            window.run();
        }
    }
}
