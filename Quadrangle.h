#pragma once

#include "Vector2D.h"

class Quadrangle
{
public:
    Vector2D a[8];

    void boundingBox( Vector2D &p0, Vector2D &p1 ) const;
    void flip();

    static double commonArea( Quadrangle q0, Quadrangle q1 );
};
