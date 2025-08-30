#pragma once

#include <string>

#include "Matrix2D.h"
#include "Vector2D.h"
#include "Matrix3D.h"
#include "Vector3D.h"

#include "ImageDataBase.h"

void Draw( ImageDataBase &image, const std::wstring &text, int &x, int &y, const Pixel &background );

void Draw( ImageDataBase &image, const std::wstring &name, const Vector2D &v, int &x, int &y );
void Draw( ImageDataBase &image, const std::wstring &name, const Matrix2D &m, int &x, int &y );

void Draw( ImageDataBase &image, const std::wstring &name, const Vector3D &v, int &x, int &y );
void Draw( ImageDataBase &image, const std::wstring &name, const Matrix3D &m, int &x, int &y );
