#include "Test_18_color_vector.h"

#include "../ImageData.h"

void Test_18_color_vector( Context &context )
{
    auto &text = context.output();
    auto s = context.scope( __FUNCTION__ );

    Color a( 0.2, 0.7, 0.4, 0.6 ), b( 0.7, 0.3, 0.6, 0.4 ), c( 0.4, 0.2, 0.3, 0.1 );

    auto output = [&text]( const std::wstring & description, const Color & x )
    {
        text << description;
        text << L" ( " << x.r << L"; " << x.g << L"; " << x.b << L" | " << x.a << L" )\n";
    };

    output( L"a = ", a );
    output( L"b = ", b );
    output( L"c = ", c );

    output( L"a.layer( b ) = ", a.layer( b ) );
    output( L"b.pad( a ) = ", b.pad( a ) );

    output( L"a.layer( b ).layer( c ) = ", a.layer( b ).layer( c ) );
    output( L"a.layer( b.layer( c ) ) = ", a.layer( b.layer( c ) ) );

    auto x = a.layer( b, 3 );

    auto y = a.layer( b );
    y = y.layer( b );
    y = y.layer( b );

    makeException( x == y );

    x = x.layer( b, -3 );

    makeException( x == a );

    y = y.layer( b, -1 );
    y = y.layer( b, -1 );
    y = y.layer( b, -1 );

    makeException( y == a );

    x = a.pad( b, 4 );

    y = a.pad( b );
    y = y.pad( b );
    y = y.pad( b );
    y = y.pad( b );

    makeException( x == y );

    x = x.pad( b, -4 );

    makeException( x == a );

    y = y.pad( b, -1 );
    y = y.pad( b, -1 );
    y = y.pad( b, -1 );
    y = y.pad( b, -1 );

    makeException( y == a );
}
