#include "fieldmen.h"

/*! \file
    \ingroup Node
*/

#include "registry.h"
#include "fields.h"

#include <glutm/glut.h>

#include <misc/string.h>

#include <cstdio>
#include <fstream>
using namespace std;

////////////////////// GltRegistryMenu //////////////////////

GltRegistryMenu::GltRegistryMenu(GltRegistry &registry)
: _registry(registry),
  _active(false),
  _position(0),
  _current(NULL)
{
}

GltRegistryMenu::~GltRegistryMenu()
{
}

void
GltRegistryMenu::reset()
{
    _position = 0;
    _stack.clear();
    _current = NULL;
}

bool &GltRegistryMenu::active() { return _active; }


bool
GltRegistryMenu::OnKeyboard(unsigned char key, int x, int y)
{
    if (!_current)
        return false;

    vector<GltFieldPtr> &fields = _current->_fields;

    switch (key)
    {
    case 27:
        while (_stack.size())
            pop();
        pop();
        return true;

    default:
        {
            if (_position>=(int)fields.size())
                return false;

            GltField *field = fields[_position].get();

            if (!field)
                return false;

            //
            // GltFields::GltFieldBool
            //

            GltFields::GltFieldBool *fieldBool = dynamic_cast<GltFields::GltFieldBool *>(field);
            if (fieldBool)
            {
                switch (key)
                {
                    case ' ':
                    case 13:
                    case '!':
                    case '+':
                    case '-':
                        fieldBool->val() = !fieldBool->val();
                        break;
                    case 't':
                    case 'T':
                    case '1':
                        fieldBool->val() = true;
                        break;
                    case 'f':
                    case 'F':
                    case '0':
                        fieldBool->val() = false;
                        break;
                    case 127:
                        fieldBool->reset();
                        break;

                    default:
                        return false;
                }

                return true;
            }

            //
            // GltFields::GltFieldDouble
            //

            GltFields::GltFieldDouble *fieldDouble = dynamic_cast<GltFields::GltFieldDouble *>(field);
            if (fieldDouble)
            {
                switch (key)
                {
                    case '+':
                        ++(*fieldDouble);
                        break;
                    case '-':
                        --(*fieldDouble);
                        break;
                    case '*':
                        fieldDouble->val() *= 1.25;
                        break;
                    case '/':
                        fieldDouble->val() *= 0.8;
                        break;
                    case 'r':
                    case 'n':
                        fieldDouble->val() *= -1.0;
                        break;
                    case '0':
                        fieldDouble->val() = 0.0;
                        break;
                    case '1':
                        fieldDouble->val() = 1.0;
                        break;
                    case 127:
                        fieldDouble->reset();
                        break;

                    default:
                        return false;
                }

                return true;
            }

            //
            // GltFields::GltFieldFloat
            //

            GltFields::GltFieldFloat *fieldFloat = dynamic_cast<GltFields::GltFieldFloat *>(field);
            if (fieldFloat)
            {
                switch (key)
                {
                    case '+':
                        ++(*fieldFloat);
                        break;
                    case '-':
                        --(*fieldFloat);
                        break;
                    case '*':
                        fieldFloat->val() *= 1.25;
                        break;
                    case '/':
                        fieldFloat->val() *= 0.8;
                        break;
                    case 'r':
                    case 'n':
                        fieldFloat->val() *= -1.0;
                        break;
                    case '0':
                        fieldFloat->val() = 0.0;
                        break;
                    case '1':
                        fieldFloat->val() = 1.0;
                        break;
                    case 127:
                        fieldFloat->reset();
                        break;

                    default:
                        return false;
                }

                return true;
            }

            //
            // GltFields::GltFieldInt
            //

            GltFields::GltFieldInt *fieldInt = dynamic_cast<GltFields::GltFieldInt *>(field);
            if (fieldInt)
            {
                switch (key)
                {
                    case '+':
                        fieldInt->val()++;
                        break;
                    case '-':
                        fieldInt->val()--;
                        break;
                    case '0':
                        fieldInt->val() = 0;
                        break;
                    case '1':
                        fieldInt->val() = 1;
                        break;
                    case 127:
                        fieldInt->reset();
                        break;

                    default:
                        return false;
                }

                return true;
            }

            //
            // GltFields::GltFieldEnum
            //

            GltFields::GltFieldEnum *fieldEnum = dynamic_cast<GltFields::GltFieldEnum *>(field);
            if (fieldEnum)
            {
                if (key>='0' && key<='9')
                {
                    *fieldEnum = key-'0';
                    return true;
                }

                switch (key)
                {
                    case '+':
                        ++(*fieldEnum);
                        break;
                    case '-':
                        --(*fieldEnum);
                        break;
                    case 127:
                        fieldEnum->reset();
                        break;

                    default:
                        return false;
                }

                return true;
            }

            //
            // GltFieldFunc<T> ??
            //

            if (key==' ' || key==13)
                return field->set("");

            return false;
        }
    }

    return true;
}

bool
GltRegistryMenu::OnSpecial(int key, int x, int y)
{
    if (key==GLUT_KEY_RIGHT)
        return push();

    if (!_current)
        return false;

    vector<GltFieldPtr> &fields = _current->_fields;

    switch (key)
    {
    case GLUT_KEY_UP:    _position = (_position + fields.size() - 1)%fields.size(); break;
    case GLUT_KEY_DOWN:  _position = (_position                 + 1)%fields.size(); break;

    case GLUT_KEY_LEFT:  return pop();

    default:
        return false;
    }

    return true;
}

bool GltRegistryMenu::pop()
{
    if (_stack.size())
    {
        _current  = _stack.front().first;
        _position = _stack.front().second;
        _stack.erase(_stack.begin());
        return true;
    }
    else
    {
        _current = NULL;
        _position = 0;
    }

    if (_active)
    {
        _active = false;
        return true;
    }

    return false;
}

bool GltRegistryMenu::push()
{
    if (!_current)
    {
        reset();
        _active = true;
        _current = dynamic_cast<GltFields *>(_registry._root.get());
        _position = 0;
        return true;
    }

    vector<GltFieldPtr> &fields = _current->_fields;
    GltFields *submenu = dynamic_cast<GltFields *>(fields[_position].get());

    if (submenu)
    {
        _stack.push_front(make_pair(_current,_position));
        _current = submenu;
        _position = 0;
        return true;
    }

    return false;
}

string
GltRegistryMenu::menu() const
{
    if (!_current)
        return string();

    string tmp;
    const vector<GltFieldPtr> &fields = _current->_fields;

    // Context

    for (std::list< std::pair<GltFields *,int> >::const_reverse_iterator j = _stack.rbegin(); j!=_stack.rend(); j++)
    {
        tmp += j->first->name();
        tmp += '.';
    }
    tmp += _current->name();
    tmp += "\n\n";

    // Menu Items

    for (int i=0; i<(int)fields.size(); i++)
    {
        if (i>0)
            tmp += '\n';

        const GltFields *submenu = dynamic_cast<const GltFields *>(fields[i].get());

        char buffer[1024];
        if (submenu)
            sprintf(buffer," %-15s %10s ",fields[i]->name().c_str(),">>");
        else
            sprintf(buffer," %-15s %10s ",fields[i]->name().c_str(),fields[i]->display().c_str());

        tmp += (i==_position ? "->" : "  ");
        tmp += buffer;
    }

    return tmp;
}

