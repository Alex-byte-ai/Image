#include "Palette.h"

void Palette::add( const Pixel &c )
{
    colors.insert( c );
}

bool Palette::contains( const Pixel &c ) const
{
    return colors.find( c ) != colors.end();
}

unsigned Palette::size() const
{
    return colors.size();
}

std::string Palette::out() const
{
    std::string result;
    for( auto i = colors.begin(); i != colors.end(); ++i )
    {
        result += "this->add(";
        result += std::to_string( ( unsigned )i->r );
        result += ", ";
        result += std::to_string( ( unsigned )i->g );
        result += ", ";
        result += std::to_string( ( unsigned )i->b );
        result += ", ";
        result += std::to_string( ( unsigned )i->a );
        result += ");\n";
    }
    return result;
}
