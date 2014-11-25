#include "winiv.h"

/*! \file
    \ingroup GlutMaster
*/

#ifdef GLUTM_OPEN_INVENTOR

#include <Inventor/SoDB.h>
#include <Inventor/SoInteraction.h>
#include <Inventor/nodekits/SoNodeKit.h>

#include <Inventor/events/SoLocation2Event.h>
#include <Inventor/events/SoMouseButtonEvent.h>
#include <Inventor/events/SoKeyboardEvent.h>

#include <cassert>
using namespace std;

///////////////////////

bool GlutWindowInventor::_initOpenInventor = false;

void
GlutWindowInventor::initOpenInventor()
{
    if (!_initOpenInventor)
    {
        // Initialise Open Inventor
        SoDB::init();
        SoNodeKit::init();
        SoInteraction::init();
        _initOpenInventor = true;
    }
}

static void
renderCallback(void *userData, SoSceneManager *mgr)
{
    GlutWindow *window = (GlutWindow *) userData;
    window->postRedisplay();
}

GlutWindowInventor::GlutWindowInventor(const std::string &title,int width,int height,int x,int y,unsigned int displayMode)
: GlutWindow(title,width,height,x,y,displayMode)
{
}

GlutWindowInventor::~GlutWindowInventor()
{
}

void
GlutWindowInventor::OnOpen()
{
    assert(_initOpenInventor);
    activate();
    setRenderCallback(renderCallback,this);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

void
GlutWindowInventor::OnClose()
{
    deactivate();
    setRenderCallback(NULL,NULL);
}

void
GlutWindowInventor::OnDisplay()
{
    render();
}

void
GlutWindowInventor::inventorTick()
{
    SoDB::getSensorManager()->processTimerQueue();
    SoDB::getSensorManager()->processDelayQueue(TRUE);
}

void
GlutWindowInventor::OnTick()
{
        inventorTick();
}

void
GlutWindowInventor::OnReshape(int w, int h)
{
    GlutWindow::OnReshape(w,h);

    setWindowSize(SbVec2s(w, h));
    setSize(SbVec2s(w, h));
    setViewportRegion(SbViewportRegion(w, h));
    scheduleRedraw();
    inventorTick();
}

void
GlutWindowInventor::OnKeyboard(unsigned char key, int x, int y)
{
    GlutWindow::OnKeyboard(key,x,y);

    // TODO: Check that delete, enter, backspace and insert are handled correctly

    SoKeyboardEvent event;
    SoKeyboardEvent::Key k;

    switch (key)
    {

    case '0':  k = SoKeyboardEvent::NUMBER_0; break;
    case '1':  k = SoKeyboardEvent::NUMBER_1; break;
    case '2':  k = SoKeyboardEvent::NUMBER_2; break;
    case '3':  k = SoKeyboardEvent::NUMBER_3; break;
    case '4':  k = SoKeyboardEvent::NUMBER_4; break;
    case '5':  k = SoKeyboardEvent::NUMBER_5; break;
    case '6':  k = SoKeyboardEvent::NUMBER_6; break;
    case '7':  k = SoKeyboardEvent::NUMBER_7; break;
    case '8':  k = SoKeyboardEvent::NUMBER_8; break;
    case '9':  k = SoKeyboardEvent::NUMBER_9; break;

    case 'a':  case 'A': k = SoKeyboardEvent::A; break;
    case 'b':  case 'B': k = SoKeyboardEvent::B; break;
    case 'c':  case 'C': k = SoKeyboardEvent::C; break;
    case 'd':  case 'D': k = SoKeyboardEvent::D; break;
    case 'e':  case 'E': k = SoKeyboardEvent::E; break;
    case 'f':  case 'F': k = SoKeyboardEvent::F; break;
    case 'g':  case 'G': k = SoKeyboardEvent::G; break;
    case 'h':  case 'H': k = SoKeyboardEvent::H; break;
    case 'i':  case 'I': k = SoKeyboardEvent::I; break;
    case 'j':  case 'J': k = SoKeyboardEvent::J; break;
    case 'k':  case 'K': k = SoKeyboardEvent::K; break;
    case 'l':  case 'L': k = SoKeyboardEvent::L; break;
    case 'm':  case 'M': k = SoKeyboardEvent::M; break;
    case 'n':  case 'N': k = SoKeyboardEvent::N; break;
    case 'o':  case 'O': k = SoKeyboardEvent::O; break;
    case 'p':  case 'P': k = SoKeyboardEvent::P; break;
    case 'q':  case 'Q': k = SoKeyboardEvent::Q; break;
    case 'r':  case 'R': k = SoKeyboardEvent::R; break;
    case 's':  case 'S': k = SoKeyboardEvent::S; break;
    case 't':  case 'T': k = SoKeyboardEvent::T; break;
    case 'u':  case 'U': k = SoKeyboardEvent::U; break;
    case 'v':  case 'V': k = SoKeyboardEvent::V; break;
    case 'w':  case 'W': k = SoKeyboardEvent::W; break;
    case 'x':  case 'X': k = SoKeyboardEvent::X; break;
    case 'y':  case 'Y': k = SoKeyboardEvent::Y; break;
    case 'z':  case 'Z': k = SoKeyboardEvent::Z; break;

    case ' ':  k = SoKeyboardEvent::SPACE;  break;
    case '\n': k = SoKeyboardEvent::RETURN; break;
    case ',':  k = SoKeyboardEvent::COMMA;  break;
    case '-':  k = SoKeyboardEvent::MINUS;  break;
    case '.':  k = SoKeyboardEvent::PERIOD; break;
    case '/':  k = SoKeyboardEvent::SLASH;  break;
    case ';':  k = SoKeyboardEvent::SEMICOLON;  break;
    case '=':  k = SoKeyboardEvent::EQUAL;  break;
    case '(':  k = SoKeyboardEvent::BRACKETLEFT;  break;
    case '\\': k = SoKeyboardEvent::BACKSLASH;    break;
    case ')':  k = SoKeyboardEvent::BRACKETRIGHT; break;

    default:
        return;
    }

    event.setKey(k);
    event.setPosition(SbVec2s(x,_viewport.height()-y));
    event.setState(SoButtonEvent::DOWN);
    processEvent(&event);
    inventorTick();
}

void
GlutWindowInventor::OnSpecial(int key, int x, int y)
{
    GlutWindow::OnSpecial(key,x,y);

    SoKeyboardEvent event;
    SoKeyboardEvent::Key k;

    switch (key)
    {

    case GLUT_KEY_F1:   k = SoKeyboardEvent::F1;  break;
    case GLUT_KEY_F2:   k = SoKeyboardEvent::F2;  break;
    case GLUT_KEY_F3:   k = SoKeyboardEvent::F3;  break;
    case GLUT_KEY_F4:   k = SoKeyboardEvent::F4;  break;
    case GLUT_KEY_F5:   k = SoKeyboardEvent::F5;  break;
    case GLUT_KEY_F6:   k = SoKeyboardEvent::F6;  break;
    case GLUT_KEY_F7:   k = SoKeyboardEvent::F7;  break;
    case GLUT_KEY_F8:   k = SoKeyboardEvent::F8;  break;
    case GLUT_KEY_F9:   k = SoKeyboardEvent::F9;  break;
    case GLUT_KEY_F10:  k = SoKeyboardEvent::F10; break;
    case GLUT_KEY_F11:  k = SoKeyboardEvent::F11; break;
    case GLUT_KEY_F12:  k = SoKeyboardEvent::F12; break;

    case GLUT_KEY_LEFT:  k = SoKeyboardEvent::LEFT_ARROW;  break;
    case GLUT_KEY_UP:    k = SoKeyboardEvent::UP_ARROW;    break;
    case GLUT_KEY_RIGHT: k = SoKeyboardEvent::RIGHT_ARROW; break;
    case GLUT_KEY_DOWN:  k = SoKeyboardEvent::DOWN_ARROW;  break;

    case GLUT_KEY_PAGE_UP:   k = SoKeyboardEvent::PAGE_UP;   break;
    case GLUT_KEY_PAGE_DOWN: k = SoKeyboardEvent::PAGE_DOWN; break;
    case GLUT_KEY_HOME:      k = SoKeyboardEvent::HOME;      break;
    case GLUT_KEY_END:       k = SoKeyboardEvent::END;       break;
    case GLUT_KEY_INSERT:    k = SoKeyboardEvent::INSERT;    break;

    default:
        return;
    }

    event.setKey(k);
    event.setPosition(SbVec2s(x,_viewport.height()-y));
    event.setState(SoButtonEvent::DOWN);
    processEvent(&event);
    inventorTick();
}

void
GlutWindowInventor::OnMouse(int button, int state, int x, int y)
{
    GlutWindow::OnMouse(button,state,x,y);

    SoMouseButtonEvent event;

    switch (button)
    {
    default:
    case GLUT_LEFT_BUTTON:   event.setButton(SoMouseButtonEvent::BUTTON1); break;
    case GLUT_MIDDLE_BUTTON: event.setButton(SoMouseButtonEvent::BUTTON2); break;
    case GLUT_RIGHT_BUTTON:  event.setButton(SoMouseButtonEvent::BUTTON3); break;
    }

    switch (state)
    {
    default:
    case GLUT_UP:   event.setState(SoButtonEvent::UP);   break;
    case GLUT_DOWN: event.setState(SoButtonEvent::DOWN); break;
    }

    event.setPosition(SbVec2s(x,_viewport.height()-y));

    processEvent(&event);
    inventorTick();
}

void
GlutWindowInventor::OnMotion(int x, int y)
{
    GlutWindow::OnMotion(x,y);

    SoLocation2Event event;
    event.setPosition(SbVec2s(x,_viewport.height()-y));
    processEvent(&event);
    inventorTick();
}

void
GlutWindowInventor::OnPassiveMotion(int x, int y)
{
    GlutWindow::OnPassiveMotion(x,y);

    SoLocation2Event event;
    event.setPosition(SbVec2s(x,_viewport.height()-y));
    processEvent(&event);
    inventorTick();
}

#endif
