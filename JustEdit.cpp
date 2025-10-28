#include "JustEdit.h"

#include <type_traits>
#include <algorithm>
#include <set>

#include "GetPathToFile.h"
#include "Information.h"
#include "Exception.h"
#include "ImageData.h"
#include "Text.h"

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

struct SerializationDescription
{
    Information::Item info;
};

template<typename Tuple, std::size_t... Is>
using SubTupleType = typename std::tuple<std::tuple_element_t<Is, Tuple>...>;

template<std::size_t... Is, typename... Ts>
inline auto selectTuple( const std::tuple<Ts...> &t )
{
    return SubTupleType<std::tuple<Ts...>, Is...>
    {
        std::get<Is>( t )...
    };
}

class MetaData
{
private:

public:
    using Entry = std::tuple<std::wstring, std::any, void*, unsigned, SerializationDescription, std::vector<std::wstring>, std::function<bool( const std::wstring& )>, std::function<std::wstring()>>;
    std::vector<Entry> data;

    static std::wstring fixName( const std::wstring & string )
    {
        auto result = string;
        for( auto& c : result )
        {
            if( c == L' ' )
                c = L'_';
        }
        return result;
    }

    template<typename T>
    inline void add( const wchar_t* name, T& value, std::vector<std::wstring> options = {}, std::set<std::wstring> banedWords = {} )
    {
        SerializationDescription s;
        s.info = value;

        std::function<bool( const std::wstring& )> set;
        std::function<std::wstring()> get;

        if constexpr( std::is_same<T, std::wstring>() )
        {
            auto pointer = &value;
            set = [banedWords, pointer]( const std::wstring & v )
            {
                if( !Information::verifyVerbatim( v ) )
                    return false;

                if( banedWords.find( v ) != banedWords.end() )
                    return false;

                *pointer = v;
                return true;
            };
            get = [pointer]()
            {
                return *pointer;
            };
        }
        else if constexpr( std::is_same<T, int64_t>() )
        {
            auto pointer = &value;
            set = [pointer]( const std::wstring & v )
            {
                try
                {
                    size_t p = 0;
                    auto result = std::stoll( v, &p );
                    if( p != v.length() )
                        return false;
                    *pointer = result;
                    return true;
                }
                catch( ... )
                {}
                return false;
            };
            get = [pointer]()
            {
                return std::to_wstring( *pointer );
            };
        }
        else if constexpr( std::is_same<T, double>() )
        {
            auto pointer = &value;
            set = [pointer]( const std::wstring & v )
            {
                try
                {
                    size_t p = 0;
                    auto result = std::stod( v, &p );
                    if( p != v.length() )
                        return false;
                    *pointer = result;
                    return true;
                }
                catch( ... )
                {}
                return false;
            };
            get = [pointer]()
            {
                return std::to_wstring( *pointer );
            };
        }
        else if constexpr( std::is_same<T, bool>() )
        {
            if( options.size() != 2 )
                options = {L"off", L"on"};
            auto pointer = &value;
            set = [pointer, options]( const std::wstring & v )
            {
                auto i = std::find( options.begin(), options.end(), v );
                if( i == options.end() )
                    return false;
                *pointer = i - options.begin();
                return true;
            };
            get = [pointer, options]()
            {
                return options[*pointer];
            };
        }
        else if constexpr( std::is_same<T, uint16_t>() )
        {
            auto pointer = &value;
            set = [pointer, options]( const std::wstring & v )
            {
                auto i = std::find( options.begin(), options.end(), v );
                if( i == options.end() )
                    return false;
                *pointer = i - options.begin();
                return true;
            };
            get = [pointer, options]()
            {
                return options.empty() ? L"none" : options[*pointer];
            };
        }
        else
        {
            // Leaving setter and getter to be nullptr
            // makeException( false );
        }

        data.emplace_back( std::wstring( name ), &value, ( void* ) & value, sizeof( value ), s, options, set, get );
    }

    template<std::size_t... Is>
    inline auto select() const
    {
        using Element = decltype( selectTuple<Is...>( data[0] ) );
        std::vector<Element> subList;
        for( auto& meta : data )
            subList.push_back( selectTuple<Is...>( meta ) );
        return subList;
    }

    inline void assemble( SerializationDescription& s )
    {
        s.info = Information::Object();
        for( auto& [name, value] : select<0, 4>() )
            s.info( fixName( name ) ) =  value.info;
    }
};

template<typename Object>
inline MetaData entityData( Object& o )
{
    decltype( o.getRoot() ) root = &o;
    auto next = root->getRoot();
    while( next )
    {
        root = next;
        next = next->getRoot();
    }

    std::set<std::wstring> names;
    ( *root )( [&names]( decltype( o.getRoot() ) s )
    {
        names.insert( s->name );
        return true;
    } );

    MetaData l;
    l.add( L"name", o.name, {}, names );
    l.add( L"x", o.position.shift.x );
    l.add( L"y", o.position.shift.y );
    l.add( L"x scale", o.position.scaleX );
    l.add( L"y scale", o.position.scaleY );
    l.add( L"shear", o.position.shear );
    l.add( L"rotation", o.position.rotation );
    l.add( L"contour thickness", o.thickness );
    l.add( L"contour red", o.contour.r );
    l.add( L"contour green", o.contour.g );
    l.add( L"contour blue", o.contour.b );
    l.add( L"contour alpha", o.contour.a );
    l.add( L"fill red", o.fill.r );
    l.add( L"fill green", o.fill.g );
    l.add( L"fill blue", o.fill.b );
    l.add( L"fill alpha", o.fill.a );
    return l;
}

template<typename Object>
inline MetaData rasterData( Object& o )
{
    auto l = entityData( o );
    l.add( L"width", o.w );
    l.add( L"height", o.h );
    return l;
}

template<typename Object>
inline MetaData lineData( Object& o )
{
    auto l = entityData( o );
    l.add( L"end point x", o.point.x );
    l.add( L"end point y", o.point.y );
    return l;
}

template<typename Object>
inline MetaData rectangleData( Object& o )
{
    auto l = entityData( o );
    l.add( L"width", o.w );
    l.add( L"height", o.h );
    return l;
}

template<typename Object>
inline MetaData circleData( Object& o )
{
    auto l = entityData( o );
    l.add( L"radius", o.r );
    return l;
}

template<typename Object>
inline MetaData textData( Object& o )
{
    auto l = entityData( o );
    l.add( L"text", o.text );
    l.add( L"width", o.w );
    l.add( L"height", o.h );
    return l;
}

template<typename Object>
inline MetaData pointData( Object& o )
{
    MetaData l;
    l.add( L"x", o.position.shift.x );
    l.add( L"y", o.position.shift.y );
    l.add( L"sprite", o.spriteId, {L"circle", L"square", L"rhombus", L"large"} );
    return l;
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

Entity::Entity() : root( nullptr ), contour( 0, 0, 1 ), fill( 1, 1, 0 ), thickness( 2 )
{}

Entity::Entity( std::wstring n, const Position& p ) : Entity()
{
    position = p;
    name = std::move( n );
}

Entity::~Entity()
{}

std::wstring Entity::description() const
{
    return type() + L"( \"" + name + L"\" )";
}

Entity *Entity::add( std::shared_ptr<Entity> node )
{
    if( isComplex() && node )
    {
        node->root = this;
        return nodes.emplace_back( std::move( node ) ).get();
    }
    return nullptr;
}

std::shared_ptr<Entity> Entity::remove( const Entity* node )
{
    std::vector<std::shared_ptr<Entity>> newNodes;
    newNodes.reserve( nodes.size() );

    std::shared_ptr<Entity> removed;

    for( auto&& candidate : nodes )
    {
        if( candidate.get() != node )
            newNodes.emplace_back( candidate );
        else
            removed = candidate;
    }

    nodes = std::move( newNodes );
    return removed;
}

std::shared_ptr<Entity> Entity::detach()
{
    if( root )
        return root->remove( this );
    return nullptr;
}

std::vector<const Entity*> Entity::getNodes() const
{
    std::vector<const Entity*> result;
    result.reserve( nodes.size() );
    for( auto& node : nodes )
        result.push_back( node.get() );
    return result;
}

std::vector<Entity*> Entity::getNodes()
{
    std::vector<Entity*> result;
    result.reserve( nodes.size() );
    for( auto& node : nodes )
        result.push_back( node.get() );
    return result;
}

const Entity *Entity::getRoot() const
{
    return root;
}

Entity *Entity::getRoot()
{
    return root;
}

template<typename E>
E foreachRoot( E object, const std::function<bool( E )>& f )
{
    if( !f( object ) )
        return object;

    for( auto node : object->getNodes() )
    {
        if( ( object = foreachRoot( node, f ) ) )
            return object;
    }

    return nullptr;
}

const Entity *Entity::operator()( const std::function<bool( const Entity* )>& f ) const
{
    return foreachRoot( this, f );
}

Entity *Entity::operator()( const std::function<bool( Entity* )>& f )
{
    return foreachRoot( this, f );
}

static void append( Information::Wrapper& storage, const Entity & object, const std::filesystem::path& path )
{
    using namespace Information;

    if( object.type() == L"Selection" )
        return;

    SerializationDescription s;
    object.data( s );
    s.info( L"class" ) = object.type();

    storage = std::move( s.info );

    storage( L"node_names" ) = Array();
    auto& array = storage( L"node_names" ).as<Array>();

    auto fname = MetaData::fixName( object.name );

    if( object.type() == L"Raster" )
    {
        auto raster = dynamic_cast<const Raster*>( &object );
        if( raster )
            raster->image->output( path / ( fname + L".png" ) );
    }

    for( auto node : object.getNodes() )
    {
        Item item;
        item = String( MetaData::fixName( node->name ) );
        array.push( item );
        auto wrapper = storage( node->name );
        append( wrapper, *node, path / fname );
    }
}

bool Entity::save( const std::filesystem::path& path ) const
{
    using namespace Information;

    Item self;
    Wrapper wrapper( self );
    append( wrapper, *this, path.parent_path() );
    return self.output( path );
}

static std::shared_ptr<Entity> extract( const Information::Item & storage, const std::filesystem::path& path )
{
    using namespace Information;

    std::shared_ptr<Entity> object;
    ImageDataBase *image = nullptr;

    auto type = ( std::wstring )storage( L"class" ).as<String>();
    if( type == L"Group" )
    {
        object = std::make_shared<Group>();
    }
    else if( type == L"Raster" )
    {
        object = std::make_shared<Raster>();
        auto raster = dynamic_cast<Raster*>( object.get() );
        makeException( raster );
        image = raster->image.get();
    }
    else if( type == L"Line" )
    {
        object = std::make_shared<Line>();
    }
    else if( type == L"Rectangle" )
    {
        object = std::make_shared<Rectangle>();
    }
    else if( type == L"Circle" )
    {
        object = std::make_shared<Circle>();
    }
    else if( type == L"Text" )
    {
        object = std::make_shared<Text>();
    }
    else if( type == L"Point" )
    {
        object = std::make_shared<Point>();
    }
    else
    {
        makeException( false );
    }

    for( auto& [nameOriginal, value] : object->deserializationData() )
    {
        auto name = MetaData::fixName( nameOriginal );

        makeException( value.has_value() );
        if( value.type() == typeid( std::wstring* ) )
        {
            *std::any_cast<std::wstring*>( value ) = ( std::wstring )storage( name ).as<String>();
        }
        else if( value.type() == typeid( int64_t* ) )
        {
            *std::any_cast<int64_t*>( value ) = storage( name ).as<long long>();
        }
        else if( value.type() == typeid( double* ) )
        {
            *std::any_cast<double*>( value ) = storage( name ).as<long double>();
        }
        else if( value.type() == typeid( bool* ) )
        {
            *std::any_cast<bool*>( value ) = storage( name ).as<bool>();
        }
        else if( value.type() == typeid( uint16_t* ) )
        {
            *std::any_cast<uint16_t*>( value ) = storage( name ).as<long long unsigned>();
        }
        else
        {
            makeException( false );
        }
    }

    auto fname = MetaData::fixName( object->name );
    if( image )
        makeException( image->input( path / ( fname + L".png" ) ) );

    auto& array = storage( L"node_names" ).as<Array>();
    for( auto& name : array )
        object->add( extract( storage( ( std::wstring )name.as<String>() ), path / fname ) );

    return object;
}

std::shared_ptr<Entity> Entity::load( const std::filesystem::path& path )
{
    using namespace Information;

    Item self;
    if( !self.input( path ) )
        return nullptr;

    try
    {
        return extract( self, path.parent_path() );
    }
    catch( ... )
    {}
    return nullptr;
}

bool Entity::save() const
{
    auto path = SavePath();
    if( path )
        return save( *path );
    return false;
}

std::shared_ptr<Entity> Entity::load()
{
    auto path = OpenPath();
    if( path )
        return load( *path );
    return nullptr;
}

Affine2D Entity::globalPosition( const Entity* r ) const
{
    return root == r ? position() : root->globalPosition( r ) * position();
}

DeserializationData Entity::deserializationData()
{
    return entityData( *this ).select<0, 1>();
}

EditData Entity::editData()
{
    return entityData( *this ).select<0, 6, 7>();
}

void Entity::data( SerializationDescription& s ) const
{
    entityData( *this ).assemble( s );
}

bool Entity::establishVirtualStructure()
{
    return false;
}

void Entity::dumpVirtualStructure()
{}

Group::Group() : Entity()
{}

Group::Group( std::wstring n, const Position& p ) : Entity( std::move( n ), p )
{}

Entity *Group::pointsTo( const Affine2D& transform, const Vector2D& point, SelectionMode mode )
{
    auto p = transform.inv()( point );
    for( auto i = nodes.rbegin(); i != nodes.rend(); ++i )
    {
        auto& node = *i;
        auto object = node->pointsTo( node->position(), p, mode == SelectionMode::Object ? SelectionMode::Object : SelectionMode::Group );
        if( object )
            return mode == SelectionMode::Group ? this : object;
    }
    return nullptr;
}

bool Group::draw( const Affine2D& transform, Overlap::Canvas& image ) const
{
    for( auto& node : nodes )
    {
        if( !node->draw( transform * node->position(), image ) )
            return false;
    }
    return true;
}

bool Group::size( const Affine2D& transform, Vector2D& topLeft, Vector2D& bottomRight ) const
{
    Vector2D nodeTopLeft, nodeBottomRight;
    std::vector<Vector2D> points;

    bool hasSize = false;

    for( auto& node : nodes )
    {
        if( !node->size( transform * node->position(), nodeTopLeft, nodeBottomRight ) )
            continue;
        points.push_back( nodeTopLeft );
        points.push_back( nodeBottomRight );
        hasSize = true;
    }

    boundingBox( points, Affine2D( Vector2D() ), topLeft, bottomRight );
    return hasSize;
}

std::wstring Group::type() const
{
    return L"Group";
}

bool Group::isComplex() const
{
    return true;
}

Raster::Raster() : Entity(), image( std::make_shared<ImageData>() )
{}

Raster::Raster( std::wstring n, int64_t width, int64_t height, const Position& p ) : Entity( std::move( n ), p ), image( std::make_shared<ImageData>() ), w( width ), h( height )
{}

Entity *Raster::pointsTo( const Affine2D& transform, const Vector2D& point, SelectionMode mode )
{
    auto p = transform.inv()( point );
    if( !( 0 <= p.x && p.x <= image->w() && 0 <= p.y && p.y <= image->h() ) )
        return nullptr;

    for( auto i = nodes.rbegin(); i != nodes.rend(); ++i )
    {
        auto& node = *i;
        auto object = node->pointsTo( node->position(), p, mode == SelectionMode::Object ? SelectionMode::Object : SelectionMode::Group );
        if( object )
            return mode == SelectionMode::Group ? this : object;
    }

    return this;
}

bool Raster::draw( const Affine2D& transform, Overlap::Canvas& canvas ) const
{
    image->crop( *image, 0, 0, Max( Abs( w ), 1 ), Max( Abs( h ), 1 ), ( Pixel )fill );

    Overlap::Canvas self( *image );
    for( auto& node : nodes )
    {
        if( !node->draw( node->position(), self ) )
            return false;
    }

    Overlap::Picture picture( self );
    picture.set( transform );
    canvas.draw( picture );
    canvas.bake();
    return true;
}

bool Raster::size( const Affine2D& transform, Vector2D& topLeft, Vector2D& bottomRight ) const
{
    double width = Max( Abs( w ), 1 );
    double height = Max( Abs( h ), 1 );

    boundingBox(
    {
        {0.0, 0.0},
        {width, 0.0},
        {width, height},
        {0.0, height}
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

DeserializationData Raster::deserializationData()
{
    return rasterData( *this ).select<0, 1>();
}

EditData Raster::editData()
{
    return rasterData( *this ).select<0, 6, 7>();
}

void Raster::data( SerializationDescription& s ) const
{
    rasterData( *this ).assemble( s );
}

Line::Line() : Entity()
{}

Line::Line( std::wstring n, const Vector2D& start, const Vector2D& finish ) : Entity( std::move( n ), Position( start ) )
{
    point = finish - start;
}

Entity *Line::pointsTo( const Affine2D& transform, const Vector2D& pt, SelectionMode )
{
    auto p = transform.inv()( pt );
    p = Affine2D( Matrix2D::Rotation( -ArcTan2( point.y, point.x ) ) )( p );
    return 0 <= p.x && p.x <= point.Abs() && -thickness * 0.5 <= p.y && p.y <= thickness * 0.5 ? this : nullptr;
}

bool Line::draw( const Affine2D& transform, Overlap::Canvas& canvas ) const
{
    auto a = transform( Vector2D() );
    auto b = transform( point );
    canvas.draw( a, b, thickness, contour );
    canvas.bake();
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

DeserializationData Line::deserializationData()
{
    return lineData( *this ).select<0, 1>();
}

EditData Line::editData()
{
    return lineData( *this ).select<0, 6, 7>();
}

void Line::data( SerializationDescription& s ) const
{
    lineData( *this ).assemble( s );
}

Rectangle::Rectangle() : Entity()
{}

Rectangle::Rectangle( std::wstring n, double width, double height, const Position& p ) : Entity( std::move( n ), Position( p ) ), w( width ), h( height )
{}

Entity *Rectangle::pointsTo( const Affine2D& transform, const Vector2D& point, SelectionMode )
{
    auto p = transform.inv()( point );
    return 0 <= p.x && p.x <= w && 0 <= p.y && p.y <= h ? this : nullptr;
}

bool Rectangle::draw( const Affine2D& transform, Overlap::Canvas& canvas ) const
{
    canvas.draw( transform, w, h, thickness, contour, fill );
    canvas.bake();
    return true;
}

bool Rectangle::size( const Affine2D& transform, Vector2D& topLeft, Vector2D& bottomRight ) const
{
    double width = Abs( w );
    double height = Abs( h );
    boundingBox(
    {
        {0.0, 0.0},
        {width, 0.0},
        {width, height},
        {0.0, height},
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

DeserializationData Rectangle::deserializationData()
{
    return rectangleData( *this ).select<0, 1>();
}

EditData Rectangle::editData()
{
    return rectangleData( *this ).select<0, 6, 7>();
}

void Rectangle::data( SerializationDescription& s ) const
{
    rectangleData( *this ).assemble( s );
}

Circle::Circle() : Entity()
{}

Circle::Circle( std::wstring n, const Vector2D& center, double radius ) : Entity( std::move( n ), Position( center ) ), r( radius )
{}

Entity *Circle::pointsTo( const Affine2D& transform, const Vector2D& point, SelectionMode )
{
    auto p = transform.inv()( point );
    return p.Abs() < Abs( r ) ? this : nullptr;
}

bool Circle::draw( const Affine2D& transform, Overlap::Canvas& canvas ) const
{
    canvas.draw( transform, r, thickness, contour, fill );
    canvas.bake();
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

DeserializationData Circle::deserializationData()
{
    return circleData( *this ).select<0, 1>();
}

EditData Circle::editData()
{
    return circleData( *this ).select<0, 6, 7>();
}

void Circle::data( SerializationDescription& s ) const
{
    circleData( *this ).assemble( s );
}

Text::Text() : Entity(), w( -1 ), h( -1 )
{}

Text::Text( std::wstring n, std::wstring t, const Position& p ) : Entity( std::move( n ), p ), text( std::move( t ) ), w( -1 ), h( -1 )
{}

Entity *Text::pointsTo( const Affine2D& transform, const Vector2D& point, SelectionMode )
{
    TextGraphics g;
    g.set( L"text", text );

    if( w <= 0 || h <= 0 )
        g.measure( w, h );

    auto p = transform.inv()( point );
    return 0 < p.x && p.x < w && 0 < p.y && p.y < h ? this : nullptr;
}

bool Text::draw( const Affine2D& transform, Overlap::Canvas& canvas ) const
{
    TextGraphics g;
    g.set( L"text", text );

    int width = w;
    int height = h;
    if( width <= 0 || height <= 0 )
        g.measure( width, height );

    ImageData self( width, height );
    self.text( g );

    Overlap::Picture picture( self );
    picture.set( transform );

    canvas.draw( picture );
    canvas.bake();
    return true;
}

bool Text::size( const Affine2D& transform, Vector2D& topLeft, Vector2D& bottomRight ) const
{
    boundingBox(
    {
        {0.0, 0.0},
        {double( w ), 0.0},
        {double( w ), double( h )},
        {0.0, double( h )},
    },
    transform, topLeft, bottomRight );
    return true;
}

std::wstring Text::type() const
{
    return L"Text";
}

bool Text::isComplex() const
{
    return false;
}

DeserializationData Text::deserializationData()
{
    return textData( *this ).select<0, 1>();
}

EditData Text::editData()
{
    return textData( *this ).select<0, 6, 7>();
}

void Text::data( SerializationDescription& s ) const
{
    textData( *this ).assemble( s );
}

Point::Point() : Entity(), spriteId( 0 )
{}

Point::Point( std::wstring n, uint16_t id, const Vector2D& p ) : Entity( std::move( n ), Position( p ) ), spriteId( id )
{}

Entity *Point::pointsTo( const Affine2D& transform, const Vector2D& point, SelectionMode )
{
    Affine2D shift = transform;
    shift.t = Matrix2D::Identity();
    auto p = shift.inv()( point );
    return p.Abs() < ( spriteId < 3 ? 4 : 12 ) ? this : nullptr;
}

bool Point::draw( const Affine2D& transform, Overlap::Canvas& canvas ) const
{
    Affine2D shift = transform;
    shift.t = Matrix2D::Identity();

    ImageData self;
    switch( spriteId )
    {
    case 0:
        self.reset( 9, 9 );
        self.circle( 4, 4, 4, ( Pixel )contour );
        *self( 4, 4 ) = ( Pixel )contour;
        shift.s.x -= 4;
        shift.s.y -= 4;
        break;
    case 1:
        self.reset( 9, 9 );
        self.rectangle( 0, 0, 9, 9, ( Pixel )contour );
        *self( 4, 4 ) = ( Pixel )contour;
        shift.s.x -= 4;
        shift.s.y -= 4;
        break;
    case 2:
        self.reset( 9, 9 );
        self.line( 0, 4, 5, -1, ( Pixel )contour );
        self.line( 4, 0, 9, 5, ( Pixel )contour );
        self.line( 8, 4, 3, 9, ( Pixel )contour );
        self.line( 4, 8, -1, 3, ( Pixel )contour );
        *self( 4, 4 ) = ( Pixel )contour;
        shift.s.x -= 4;
        shift.s.y -= 4;
        break;
    case 3:
        self.reset( 25, 25 );
        self.circle( 12, 12, 12, ( Pixel )contour );
        shift.s.x -= 12;
        shift.s.y -= 12;
        break;
    default:
        makeException( false );
    }

    Overlap::Picture picture( self );
    picture.set( shift );

    canvas.draw( picture );
    canvas.bake();
    return true;
}

bool Point::size( const Affine2D& transform, Vector2D& topLeft, Vector2D& bottomRight ) const
{
    Vector2D shift;
    shift.x = shift.y = spriteId < 3 ? 4 : 12;

    topLeft = bottomRight = transform.s;
    topLeft -= shift;
    bottomRight += shift;
    return true;
}

std::wstring Point::type() const
{
    return L"Point";
}

bool Point::isComplex() const
{
    return false;
}

DeserializationData Point::deserializationData()
{
    return pointData( *this ).select<0, 1>();
}

EditData Point::editData()
{
    return pointData( *this ).select<0, 6, 7>();
}

void Point::data( SerializationDescription& s ) const
{
    pointData( *this ).assemble( s );
}

Polygon::Polygon() : Group()
{
    setup();
}

Polygon::Polygon( std::wstring n, const Position& p ) : Group( std::move( n ), p )
{
    setup();
}

void Polygon::setup()
{
    points = {{0.0, 0.0}, {96.0, 0.0}, {96.0, 96.0}, {0.0, 96.0}};
}

Entity *Polygon::pointsTo( const Affine2D& transform, const Vector2D& point, SelectionMode mode )
{
    if( nodes.empty() )
    {
        auto p = transform.inv()( point );

        Vector2D topLeft, bottomRight;
        if( !size( Affine2D( Vector2D() ), topLeft, bottomRight ) )
            return nullptr;

        return topLeft.x <= p.x && p.x <= bottomRight.x && topLeft.y <= p.y && p.y <= bottomRight.y ? this : nullptr;
    }
    return Group::pointsTo( transform, point, mode );
}

bool Polygon::draw( const Affine2D& transform, Overlap::Canvas& canvas ) const
{
    if( nodes.empty() )
    {
        Vector2D p0, p1;
        p1 = transform( points[0] );

        auto size = points.size();
        for( size_t i = 0; i < size; ++i )
        {
            p0 = p1;
            p1 = transform( points[( i + 1 ) % size] );
            canvas.draw( p0, p1, thickness, contour );
            canvas.bake();
        }
        return true;
    }
    return Group::draw( transform, canvas );
}

bool Polygon::size( const Affine2D& transform, Vector2D& topLeft, Vector2D& bottomRight ) const
{
    boundingBox( points, transform, topLeft, bottomRight );
    return true;
}

std::wstring Polygon::type() const
{
    return L"Polygon";
}

bool Polygon::establishVirtualStructure()
{
    size_t i = 0;
    for( auto& point : points )
    {
        add( std::make_shared<Point>( std::to_wstring( i ), 0, point ) );
        ++i;
    }
    return true;
}

void Polygon::dumpVirtualStructure()
{
    points.clear();

    size_t i = 0;
    for( auto& node : nodes )
    {
        points.push_back( node->position.shift );
        ++i;
    }

    nodes.clear();
    if( points.empty() )
        setup();
}

Selection::Selection() : Group( L"selection" ), marker( nullptr ), cramped( false )
{
    add( std::make_shared<Point>( L".l.t", 1 ) );
    add( std::make_shared<Point>( L".r.t", 1 ) );
    add( std::make_shared<Point>( L".r.b", 1 ) );
    add( std::make_shared<Point>( L".l.b", 1 ) );

    add( std::make_shared<Point>( L".t", 0 ) );
    add( std::make_shared<Point>( L".r", 0 ) );
    add( std::make_shared<Point>( L".b", 0 ) );
    add( std::make_shared<Point>( L".l", 0 ) );

    add( std::make_shared<Point>( L".~t", 2 ) );
    add( std::make_shared<Point>( L".~r", 2 ) );
    add( std::make_shared<Point>( L".~b", 2 ) );
    add( std::make_shared<Point>( L".~l", 2 ) );

    add( std::make_shared<Point>( L".*", 3 ) );
    add( std::make_shared<Point>( L".#", 0 ) );
}

Entity *Selection::pointsTo( const Affine2D& transform, const Vector2D& point, SelectionMode mode )
{
    if( cramped )
    {
        auto p = transform.inv()( point );
        if( !( angle.x <= p.x && p.x <= angle.x + area.x && angle.y <= p.y && p.y <= angle.y + area.y ) )
            return nullptr;

        return mode == SelectionMode::Group ? this : nodes[13].get();
    }
    return Group::pointsTo( transform, point, mode );
}

bool Selection::draw( const Affine2D& transform, Overlap::Canvas& canvas ) const
{
    if( cramped )
    {
        canvas.draw( transform * Affine2D( angle ), area.x, area.y, 0, {}, Color( 1, 0, 0, 0.5 ) );
        canvas.bake();
        return true;
    }
    return Group::draw( transform, canvas );
}

std::shared_ptr<Entity> Selection::extract()
{
    return unselection ? std::move( unselection ) : detach();
}

void Selection::select( Entity *t, bool add )
{
    auto unselect = [this]()
    {
        if( !unselection )
            unselection = std::dynamic_pointer_cast<Selection>( detach() );
        targets.clear();
    };

    marker = nullptr;

    if( !t || t == this || !t->getRoot() || !t->getRoot()->isComplex() )
    {
        if( !add )
            unselect();
        return;
    }

    Vector2D tl, br;
    if( !t->size( t->position(), tl, br ) )
    {
        if( !add )
            unselect();
        return;
    }

    if( !add || targets.empty() )
    {
        unselect();
        if( !t->getRoot()->add( unselection ) )
            return;
        unselection = nullptr;
    }
    else
    {
        if( targets[0]->getRoot() != t->getRoot() )
            return;
    }

    auto i = std::find( targets.begin(), targets.end(), t );
    if( i != targets.end() )
    {
        targets.erase( i );
        if( targets.empty() )
            unselect();
    }
    else
    {
        targets.push_back( t );
    }

    update();
}

Entity *Selection::getTarget()
{
    return targets.size() == 1 ? targets[0] : nullptr;
}

std::vector<Entity*> Selection::getTargets()
{
    return targets;
}

bool Selection::grab( const Vector2D& point )
{
    marker = pointsTo( position(), point, JustEdit::SelectionMode::Part );
    auto selected = getTarget();
    if( selected && marker )
    {
        initialPosition = selected->position();
        if( marker->name.find( L".*" ) == std::wstring::npos )
        {
            grabOrigin = initialPosition.inv()( point );
        }
        else
        {
            grabOrigin = marker->position.shift;
            auto v = point - grabOrigin;
            initialRotation = ArcTan2( v.y, v.x );
        }
        return true;
    }
    return false;
}

bool Selection::move( const Vector2D& point )
{
    auto selected = getTarget();
    if( selected && marker )
    {
        Vector2D topLeft, bottomRight;
        if( !selected->size( Affine2D( Vector2D() ), topLeft, bottomRight ) )
            return false;

        if( marker->name.find( L".*" ) != std::wstring::npos )
        {
            auto v = point - grabOrigin;
            auto rotation = ArcTan2( v.y, v.x ) - initialRotation;

            selected->position( initialPosition );
            auto grabOriginLocal = initialPosition.inv()( grabOrigin );

            selected->position.rotation += rotation;
            auto grabOriginNew = selected->position()( grabOriginLocal );

            selected->position.shift += grabOrigin - grabOriginNew;
            update();
            return true;
        }

        auto fix = []( double s )
        {
            if( Abs( s ) < 0.01 )
            {
                auto sign = Sign( s );
                s = sign ? 0.01 * sign : 0.01;
            }
            return s;
        };

        auto size = bottomRight - topLeft;
        auto delta = initialPosition.inv()( point ) - grabOrigin;
        Affine2D transform;

        transform = Affine2D( Vector2D() );

        if( marker->name.find( L".~t" ) != std::wstring::npos )
        {
            auto shearX = delta.x / ( grabOrigin.y - bottomRight.y );
            transform.t = Matrix2D( 1, shearX, 0, 1 );
            transform.s = Vector2D( -shearX * bottomRight.y, 0 );
        }
        else if( marker->name.find( L".~r" ) != std::wstring::npos )
        {
            auto shearY = delta.y / ( grabOrigin.x - topLeft.x );
            transform.t = Matrix2D( 1, 0, shearY, 1 );
            transform.s = Vector2D( 0, -shearY * topLeft.x );
        }
        else if( marker->name.find( L".~b" ) != std::wstring::npos )
        {
            auto shearX = delta.x / ( grabOrigin.y - topLeft.y );
            transform.t = Matrix2D( 1, shearX, 0, 1 );
            transform.s = Vector2D( -shearX * topLeft.y, 0 );
        }
        else if( marker->name.find( L".~l" ) != std::wstring::npos )
        {
            auto shearY = delta.y / ( grabOrigin.x - bottomRight.x );
            transform.t = Matrix2D( 1, 0, shearY, 1 );
            transform.s = Vector2D( 0, -shearY * bottomRight.x );
        }

        if( marker->name.find( L".l" ) != std::wstring::npos )
        {
            auto scaleX = fix( ( size.x - delta.x ) / size.x );
            auto scaleBorder = topLeft.x * scaleX;
            auto border = topLeft.x + delta.x;
            transform.t.a00 = scaleX;
            transform.s.x = border - scaleBorder;
        }
        else if( marker->name.find( L".r" ) != std::wstring::npos )
        {
            auto scaleX = fix( ( size.x + delta.x ) / size.x );
            auto scaleBorder = bottomRight.x * scaleX;
            auto border = bottomRight.x + delta.x;
            transform.t.a00 = scaleX;
            transform.s.x = border - scaleBorder;
        }

        if( marker->name.find( L".t" ) != std::wstring::npos )
        {
            auto scaleY = fix( ( size.y - delta.y ) / size.y );
            auto scaleBorder = topLeft.y * scaleY;
            auto border = topLeft.y + delta.y;
            transform.t.a11 = scaleY;
            transform.s.y = border - scaleBorder;
        }
        else if( marker->name.find( L".b" ) != std::wstring::npos )
        {
            auto scaleY = fix( ( size.y + delta.y ) / size.y );
            auto scaleBorder = bottomRight.y * scaleY;
            auto border = bottomRight.y + delta.y;
            transform.t.a11 = scaleY;
            transform.s.y = border - scaleBorder;
        }

        if( marker->name.find( L".#" ) != std::wstring::npos )
            transform.s = delta;

        Position modification;
        modification( initialPosition * transform );
        selected->position = modification;
        update();
        return true;
    }
    return false;
}

void Selection::release()
{
    marker = nullptr;
}

void Selection::update()
{
    auto target = getTarget();

    std::vector<Vector2D> points;
    Vector2D topLeft, bottomRight;
    for( auto object : targets )
    {
        if( object->size( target ? Affine2D( Vector2D() ) : object->position(), topLeft, bottomRight ) )
        {
            points.push_back( topLeft );
            points.push_back( bottomRight );
        }
    }

    boundingBox( points, Affine2D( Vector2D() ), topLeft, bottomRight );

    auto transformation = target ? target->position() : Affine2D( Vector2D() );

    auto p0 = nodes[0]->position.shift = transformation( topLeft );
    auto p1 = nodes[1]->position.shift = transformation( Vector2D( bottomRight.x, topLeft.y ) );
    auto p2 = nodes[2]->position.shift = transformation( bottomRight );
    auto p3 = nodes[3]->position.shift = transformation( Vector2D( topLeft.x, bottomRight.y ) );

    auto p4 = nodes[4]->position.shift = ( p0 + p1 ) * 0.5;
    auto p5 = nodes[5]->position.shift = ( p1 + p2 ) * 0.5;
    auto p6 = nodes[6]->position.shift = ( p2 + p3 ) * 0.5;
    auto p7 = nodes[7]->position.shift = ( p3 + p0 ) * 0.5;

    auto p12 = nodes[12]->position.shift = ( p0 + p1 + p2 + p3 ) * 0.25;
    nodes[13]->position.shift = p12;

    nodes[8]->position.shift = p4 + 12 * ( p12 - p4 ).Normal();
    nodes[9]->position.shift = p5 + 12 * ( p12 - p5 ).Normal();
    nodes[10]->position.shift = p6 + 12 * ( p12 - p6 ).Normal();
    nodes[11]->position.shift = p7 + 12 * ( p12 - p7 ).Normal();

    points.clear();
    for( auto& node : nodes )
    {
        if( node->size( node->position(), topLeft, bottomRight ) )
        {
            points.push_back( topLeft );
            points.push_back( bottomRight );
        }
    }

    boundingBox( points, Affine2D( Vector2D() ), topLeft, bottomRight );

    area = bottomRight - topLeft;
    angle = topLeft;
    cramped = area.x < 65 || area.y < 65;
}

std::wstring Selection::type() const
{
    return L"Selection";
}
}
