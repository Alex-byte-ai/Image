#pragma once

#include <vector>

#include "Context.h"

#include "../ImageData.h"
#include "../ImageWindow.h"

class Core
{
public:
    const ImageData *image;
    unsigned i, j;
    Pixel c;

    Core()
    {
        image = nullptr;
        i = j = 0;
    }

    double distance( const Core &other, const Interval<unsigned char> &r, const Interval<unsigned char> &g, const Interval<unsigned char> &b ) const;
    double distance2( const Core &other ) const;
};

class Cores
{
public:
    ImageData *image;
    std::vector<Core> cores;

    Cores()
    {
        image = nullptr;
    }
};

Cores &getCores();
ConsoleOutput *&getText();

void ManualPicks( const ImageWindow::InputData &inputData, ImageWindow::OutputData &outputData );

bool LimitPalette( const ImageData &in, ImageData &out, double size );
bool ReplacePinkWithTransparent( const ImageData &in, const ImageData &mask, ImageData &out );
