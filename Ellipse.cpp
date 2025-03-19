#include "Ellipse.h"

DrawEllipse::DrawEllipse()
    : varX( 0 ), varY( 0 ), centerX( 0 ), centerY( 0 ), radiusX( 0 ), radiusY( 0 ),
      twoASquared( 0 ), twoBSquared( 0 ), xChange( 0 ), yChange( 0 ),
      ellipseError( 0 ), stoppingX( 0 ), stoppingY( 0 ),
      finished( true ), inSecondRegion( false )
{}

DrawEllipse::DrawEllipse( int x0, int y0, int rx, int ry )
    : centerX( x0 ), centerY( y0 ), radiusX( rx ), radiusY( ry ), finished( false ), inSecondRegion( false )
{
    twoASquared = 2 * rx * rx;
    twoBSquared = 2 * ry * ry;

    varX = 0;
    varY = ry;

    xChange = ry * ry * ( 1 - 2 * ry );
    yChange = rx * rx;
    ellipseError = 0;
    stoppingX = twoBSquared * ry;
    stoppingY = 0;
}

bool DrawEllipse::nextPixel()
{
    if( finished )
    {
        return false;
    }

    if( !inSecondRegion )
    {
        varX++;

        stoppingY += twoASquared;
        ellipseError += xChange;
        xChange += twoBSquared;

        if( 2 * ellipseError + yChange > 0 )
        {
            varY--;
            stoppingX -= twoBSquared;
            ellipseError += yChange;
            yChange += twoASquared;
        }

        if( stoppingY >= stoppingX )
        {
            inSecondRegion = true;
        }
    }
    else
    {
        varY--;

        stoppingX -= twoBSquared;
        ellipseError += yChange;
        yChange += twoASquared;

        if( 2 * ellipseError + xChange < 0 )
        {
            varX++;
            stoppingY += twoASquared;
            ellipseError += xChange;
            xChange += twoBSquared;
        }

        if( varY < 0 )
        {
            finished = true;
        }
    }

    return true;
}

bool DrawEllipse::isFinished() const
{
    return finished;
}

int DrawEllipse::x() const
{
    return centerX + varX;
}

int DrawEllipse::y() const
{
    return centerY + varY;
}
