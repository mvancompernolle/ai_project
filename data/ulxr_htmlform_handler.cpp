/***************************************************************************
        ulxr_htmlform_handler.cpp  -  work with html pages and forms
                             -------------------
    begin                : Fri Dec 05 2003
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_htmlform_handler.cpp 1151 2009-08-12 15:12:01Z ewald-arnold $

 ***************************************************************************/

/**************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 ***************************************************************************/

//#define ULXR_DEBUG_OUTPUT

#define ULXR_NEED_EXPORTS
#include <ulxmlrpcpp/ulxmlrpcpp.h>  // always first header

#include <cstdio>
#include <cctype>
#include <cstdlib>

#include <ulxmlrpcpp/ulxr_htmlform_handler.h>
#include <ulxmlrpcpp/ulxr_http_protocol.h>
#include <ulxmlrpcpp/ulxr_except.h>


namespace ulxr {


namespace hidden {


ULXR_API_IMPL0 SubResourceBase::SubResourceBase(const CppString &in_name, const CppString &in_descr)
 : name(in_name)
 , descr(in_descr)
{
}


ULXR_API_IMPL0 SubResourceBase::~SubResourceBase()
{
}


ULXR_API_IMPL(CppString) SubResourceBase::getName() const
{
  return name;
}


ULXR_API_IMPL(CppString) SubResourceBase::getDescription() const
{
  return descr;
}


}


///////////////////////////////////////////////////////////////////////////////////////


ULXR_API_IMPL0 HtmlFormHandler::HtmlFormHandler(const CppString &in_resource)
  : masterResource(in_resource)
{
  addSubResource(getCssName(), this, &HtmlFormHandler::handle_css_file, ULXR_PCHAR("Common style sheet"));
//  addSubResource(ULXR_PCHAR("subresources.html"), this, &HtmlFormHandler::handle_subresource_list, ULXR_PCHAR("Display subresources"));
}


ULXR_API_IMPL(CppString) HtmlFormHandler::getMasterResource() const
{
  return masterResource;
}


ULXR_API_IMPL0 HtmlFormHandler::~HtmlFormHandler()
{
  for (unsigned i = 0; i < subResources.size(); ++i)
    delete subResources[i];

  subResources.clear();
}


ULXR_API_IMPL(bool) HtmlFormHandler::handler(HttpProtocol *prot,
                                          const CppString &method,
                                          const CppString &resource,
                                          const Cpp8BitString &conn_data)
{
  ULXR_TRACE(ULXR_PCHAR("HtmlFormHandler::handler ")
             << method << ULXR_PCHAR(" ")
             << resource << std::endl);

  CppString target;
  if (resourceSplit(method, resource, getMasterResource(), target))
  {

    HtmlFormData formdata;
    formdata = extractFormElements(prot, method, target, conn_data);

    for (unsigned i = 0; i < subResources.size(); ++i)
    {
      if (subResources[i]->getName() == target)
      {
        CppString mimetype = ULXR_PCHAR("text/html");
        std::string resp = encodeForHtml(subResources[i]->call(formdata, mimetype));
        prot->sendResponseHeader(200, ULXR_PCHAR("OK"), mimetype, resp.length());
        prot->writeRaw(resp.data(), resp.length());
        return true;
      }
    }
    return false;
  }
  else
    return false;
}


ULXR_API_IMPL(std::string) HtmlFormHandler::encodeForHtml(const CppString &data)
{
#ifdef ULXR_UNICODE
  return unicodeToUtf8(data);
#else
  return data;
#endif
}


ULXR_API_IMPL(CppString) HtmlFormHandler::encodeFromHtml(const std::string &data)
{
#ifdef ULXR_UNICODE
  return utf8ToUnicode(data);
#else
  return data;
#endif
}


ULXR_API_IMPL(bool) HtmlFormHandler::resourceSplit(const CppString &method,
                                                const CppString &resource,
                                                const CppString &rsc_start,
                                                CppString &target)
{
  if (resource.substr(0, rsc_start.length()) == rsc_start)
  {
    target = resource.substr(rsc_start.length());
    if (method == ULXR_PCHAR("GET"))
    {
      std::size_t pos = target.find('?');
      if (pos != CppString::npos)
        target.erase(0, pos);
    }

    return true;
  }
  else
  {
    target = ULXR_PCHAR("");
    return false;
  }
}


ULXR_API_IMPL(HtmlFormData) HtmlFormHandler::extractFormElements(HttpProtocol *prot,
                                                              const CppString &method,
                                                              const CppString &resource,
                                                              const Cpp8BitString &conn_data)
{
  ULXR_TRACE(ULXR_PCHAR("HtmlFormHandler::extractFormElements")) ;
  HtmlFormData formdata;
  std::string post_data = conn_data;

  if (method == ULXR_PCHAR("POST"))
  {
    char conn_buffer [ULXR_RECV_BUFFER_SIZE];
    long conn_readed;
    bool done = false;
    while (!done && ((conn_readed = prot->readRaw(conn_buffer, sizeof(conn_buffer))) > 0) )
    {
      if (!prot->hasBytesToRead())
        done = true;
      post_data.append(conn_buffer, conn_readed);
    }
  }
  else
  {
    std::size_t pos = resource.find(ULXR_CHAR('?'));
    if (pos != CppString::npos)
      post_data = getLatin1(resource.substr(pos+1));
  }

  while (post_data.length() != 0)
  {
    ULXR_TRACE(ULXR_PCHAR("formdata ") << ULXR_GET_STRING(post_data));
    std::size_t pos = 0;
    std::string element;
    if ( (pos = post_data.find(ULXR_CHAR('&'))) != CppString::npos)
    {
      element = post_data.substr(0, pos);
      post_data.erase(0, pos+1);
    }
    else
    {
      element = post_data.substr(0, pos);
      post_data = "";
    }

    ULXR_TRACE(ULXR_PCHAR("element ") << ULXR_GET_STRING(element));

    if ( (pos = element.find(ULXR_CHAR('='))) != CppString::npos)
    {
      std::string name = element.substr(0, pos);
      std::string value = element.substr(pos+1);
      formdata.addElement(encodeFromHtml(formDecode(name)), encodeFromHtml(formDecode(value)));
    }
    else
    {
      // ignore bad pair??
    }
  }

  return formdata;
}


ULXR_API_IMPL(std::string) HtmlFormHandler::formDecode(std::string &value)
{
  ULXR_TRACE(ULXR_PCHAR("formDecode 1 ") << ULXR_GET_STRING(value));

  std::size_t pos = 0;
  while ((pos = value.find('+')) != std::string::npos)
    value.replace(pos, 1, 1, ' ');

  ULXR_TRACE(ULXR_PCHAR("formDecode 2 ") << ULXR_GET_STRING(value));

  pos = 0;
  while ((pos = value.find('%', pos)) != std::string::npos)
  {
    if (value.length() >= pos+3)
    {
      int c1 = ulxr_toupper(value[pos+1]) - '0';
      if (c1 > 9)
        c1 += '0' - 'A' + 0x0A;

      int c2 = ulxr_toupper(value[pos+2]) - '0';
      if (c2 > 9)
        c2 += '0' - 'A' + 0x0A;

      ULXR_TRACE(ULXR_PCHAR("formDecode 3 ")
                 << (unsigned int) value[pos+1] << ULXR_PCHAR(" ")
                 << (unsigned int) c1 << ULXR_PCHAR(" ")
                 << (unsigned int) c2 );

      if (   c1 >= 0 && c1 <= 0x0F
          && c2 >= 0 && c2 <= 0x0F)
      {
        unsigned char x = c1 << 4 | c2;
        if (x !=0) // a remote bug or an attack?
          value.replace(pos, 3, 1, x);
      }
      pos++;
     }
     ULXR_TRACE(ULXR_PCHAR("formDecode 4 ") << ULXR_GET_STRING(value));
  }
  return value;
}


ULXR_API_IMPL(CppString) HtmlFormHandler::handle_subresource_list(const HtmlFormData &, CppString &mimetype)
{
  mimetype = ULXR_PCHAR("text/html");
  CppString resp;
  resp += ULXR_PCHAR("<table border=\"3\">\n")
          ULXR_PCHAR("<tr><td>Resource</td>")
          ULXR_PCHAR("<td>Description</td></tr>");

  for (unsigned i = 0; i < subResources.size(); ++i)
  {
    CppString s = applyTags(makeAnchor(subResources[i]->getName(), subResources[i]->getName()),
                            ULXR_PCHAR("td"))
                + applyTags(subResources[i]->getDescription(), ULXR_PCHAR("td"));
    resp += applyTags(s, ULXR_PCHAR("tr"));
  }

  return resp;
}


ULXR_API_IMPL(CppString) HtmlFormHandler::handle_css_file(const HtmlFormData &, CppString &mimetype)
{
  mimetype = ULXR_PCHAR("text/css");
  return ULXR_PCHAR("// No css data set\n");
}


ULXR_API_IMPL(CppString) HtmlFormHandler::getCssName() const
{
  return ULXR_PCHAR("ulxr.css");
}


ULXR_API_IMPL(CppString) HtmlFormHandler::getHeader(const CppString &title) const
{
  return ULXR_PCHAR("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\n")
         ULXR_PCHAR("<html><head><meta http-equiv=\"Content-Type\" content=\"text/html;charset=")
         + getEncoding()
         + ULXR_PCHAR("\">\n<title>")
         + title
         + ULXR_PCHAR("</title>\n<link href=\"")
         + getCssName()
         + ULXR_PCHAR("\" rel=\"stylesheet\" type=\"text/css\"></head><body>\n");
}


ULXR_API_IMPL(CppString) HtmlFormHandler::getTail() const
{
  return ULXR_PCHAR("\n</body></html>\n");
}


ULXR_API_IMPL(CppString) HtmlFormHandler::makeAnchor(const CppString &url, const CppString &descr)
{
  return ULXR_PCHAR("<a href=\"") + url + ULXR_PCHAR("\">")
         + descr + ULXR_PCHAR("</a>");
}


ULXR_API_IMPL(CppString) HtmlFormHandler::makeIdent(const CppString &name, unsigned index)
{
  return name + makeNumber(index);
}


ULXR_API_IMPL(CppString) HtmlFormHandler::openForm(const CppString &name, const CppString &method)
{
  return ULXR_PCHAR("<form name=\"")
         + name
         + ULXR_PCHAR("\" accept-charset=\"")
         + getEncoding()
         + ULXR_PCHAR("\" method=\"")
         + method
         + ULXR_PCHAR("\">\n");
}


ULXR_API_IMPL(CppString) HtmlFormHandler::closeForm()
{
  return ULXR_PCHAR("</form>\n");
}


ULXR_API_IMPL(CppString) HtmlFormHandler::applyTags(const CppString &data,
                                                 const CppString &tag,
                                                 bool newline)
{
  std::size_t pos = 0;
  CppString end_tag = tag;
  if ( (pos = tag.find(ULXR_CHAR(' '))) != CppString::npos)  // start tag contains attributes
    end_tag = tag.substr(0, pos-1);

  CppString resp = ULXR_PCHAR("<") + tag + ULXR_PCHAR(">")
                   + data
                   + ULXR_PCHAR("</") + end_tag + ULXR_PCHAR(">\n");

  if (newline)
    resp += ULXR_PCHAR("\n");

  return resp;

}


ULXR_API_IMPL(CppString) HtmlFormHandler::makeLineBreak()
{
  return ULXR_PCHAR("<br />");
}


ULXR_API_IMPL(CppString) HtmlFormHandler::makeCheckBox(const CppString &name,
                                                      const CppString &value,
                                                      bool checked)
{
  CppString chkd;
  if (checked)
    chkd = ULXR_PCHAR(" checked=\"checked\"");

  return  ULXR_PCHAR("<input type=\"checkbox\" name=\"")
          + name
          + ULXR_PCHAR("\" value=\"")
          + value
          + ULXR_PCHAR("\"")
          + chkd
          + ULXR_PCHAR("></input>\n");
}


ULXR_API_IMPL(CppString) HtmlFormHandler::openSelect(const CppString &name,
                                                    int size,
                                                    bool multi)
{
  CppString ret = ULXR_PCHAR("<select name=\"")
                  + name
                  + ULXR_PCHAR("\"");
  if (size >= 0)
    ret += ULXR_PCHAR(" size=\"")
           + makeNumber(size)
           + ULXR_PCHAR("\"");

  if (multi)
    ret += ULXR_PCHAR(" multiple=\"multiple\"");

  return ret + ULXR_PCHAR(">\n");
}


ULXR_API_IMPL(CppString) HtmlFormHandler::makeOption(const CppString &data,
                                                    const CppString &value,
                                                    bool selected)
{
  CppString ret = ULXR_PCHAR("<option");

  if (value.length() != 0)
    ret += ULXR_PCHAR(" value=\"")
           + value
           + ULXR_PCHAR("\"");

  if (selected)
    ret += ULXR_PCHAR(" selected=\"selected\"");

  return ret + ULXR_PCHAR(">") + data + ULXR_PCHAR("</option>\n");
}


ULXR_API_IMPL(CppString) HtmlFormHandler::closeSelect()
{
  return ULXR_PCHAR("</select>\n");
}


ULXR_API_IMPL(CppString) HtmlFormHandler::makeTextArea(const CppString &name,
                                                      const CppString &value,
                                                      int cols, int rows)
{
  CppString ret = ULXR_PCHAR("<textarea name=\"")
                  + name
                  + ULXR_PCHAR("\"");

  if (cols >= 0)
    ret += ULXR_PCHAR(" cols=\"")
           + makeNumber(cols)
           + ULXR_PCHAR("\"");

  if (rows >= 0)
    ret += ULXR_PCHAR(" rows=\"")
           + makeNumber(rows)
           + ULXR_PCHAR("\"");

  return ret + ULXR_PCHAR(">") + value + ULXR_PCHAR("</textarea>");
}


ULXR_API_IMPL(CppString) HtmlFormHandler::makeTextField(const CppString &name,
                                                       const CppString &value)
{
  return  ULXR_PCHAR("<input type=\"text\" name=\"")
          + name
          + ULXR_PCHAR("\" value=\"")
          + value
          + ULXR_PCHAR("\"></input>\n");
}


ULXR_API_IMPL(CppString) HtmlFormHandler::makeRadioButton(const CppString &name,
                                                         const CppString &value,
                                                         bool checked)
{
  CppString chkd;
  if (checked)
    chkd = ULXR_PCHAR(" checked=\"checked\"");

  return  ULXR_PCHAR("<input type=\"radio\" name=\"")
          + name
          + ULXR_PCHAR("\" value=\"")
          + value
          + ULXR_PCHAR("\"")
          + chkd
          + ULXR_PCHAR("></input>\n");
}


ULXR_API_IMPL(CppString) HtmlFormHandler::makeSubmitButton(const CppString &name,
                                                          const CppString &value)
{
  return ULXR_PCHAR("<input type=\"submit\" name=\"")
         + name
         + ULXR_PCHAR("\" value=\"")
         + value
         + ULXR_PCHAR("\"></input>\n");
}


ULXR_API_IMPL(CppString) HtmlFormHandler::makeResetButton(const CppString &name,
                                                         const CppString &value)
{
  return ULXR_PCHAR("<input type=\"reset\" name=\"")
         + name
         + ULXR_PCHAR("\" value=\"")
         + value
         + ULXR_PCHAR("\"></input>\n");
}


ULXR_API_IMPL(CppString) HtmlFormHandler::makeText(const CppString &name)
{
  return xmlEscape(name);
}


ULXR_API_IMPL(CppString) HtmlFormHandler::makeNumber(unsigned index)
{
#ifndef ULXR_UNICODE
  char cb[40];
  ulxr_sprintf(cb, "%d", index);
#else
  wchar_t cb[40];
# if defined(__BORLANDC__) || defined(_MSC_VER)
  ulxr_swprintf(cb, L"%d", index);
# else
  ulxr_swprintf(cb, sizeof(cb), L"%d", index);
# endif
#endif
  return CppString(cb);
}


ULXR_API_IMPL(CppString) HtmlFormHandler::makeHexNumber(unsigned index)
{
#ifndef ULXR_UNICODE
  char cb[40];
  if (index <= 0x0F)
    ulxr_sprintf(cb, "0x0%x", index);
  else
    ulxr_sprintf(cb, "0x%x", index);
#else
  wchar_t cb[40];
# if defined(__BORLANDC__) || defined(_MSC_VER)
  if (index <= 0x0F)
    ulxr_swprintf(cb, L"0x0%x", index);
  else
    ulxr_swprintf(cb, L"0x%x", index);
# else
  if (index <= 0x0F)
    ulxr_swprintf(cb, sizeof(cb), L"0x0%x", index);
  else
    ulxr_swprintf(cb, sizeof(cb), L"0x%x", index);
# endif
#endif
  return CppString(cb);
}


ULXR_API_IMPL(long) HtmlFormHandler::getLong(const CppString &num)
{
#ifndef ULXR_UNICODE
  return ulxr_atoi(num.c_str());
#else
  std::string asc = getLatin1(num);
  return ulxr_atoi(asc.c_str());
#endif
}


ULXR_API_IMPL(CppString) HtmlFormHandler::getEncoding() const
{
#ifdef ULXR_UNICODE
  return ULXR_PCHAR("UTF-8");
#else
  return ULXR_PCHAR("ISO-8859-1");
#endif
}


//////////////////////////////////////////////////////////////////////


ULXR_API_IMPL(unsigned) HtmlFormData::size() const
{
  return elements.size();
}


ULXR_API_IMPL(void) HtmlFormData::addElement( const CppString &name, const CppString &value)
{
  elements[name].push_back(value);
}


ULXR_API_IMPL(std::vector<CppString>) HtmlFormData::getElement(const CppString &name) const
{
  Elements::const_iterator it;
  if ((it = elements.find(name)) == elements.end())
    throw RuntimeException(ApplicationError, ulxr_i18n(ULXR_PCHAR("Index out out range for HtmlFormData::getElement() call")));
  else
    return (*it).second;
}


ULXR_API_IMPL(std::vector<CppString>) HtmlFormData::getElement(const CppString &base_name, unsigned index) const
{
  return getElement(base_name + HtmlFormHandler::makeNumber(index));
}


ULXR_API_IMPL(bool) HtmlFormData::hasElement(const CppString &name) const
{
  return elements.find(name) != elements.end();
}


ULXR_API_IMPL(bool) HtmlFormData::hasElement(const CppString &base_name, unsigned index) const
{
  return hasElement(base_name + HtmlFormHandler::makeNumber(index));
}


} // namespace ulxr

