#include "Test_23_Translate.h"

#include <algorithm>
#include <fstream>
#include <set>

#include "Image/Translate.h"
#include "Window.h"
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

    if( !readDisk )
        return;

    ImageData image;

    struct Compare
    {
        bool operator()( const std::filesystem::path& a, const std::filesystem::path& b ) const
        {
            return compare( a.wstring().c_str(), b.wstring().c_str() );
        }
    };

    struct Image
    {
        const std::filesystem::path& path;
        std::vector<uint8_t> content;

        Image( const Image& other ) : path( other.path ), content( other.content )
        {}

        Image( Image&& other ) : path( other.path ), content( std::move( other.content ) )
        {}

        Image( const std::filesystem::path& p, std::vector<uint8_t>&& c ) : path( p ), content( c )
        {}
    };

    std::map<std::filesystem::path, size_t, Compare> index;
    std::vector<Image> samples;
    size_t volume = 0;

    auto get = [&]( std::filesystem::path path, bool load = true )
    {
        std::vector<uint8_t> content;
        if( load )
        {
            std::ifstream file( path, std::ios::binary );

            file.seekg( 0, std::ios::end );
            auto size = ( size_t )file.tellg();
            file.seekg( 0, std::ios::beg );

            volume += size;

            content.resize( size );
            makeException( file.read( ( char * )content.data(), size ) );
        }

        auto i = index.find( path );
        if( i == index.end() )
        {
            auto& p = index.emplace( std::move( path ), samples.size() ).first->first;
            samples.emplace_back( p, std::move( content ) );
        }
        else
        {
            samples[i->second].content = std::move( content );
        }
    };

    auto unload = [&]()
    {
        if( volume <= 100000 )
            return false;

        volume = 0;
        for( auto& sample : samples )
            sample.content.clear();

        return true;
    };

    auto erase = [&]()
    {
        samples.clear();
        index.clear();
        volume = 0;
    };

    auto sort = [&]()
    {
        std::vector<Image> sortSamples;
        sortSamples.reserve( samples.size() );

        size_t i = 0;
        for( auto& [path, id] : index )
        {
            sortSamples.emplace_back( std::move( samples[id] ) );
            id = i++;
        }

        samples = std::move( sortSamples );
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
    int width = 128, height = 128;
    bool scale = true;

    auto redrawBase = [&]()
    {
        auto& sample = samples[sId];
        auto& content = sample.content;

        unload();
        if( content.empty() )
            get( sample.path );

        input.link = content.data();
        input.bytes = content.size();

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
            text << L"Currently displaying: " << samples[sId].path.native() << L"\n";
        }
        catch( const Exception &e )
        {
            generateErrorImage( Pixel( 0, 255, 0 ) );
            text << L"Can't display: " << samples[sId].path.native() << L"\n";
            ++text;
            text << L"Error: " << e.message() << L"\n";
            --text;
        }
        catch( const std::exception &e )
        {
            generateErrorImage( Pixel( 255, 0, 0 ) );
            text << L"Can't display: " << samples[sId].path.native() << L"\n";
            ++text;
            text << L"Error: " << e.what() << L"\n";
            --text;
        }
        catch( ... )
        {
            generateErrorImage( Pixel( 255, 0, 255 ) );
            text << L"Can't display: " << samples[sId].path.native() << L"\n";
            ++text;
            text << L"Unknown error.\n";
            --text;
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
            sId = ( sId + 1 ) % samples.size();
            redraw();
        }

        if( keyDown( 'Q' ) )
        {
            sId = ( sId + samples.size() - 1 ) % samples.size();
            redraw();
        }

        if( keyDown( 'R' ) )
        {
            redraw();
        }

        if( sId == 0 && keyDown( 'C' ) )
        {
            sId = 0;
            do
            {
                ++sId;
                redraw();
            }
            while( sId + 1 < samples.size() );
        }

        if( keyDown( 'B' ) )
        {
            scale = !scale;
            redraw();
        }

        if( inputVariableData && keyDown( 'L' ) )
        {
            auto p = openPath();
            if( p )
            {
                std::set<std::filesystem::path> folders;
                /*
                for( const auto &sample : samples )
                    folders.insert( sample.path.parent_path() );
                */
                folders.insert( p->parent_path() );

                std::set<std::filesystem::path> files;
                for( const auto &path : folders )
                {
                    for( const auto &file : std::filesystem::recursive_directory_iterator( path ) )
                    {
                        std::filesystem::path f = file;
                        if( f.extension() == L".bmp" || f.extension() == L".rle" || f.extension() == L".dib" || f.extension() == L".jpg" || f.extension() == L".jpeg" || f.extension() == L".png" )
                            files.emplace( std::move( f ) );
                    }
                }

                erase();
                for( auto &file : files )
                    get( file, false );
                sort();

                sId = 0;
                redraw();
            }
        }

        if( writeDisk && outputVariableData && keyDown( 'F' ) && *inputData.keys.letter( 'S' ) )
        {
            auto path = savePath();
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
}
