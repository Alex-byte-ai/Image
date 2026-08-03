#pragma once

#include <filesystem>
#include <any>

#include "ImageDataBase.h"
#include "Affine2D.h"
#include "Overlap.h"

namespace JustEdit
{
class Position
{
public:
    double shear, scaleX, scaleY, rotation;
    Vector2D shift;

    Position( const Vector2D& s = Vector2D(), double x = 1, double y = 1, double r = 0, double w = 0 );

    Affine2D operator()() const;
    void operator()( const Affine2D& p );
};

struct SerializationDescription;
using EditData = std::vector <std::tuple <std::wstring, std::function<bool( const std::wstring& )>, std::function<std::wstring()>, std::vector<std::wstring>>>;
using DeserializationData = std::vector<std::tuple<std::wstring, std::any>>;

class Entity
{
protected:
    std::vector<std::shared_ptr<Entity>> nodes;
    bool structure;
    Entity *root;
    size_t id;

public:
    Color contour, fill;
    Position position;
    std::wstring name;
    double thickness;

    Entity();
    Entity( std::wstring name, const Position& position = Position() );
    virtual ~Entity();

    std::wstring description() const;

    Entity *add( std::shared_ptr<Entity> node );
    Entity *add( std::shared_ptr<Entity> node, size_t id );

    std::shared_ptr<Entity> detach();

    std::vector<size_t> getPath() const;
    Entity *getObject( const std::vector<size_t>& path, size_t skip = 0 );

    size_t getId() const;

    std::vector<const Entity*> getNodes() const;
    std::vector<Entity*> getNodes();

    const Entity *getRoot() const;
    Entity *getRoot();

    const Entity* operator()( const std::function<bool( const Entity* )>& f ) const;
    Entity* operator()( const std::function<bool( Entity* )>& f );

    bool save( const std::filesystem::path& path ) const;
    static std::shared_ptr<Entity> load( const std::filesystem::path& path );

    void save( bool *success = nullptr ) const;
    static void load( std::function<void( std::shared_ptr<Entity>& )> callback );

    Affine2D globalPosition( const Entity* level ) const;

    virtual Entity *pointsTo( const Affine2D& transform, const Vector2D& point ) = 0;

    virtual bool draw( const Affine2D& transform, Overlap::Canvas& canvas ) const = 0;
    virtual bool size( const Affine2D& transform, Vector2D& topLeft, Vector2D& bottomRight ) const = 0;

    virtual std::wstring type() const = 0;
    virtual bool isComplex() const = 0;

    virtual DeserializationData deserializationData();
    virtual EditData editData();
    virtual void data( SerializationDescription& s ) const;

    virtual bool establishStructure();
    virtual bool dumpStructure();

    bool hasStructure() const;
};

class Group : public Entity
{
public:
    Group();
    Group( std::wstring name, const Position& position = Position() );

    virtual Entity *pointsTo( const Affine2D& transform, const Vector2D& point ) override;

    virtual bool draw( const Affine2D& transform, Overlap::Canvas& canvas ) const override;
    virtual bool size( const Affine2D& transform, Vector2D& topLeft, Vector2D& bottomRight ) const override;

    virtual std::wstring type() const override;
    virtual bool isComplex() const override;
};

class Raster : public Entity
{
public:
    std::shared_ptr<ImageDataBase> image;
    int64_t w, h;

    Raster();
    Raster( std::wstring name, int64_t w, int64_t h, const Position& position = Position() );

    virtual Entity *pointsTo( const Affine2D& transform, const Vector2D& point ) override;

    virtual bool draw( const Affine2D& transform, Overlap::Canvas& canvas ) const override;
    virtual bool size( const Affine2D& transform, Vector2D& topLeft, Vector2D& bottomRight ) const override;

    virtual std::wstring type() const override;
    virtual bool isComplex() const override;

    virtual DeserializationData deserializationData() override;
    virtual EditData editData() override;
    virtual void data( SerializationDescription& s ) const override;
};

class Line : public Entity
{
public:
    Vector2D point;

    Line();
    Line( std::wstring name, const Vector2D& start, const Vector2D& finish );

    virtual Entity *pointsTo( const Affine2D& transform, const Vector2D& point ) override;

    virtual bool draw( const Affine2D& transform, Overlap::Canvas& canvas ) const override;
    virtual bool size( const Affine2D& transform, Vector2D& topLeft, Vector2D& bottomRight ) const override;

    virtual std::wstring type() const override;
    virtual bool isComplex() const override;

    virtual DeserializationData deserializationData() override;
    virtual EditData editData() override;
    virtual void data( SerializationDescription& s ) const override;
};

class Rectangle : public Entity
{
public:
    double w, h;

    Rectangle();
    Rectangle( std::wstring name, double w, double h, const Position& position = Position() );

    virtual Entity *pointsTo( const Affine2D& transform, const Vector2D& point ) override;

    virtual bool draw( const Affine2D& transform, Overlap::Canvas& canvas ) const override;
    virtual bool size( const Affine2D& transform, Vector2D& topLeft, Vector2D& bottomRight ) const override;

    virtual std::wstring type() const override;
    virtual bool isComplex() const override;

    virtual DeserializationData deserializationData() override;
    virtual EditData editData() override;
    virtual void data( SerializationDescription& s ) const override;
};

class Circle : public Entity
{
public:
    double r;

    Circle();
    Circle( std::wstring name, const Vector2D& center, double radius );

    virtual Entity *pointsTo( const Affine2D& transform, const Vector2D& point ) override;

    virtual bool draw( const Affine2D& transform, Overlap::Canvas& canvas ) const override;
    virtual bool size( const Affine2D& transform, Vector2D& topLeft, Vector2D& bottomRight ) const override;

    virtual std::wstring type() const override;
    virtual bool isComplex() const override;

    virtual DeserializationData deserializationData() override;
    virtual EditData editData() override;
    virtual void data( SerializationDescription& s ) const override;
};

class Text : public Entity
{
public:
    std::wstring text;
    int64_t w, h;

    Text();
    Text( std::wstring name, std::wstring text, const Position& position = Position() );

    virtual Entity *pointsTo( const Affine2D& transform, const Vector2D& point ) override;

    virtual bool draw( const Affine2D& transform, Overlap::Canvas& canvas ) const override;
    virtual bool size( const Affine2D& transform, Vector2D& topLeft, Vector2D& bottomRight ) const override;

    virtual std::wstring type() const override;
    virtual bool isComplex() const override;

    virtual DeserializationData deserializationData() override;
    virtual EditData editData() override;
    virtual void data( SerializationDescription& s ) const override;
};

class Point : public Entity
{
public:
    uint16_t spriteId;

    Point();
    Point( std::wstring name, uint16_t spriteId = 0, const Vector2D& point = {} );

    virtual Entity *pointsTo( const Affine2D& transform, const Vector2D& point ) override;

    virtual bool draw( const Affine2D& transform, Overlap::Canvas& canvas ) const override;
    virtual bool size( const Affine2D& transform, Vector2D& topLeft, Vector2D& bottomRight ) const override;

    virtual std::wstring type() const override;
    virtual bool isComplex() const override;

    virtual DeserializationData deserializationData() override;
    virtual EditData editData() override;
    virtual void data( SerializationDescription& s ) const override;
};

class Polygon : public Group
{
public:
    std::vector<Vector2D> points;

    Polygon();
    Polygon( std::wstring name, const Position& position = Position() );

    void setup();

    virtual Entity *pointsTo( const Affine2D& transform, const Vector2D& point ) override;

    virtual bool draw( const Affine2D& transform, Overlap::Canvas& canvas ) const override;
    virtual bool size( const Affine2D& transform, Vector2D& topLeft, Vector2D& bottomRight ) const override;

    virtual std::wstring type() const override;

    virtual bool establishStructure() override;
    virtual bool dumpStructure() override;

    virtual void data( SerializationDescription& s ) const override;
};

class Perspective : public Group
{
public:
    Overlap::Frame frame0, frame1;

    Perspective();
    Perspective( std::wstring name, std::shared_ptr<Entity> target, const Position& position = Position() );

    virtual Entity *pointsTo( const Affine2D& transform, const Vector2D& point ) override;

    virtual bool draw( const Affine2D& transform, Overlap::Canvas& canvas ) const override;
    virtual bool size( const Affine2D& transform, Vector2D& topLeft, Vector2D& bottomRight ) const override;

    virtual std::wstring type() const override;

    virtual bool establishStructure() override;
    virtual bool dumpStructure() override;
};

class Selection : public Group
{
private:
    std::vector<Entity*> targets;

    Affine2D initialPositionGlobal, initialPositionLocal;
    Entity *marker, *positionRoot;
    double initialRotation;
    Vector2D grabOrigin;
public:
    Selection();

    virtual Entity *pointsTo( const Affine2D& transform, const Vector2D& point ) override;

    virtual bool draw( const Affine2D& transform, Overlap::Canvas& canvas ) const override;

    void select( Entity *target, bool add );
    Entity *getTarget();
    std::vector<Entity*> getTargets();

    void setRoot( Entity *root );

    bool grab( const Affine2D& transform, const Vector2D& point );
    bool move( const Affine2D& transform, const Vector2D& point );
    void release();

    void update();

    bool isSelected() const;

    virtual std::wstring type() const override;
};
}
