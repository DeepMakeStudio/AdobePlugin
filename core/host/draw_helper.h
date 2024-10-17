#ifndef _CORE_HOST_DRAW_HELPER_H
#define _CORE_HOST_DRAW_HELPER_H

#include "ark_types.h"

class DrawHelper
{
public:
    virtual bool DrawCircle(Point2D inCenter, float inRadius, Color inColor, float inSize, bool inFill) = 0;    
    virtual bool DrawLine(Point2D inStart, Point2D inEnd, Color inColor, float inSize) = 0;
};


#endif // _CORE_HOST_DRAW_HELPER_H
