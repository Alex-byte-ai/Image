#pragma once

#include <filesystem>
#include <any>

#include "ImageDataBase.h"
#include "Affine2D.h"

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

enum class SelectionMode
{
    Group,
    Part,
    Object
};

class Entity
{
protected:
    std::vector<std::shared_ptr<Entity>> children;
    Entity *parent;

public:
    Color contour, fill;
    Position position;
    std::wstring name;

    Entity( std::wstring name, const Position& position = Position() );
    virtual ~Entity();

    virtual std::wstring description() const final;
    virtual Entity *add( std::shared_ptr<Entity> child ) final;
    virtual Entity *getParent() final;

    virtual bool save( const std::filesystem::path& path ) const = 0;
    virtual bool load( const std::filesystem::path& path ) = 0;

    virtual Entity *pointsTo( const Affine2D& transform, const Vector2D& point, SelectionMode mode ) = 0;

    virtual bool draw( const Affine2D& transform, ImageDataBase& image ) const = 0;
    virtual bool size( const Affine2D& transform, Vector2D& topLeft, Vector2D& bottomRight ) const = 0;

    virtual std::wstring type() const = 0;
    virtual bool isComplex() const = 0;

    virtual std::vector<std::tuple<std::wstring, std::any>> data();
};

class Group : public Entity
{
    Group( std::wstring name, const Position& position = Position() );

    virtual bool save( const std::filesystem::path& path ) const override;
    virtual bool load( const std::filesystem::path& path ) override;

    virtual Entity *pointsTo( const Affine2D& transform, const Vector2D& point, SelectionMode mode ) override;

    virtual bool draw( const Affine2D& transform, ImageDataBase& image ) const override;
    virtual bool size( const Affine2D& transform, Vector2D& topLeft, Vector2D& bottomRight ) const override;

    virtual std::wstring type() const override;
    virtual bool isComplex() const override;
};

class Raster : public Entity
{
private:
    std::shared_ptr<ImageDataBase> image;
public:
    int64_t w, h;

    Raster( std::wstring name, int64_t w, int64_t h, const Position& position = Position() );

    virtual bool save( const std::filesystem::path& path ) const override;
    virtual bool load( const std::filesystem::path& path ) override;

    virtual Entity *pointsTo( const Affine2D& transform, const Vector2D& point, SelectionMode mode ) override;

    virtual bool draw( const Affine2D& transform, ImageDataBase& image ) const override;
    virtual bool size( const Affine2D& transform, Vector2D& topLeft, Vector2D& bottomRight ) const override;

    virtual std::wstring type() const override;
    virtual bool isComplex() const override;

    virtual std::vector<std::tuple<std::wstring, std::any>> data() override;
};

class Line : public Entity
{
public:
    Vector2D point;

    Line( std::wstring name, const Vector2D& start, const Vector2D& finish );

    virtual bool save( const std::filesystem::path& path ) const override;
    virtual bool load( const std::filesystem::path& path ) override;

    virtual Entity *pointsTo( const Affine2D& transform, const Vector2D& point, SelectionMode mode ) override;

    virtual bool draw( const Affine2D& transform, ImageDataBase& image ) const override;
    virtual bool size( const Affine2D& transform, Vector2D& topLeft, Vector2D& bottomRight ) const override;

    virtual std::wstring type() const override;
    virtual bool isComplex() const override;

    virtual std::vector<std::tuple<std::wstring, std::any>> data() override;
};

class Rectangle : public Entity
{
public:
    double w, h;

    Rectangle( std::wstring name, const Vector2D& start, const Vector2D& finish );

    virtual bool save( const std::filesystem::path& path ) const override;
    virtual bool load( const std::filesystem::path& path ) override;

    virtual Entity *pointsTo( const Affine2D& transform, const Vector2D& point, SelectionMode mode ) override;

    virtual bool draw( const Affine2D& transform, ImageDataBase& image ) const = 0;
    virtual bool size( const Affine2D& transform, Vector2D& topLeft, Vector2D& bottomRight ) const = 0;

    virtual std::wstring type() const override;
    virtual bool isComplex() const override;

    virtual std::vector<std::tuple<std::wstring, std::any>> data() override;
};

class Circle : public Entity
{
public:
    double r;

    Circle( std::wstring name, const Vector2D& center, double radius );

    virtual bool save( const std::filesystem::path& path ) const override;
    virtual bool load( const std::filesystem::path& path ) override;

    virtual Entity *pointsTo( const Affine2D& transform, const Vector2D& point, SelectionMode mode ) override;

    virtual bool draw( const Affine2D& transform, ImageDataBase& image ) const override;
    virtual bool size( const Affine2D& transform, Vector2D& topLeft, Vector2D& bottomRight ) const override;

    virtual std::wstring type() const override;
    virtual bool isComplex() const override;

    virtual std::vector<std::tuple<std::wstring, std::any>> data() override;
};
}
