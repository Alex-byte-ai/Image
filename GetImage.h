#pragma once

#include <Image/Translate.h>

#include "ImageDataBase.h"

bool screenCapture( ImageDataBase &image );
bool windowCapture( ImageDataBase &image );

void makeReference( ImageDataBase &image, ImageConvert::Reference &reference );
