#include "blend.h"

/*! \file
    \ingroup Node
*/

GltBlend::GltBlend(const GLenum sfactor,const GLenum dfactor)
: _sfactor(sfactor), _dfactor(dfactor)
{
}

GltBlend::~GltBlend()
{
}

void
GltBlend::draw() const
{
    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);

        glEnable(GL_BLEND);
        glBlendFunc(_sfactor,_dfactor);
        GltShapes::draw();
        glDisable(GL_BLEND);

    glPopAttrib();
}

