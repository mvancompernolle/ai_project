/***************************************************************************
              https_server.cpp  --  test file for an https server

    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: https_server.cpp 1062 2007-08-19 09:07:58Z ewald-arnold $

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
//#define ULXR_SHOW_HTTP

#include <ulxmlrpcpp/ulxmlrpcpp.h>  // always first header

#include <iostream>
#include <ctime>
#include <cstdio>
#include <cstring>

#include <ulxmlrpcpp/ulxr_ssl_connection.h>// first, don't move: msvc #include bug
#include <ulxmlrpcpp/ulxr_htmlform_handler.h>
#include <ulxmlrpcpp/ulxr_http_server.h>
#include <ulxmlrpcpp/ulxr_http_protocol.h>
#include <ulxmlrpcpp/ulxr_dispatcher.h>
#include <ulxmlrpcpp/ulxr_except.h>
#include <ulxmlrpcpp/ulxr_signature.h>

#ifndef ULXR_OMIT_REENTRANT_PROTECTOR
#include <ulxmlrpcpp/ulxr_mutex.h>
#endif

#ifdef __WIN32__
const unsigned num_threads = 10;
#else
const unsigned num_threads = 10;
#endif

ulxr::HttpServer *my_http_server;

////////////////////////////////////////////////////////////////////////////////////
// C-style handler, do everything manually


std::string handle_css_c()
{
  return "H1 { text-align: center; color: blue; }\n"
         "A  { color: green; }\n";
}


std::string get_head_c(const std::string &title)
{
  return std::string("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">"
           "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html;charset=iso-8859-1\">"
           "<title>")
         + title
         + std::string("</title>"
             "<link href=\"ulxr_system.css\" rel=\"stylesheet\" type=\"text/css\">"
             "</head><body>");
}


std::string get_tail_c()
{
  return "</body></html>";
}


std::string handle_index_c()
{
  ULXR_TRACE(ULXR_PCHAR("handle_index_c"));
  std::string resp = get_head_c("Index") +
                     "<h1>Index</h1>"
                     "<p>This page contains information about the example http server"
                     " shipped with <a href=\"http://ulxmlrpcpp.sf.net\">ulxmlrpcpp</a>.</p>"
                     "<a href=\"general.html\">General System information<a><br />"
                     "<a href=\"special.html\">Special System information<a><br /><br />"
                     "<a href=\"/rpc-info/index.html\">Information about the built-in XML-RPC server<a><br />"
                     "<a href=\"/rpc-info/calc.html\">A simple calculator<a><br />"
                     "<a href=\"/rpc-info/elements.html\">Display all form elements<a>"
                     + get_tail_c();
  return resp;
}


std::string handle_special_c()
{
  ULXR_TRACE(ULXR_PCHAR("handle_special_c"));
  std::string resp = get_head_c("Special") +
                     "<h1>Special Information</h1>"
                     "<p>This page contains really special information.</p>"
                     "<a href=\"index.html\">Index<a><br />"
                     "<a href=\"general.html\">General System information<a>"
                     + get_tail_c();
  return resp;
}


std::string handle_general_c()
{
  ULXR_TRACE(ULXR_PCHAR("handle_general_c"));

  int ma;
  int mi;
  int pa;
  bool deb;
  ulxr::CppString info;
  ulxr::getVersion (ma, mi, pa, deb, info);
  char s_ma[40];
  char s_mi[40];

  ulxr_sprintf(s_ma, "%d", ma);
  ulxr_sprintf(s_mi, "%d", mi);

#ifndef ULXR_OMIT_REENTRANT_PROTECTOR   // todo: optionally replace with ctime_r
  ulxr::Mutex::Locker lock(ulxr::ctimeMutex);
#endif

  ulxr_time_t tt = ulxr_time(0);
  std::string tm (ulxr_ctime(&tt));

  std::string resp = get_head_c("General") +
                     "<h1>General Information</h1>"
                     "<p>This page contains very general information.</p>"
                     "<p>"
                     "Current time: " + tm + "<br />"
                     "Library version: " + s_ma + "." + s_mi + "<br />"
                     "</p>"
                     "<a href=\"index.html\">Index<a><br />"
                     "<a href=\"special.html\">Special System information<a>"
                     + get_tail_c();
  return resp;
}


#ifdef ULXR_SHOW_TRACE
bool systemInfoHandler(ulxr::HttpProtocol *prot,
                       const ulxr::CppString &method,
                       const ulxr::CppString &resource,
                       const ulxr::Cpp8BitString &/*conn_data*/)
#else
bool systemInfoHandler(ulxr::HttpProtocol *prot,
                       const ulxr::CppString &/*method*/,
                       const ulxr::CppString &resource,
                       const ulxr::Cpp8BitString &/*conn_data*/)
#endif
{
  ULXR_TRACE(ULXR_PCHAR("systemInfoHandler ") << method << " " << resource);

  ulxr::CppString rsc_start = ULXR_PCHAR("/system/");
  if (resource.substr(0, rsc_start.length()) == rsc_start)
  {
    ulxr::CppString target = resource.substr(rsc_start.length());
    std::string resp;

    if (target.length() == 0 || target == ULXR_PCHAR("index.html"))
      resp = handle_index_c();

    else if (target == ULXR_PCHAR("general.html"))
      resp = handle_general_c();

    else if (target == ULXR_PCHAR("special.html"))
      resp = handle_special_c();

    else if (target == ULXR_PCHAR("ulxr_system.css"))
      resp = handle_css_c();

    else
      return false;

    prot->sendResponseHeader(200, ULXR_PCHAR("OK"), ULXR_PCHAR("text/html"), resp.length());
    prot->writeRaw(resp.data(), resp.length());

    return true;
  }
  else
    return false;
}

// C-style handler
////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////
// class-style handler, use helper class extensively

class RpcInfoHandler : public ulxr::HtmlFormHandler
{
  public:

    RpcInfoHandler(ulxr::Dispatcher *in_disp)
      : ulxr::HtmlFormHandler(ULXR_PCHAR("/rpc-info/"))
    {
      disp = in_disp;

      addSubResource(ULXR_PCHAR(""),              this, &RpcInfoHandler::handle_index, ULXR_PCHAR("The main index page in this subresource"));
      addSubResource(ULXR_PCHAR("index.html"),    this, &RpcInfoHandler::handle_index, ULXR_PCHAR("The main index page in this subresource"));
      addSubResource(ULXR_PCHAR("calc.html"),     this, &RpcInfoHandler::handle_calc, ULXR_PCHAR("A simple calculator"));
      addSubResource(ULXR_PCHAR("elements.html"), this, &RpcInfoHandler::handle_elements, ULXR_PCHAR("A sample page with form elements"));

      addSubResource(ULXR_PCHAR("subresources.html"), (ulxr::HtmlFormHandler*)this, &ulxr::HtmlFormHandler::handle_subresource_list, ULXR_PCHAR("Display subresources"));
      // css already handled by parent
    }


    virtual ulxr::CppString handle_css_file(const ulxr::HtmlFormData &, ulxr::CppString &mimetype)
    {
      mimetype = ULXR_PCHAR("text/css");
      return ULXR_PCHAR("H1 { text-align: center; color: red; }\n")
             ULXR_PCHAR("A  { color: gray; }\n");
    }


    ulxr::CppString handle_index(const ulxr::HtmlFormData &formdata, ulxr::CppString &mimetype)
    {
      mimetype = ULXR_PCHAR("text/html");
      if (formdata.size() != 0 && formdata.hasElement(ULXR_PCHAR("method_modifier")))  // first update rpc-server
        for (unsigned i = 0; i < disp->numMethods(); ++i)
          disp->getMethod(i)->setEnabled(formdata.hasElement(ULXR_PCHAR("method_enable_"), i) == true);

      ULXR_TRACE(ULXR_PCHAR("handle_index"));
      ulxr::CppString resp = getHeader(ULXR_PCHAR("Index"))
                             + ULXR_PCHAR("<h1>Index</h1>")
                             + makeAnchor(ULXR_PCHAR("subresources.html"), ULXR_PCHAR("Show all subresources of this branch"))
                             + makeLineBreak()
                             + ULXR_PCHAR("<p>The following table contains information about the available rpc methods.</p>\n");

      resp += openForm(ULXR_PCHAR("rpc-controller"), ULXR_PCHAR("POST"));

      resp += ULXR_PCHAR("<table border=\"3\">\n")
              ULXR_PCHAR("<tr><td>Return type</td>")
              ULXR_PCHAR("<td>Method name</td>")
              ULXR_PCHAR("<td>Parameter signature</td>")
              ULXR_PCHAR("<td>Type</td>")
              ULXR_PCHAR("<td>Description</td>")
              + applyTags(ULXR_PCHAR("Invoked"), ULXR_PCHAR("td"))
              + applyTags(ULXR_PCHAR("Enabled"), ULXR_PCHAR("td"))
              + makeSubmitButton(ULXR_PCHAR("method_modifier"), ULXR_PCHAR("Update"))
              + ULXR_PCHAR("</td>");

      ULXR_TRACE(ULXR_PCHAR("handle_index 1"));
      for (unsigned i = 0; i < disp->numMethods(); ++i)
      {
        ulxr::CppString id = makeNumber(i);

        const ulxr::Dispatcher::MethodCallDescriptor *meth = disp->getMethod(i);
        resp += ULXR_PCHAR("<tr>");
        resp += ULXR_PCHAR("<td>") + meth->getReturnValueSignature() + ULXR_PCHAR("</td>");
        resp += ULXR_PCHAR("<td>") + meth->getMethodName()           + ULXR_PCHAR("</td>");
        resp += ULXR_PCHAR("<td>") + meth->getParameterSignature()   + ULXR_PCHAR("</td>");

        if (meth->getCallType() == ulxr::Dispatcher::CallSystem)
          resp +=   ULXR_PCHAR("<td>system</td>");
        else
          resp +=   ULXR_PCHAR("<td>external</td>");

        resp += ULXR_PCHAR("<td>") + meth->getDocumentation() + ULXR_PCHAR("</td>");

        resp += ULXR_PCHAR("<td>")  + makeNumber(meth->getInvoked()) +  ULXR_PCHAR("</td>");

        resp += ULXR_PCHAR("<td>")
                + makeCheckBox(makeIdent(ULXR_PCHAR("method_enable_"), i), ULXR_PCHAR("on"), meth->isEnabled())
                + ULXR_PCHAR("</td>");

        resp += ULXR_PCHAR("</tr>\n");
      }
      resp += ULXR_PCHAR("</table>");
      resp += closeForm();
      resp += getTail();
      return resp;
    }


    ulxr::CppString handle_calc(const ulxr::HtmlFormData &formdata, ulxr::CppString &mimetype)
    {
      mimetype = ULXR_PCHAR("text/html");
      ulxr::CppString rhs;
      ulxr::CppString lhs;
      ulxr::CppString result;
      ulxr::CppString op = ULXR_PCHAR("plus");

      if (   formdata.size() != 0
          && formdata.hasElement(ULXR_PCHAR("lhs"))
          && formdata.hasElement(ULXR_PCHAR("rhs"))
          && formdata.hasElement(ULXR_PCHAR("operator"))
         )
      {
        int l = getLong(formdata.getElement(ULXR_PCHAR("lhs"))[0]);
        int r = getLong(formdata.getElement(ULXR_PCHAR("rhs"))[0]);
        int res;
        op = formdata.getElement(ULXR_PCHAR("operator"))[0];
        if (op == ULXR_PCHAR("plus"))
          res = l + r;

        else if (op == ULXR_PCHAR("minus"))
          res = l - r;

        else if (op == ULXR_PCHAR("mul"))
          res = l * r;

        else if (op == ULXR_PCHAR("div"))
        {
          if (r == 0)
            res = 0;
          else
            res = l / r;
        }
        else
          res = 0;

        rhs = makeNumber(r);
        lhs = makeNumber(l);
        result = makeNumber(res);
      }

      ULXR_TRACE(ULXR_PCHAR("handle_calc"));
      ulxr::CppString resp = getHeader(ULXR_PCHAR("Calculator"))
                             + ULXR_PCHAR("<h1>Calculator</h1>")
                               ULXR_PCHAR("<p>Enter two numbers and select an operator.</p>\n");

      resp += openForm(ULXR_PCHAR("magic-calculator"), ULXR_PCHAR("POST"));

      resp += makeTextField(ULXR_PCHAR("lhs"), lhs);

      resp += openSelect(ULXR_PCHAR("operator"));
      resp += makeOption(ULXR_PCHAR("+"), ULXR_PCHAR("plus"),  op == ULXR_PCHAR("plus"));
      resp += makeOption(ULXR_PCHAR("-"), ULXR_PCHAR("minus"), op == ULXR_PCHAR("minus"));
      resp += makeOption(ULXR_PCHAR("*"), ULXR_PCHAR("mul"),   op == ULXR_PCHAR("mul"));
      resp += makeOption(ULXR_PCHAR("/"), ULXR_PCHAR("div"),   op == ULXR_PCHAR("div"));
      resp += closeSelect();

      resp += makeTextField(ULXR_PCHAR("rhs"), rhs);

      resp += ULXR_PCHAR(" = ");

      resp += makeTextField(ULXR_PCHAR("result"), result);

      resp += ULXR_PCHAR("<br /><br />");

      resp += makeSubmitButton(ULXR_PCHAR("start_calc"), ULXR_PCHAR("Calculate"));
      resp += closeForm();

      resp += getTail();
      return resp;
    }


    ulxr::CppString handle_elements(const ulxr::HtmlFormData &formdata, ulxr::CppString &mimetype)
    {
      mimetype = ULXR_PCHAR("text/html");
      ULXR_TRACE(ULXR_PCHAR("handle_elements"));
      ulxr::CppString resp = getHeader(ULXR_PCHAR("Available Form Elements"))
                             + ULXR_PCHAR("<h1>Available Form Elements</h1>");

      resp += openForm(ULXR_PCHAR("testform"), ULXR_PCHAR("POST"))
              + ULXR_PCHAR("Checkbox 1")
              + makeCheckBox (ULXR_PCHAR("Check1"), ULXR_PCHAR("on"), true)
              + makeLineBreak()

              + ULXR_PCHAR("Checkbox 2")
              + makeCheckBox (ULXR_PCHAR("Check2"), ULXR_PCHAR("on"), false)
              + makeLineBreak()

              + ULXR_PCHAR("<br /><br />")

              + makeTextField(ULXR_PCHAR("Textfield"), ULXR_PCHAR("textdata"))
              + makeLineBreak();

      if (formdata.hasElement(ULXR_PCHAR("Textfield")))
      {
        resp += makeLineBreak()
                + makeText(ULXR_PCHAR("Data from text field above:"))
                + makeLineBreak()
                + formdata.getElement(ULXR_PCHAR("Textfield"))[0]
                + makeLineBreak()
                + makeLineBreak();
      }

      resp += ULXR_PCHAR("<br /><br />")
              + makeTextArea(ULXR_PCHAR("TextArea"), ULXR_PCHAR("text +&%= line1\nline 2\nline 3"), 40,  5)
              + makeLineBreak();

      if (formdata.hasElement(ULXR_PCHAR("TextArea")))
      {
        resp += makeLineBreak()
                + makeText(ULXR_PCHAR("Data from text area above:"))
                + makeLineBreak()
                + formdata.getElement(ULXR_PCHAR("TextArea"))[0]
                + makeLineBreak()
                + makeLineBreak();
      }

      resp += ULXR_PCHAR("<br /><br />")

              + openSelect(ULXR_PCHAR("Select1"), 5, true)
              + makeOption(ULXR_PCHAR("Option 1"))
              + makeOption(ULXR_PCHAR("Option 2"), ULXR_PCHAR("opt2"))
              + makeOption(ULXR_PCHAR("Option 3"), ULXR_PCHAR(""), true)
              + makeOption(ULXR_PCHAR("Option 4"), ULXR_PCHAR(""), true)
              + closeSelect()

              + makeLineBreak()

              + openSelect(ULXR_PCHAR("Select2"), 1, false)
              + makeOption(ULXR_PCHAR("Option 21"))
              + makeOption(ULXR_PCHAR("Option 22"), ULXR_PCHAR("opt2"))
              + makeOption(ULXR_PCHAR("Option 23"), ULXR_PCHAR(""), true)
              + makeOption(ULXR_PCHAR("Option 24"), ULXR_PCHAR(""), true)
              + closeSelect()

              + ULXR_PCHAR("<br /><br />")

              + makeRadioButton(ULXR_PCHAR("radio1"), ULXR_PCHAR("val1")) + ULXR_PCHAR("Radio 1a") + makeLineBreak()
              + makeRadioButton(ULXR_PCHAR("radio1"), ULXR_PCHAR("val2"), true) + ULXR_PCHAR("Radio 1b") + makeLineBreak()
              + makeRadioButton(ULXR_PCHAR("radio2"), ULXR_PCHAR("val3"), true) + ULXR_PCHAR("Radio 3") + makeLineBreak()

              + makeLineBreak()

              + makeSubmitButton(ULXR_PCHAR("submit"), ULXR_PCHAR("Do Submit"))
              + makeResetButton(ULXR_PCHAR("reset"), ULXR_PCHAR("Do Reset"))

              + closeForm()

              + getTail();

      return resp;
    }


  private:
    ulxr::Dispatcher *disp;
};

// class-style handler
////////////////////////////////////////////////////////////////////////////////////


ulxr::MethodResponse testcall (const ulxr::MethodCall &calldata)
{
  ulxr::Integer i = calldata.getParam(0);
  ulxr::Integer i2 = calldata.getParam(1);

  ulxr::Struct st;
  st.addMember(ULXR_PCHAR("before"), i);
  st.addMember(ULXR_PCHAR("sum"), ulxr::Integer(i.getInteger()+i2.getInteger()));
  st.addMember(ULXR_PCHAR("difference"), ulxr::Integer(i.getInteger()-i2.getInteger()));

  return ulxr::MethodResponse (st);
}


ulxr::MethodResponse numThreads (const ulxr::MethodCall &/*calldata*/)
{
#ifdef ULXR_MULTITHREADED
  return ulxr::MethodResponse(ulxr::Integer(my_http_server->numThreads()));
#else
  return ulxr::MethodResponse(ulxr::Integer(1));
#endif
}


ulxr::MethodResponse shutdown (const ulxr::MethodCall &/*calldata*/)
{
  ULXR_COUT << ULXR_PCHAR("got signal to shut down\n");
  my_http_server->requestTermination();
  return ulxr::MethodResponse(ulxr::Boolean(true));
}


int main(int argc, char **argv)
{
  int ret = 0;

#ifndef ULXR_OMIT_TCP_STUFF
#ifdef ULXR_INCLUDE_SSL_STUFF

  try
  {
    ulxr::intializeLog4J(argv[0]);

    ulxr::CppString host = ULXR_PCHAR("localhost");
    if (argc > 1)
      host = ULXR_GET_STRING(argv[1]);

    unsigned port = 32006;
    if (argc > 2)
      port = ulxr_atoi(argv[2]);

    bool auth = true;
    if (argc > 3)
      auth = (bool) ulxr_atoi(argv[3]);

    bool wbxml = false;
    if (argc > 4)
      wbxml = ulxr_atoi(argv[4]) != 0;

    ulxr::SSLConnection conn (true, host, port); // SSL
    conn.setConnectionTimeout(conn.getTimeout(), 60 * 60);
    conn.setCryptographyData("password", "foo-cert.pem", "foo-cert.pem");
    ulxr::HttpProtocol prot(&conn);
    ulxr::HttpServer http_server (&prot, num_threads, wbxml);
    my_http_server = &http_server;
    http_server.addHttpHandler(ULXR_PCHAR("get"), ulxr::make_methodhandler(&systemInfoHandler));

    ulxr::Dispatcher rpc_server(0, wbxml);

    RpcInfoHandler rpcinfo(&rpc_server);
    http_server.addHttpHandler(ULXR_PCHAR("get"),  ulxr::make_methodhandler(rpcinfo, &RpcInfoHandler::handler));
    http_server.addHttpHandler(ULXR_PCHAR("post"), ulxr::make_methodhandler(rpcinfo, &RpcInfoHandler::handler));

    if (auth)
    {
      http_server.addRealm(ULXR_CHAR("/"), ULXR_CHAR("http-root-resource"));
      http_server.addAuthentication(ULXR_CHAR("ali-baba"),
                                    ULXR_CHAR("open-sesame"),
                                    ULXR_CHAR("http-root-resource"));

      http_server.addRealm(ULXR_CHAR("/RPC2"), ULXR_CHAR("rpc2-resource"));
      http_server.addAuthentication(ULXR_CHAR("ali-baba-rpc"),
                                    ULXR_CHAR("open-sesame-rpc"),
                                    ULXR_CHAR("rpc2-resource"));
    }

    ulxr::CppString xs = conn.getErrorString(10035);

#ifdef _WIN32
    ulxr::CppString root_dir = ULXR_PCHAR("x:\\public_html");
#else
    std::string home = getenv("HOME");
    ulxr::CppString root_dir = ULXR_GET_STRING(home)+ULXR_PCHAR("/public_html");
#endif
    ULXR_COUT << ULXR_PCHAR("Server root directory is: ") << root_dir << std::endl;
    ULXR_COUT << ULXR_PCHAR("Accepting https request at ") << host << ULXR_PCHAR(":") << port << std::endl;
    ULXR_COUT << ULXR_PCHAR("Please point your browser to \n  https://")
              << host << ULXR_PCHAR(":") << port << ULXR_PCHAR("/system/")
              << ULXR_PCHAR("\nor to \n  https://")
              << host << ULXR_PCHAR(":") << port << ULXR_PCHAR("/rpc-info/")<< std::endl
              << ULXR_PCHAR("username is \"ali-baba\" and password is \"open-sesame\"") << std::endl;
    ULXR_COUT << ULXR_PCHAR("WBXML: ") << wbxml << std::endl;

    http_server.setHttpRoot(root_dir);

    rpc_server.addMethod(&testcall,
                         ulxr::Struct::getValueName(),
                         ULXR_CHAR("testcall"),
                         ulxr::Integer::getValueName() + ULXR_CHAR(",") + ulxr::Integer::getValueName(),
                         ULXR_CHAR("Testcase with a c-function"));

    rpc_server.addMethod(ulxr::make_method(&shutdown),
                         ulxr::Signature(),
                         ULXR_PCHAR("shutdown"),
                         ulxr::Signature(),
                         ULXR_PCHAR("Shut down Worker"));

    rpc_server.addMethod(ulxr::make_method(&numThreads),
                         ulxr::Signature(ulxr::Integer()),
                         ULXR_PCHAR("numthreads"),
                         ulxr::Signature(),
                         ULXR_PCHAR("Returns number of installed threads at startup"));

    http_server.setRpcDispatcher(&rpc_server);

    // run forever...
    unsigned started = http_server.runPicoHttpd();
    ULXR_COUT << ULXR_PCHAR("Started ") << started << ULXR_PCHAR(" threads for dispatching http and rpc requests\n");
    if (started > 1)
      http_server.waitAsync(false, true);
    ULXR_COUT << ULXR_PCHAR("Done.\n");
  }

  catch(ulxr::Exception &ex)
  {
     ULXR_COUT << ULXR_CHAR("Error occured: ") << ex.why() << std::endl;
     ret = 1;
  }

  catch(...)
  {
     ULXR_COUT << ULXR_CHAR("unknown Error occured.\n");
     ret = 1;
  }

  if (!ret)
    ULXR_COUT << ULXR_CHAR("Ready and ok.\n");
  else
    ULXR_COUT << ULXR_CHAR("Ready but not ok.\n");

#endif  // ULXR_OMIT_TCP_STUFF
#endif  // ULXR_INCLUDE_SSL_STUFF

  return ret;
}


