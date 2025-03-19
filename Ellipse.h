#pragma once

#include "Curve.h"

class DrawEllipse : public DrawCurve
{
private:
    int varX, varY, centerX, centerY, radiusX, radiusY;
    int twoASquared, twoBSquared, xChange, yChange, ellipseError, stoppingX, stoppingY;
    bool finished;
    bool inSecondRegion;

public:
    DrawEllipse();
    DrawEllipse( int x0, int y0, int rx, int ry );

    bool nextPixel() override;
    bool isFinished() const override;
    int x() const override;
    int y() const override;
};
