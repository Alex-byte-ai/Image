#pragma once

#include <windows.h>

#include <vector>
#include <cstdint>
#include <optional>

#include "Buffer.h"

#include "ImageDataBase.h"

class BitmapTools
{
public:
    BitmapTools( ImageDataBase &image );
    ~BitmapTools();

    void setCropping( int x0, int y0, int x1, int y1 );
    void setCropping( int x0, int y0, int x1, int y1, const Pixel &background );

    [[nodiscard]] bool operator<<( HDC hdc );
    [[nodiscard]] bool operator>>( HDC hdc ) const;

    [[nodiscard]] bool operator<<( HBITMAP hBitmap );
    [[nodiscard]] bool operator>>( HBITMAP hBitmap ) const;

    [[nodiscard]] bool operator<<( const ImageDataBase &other );
    [[nodiscard]] bool operator>>( ImageDataBase &other ) const;

    [[nodiscard]] inline bool fits( int w, int h ) const
    {
        return image.w() == w && image.h() == h;
    }

    static void init( BITMAPINFOHEADER &bmiHeader, int w, int h );
    static void init( BITMAPINFO &bmpInfo, int w, int h );
private:
    ImageDataBase &image;

    struct Cropping
    {
        int x0, y0, x1, y1;
        std::optional<Pixel> background;
    };
    std::optional<Cropping> cropping;

    void crop();
};
