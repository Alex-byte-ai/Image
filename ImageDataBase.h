#pragma once

#include <filesystem>
#include <functional>
#include <string>

#include "Matrix.h"

class Color;

class Pixel
{
public:
    unsigned char b, g, r, a;

    Pixel();
    Pixel( unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255 );

    bool operator==( Pixel other ) const;
    bool operator!=( Pixel other ) const;

    Pixel invert() const;

    explicit operator const Color() const;
};

class Color
{
public:
    static double epsilon;

    double r, g, b, a;

    Color();
    Color( double r, double g, double b, double a = 1 );

    bool operator==( Color other ) const;
    bool operator!=( Color other ) const;

    Color invert() const;

    explicit operator const Pixel() const;

    Color operator+( Color other ) const;
    Color operator-( Color other ) const;

    Color operator*( double k ) const;
    Color operator/( double k ) const;

    Color layer( Color other, int count = 1 ) const;
    Color pad( Color other, int count = 1 ) const;
};

class ImageDataBase;

class TextBase
{
public:
    virtual bool measure( int &w, int &h ) const = 0;
    virtual bool operator()( ImageDataBase &image ) const = 0;
    virtual ~TextBase()
    {}
};

class ImageDataBase : public MatrixBase<Pixel>
{
public:
    virtual bool input( const std::filesystem::path &fname ) = 0;
    virtual bool output( const std::filesystem::path &fname ) const = 0;

    virtual bool input() = 0;
    virtual bool output() const = 0;

    virtual void rectangle( int x0, int y0, int w, int h, Pixel c ) = 0;
    virtual void line( int x0, int y0, int x1, int y1, Pixel c ) = 0;
    virtual void circle( int x0, int y0, int r, Pixel c ) = 0;
    virtual void ellipse( int x0, int y0, int rx, int ry, Pixel c ) = 0;
    virtual void text( const TextBase &text ) = 0;

    virtual void invert( ImageDataBase &out ) const = 0;

    virtual void shiftRGB( ImageDataBase &out, int rx, int ry, int gx, int gy, int bx, int by ) const = 0;

    virtual void function( ImageDataBase &out, const std::function<void( int, int, int, int, const Pixel &, Pixel & )> &f ) const = 0;
    virtual void function( ImageDataBase &out, const std::function<void( double, double, const Color &, Color & )> &f ) const = 0;

    virtual void placeTransperent( ImageDataBase &out, int x, int y ) const = 0;

    virtual ~ImageDataBase() {}
};
