#pragma once

#include <string>
#include <deque>

#include "RandomNumber.h"
#include "Matrix2D.h"
#include "Vector2D.h"
#include "Matrix3D.h"
#include "Vector3D.h"

#include "ImageDataBase.h"

struct RandomFunction
{
    std::vector<std::array<double, 2>> coefficients;

    RandomFunction( RandomNumber& random, size_t intervalCount, size_t coefficientsCount, double min, double max );

    double operator()( double x ) const;
};

struct Stripe
{
    Vector2D start, end, dir;
    double width, half, d, d2;

    Stripe( const Vector2D& s, const Vector2D& e, double w );
    Vector2D middle() const;
    void relative( const Vector2D& point, double& h, double& v, bool& inside ) const;
    void move( const Vector2D& shift );
};

struct Tissue
{
    int width, height, granule, vertical, horizontal;
    double tangent, thickness;
    Color light, dark;

    Tissue( int w, int h, int g, int ver, int hor, double tg, double th, const Color& l, const Color& d );
};

struct Trunk
{
    int diameter, layerCount;
    double variation, coreRatio, monotoneSliverAreaRatio, maximalDentRatio, power;
    Color innerInside, innerOutside, outerInside, outerOutside;

    Trunk( int w, int l, double v, double c, double s, double d, double p, const Color& ii, const Color& io, const Color& oi, const Color& oo );
};

class Arch;

class Sphere
{
private:
    short xCopy, yCopy;
    Vector2D p;
    double r;

public:
    Sphere( const Vector2D& center, double radius, bool normalize = true );

    Vector2D position() const;
    double radius() const;
    double height( const Vector2D& param ) const;

    bool inside( const Sphere & other ) const;
    bool contains( const Vector2D & point ) const;
    std::optional<Arch> intersect( const Sphere & other, bool trimmer = false ) const;

    std::vector<Vector2D> shift() const;
    std::vector<Vector2D> copy() const;
    static Vector2D uncopy( Vector2D point );
};

class Arch
{
private:
    Sphere host;
    double a, b;
    Vector2D v;

public:
    Arch( const Vector2D& center, const Vector2D& radius, double startParameter, double finishParameter, bool normalize = true );

    Vector2D position( double param ) const;
    Vector2D start() const;
    Vector2D finish() const;

    Vector2D vector() const;
    std::vector<Vector2D> shift() const;
    double startParam() const;
    double finishParam() const;
    double radius() const;

    double height( double param ) const;
    bool valid() const;

    void trimBy( const Sphere& s );
};

void threadSegment( ImageDataBase& image, const Stripe& s, double side, double height, const Stripe& area );
void ropeSegment( ImageDataBase& image, const Stripe& s, double tangent, double step );

void cell( MatrixBase<double>& map, std::deque<Vector2D> shape, const std::vector<Vector2D>& positions );
void cellStructure( ImageDataBase& image, RandomNumber& random, int size, double minRadius, double maxRadius, size_t cellCount );

void tissueFragment( ImageDataBase& image, const Tissue& t );

void woodSlice( ImageDataBase& image, RandomNumber& random, const Trunk& t );

void watermelonPeel( ImageDataBase& image, RandomNumber& random );
void watermelonPulp( ImageDataBase& image, RandomNumber& random );

void randomImage( ImageDataBase& image, RandomNumber& random, int width, int height, int m, int n, int granulePower, double decayCoefficient );
