#include "interp.h"

/*! \file
    \ingroup Node
*/

#include <cmath>
#include <math/matrix4.h>

////////////////////////////////////////////////////////////////////////

GltInterpolator::GltInterpolator()
: _active(false),
  _loop(false),
  _realTime(true),
  _duration(0.0),
  _frame(0)
{
}

GltInterpolator::~GltInterpolator()
{
}

void
GltInterpolator::start(const double duration,const bool loop)
{
    _duration = duration;
    _active   = true;
    _loop     = loop;
    _realTime = true;
    _timer.start();
}

void
GltInterpolator::start(const bool loop)
{
    _active   = true;
    _loop     = loop;
    _realTime = false;
    _frame    = 0;
}

void
GltInterpolator::stop()
{
    _active = false;
    expired() = true;
}

void
GltInterpolator::restart(void)
{
    _active   = true;
    _frame    = 0;
    _timer.start();
}

const bool   &GltInterpolator::active()   const { return _active;   }
const bool   &GltInterpolator::loop()     const { return _loop;     }
const bool   &GltInterpolator::realTime() const { return _realTime; }
const double &GltInterpolator::duration() const { return _duration; }
const int    &GltInterpolator::frame()    const { return _frame;    }

      bool   &GltInterpolator::active()   { return _active;   }
      bool   &GltInterpolator::loop()     { return _loop;     }
      bool   &GltInterpolator::realTime() { return _realTime; }
      double &GltInterpolator::duration() { return _duration; }
      int    &GltInterpolator::frame()    { return _frame;    }

////////////////////////////////////////////////////////////////////////

GltInterpolatorColor::GltInterpolatorColor(GltColor &color)
: _color(color)
{
}

GltInterpolatorColor::~GltInterpolatorColor()
{
}

void
GltInterpolatorColor::draw() const
{
    if (_active)
    {
        // REAL TIME

        if (_realTime)
        {
            if (_duration==0.0)
                _color =  _colorMap.lookup(_colorMap.end());
            else
            {
                double t = _timer.elapsed();

                if (_loop)
                    t = fmod(t,_duration)/_duration;
                else
                    t = clamp(t/_duration,0.0,1.0);

                _color =  _colorMap.lookup(_colorMap.start() + _colorMap.duration()*t);
            }

            // Cast away the const to stop

            if (_timer.elapsed()>_duration && !_loop)
                ((GltInterpolatorColor *) this)->stop();
        }
        else

        // PER FRAME

        {
            _color = _colorMap.lookup(double(_frame));
            ((GltInterpolatorColor *) this)->_frame++;
            if (double(_frame)>_colorMap.end())
            {
                if (_loop)
                    ((GltInterpolatorColor *) this)->_frame = 0;
                else
                    ((GltInterpolatorColor *) this)->stop();
            }
        }
    }
}

      GltColorMap &GltInterpolatorColor::colorMap()       { return _colorMap; }
const GltColorMap &GltInterpolatorColor::colorMap() const { return _colorMap; }

////////////////////////////

GltInterpolatorRotation::GltInterpolatorRotation(Matrix &matrix)
: _matrix(matrix), _axis(VectorY), _center(Vector0)
{
}

GltInterpolatorRotation::~GltInterpolatorRotation()
{
}

void
GltInterpolatorRotation::draw() const
{
    if (_active && _duration!=0.0 && _axis!=Vector0)
    {
        const double t = _timer.elapsed();

        _matrix *=
            matrixTranslate(_center) *
            matrixRotate(_axis,t/_duration*360.0) *
            matrixTranslate(-_center);

        ((GltInterpolatorRotation *) this)->_timer.start();

        // Cast away the const to stop

        if (t>_duration && !_loop)
            ((GltInterpolatorRotation *) this)->stop();
    }
}

      Vector &GltInterpolatorRotation::axis()         { return _axis; }
const Vector &GltInterpolatorRotation::axis() const   { return _axis; }

      Vector &GltInterpolatorRotation::center()       { return _center; }
const Vector &GltInterpolatorRotation::center() const { return _center; }
