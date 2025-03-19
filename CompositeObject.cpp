#include "CompositeObject.h"

#include <sstream>

#include "RandomNumber.h"

#include "Text.h"

void Draw( ImageDataBase &image, const std::wstring &name, const Vector2D &v, int &x, int &y )
{
    std::wostringstream buffer;
    int w, h;
    Pixel b;

    buffer << name << L"=\n" << v.x << L"\n" << v.y;

    static RandomNumber randomNumber( 274497 );
    RandomNumber::IntegerInterval integerInterval( 0, 128 );
    b = Pixel(
            randomNumber.getInteger( integerInterval ),
            randomNumber.getInteger( integerInterval ),
            randomNumber.getInteger( integerInterval )
        );

    TextGraphics text;
    text.set( L"text", buffer.str() );
    text.set( L"x", std::to_wstring( x ) );
    text.set( L"y", std::to_wstring( y ) );
    text.measure( w, h );
    image.rectangle( x, y, w, h, b );
    image.text( text );

    x += w;
    if( x >= image.w() )
    {
        x = 0;
        y += h;
    }
}

void Draw( ImageDataBase &image, const std::wstring &name, const Matrix2D &m, int &x, int &y )
{
    std::wostringstream buffer;
    int w, h;
    Pixel b;

    buffer << name << L"=\n" << m.a00 << L", " << m.a01 << L"\n" << m.a10 << L", " << m.a11;

    static RandomNumber randomNumber( 209851 );
    RandomNumber::IntegerInterval integerInterval( 0, 128 );
    b = Pixel(
            randomNumber.getInteger( integerInterval ),
            randomNumber.getInteger( integerInterval ),
            randomNumber.getInteger( integerInterval )
        );

    TextGraphics text;
    text.set( L"text", buffer.str() );
    text.set( L"x", std::to_wstring( x ) );
    text.set( L"t", std::to_wstring( y ) );
    text.measure( w, h );
    image.rectangle( x, y, w, h, b );
    image.text( text );

    x += w;
    if( x >= image.w() )
    {
        x = 0;
        y += h;
    }
}
