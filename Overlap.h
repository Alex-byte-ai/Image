#pragma once

#include <vector>

#include "Affine2D.h"

#include "ImageDataBase.h"
#include "Quadrangle.h"
#include "Polygon.h"

template<typename E>
class Array2D
{
public:
    using Function = std::function<void( int, int, E & )>;
    using FunctionConst = std::function<void( int, int, const E & )>;

    inline Array2D( int width, int height ) : w( width ), h( height )
    {
        array = w > 0 && h > 0 ? new E[w * h] : nullptr;
    }

    inline virtual ~Array2D()
    {
        delete[] array;
    }

    inline void apply( const Function &f )
    {
        for( int i = 0; i < h; ++i )
        {
            for( int j = 0; j < w; ++j )
            {
                f( j, i, operator()( j, i ) );
            }
        }
    }

    inline void apply( const FunctionConst &f ) const
    {
        for( int i = 0; i < h; ++i )
        {
            for( int j = 0; j < w; ++j )
            {
                f( j, i, operator()( j, i ) );
            }
        }
    }

    inline E &operator()( int j, int i )
    {
        return array[i * w + j];
    }

    inline const E &operator()( int j, int i ) const
    {
        return array[i * w + j];
    }
protected:
    int w, h;
private:
    E *array;
};

class Picture
{
public:
    using FunctionConst = std::function<void( int, int, const Color &c, const Quadrangle &q )>;

    Picture( const ImageDataBase &picture );

    void set( const Affine2D &transformation );
    void apply( const Affine2D &transformation );
    void apply( const FunctionConst &f ) const;
private:
    Array2D<Color> colors;
    Array2D<Vector2D> mesh;
};

class PixelObject
{
private:
    struct Overlap
    {
        Color color;
        double area;
    };
    std::vector<Overlap> overlaping;

public:
    Color color;

    void draw( const Color &color, double area );
    void bake();
    Color calculate();
    void clear();
};

class Canvas : private Array2D<PixelObject>
{
public:
    using Parent = Array2D<PixelObject>;

    Canvas( const ImageDataBase &canvas );

    void draw( const Quadrangle &q, const Color &c );
    void draw( const Picture &picture );

    void bake();
    void render( ImageDataBase &out );
    void clear();
};
