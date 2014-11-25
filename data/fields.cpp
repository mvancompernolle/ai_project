#include "fields.h"

/*! \file
    \ingroup Node
*/

#include <math/vector3.h>
#include <math/matrix4.h>
#include <math/bbox.h>

#include <glt/color.h>
#include <glt/light.h>
#include <glt/material.h>
#include <glt/viewport.h>
#include <glt/project.h>

#include <misc/hex.h>
#include <misc/string.h>

#include <glutm/glut.h>

#include <iostream>
#include <fstream>
#include <cstdio>
using namespace std;

/////////////////////////////////////////////

GltFields::GltFields(const string &name)
: GltField(name)
{
    _fields.reserve(5);     // Conserve memory
}

GltFields::GltFields(const GltFields &other)
: GltField(other.name()), _fields(other._fields)
{
}

GltFields::~GltFields()
{
}

GltFields &
GltFields::operator=(const GltFields &fields)
{
    if (this!=&fields)
    {
        GltField::operator=(fields);
        _fields = fields._fields;
    }
    return *this;
}

void
GltFields::add(bool &val,const string &name)
{
    _fields.push_back(GltFieldPtr(new GltFieldBool(val,name)));
}

void
GltFields::add(int &val,const string &name)
{
    _fields.push_back(GltFieldPtr(new GltFieldInt(val,name)));
}

void
GltFields::add(unsigned int &val,const string &name)
{
    int &v = reinterpret_cast<int &>(val);
    _fields.push_back(GltFieldPtr(new GltFieldInt(v,name)));
}

void
GltFields::add(float &val,const string &name)
{
    _fields.push_back(GltFieldPtr(new GltFieldFloat(val,0.1,name)));
}

void
GltFields::add(float &val,const float step,const string &name)
{
    _fields.push_back(GltFieldPtr(new GltFieldFloat(val,step,name)));
}

void
GltFields::add(double &val,const string &name)
{
    _fields.push_back(GltFieldPtr(new GltFieldDouble(val,0.1,name)));
}

void
GltFields::add(double &val,const double step,const string &name)
{
    _fields.push_back(GltFieldPtr(new GltFieldDouble(val,step,name)));
}

void
GltFields::add(string &val,const string &name)
{
    _fields.push_back(GltFieldPtr(new GltFieldString(val,name)));
}

void
GltFields::add(Vector &val,const string &name)
{
    GltFields *root = new GltFields(name);
    _fields.push_back(GltFieldPtr(root));

    root->add(val.x(),1.0,"x");
    root->add(val.y(),1.0,"y");
    root->add(val.z(),1.0,"z");
}

void
GltFields::add(Matrix &val,const string &name)
{
    GltFields *root = new GltFields(name);
    _fields.push_back(GltFieldPtr(root));

    root->add(val[ 0],1.0,"00");
    root->add(val[ 1],1.0,"01");
    root->add(val[ 2],1.0,"02");
    root->add(val[ 3],1.0,"03");
    root->add(val[ 4],1.0,"04");
    root->add(val[ 5],1.0,"05");
    root->add(val[ 6],1.0,"06");
    root->add(val[ 7],1.0,"07");
    root->add(val[ 8],1.0,"08");
    root->add(val[ 9],1.0,"09");
    root->add(val[10],1.0,"10");
    root->add(val[11],1.0,"11");
    root->add(val[12],1.0,"12");
    root->add(val[13],1.0,"13");
    root->add(val[14],1.0,"14");
    root->add(val[15],1.0,"15");
    root->add(new GltFieldFunc<Matrix>(val,&Matrix::reset,"reset"));
}

void
GltFields::add(BoundingBox &val,const string &name)
{
    GltFields *root = new GltFields(name);
    _fields.push_back(GltFieldPtr(root));

    root->add(val.defined(),"defined");
    root->add(val.min()    ,"min");
    root->add(val.max()    ,"max");
}

void
GltFields::add(GltColor &val,const string &name)
{
    GltFields *root = new GltFields(name);
    _fields.push_back(GltFieldPtr(root));

    root->add(val.red()   ,0.05,"red"   );
    root->add(val.green() ,0.05,"green" );
    root->add(val.blue()  ,0.05,"blue"  );
    root->add(val.alpha() ,0.05,"alpha" );
}

void
GltFields::add(GltLight &val,const string &name)
{
    GltFields *root = new GltFields(name);
    _fields.push_back(GltFieldPtr(root));

    root->add(val.enabled(),      "enabled");
    root->add(val.ambient(),      "ambient");
    root->add(val.diffuse(),      "diffuse");
    root->add(val.specular(),     "specular");
    root->add(val.position(),     "position");
    root->add(val.spotDirection(),"spotDirection");
    root->add(val.spotExponent(), "spotExponent");
    root->add(val.spotCutoff(),   "spotCutoff");

    GltFields *atten = new GltFields("attenuation");
    atten->add(val.attenutationConstant() ,"constant");
    atten->add(val.attenutationLinear()   ,"linear");
    atten->add(val.attenutationQuadratic(),"quadratic");

    root->add(atten);
}

void
GltFields::add(GltMaterial &val,const string &name)
{
    GltFields *root = new GltFields(name);
    _fields.push_back(GltFieldPtr(root));

    root->add(val.ambient(),  "ambient");
    root->add(val.diffuse(),  "diffuse");
    root->add(val.specular(), "specular");
    root->add(val.emission(), "emission");
    root->add(val.shininess(),"shininess");
}

void
GltFields::add(GltViewport &val,const string &name)
{
    GltFields *root = new GltFields(name);
    _fields.push_back(GltFieldPtr(root));

    root->add(val.x()     ,"x"     );
    root->add(val.y()     ,"y"     );
    root->add(val.width() ,"width" );
    root->add(val.height(),"height");
}

void
GltFields::add(GltOrtho &val,const std::string &name)
{
    GltFields *root = new GltFields(name);
    _fields.push_back(GltFieldPtr(root));

    root->add(val.left()   ,"x"     );
    root->add(val.right()  ,"y"     );
    root->add(val.bottom() ,"width" );
    root->add(val.top()    ,"height");
    root->add(val.zNear()  ,"zNear" );
    root->add(val.zFar()   ,"zFar"  );
}

void
GltFields::add(int &val,const int max,const string *labels,const string &name)
{
    _fields.push_back(GltFieldPtr(new GltFieldEnum(val,max,labels,name)));
}

void
GltFields::add(const GltFieldPtr &field)
{
    _fields.push_back(field);
}

void
GltFields::add(const GltFieldPtr &field,const std::string &rename)
{
    _fields.push_back(field);
    if (GltFields *f = dynamic_cast<GltFields *>(_fields.back().get()))
        f->name() = rename;
}

void
GltFields::add(GltField *field)
{
    _fields.push_back(field);
}

bool
GltFields::reset()
{
    bool ok = true;
    for (uint32 i=0; i<_fields.size(); i++)
        ok &= _fields[i]->reset();
    return ok;
}

void
GltFields::merge(const GltFieldPtr &root)
{
    const GltFields *fields = dynamic_cast<const GltFields *>(root.get());

    if (fields)
    {
        for (uint32 i=0; i<fields->_fields.size(); i++)
            add(fields->_fields[i]);
    }
    else
        add(root);
}

bool
GltFields::write(ostream &os,const string &prefix) const
{
    bool ok = true;

    const string p = (prefix.size() ? prefix + "." + name() : name());

    for (uint32 i=0; i<_fields.size(); i++)
        ok &= _fields[i]->write(os,p);

    return ok;
}

/////////////////////////////////////////////

GltFields::GltFieldBool::GltFieldBool(bool &val,const string &name)
: GltField(name), _val(val), _default(val)
{
}

GltFields::GltFieldBool::GltFieldBool(const GltFieldBool &field)
: GltField(field), _val(field._val),  _default(field._val)
{
}

GltFields::GltFieldBool::~GltFieldBool()
{
}

      bool &GltFields::GltFieldBool::val()       { return _val; }
const bool &GltFields::GltFieldBool::val() const { return _val; }

string
GltFields::GltFieldBool::get() const
{
    return _val ? "true" : "false";
}

bool
GltFields::GltFieldBool::set(const std::string &value)
{
    _val = !(value.length()==0 || value=="0" || value=="no" || value=="false");
    return true;
}

bool
GltFields::GltFieldBool::reset()
{
    _val = _default;
    return true;
}

bool
GltFields::GltFieldBool::isDefault() const
{
    return _val==_default;
}

///////////////////////////////

GltFields::GltFieldDouble::GltFieldDouble(double &val,const double step,const string &name)
: GltField(name),
  _val(val),
  _step(step),
  _default(val)
{
}

GltFields::GltFieldDouble::GltFieldDouble(const GltFieldDouble &field)
: GltField(field),
  _val(field._val),
  _step(field._step),
  _default(field._val)
{
}

GltFields::GltFieldDouble::~GltFieldDouble()
{
}

      double &GltFields::GltFieldDouble::val()       { return _val; }
const double &GltFields::GltFieldDouble::val() const { return _val; }

string
GltFields::GltFieldDouble::get() const
{
    string hex = toHex(_val);

    string tmp;
    sprintf(tmp,"%8.6lf\t# %s ",_val,hex.c_str());
    return tmp;
}

string
GltFields::GltFieldDouble::display() const
{
    string tmp;
    sprintf(tmp,"%8.6lf",_val);
    return tmp;
}

bool
GltFields::GltFieldDouble::set(const string &value)
{
    // Doubles can be stored as either text (1.234)
    // or binary hex string to preserve precision

    if (value.size()>=(2+sizeof(_val)*2))
    {
        // Hex string begins with 0x
        if (value.find("0x")!=string::npos)
        {
            _val = fromHex<double>(value);
            return true;
        }
    }

    _val = atof(value);

    return true;
}

bool
GltFields::GltFieldDouble::reset()
{
    _val = _default;
    return true;
}

bool
GltFields::GltFieldDouble::isDefault() const
{
    return _val==_default;
}

GltFields::GltFieldDouble &
GltFields::GltFieldDouble::operator++()
{
    _val += _step;
    return *this;
}

GltFields::GltFieldDouble &
GltFields::GltFieldDouble::operator--()
{
    _val -= _step;
    return *this;
}

///////////////////////////////

GltFields::GltFieldFloat::GltFieldFloat(float &val,const float step,const string &name)
: GltField(name),
  _val(val),
  _step(step),
  _default(val)
{
}

GltFields::GltFieldFloat::GltFieldFloat(const GltFieldFloat &field)
: GltField(field),
  _val(field._val),
  _step(field._step),
  _default(field._val)
{
}

GltFields::GltFieldFloat::~GltFieldFloat()
{
}

      float &GltFields::GltFieldFloat::val()       { return _val; }
const float &GltFields::GltFieldFloat::val() const { return _val; }

string
GltFields::GltFieldFloat::get() const
{
    string hex = toHex(_val);

    string tmp;
    sprintf(tmp,"%8.6f\t# %s ",_val,hex.c_str());
    return tmp;
}

string
GltFields::GltFieldFloat::display() const
{
    string tmp;
    sprintf(tmp,"%8.6f",_val);
    return tmp;
}


bool
GltFields::GltFieldFloat::set(const std::string &value)
{
    _val = atof(value);
    return true;
}

bool
GltFields::GltFieldFloat::reset()
{
    _val = _default;
    return true;
}

bool
GltFields::GltFieldFloat::isDefault() const
{
    return _val==_default;
}

GltFields::GltFieldFloat &
GltFields::GltFieldFloat::operator++()
{
    _val += _step;
    return *this;
}

GltFields::GltFieldFloat &
GltFields::GltFieldFloat::operator--()
{
    _val -= _step;
    return *this;
}

///////////////////////////////

GltFields::GltFieldInt::GltFieldInt(int &val,const string &name)
: GltField(name),
  _val(val),
  _default(val)
{
}

GltFields::GltFieldInt::GltFieldInt(const GltFieldInt &field)
: GltField(field.name()),
  _val(field._val),
  _default(field._val)
{
}

GltFields::GltFieldInt::~GltFieldInt()
{
}

      int &GltFields::GltFieldInt::val()       { return _val; }
const int &GltFields::GltFieldInt::val() const { return _val; }

string
GltFields::GltFieldInt::get() const
{
    char buffer[100];
    sprintf(buffer,"%d",_val);
    return buffer;
}

bool
GltFields::GltFieldInt::set(const std::string &value)
{
    _val = atoi(value);
    return true;
}

bool
GltFields::GltFieldInt::reset()
{
    _val = _default;
    return true;
}

bool
GltFields::GltFieldInt::isDefault() const
{
    return _val==_default;
}

//

GltFields::GltFieldString::GltFieldString(string &val,const string &name)
: GltField(name), _val(val), _default(val)
{
}

GltFields::GltFieldString::GltFieldString(const GltFieldString &other)
: GltField(other), _val(other._val),  _default(other._val)
{
}

GltFields::GltFieldString::~GltFieldString()
{
}

      string &GltFields::GltFieldString::val()       { return _val; }
const string &GltFields::GltFieldString::val() const { return _val; }

string
GltFields::GltFieldString::get() const
{
    return _val;
}

bool
GltFields::GltFieldString::set(const std::string &value)
{
    _val = value;
    return true;
}

bool
GltFields::GltFieldString::reset()
{
    _val = _default;
    return true;
}

bool
GltFields::GltFieldString::isDefault() const
{
    return _val==_default;
}

///////////////////////////////

GltFields::GltFieldEnum::GltFieldEnum(int &val,const int max,const string *labels,const string &name)
: GltField(name), _val(val), _default(val), _max(max), _labels(labels)
{
}

GltFields::GltFieldEnum::GltFieldEnum(const GltFieldEnum &field)
: GltField(field.name()), _val(field._val),  _default(field._val), _max(field._max), _labels(field._labels)
{
}

GltFields::GltFieldEnum::~GltFieldEnum()
{
}

      int &GltFields::GltFieldEnum::val()       { return _val; }
const int &GltFields::GltFieldEnum::val() const { return _val; }

string
GltFields::GltFieldEnum::get() const
{
    assert(_val>=0 && _val<_max);
    return _labels[_val];
}

bool
GltFields::GltFieldEnum::set(const std::string &value)
{
    for (int i=0; i<_max; i++)
        if (_labels[i]==value)
        {
            _val = i;
            return true;
        }

    return false;
}

bool
GltFields::GltFieldEnum::reset()
{
    _val = _default;
    return true;
}

bool
GltFields::GltFieldEnum::isDefault() const
{
    return _val==_default;
}

GltFields::GltFieldEnum &
GltFields::GltFieldEnum::operator++()
{
    if (_val+1<_max)
        _val++;
    return *this;
}

GltFields::GltFieldEnum &
GltFields::GltFieldEnum::operator--()
{
    if (_val>0)
        _val--;
    return *this;
}

GltFields::GltFieldEnum &
GltFields::GltFieldEnum::operator=(const int i)
{
    _val = clamp(i,0,_max-1);
    return *this;
}

//
// WINDOWS SPECIFIC
//

#ifdef GLT_WIN32

#include <windows.h>

bool
GltFields::writeRegistry(void *key) const
{
    if (!_name.size())
        return false;

    HKEY subKey = NULL;
    LONG res = RegCreateKeyEx((HKEY)key,_name.c_str(),0,NULL,REG_OPTION_NON_VOLATILE,KEY_WRITE,NULL,&subKey,NULL);

    if (res==ERROR_SUCCESS && subKey)
    {
        for (uint32 i=0; i<_fields.size(); i++)
            _fields[i].get()->writeRegistry(subKey);
    }

    if (subKey)
        RegCloseKey(subKey);

    return res==ERROR_SUCCESS;
}

bool
GltFields::readRegistry(void *key)
{
    if (!_name.size())
        return false;

    HKEY subKey = NULL;
    LONG res = RegOpenKeyEx((HKEY)key,_name.c_str(),0,KEY_READ,&subKey);

    if (res==ERROR_SUCCESS && subKey)
    {
        for (uint32 i=0; i<_fields.size(); i++)
            _fields[i].get()->readRegistry(subKey);
    }

    if (subKey)
        RegCloseKey(subKey);

    return res==ERROR_SUCCESS;
}

#endif

