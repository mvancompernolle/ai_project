#include "input.h"

/*! \file
    \brief   GLUT Input Receiver Class
    \ingroup GlutMaster
*/

/////////////////////////// GlutInput ////////////////////////////////////

GlutInput::GlutInput()
{
}

GlutInput::~GlutInput()
{
}

void GlutInput::OnKeyboard(unsigned char key, int x, int y)    {}
void GlutInput::OnKeyboardUp(unsigned char key, int x, int y)  {}
void GlutInput::OnSpecial(int key, int x, int y)               {}
void GlutInput::OnSpecialUp(int key, int x, int y)             {}
void GlutInput::OnMouse(int button, int state, int x, int y)   {}
void GlutInput::OnMotion(int x, int y)                         {}
void GlutInput::OnPassiveMotion(int x, int y)                  {}

