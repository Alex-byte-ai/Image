#pragma once

#include <string>
#include <set>

#include "ImageDataBase.h"

class Palette
{
private:
    class Compare
    {
    public:
        bool operator()( const Pixel &c0, const Pixel &c1 ) const
        {
            return ( c0.r < c1.r ) || ( ( c0.r == c1.r ) && ( c0.g < c1.g ) ) || ( ( c0.r == c1.r ) && ( c0.g == c1.g ) && ( c0.b < c1.b ) ) || ( ( c0.r == c1.r ) && ( c0.g == c1.g ) && ( c0.b == c1.b ) && ( c0.a < c1.a ) );
        }
    };
    std::set<Pixel, Compare> colors;
public:
    void add( const Pixel &c );
    bool contains( const Pixel &c ) const;
    unsigned size() const;
    std::string out() const;
};
