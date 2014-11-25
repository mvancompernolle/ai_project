#include "robot.h"

/*! \file
    \ingroup GlutMaster
*/

GlutRobot::GlutRobot(GltShapePtr &shape,GlutWindow &window,const GlutWindow::MenuCallbacks value)
: _shape(shape), _window(window), _value(value)
{
}

GlutRobot::~GlutRobot()
{
}

void
GlutRobot::draw() const
{
    if (_shape->expired())
    {
        _window.OnMenu(_value);
        ((GlutRobot *) this)->expired() = true;
    }
}
