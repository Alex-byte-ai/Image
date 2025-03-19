#include "ImageData.h"

#include <algorithm>
#include <fstream>

#include "GetPathToFile.h"
#include "ImageConvert.h"
#include "BasicMath.h"
#include "Lambda.h"

#include "GetImage.h"
#include "Ellipse.h"
#include "Line.h"

// https://en.wikipedia.org/wiki/ICO_(file_format)

std::map<const ImageData *, std::wstring> ImageData::errors;

struct DDS_PIXELFORMAT
{
    uint32_t dwSize;
    uint32_t dwFlags;
    uint32_t dwFourCC;
    uint32_t dwRGBBitCount;
    uint32_t dwRBitMask;
    uint32_t dwGBitMask;
    uint32_t dwBBitMask;
    uint32_t dwABitMask;
};

struct DDS_HEADER
{
    uint32_t        dwSize;
    uint32_t        dwFlags;
    uint32_t        dwHeight;
    uint32_t        dwWidth;
    uint32_t        dwPitchOrLinearSize;
    uint32_t        dwDepth;
    uint32_t        dwMipMapCount;
    uint32_t        dwReserved1[11];
    DDS_PIXELFORMAT ddspf;
    uint32_t        dwCaps;
    uint32_t        dwCaps2;
    uint32_t        dwCaps3;
    uint32_t        dwCaps4;
    uint32_t        dwReserved2;
};

struct ICONDIRENTRY
{
    uint8_t  bWidth;
    uint8_t  bHeight;
    uint8_t  bColorCount;
    uint8_t  bReserved;
    uint16_t wPlanes;
    uint16_t wBitCount;
    uint32_t dwBytesInRes;
    uint32_t dwImageOffset;
};

struct ICONDIR
{
    uint16_t idReserved;
    uint16_t idType;
    uint16_t idCount;
    //ICONDIRENTRY idEntries[1];
};

void ImageData::errorText()
{
    std::wstring error;

    auto i = errors.find( nullptr );
    if( i != errors.end() )
    {
        error = i->second;
        if( error.length() != 0 )
        {
            errorImage();
            return;
        }
    }

    i = errors.find( this );
    if( i == errors.end() )
        return;

    error = i->second;

    reset( 256, 256 );
    TextGraphics t;
    t.set( L"text", error );
    t.set( L"x", L"0" );
    t.set( L"y", L"0" );
    t.set( L"text.red", L"255" );
    t.set( L"text.green", L"0" );
    t.set( L"text.blue", L"0" );
    text( t );
}

void ImageData::errorImage()
{
    Pixel *po;
    int i, j;

    reset( 32, 32 );
    for( j = 0; j < w(); ++j )
    {
        for( i = 0; i < h(); ++i )
        {
            po = ( *this )( j, i );
            if( ( i == j ) || ( i + j == 31 ) )
            {
                po->r = 255;
            }
            else
            {
                po->r = 0;
            }
            po->g = 0;
            po->b = 0;
            po->a = 255;
        }
    }
}

void ImageData::errorMsg( ImageData *image, const std::wstring &e )
{
    auto i = errors.find( image );
    if( i != errors.end() )
    {
        i->second += e + L"\n";
    }
    else
    {
        errors.insert( { image, e + L"\n" } );
    }

    if( image )
        image->errorText();
}

void ImageData::errorMsg( const std::wstring &e )
{
    errorMsg( this, e );
}

std::wstring ImageData::getError() const
{
    std::wstring e;

    auto i = errors.find( nullptr );
    if( i != errors.end() )
        e += i->second;

    i = errors.find( this );
    if( i != errors.end() )
        e += i->second;

    return e;
}

ImageData::ImageData()
{}

ImageData::ImageData( int w, int h )
{
    reset( w, h );
}

ImageData::~ImageData()
{}

bool ImageData::input( const std::filesystem::path &path )
{
    try
    {
        std::ifstream file( path, std::ios::binary );
        if( !file )
            return false;

        file.seekg( 0, std::ios::end );
        auto size = ( size_t )file.tellg();
        file.seekg( 0, std::ios::beg );

        ImageConvert::Reference image;
        image.fill();
        image.format = ".ANYF";
        image.bytes = size;
        image.reset( image );

        if( !file.read( ( char * )image.link, size ) )
            return false;
        file.close();

        ImageConvert::Reference self;
        makeReference( *this, self );

        translate( image, self, false );

        if( setStride( false ) )
            flipY( *this );

        return true;
    }
    catch( ... )
    {}
    return false;
}

bool ImageData::output( const std::filesystem::path &path ) const
{
    try
    {
        auto ext = path.extension().string();
        std::transform( ext.begin(), ext.end(), ext.begin(), []( char c )
        {
            return std::toupper( c );
        } );

        ImageConvert::Reference image;
        image.fill();
        image.format = ext;

        ImageConvert::Reference self;
        makeReference( *const_cast<ImageData *>( this ), self );

        translate( self, image, false );

        std::filesystem::create_directories( path.parent_path() );
        std::ofstream file( path, std::ios::binary );
        if( !file )
            return false;
        if( !file.write( ( char * )image.link, image.bytes ) )
            return false;
        return true;
    }
    catch( ... )
    {}
    return false;
}

bool ImageData::input()
{
    auto path = OpenPath();
    if( !path )
        return false;
    return input( *path );
}

bool ImageData::output() const
{
    auto path = SavePath();
    if( !path )
        return false;
    return output( *path );
}

bool ImageData::readDDS( const std::filesystem::path &path, std::vector<ImageData> &images )
{
    Finalizer _;

    unsigned i, j, k, w, h;
    DDS_HEADER header;
    uint32_t dwDds;
    Pixel *cur;
    FILE *f;

    f = _wfopen( path.c_str(), L"rb" );
    if( !f )
        return false;

    _.push( [f]()
    {
        fclose( f );
    } );

    fread( &dwDds, sizeof( dwDds ), 1, f );
    fread( &header, sizeof( header ), 1, f );

    if( !(
                ( dwDds == 0x20534444 ) &&
                ( header.ddspf.dwFlags == 0x41 ) &&
                ( header.ddspf.dwRGBBitCount == 32 ) &&
                ( header.dwFlags == 0x80100f ) &&
                ( header.dwCaps == 0x1000 ) &&
                ( header.dwCaps2 == 0x200000 )
            ) ) return false;

    auto size = header.dwDepth;
    images.resize( size );

    w = header.dwWidth;
    h = header.dwHeight;
    for( k = 0; k < size; ++k )
    {
        images[k].reset( w, h );
        for( i = 0; i < h; ++i )
        {
            for( j = 0; j < w; ++j )
            {
                cur = images[k]( j, h - i - 1 );
                fread( &cur->r, sizeof( cur->r ), 1, f );
                fread( &cur->g, sizeof( cur->g ), 1, f );
                fread( &cur->b, sizeof( cur->b ), 1, f );
                fread( &cur->a, sizeof( cur->a ), 1, f );
            }
        }
    }

    return true;
}

bool ImageData::writeDDS( const std::filesystem::path &fname, const std::vector<ImageData> &img )
{
    Finalizer _;

    DDS_HEADER header;
    const Pixel *cur;
    uint32_t dwDds;
    int i, j, w, h;
    size_t k;
    FILE *f;

    auto size = img.size();
    if( size <= 0 )
        return false;

    w = img[0].w();
    h = img[0].h();

    for( k = 0; k < size; ++k )
    {
        if( img[k].w() != w || img[k].h() != h )
            return false;
    }

    dwDds = 0x20534444;
    header.dwSize = sizeof( header );
    header.dwFlags = 0x80100f;
    header.dwHeight = h;
    header.dwWidth = w;
    header.dwPitchOrLinearSize = ( header.dwWidth * 32 + 7 ) / 8;
    header.dwDepth = size;
    header.dwMipMapCount = 1;
    header.dwReserved1[0] = 0;
    header.dwReserved1[1] = 0;
    header.dwReserved1[2] = 0;
    header.dwReserved1[3] = 0;
    header.dwReserved1[4] = 0;
    header.dwReserved1[5] = 0;
    header.dwReserved1[6] = 0;
    header.dwReserved1[7] = 0;
    header.dwReserved1[8] = 0;
    header.dwReserved1[9] = 0;
    header.dwReserved1[10] = 0;
    header.ddspf.dwSize = 32;
    header.ddspf.dwFlags = 0x41;
    header.ddspf.dwFourCC = 0;
    header.ddspf.dwRGBBitCount = 32;
    header.ddspf.dwRBitMask = 0x000000ff;
    header.ddspf.dwGBitMask = 0x0000ff00;
    header.ddspf.dwBBitMask = 0x00ff0000;
    header.ddspf.dwABitMask = 0xff000000;
    header.dwCaps = 0x1000;
    header.dwCaps2 = 0x200000;
    header.dwCaps3 = 0;
    header.dwCaps4 = 0;
    header.dwReserved2 = 0;

    std::filesystem::create_directories( fname.parent_path() );

    f = _wfopen( fname.c_str(), L"wb" );
    if( !f )
        return false;

    _.push( [f]()
    {
        fclose( f );
    } );

    fwrite( &dwDds, sizeof( dwDds ), 1, f );
    fwrite( &header, sizeof( header ), 1, f );

    for( k = 0; k < size; ++k )
    {
        for( i = 0; i < h; ++i )
        {
            for( j = 0; j < w; ++j )
            {
                cur = img[k]( j, h - i - 1 );
                fwrite( &cur->r, sizeof( cur->r ), 1, f );
                fwrite( &cur->g, sizeof( cur->g ), 1, f );
                fwrite( &cur->b, sizeof( cur->b ), 1, f );
                fwrite( &cur->a, sizeof( cur->a ), 1, f );
            }
        }
    }

    return true;
}

bool ImageData::readICO( const std::filesystem::path &path, std::vector<ImageData> &images )
{
    Finalizer _;

    std::vector<std::vector<uint8_t>> masks;
    ICONDIRENTRY *entry;
    ICONDIR ico;
    FILE *f;
    int k;

    f = _wfopen( path.c_str(), L"rb" );
    if( !f )
        return false;

    _.push( [f]()
    {
        fclose( f );
    } );

    fread( &ico, 1, sizeof( ico ), f );
    if( !( ( ico.idReserved == 0 ) && ( ico.idType == 1 ) && ( ico.idCount >= 1 ) ) )
        return false;

    auto size = ico.idCount;
    images.resize( size );
    masks.resize( size );

    entry = new ICONDIRENTRY[size];
    _.push( [entry]()
    {
        delete[] entry;
    } );

    fread( entry, size, sizeof( ICONDIRENTRY ), f );

    for( k = 0; k < size; ++k )
    {
        if( ftello64( f ) != entry[k].dwImageOffset )
        {
            if( fseek( f, entry[k].dwImageOffset, SEEK_SET ) != 0 )
                return false;
        }

        auto partBytes = entry[k].dwBytesInRes;
        auto partLink = new uint8_t[partBytes];
        fread( partLink, partBytes, 1, f );

        ImageConvert::Reference part, icoImage, icoMask, image, mask;

        part.bytes = partBytes;
        part.link = partLink;

        makeException( partBytes >= 12 );

        uint32_t biSize;
        ::copy( &biSize, partLink, sizeof( biSize ) );
        bool isPng = biSize == 0x474E5089;

        if( !isPng )
        {
            // Transparency mask is included in bitmap's height, but is not organic part of BMP format
            // Gonna modify it back to image size, so this chunk is separated into proper BMP and a transparency mask
            int32_t  biHeight;
            ::copy( &biHeight, partLink + 8, sizeof( biHeight ) );
            biHeight /= 2;
            ::copy( partLink + 8, &biHeight, sizeof( biHeight ) );
        }

        makeReference( images[k], image );

        mask.reset = [&]( ImageConvert::Reference & ref )
        {
            masks[k].resize( ref.bytes );
            ref.link = masks[k].data();
            return true;
        };
        mask.format = "G8";

        icoImage.fill();
        icoImage.format = isPng ? ".PNG*SAME" : ".DIB*SAME";

        icoMask.fill();
        icoMask.format = "G1*PAD4*SAME";

        part.format = icoImage.format;
        translate( part, icoImage, false );
        translate( icoImage, image, false );

        if( !isPng )
        {
            part.format = icoMask.format;
            part.w = image.w;
            part.h = image.h;
            part.link = ( uint8_t * )part.link + icoImage.bytes;
            part.bytes = part.bytes - icoImage.bytes;

            translate( part, icoMask, false );
            translate( icoMask, mask, false );
        }

        bool opaque = true, opaqueMasked = false;

        auto applyMask = [&]( int w, int, int j, int i, Pixel x, Pixel & y )
        {
            auto m = masks[k][ i * w + j ];

            y = x;
            opaque = opaque && y.a >= 255;
            if( m > 0 )
            {
                makeException( y.a <= 0 || y.a >= 255 );
                if( y.a >= 255 )
                    opaqueMasked = true;
                y.a = 0;
            }
        };
        if( !isPng )
            images[k].function( images[k], applyMask );

        makeException( opaque || !opaqueMasked );
    }

    return true;
}

bool ImageData::writeICO( const std::filesystem::path &path, const std::vector<ImageData> &images )
{
    Finalizer _;

    std::vector<uint8_t> data;
    unsigned k, w, h, offset;
    ICONDIRENTRY *entry;
    ICONDIR ico;
    FILE *f;

    auto size = images.size();
    if( size <= 0 )
        return false;

    ico.idReserved = 0;
    ico.idType = 1;
    ico.idCount = size;

    entry = new ICONDIRENTRY[size];
    _.push( [entry]()
    {
        delete[] entry;
    } );

    offset = sizeof( ico ) + size * sizeof( ICONDIRENTRY );

    for( k = 0; k < size; ++k )
    {
        w = images[k].w();
        h = images[k].h();

        ImageConvert::Reference part, image;
        part.fill();
        part.format = ".PNG";

        makeReference( *const_cast<ImageData *>( &images[k] ), image );

        translate( image, part, false );

        auto position = data.size();
        data.resize( position + part.bytes );
        ::copy( data.data() + position, part.link, part.bytes );

        entry[k].bWidth = w % 256;
        entry[k].bHeight = h % 256;
        entry[k].bColorCount = 0;
        entry[k].bReserved = 0;
        entry[k].wPlanes = 1;
        entry[k].wBitCount = 32;
        entry[k].dwBytesInRes = part.bytes;
        entry[k].dwImageOffset = offset;

        offset += part.bytes;
    }

    std::filesystem::create_directories( path.parent_path() );

    f = _wfopen( path.c_str(), L"wb" );
    if( !f )
        return false;

    fwrite( &ico, 1, sizeof( ico ), f );
    fwrite( entry, size, sizeof( ICONDIRENTRY ), f );
    fwrite( data.data(), data.size(), 1, f );
    fclose( f );

    return true;
}

void ImageData::rectangle( int x0, int y0, int right, int bottom, Pixel p )
{
    int i, j;

    right += x0;
    bottom += y0;

    if( x0 < 0 )   x0 = 0;
    if( y0 < 0 )   y0 = 0;
    if( x0 > w() ) x0 = w();
    if( y0 > h() ) y0 = h();
    if( right < 0 )    right = 0;
    if( bottom < 0 )   bottom = 0;
    if( right > w() )  right = w();
    if( bottom > h() ) bottom = h();

    for( j = x0; j < right; ++j )
    {
        for( i = y0; i < bottom; ++i )
        {
            *( *this )( j, i ) = p;
        }
    }
}

void ImageData::line( int x0, int y0, int x1, int y1, Pixel p )
{
    DrawLine l( x0, y0, x1, y1 );
    Pixel *output;

    while( !l.isFinished() )
    {
        output = ( *this )( l.x(), l.y() );
        if( output )
            *output = p;
        l.nextPixel();
    }
}

void ImageData::circle( int x0, int y0, int r, Pixel p )
{
    int i, j, mini, minj, maxi, maxj, r2;

    mini = y0 - r;
    if( mini < 0 )
        mini = 0;

    minj = x0 - r;
    if( minj < 0 )
        minj = 0;

    maxj = x0 + r;
    if( maxj > w() )
        maxj = w();

    maxi = y0 + r;
    if( maxi > h() )
        maxi = h();

    r2 = Sqr( r );

    for( j = minj; j < maxj; ++j )
    {
        for( i = mini; i < maxi; ++i )
        {
            if( Sqr( j - x0 ) + Sqr( i - y0 ) < r2 )
            {
                *( *this )( j, i ) = p;
            }
        }
    }
}

void ImageData::ellipse( int x0, int y0, int rx, int ry, Pixel p )
{
    /*
    // ???
    DrawEllipse e( x0, y0, rx, ry );
    Pixel *output;

    while( !e.isFinished() )
    {
        output = ( *this )( e.x(), e.y() );
        if( output )
            *output = p;
        e.nextPixel();
    }
    */

    int i, j, mini, minj, maxi, maxj;

    mini = y0 - ry;
    if( mini < 0 )
        mini = 0;

    minj = x0 - rx;
    if( minj < 0 )
        minj = 0;

    maxj = x0 + rx;
    if( maxj > w() )
        maxj = w();

    maxi = y0 + ry;
    if( maxi > h() )
        maxi = h();

    for( j = minj; j < maxj; ++j )
    {
        for( i = mini; i < maxi; ++i )
        {
            if( Sqr( ( double )( j - x0 ) / rx ) + Sqr( ( double )( i - y0 ) / ry ) < 1 )
            {
                *( *this )( j, i ) = p;
            }
        }
    }
}

void ImageData::text( const TextBase &text )
{
    if( !text( *this ) )
        errorMsg( nullptr, L"Can't draw the text." );
}

void ImageData::invert( ImageDataBase &imageDataBase ) const
{
    Pixel pi, *po;
    int i, j;

    auto o = dynamic_cast<ImageData *>( &imageDataBase );
    if( !o )
        return;

    auto &output = *o;
    if( ( output.w() != w() ) || ( output.h() != h() ) )
        output.reset( w(), h() );

    for( j = 0; j < w(); ++j )
    {
        for( i = 0; i < h(); ++i )
        {
            pi = *( *this )( j, i );
            po = output( j, i );

            *po = pi.invert();
        }
    }
}

void ImageData::shiftRGB( ImageDataBase &imageDataBase, int rx, int ry, int gx, int gy, int bx, int by ) const
{
    Pixel pi, *rpo, *gpo, *bpo;
    int i, j, mx, my;

    auto o = dynamic_cast<ImageData *>( &imageDataBase );
    if( !o )
        return;

    if( o == this )
    {
        o->errorMsg( L"shiftRGB failed." );
        return;
    }

    auto &output = *o;

    mx = rx;
    if( mx < gx )
        mx = gx;
    if( mx < bx )
        mx = bx;

    my = ry;
    if( my < gy )
        my = gy;
    if( my < by )
        my = by;

    output.reset( w() + mx, h() + my, Pixel( 0, 0, 0 ) );

    for( j = 0; j < w(); ++j )
    {
        for( i = 0; i < h(); ++i )
        {
            pi = *( *this )( j, i );
            rpo = output( j + rx, i + ry );
            gpo = output( j + gx, i + gy );
            bpo = output( j + bx, i + by );

            rpo->r = pi.r;
            gpo->g = pi.g;
            bpo->b = pi.b;
        }
    }
}

void ImageData::function( ImageDataBase &out, const std::function<void( double, double, const Color &, Color & )> &f ) const
{
    Pixel pi, *po;
    int i, j;
    Color z;

    if( ( out.w() != w() ) || ( out.h() != h() ) )
        out.reset( w(), h() );

    for( j = 0; j < w(); ++j )
    {
        for( i = 0; i < h(); ++i )
        {
            pi = *( *this )( j, i );
            po = out( j, i );

            f( ( j + 0.5 ) / w() - 0.5, ( i + 0.5 ) / h() - 0.5, ( Color )pi, z );
            *po = ( Pixel )z;
        }
    }
}

void ImageData::function( ImageDataBase &out, const std::function<void( int, int, int, int, const Pixel &, Pixel & )> &f ) const
{
    transform( out, f );
}

void ImageData::placeTransperent( ImageDataBase &imageDataBase, int x, int y ) const
{
    int i, j, mini, minj, maxi, maxj;
    Pixel pi, *po;

    auto o = dynamic_cast<ImageData *>( &imageDataBase );
    if( !o )
        return;

    if( o == this )
    {
        o->errorMsg( L"placeTransperent failed." );
        return;
    }

    auto &output = *o;

    mini = y;
    if( mini < 0 )
        mini = 0;

    minj = x;
    if( minj < 0 )
        minj = 0;

    maxj = x + w();
    if( maxj > output.w() )
        maxj = output.w();

    maxi = y + h();
    if( maxi > output.h() )
        maxi = output.h();

    for( j = minj; j < maxj; ++j )
    {
        for( i = mini; i < maxi; ++i )
        {
            pi = *( *this )( j - x, i - y );
            po = output( j, i );
            *po = ( Pixel )( ( ( Color ) * po ).layer( ( Color )pi ) );
        }
    }
}
