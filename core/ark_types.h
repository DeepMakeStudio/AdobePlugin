#ifndef ARK_TYPES_H
#define ARK_TYPES_H

struct Color 
{
    Color() {};
    Color(float r, float b, float g, float a = 1.0f)
        : r(r)
        , g(g) 
        , b(b) 
        , a(a) 
    {}
    
    float r {0.0f};
    float g {0.0f};
    float b {0.0f};
    float a {1.0f};
};


struct Point2D 
{
    Point2D(float x, float y) 
        : x(x)
        , y(y) 
    {}
    
    Point2D() = default;

    float x {0.0f};
    float y {0.0f};
};

enum MouseEventType {
    eMouseDown,
    eMouseUp,
    eMouseMove,
    eMouseOver,
    eDrag
};

#endif //ARK_TYPES_H
