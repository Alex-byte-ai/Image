#pragma once

#include <filesystem>
#include <functional>
#include <string>
#include <map>

#include "Buffer.h"

#include "ImageDataBase.h"
#include "Text.h"

class ImageData : public ImageDataBase
{
private:
    static std::map<const ImageData *, std::wstring> errors;

    void errorText();
    void errorImage();
public:
    static void errorMsg( ImageData *image, const std::wstring &e );
    void errorMsg( const std::wstring &e );
    std::wstring getError() const;

    ImageData();
    ImageData( int w, int h );
    ~ImageData();

    bool input( const std::filesystem::path &path ) override;
    bool output( const std::filesystem::path &path ) const override;

    bool input() override;
    bool output() const override;

    static bool readDDS( const std::filesystem::path &path, std::vector<ImageData> &images );
    static bool writeDDS( const std::filesystem::path &path, const std::vector<ImageData> &images );

    static bool readICO( const std::filesystem::path &path, std::vector<ImageData> &images );
    static bool writeICO( const std::filesystem::path &path, const std::vector<ImageData> &images );

    void line( int x0, int y0, int x1, int y1, const std::optional<Pixel> &contour = {} ) override;
    void rectangle( int x0, int y0, int w, int h, const std::optional<Pixel> &contour = {}, const std::optional<Pixel> &fill = {} ) override;
    void circle( int x0, int y0, int r, const std::optional<Pixel> &contour = {}, const std::optional<Pixel> &fill = {} ) override;
    void ellipse( int x0, int y0, int rx, int ry, const std::optional<Pixel> &contour = {}, const std::optional<Pixel> &fill = {}, double angle0 = -Pi(), double angle1 = Pi() ) override;
    void text( const TextBase &text ) override;

    void invert( ImageDataBase &out ) const override;

    void shiftRGB( ImageDataBase &out, int rx, int ry, int gx, int gy, int bx, int by ) const override;

    void function( ImageDataBase &out, const std::function<void( int, int, int, int, const Pixel &, Pixel & )> &f ) const override;
    void function( ImageDataBase &out, const std::function<void( double, double, const Color &, Color & )> &f ) const override;

    void placeTransperent( ImageDataBase &out, int x, int y ) const override;
};
