#include "CompositeObject.h"

#include <sstream>

#include "RandomNumber.h"

#include "Text.h"

void Draw( ImageDataBase &image, const std::wstring &text, int &x, int &y, const Pixel &background )
{
    TextGraphics t;
    int w, h;

    t.set( L"text", text );
    t.set( L"x", std::to_wstring( x ) );
    t.set( L"y", std::to_wstring( y ) );

    t.measure( w, h );
    image.rectangle( x, y, w, h, {}, background );
    image.text( t );

    x += w;
    if( x >= image.w() )
    {
        x = 0;
        y += h;
    }
}

void Draw( ImageDataBase &image, const std::wstring &name, const Vector2D &v, int &x, int &y )
{
    static RandomNumber randomNumber( 274497 );
    std::wostringstream buffer;
    Pixel b;

    RandomNumber::IntegerInterval integerInterval( 0, 128 );
    b = Pixel( randomNumber.getInteger( integerInterval ),
               randomNumber.getInteger( integerInterval ),
               randomNumber.getInteger( integerInterval ) );

    buffer << name << L"=\n" << v.x << L"\n" << v.y;
    Draw( image, buffer.str(), x, y, b );
}

void Draw( ImageDataBase &image, const std::wstring &name, const Matrix2D &m, int &x, int &y )
{
    static RandomNumber randomNumber( 209851 );
    std::wostringstream buffer;
    Pixel b;

    RandomNumber::IntegerInterval integerInterval( 0, 128 );
    b = Pixel( randomNumber.getInteger( integerInterval ),
               randomNumber.getInteger( integerInterval ),
               randomNumber.getInteger( integerInterval ) );

    buffer << name << L"=\n" <<
           m.a00 << L", " << m.a01 << L"\n" <<
           m.a10 << L", " << m.a11;
    Draw( image, buffer.str(), x, y, b );
}

void Draw( ImageDataBase &image, const std::wstring &name, const Vector3D &v, int &x, int &y )
{
    static RandomNumber randomNumber( 274497 );
    std::wostringstream buffer;
    Pixel b;

    RandomNumber::IntegerInterval integerInterval( 0, 128 );
    b = Pixel( randomNumber.getInteger( integerInterval ),
               randomNumber.getInteger( integerInterval ),
               randomNumber.getInteger( integerInterval ) );

    buffer << name << L"=\n" << v.x << L"\n" << v.y << L"\n" << v.z;
    Draw( image, buffer.str(), x, y, b );
}

void Draw( ImageDataBase &image, const std::wstring &name, const Matrix3D &m, int &x, int &y )
{
    static RandomNumber randomNumber( 209851 );
    std::wostringstream buffer;
    Pixel b;

    RandomNumber::IntegerInterval integerInterval( 0, 128 );
    b = Pixel( randomNumber.getInteger( integerInterval ),
               randomNumber.getInteger( integerInterval ),
               randomNumber.getInteger( integerInterval ) );

    buffer << name << L"=\n" <<
           m.a00 << L", " << m.a01 << L", " << m.a02 << L"\n" <<
           m.a10 << L", " << m.a11 << L", " << m.a12 << L"\n" <<
           m.a20 << L", " << m.a21 << L", " << m.a22;
    Draw( image, buffer.str(), x, y, b );
}
