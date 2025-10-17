#include "JustEdit.h"

#include "Exception.h"
#include "ImageData.h"
#include "Overlap.h"

namespace JustEdit
{
static void boundingBox( const std::vector<Vector2D>& points, const Affine2D& f, Vector2D& topLeft, Vector2D& bottomRight )
{
    topLeft = Vector2D( std::numeric_limits<double>::max(), std::numeric_limits<double>::max() );
    bottomRight = Vector2D( std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest() );
    for( auto p : points )
    {
        p = f( p );

        if( p.x < topLeft.x )
            topLeft.x = p.x;
        if( p.y < topLeft.y )
            topLeft.y = p.y;

        if( p.x > bottomRight.x )
            bottomRight.x = p.x;
        if( p.y > bottomRight.y )
            bottomRight.y = p.y;
    }
}

static void drawImage( ImageDataBase& output, const ImageDataBase& entity, const Affine2D& f )
{
    auto canvas = std::make_unique<Canvas>( output );
    auto picture = std::make_unique<Picture>( entity );

    canvas->clear();
    picture->set( f );
    canvas->draw( *picture );
    canvas->render( output );
}

Position::Position( const Vector2D& s, double x, double y, double r, double w ) : shear( w ), scaleX( x ), scaleY( y ), rotation( r ), shift( s )
{}

Affine2D Position::operator()() const
{
    auto mRotation = Matrix2D::Rotation( rotation );
    auto mShear = Matrix2D( 1, shear, 0, 1 );
    auto mScale = Matrix2D::Scale( scaleX, scaleY );
    return Affine2D( mRotation * mShear * mScale, shift );
}

void Position::operator()( const Affine2D& p )
{
    auto a = p.t.a00;
    auto b = p.t.a10;
    auto a1 = p.t.a01;
    auto b1 = p.t.a11;
    auto det = p.t.det();

    scaleX = Sqrt( a * a + b * b );
    scaleY = det / scaleX;
    shear = ( a * a1 + b * b1 ) / det;
    rotation = ArcTan2( b, a );

    shift = p.s;
}

Entity::Entity( std::wstring n, const Position& p ) : position( p ), name( std::move( n ) )
{
    parent = nullptr;
}

Entity::~Entity()
{}

std::wstring Entity::description() const
{
    return type() + L"( \"" + name + L"\" )";
}

Entity *Entity::add( std::shared_ptr<Entity> child )
{
    if( isComplex() )
    {
        child->parent = this;
        return children.emplace_back( std::move( child ) ).get();
    }
    return nullptr;
}

Entity *Entity::getParent()
{
    return parent;
}

std::vector<std::tuple<std::wstring, std::any>> Entity::data()
{
    std::vector<std::tuple<std::wstring, std::any>> result;
    result.emplace_back( L"name", &name );
    result.emplace_back( L"x", &position.shift.x );
    result.emplace_back( L"y", &position.shift.y );
    result.emplace_back( L"x scale", &position.scaleX );
    result.emplace_back( L"y scale", &position.scaleY );
    result.emplace_back( L"shear", &position.shear );
    result.emplace_back( L"rotation", &position.rotation );
    result.emplace_back( L"contour red", &contour.r );
    result.emplace_back( L"contour green", &contour.g );
    result.emplace_back( L"contour blue", &contour.b );
    result.emplace_back( L"contour alpha", &contour.a );
    result.emplace_back( L"fill red", &fill.r );
    result.emplace_back( L"fill green", &fill.g );
    result.emplace_back( L"fill blue", &fill.b );
    result.emplace_back( L"fill alpha", &fill.a );
    return result;
}

Group::Group( std::wstring n, const Position& p ) : Entity( std::move( n ), p )
{}

bool Group::save( const std::filesystem::path& path ) const
{
    makeException( false );
    return false;
}

bool Group::load( const std::filesystem::path& path )
{
    makeException( false );
    return false;
}

bool Group::draw( const Affine2D& transform, ImageDataBase& image ) const
{
    for( auto i = children.rbegin(); i != children.rend(); ++i )
    {
        auto& child = *i;
        if( !child->draw( transform * child->position(), image ) )
            return false;
    }
    return true;
}

Entity *Group::pointsTo( const Affine2D& transform, const Vector2D& point, SelectionMode mode )
{
    auto p = transform.inv()( point );
    for( auto& child : children )
    {
        auto object = child->pointsTo( child->position(), p, mode == SelectionMode::Object ? SelectionMode::Object : SelectionMode::Group );
        if( object )
            return mode == SelectionMode::Group ? this : object;
    }
    return nullptr;
}

bool Group::size( const Affine2D& transform, Vector2D& topLeft, Vector2D& bottomRight ) const
{
    Vector2D childTopLeft, childBottomRight;
    std::vector<Vector2D> points;

    for( auto& child : children )
    {
        if( !child->size( transform * child->position(), childTopLeft, bottomRight ) )
            return false;
        points.push_back( childTopLeft );
        points.push_back( bottomRight );
    }

    boundingBox( points, Affine2D( Vector2D() ), topLeft, bottomRight );
    return true;
}

std::wstring Group::type() const
{
    return L"Group";
}

bool Group::isComplex() const
{
    return true;
}

Raster::Raster( std::wstring n, int64_t width, int64_t height, const Position& p ) : Entity( std::move( n ), p ), w( width ), h( height )
{
    image = std::make_shared<ImageData>();
}

bool Raster::save( const std::filesystem::path& path ) const
{
    makeException( false );
    return false;
}

bool Raster::load( const std::filesystem::path& path )
{
    makeException( false );
    return false;
}

Entity *Raster::pointsTo( const Affine2D& transform, const Vector2D& point, SelectionMode mode )
{
    auto p = transform.inv()( point );
    if( !( 0 <= p.x && p.x <= image->w() && 0 <= p.y && p.y <= image->h() ) )
        return nullptr;

    for( auto& child : children )
    {
        auto object = child->pointsTo( child->position(), p, mode == SelectionMode::Object ? SelectionMode::Object : SelectionMode::Group );
        if( object )
            return mode == SelectionMode::Group ? this : object;
    }

    return this;
}

bool Raster::draw( const Affine2D& transform, ImageDataBase& output ) const
{
    ImageData self;
    self.reset( w, h, ( Pixel )fill );
    image->place( self, 0, 0 );
    self.copy( *image );

    for( auto i = children.rbegin(); i != children.rend(); ++i )
    {
        auto& child = *i;
        if( !child->draw( child->position(), self ) )
            return false;
    }

    drawImage( output, self, transform );
    return true;
}

bool Raster::size( const Affine2D& transform, Vector2D& topLeft, Vector2D& bottomRight ) const
{
    boundingBox(
    {
        {0.0, 0.0},
        {double( w ), 0.0},
        {double( w ), double( h )},
        {0.0, double( h )}
    },
    transform, topLeft, bottomRight );
    return true;
}

std::wstring Raster::type() const
{
    return L"Raster";
}

bool Raster::isComplex() const
{
    return true;
}

std::vector<std::tuple<std::wstring, std::any>> Raster::data()
{
    auto result = Entity::data();
    result.emplace_back( L"width", &w );
    result.emplace_back( L"height", &h );
    return result;
}

Line::Line( std::wstring n, const Vector2D& start, const Vector2D& finish ) : Entity( std::move( n ), Position( start ) )
{
    point = finish - start;
}

bool Line::save( const std::filesystem::path& path ) const
{
    makeException( false );
    return false;
}

bool Line::load( const std::filesystem::path& path )
{
    makeException( false );
    return false;
}

Entity *Line::pointsTo( const Affine2D& transform, const Vector2D& point, SelectionMode )
{
    auto p = transform.inv()( point );
    p = ( Affine2D( Matrix2D::Scale( 1 / point.Abs(), 1 ) * Matrix2D::Rotation( -ArcTan2( point.y, point.x ) ) ) )( p );
    return 0 <= p.x && p.x <= 1 && -3 <= p.y && p.y <= 3 ? this : nullptr;
}

bool Line::draw( const Affine2D& transform, ImageDataBase& output ) const
{
    auto a = transform( Vector2D() );
    auto b = transform( point );

    a.x = RoundDown( a.x );
    a.y = RoundDown( a.y );

    b.x = RoundDown( b.x );
    b.y = RoundDown( b.y );

    output.line( a.x, a.y, b.x, b.y, ( Pixel )contour );
    return true;
}

bool Line::size( const Affine2D& transform, Vector2D& topLeft, Vector2D& bottomRight ) const
{
    boundingBox(
    {
        {0.0, 0.0},
        point
    },
    transform, topLeft, bottomRight );
    return true;
}

std::wstring Line::type() const
{
    return L"Line";
}

bool Line::isComplex() const
{
    return false;
}

std::vector<std::tuple<std::wstring, std::any>> Line::data()
{
    auto result = Entity::data();
    result.emplace_back( L"end point x", &point.x );
    result.emplace_back( L"end point y", &point.y );
    return result;
}

Rectangle::Rectangle( std::wstring n, const Vector2D& start, const Vector2D& finish )  : Entity( std::move( n ), Position( start ) )
{
    auto size = finish - start;
    w = Abs( size.x );
    h = Abs( size.y );
}

bool Rectangle::save( const std::filesystem::path& path ) const
{
    makeException( false );
    return false;
}

bool Rectangle::load( const std::filesystem::path& path )
{
    makeException( false );
    return false;
}

Entity *Rectangle::pointsTo( const Affine2D& transform, const Vector2D& point, SelectionMode )
{
    auto p = transform.inv()( point );
    return 0 <= p.x && p.x <= w && 0 <= p.y && p.y <= h ? this : nullptr;
}

bool Rectangle::draw( const Affine2D& transform, ImageDataBase& output ) const
{
    Position p;
    p( transform );

    int width = Round( w * p.scaleX );
    int height = Round( h * p.scaleY );
    p.scaleX = 1;
    p.scaleY = 1;

    ImageData self;
    self.reset( width, height );
    self.rectangle( 0, 0, width, height, ( Pixel )contour, ( Pixel )fill );

    drawImage( output, self, p() );
    return true;
}

bool Rectangle::size( const Affine2D& transform, Vector2D& topLeft, Vector2D& bottomRight ) const
{
    boundingBox(
    {
        {0.0, 0.0},
        {w, 0.0},
        {w, h},
        {0, h},
    },
    transform, topLeft, bottomRight );
    return true;
}

std::wstring Rectangle::type() const
{
    return L"Rectangle";
}

bool Rectangle::isComplex() const
{
    return false;
}

std::vector<std::tuple<std::wstring, std::any>> Rectangle::data()
{
    auto result = Entity::data();
    result.emplace_back( L"width", &w );
    result.emplace_back( L"height", &h );
    return result;
}

Circle::Circle( std::wstring n, const Vector2D& center, double radius ) : Entity( std::move( n ), Position( center ) )
{
    r = Abs( radius );
}

bool Circle::save( const std::filesystem::path& path ) const
{
    makeException( false );
    return false;
}

bool Circle::load( const std::filesystem::path& path )
{
    makeException( false );
    return false;
}

Entity *Circle::pointsTo( const Affine2D& transform, const Vector2D& point, SelectionMode )
{
    auto p = transform.inv()( point );
    return p.Abs() < r ? this : nullptr;
}

bool Circle::draw( const Affine2D& transform, ImageDataBase& output ) const
{
    Position p;
    p( transform );

    int rx = Round( r * position.scaleX );
    int ry = Round( r * position.scaleY );
    p.scaleX = 1;
    p.scaleY = 1;
    p.shift -= Vector2D( rx, ry );

    ImageData self;
    self.reset( 2 * rx + 1, 2 * ry + 1 );
    self.ellipse( rx, ry, rx, ry, ( Pixel )contour, ( Pixel )fill );

    drawImage( output, self, p() );
    return true;
}

bool Circle::size( const Affine2D& transform, Vector2D& topLeft, Vector2D& bottomRight ) const
{
    boundingBox(
    {
        {-r, -r},
        {r, -r},
        {r, r},
        {-r, r},
    },
    transform, topLeft, bottomRight );
    return true;
}

std::wstring Circle::type() const
{
    return L"Circle";
}

bool Circle::isComplex() const
{
    return false;
}

std::vector<std::tuple<std::wstring, std::any>> Circle::data()
{
    auto result = Entity::data();
    result.emplace_back( L"radius", &r );
    return result;
}
}
