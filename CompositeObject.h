#pragma once

#include <string>

#include "Matrix2D.h"
#include "Vector2D.h"

#include "ImageDataBase.h"

void Draw( ImageDataBase &image, const std::wstring &name, const Vector2D &v, int &x, int &y );
void Draw( ImageDataBase &image, const std::wstring &name, const Matrix2D &m, int &x, int &y );
