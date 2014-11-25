// Vaca - Visual Application Components Abstraction
// Copyright (c) 2005, 2006, 2007, 2008, David A. Capello
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
// * Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in
//   the documentation and/or other materials provided with the
//   distribution.
// * Neither the name of the Vaca nor the names of its contributors
//   may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.

// we include the entire Vaca namespace, in this way we don't worry
// about what classes are accesible: we have all the classes available
#include <Vaca/Vaca.h>

// we can use the Vaca namespace by default to avoid writting things
// like Vaca::Frame, Vaca::LinkLabel, Vaca::Font, etc.
using namespace Vaca;

// the main window
class MainFrame : public Frame
{
  // a label to represent a link to Internet
  LinkLabel m_label;

  // a font to use with the link
  Font m_linkFont;

public:

  // constructs the main frame
  MainFrame()
    // call the parent class constructor, with the title "HelloWorld"
    : Frame("HelloWorld")
    // construct the link with the URL and the label to show,
    // every Widget receive the parent, in this case, "this" pointer
    , m_label("http://vaca.sourceforge.net/", "Hello World!", this)
    // construct a font to use in the link label
    , m_linkFont("Verdana", 14)
  {
    // set the layout to the client area, because the link'll use the
    // entire client area
    setLayout(new ClientLayout);

    // set the horizontal label text alignment to the center
    m_label.setTextAlign(TextAlign::Center);

    // set the link font to the loaded font
    m_label.setFont(m_linkFont);

    // finally we set the main frame's size to its preferred size
    setSize(getPreferredSize());
  }

};

// the example application, every Vaca application must to have one
// instance of Application (or a class derived from it)
class Example : public Application
{
  // the main frame
  MainFrame m_mainFrame;

public:

  // it's called when we use Application::run...
  virtual void main() {
    // at this point the frame is created (the constructor was called
    // from the Example constructor)...

    // ...so we can make it visible to the user
    m_mainFrame.setVisible(true);

    // After Application::main the Application::doMessageLoop method
    // is called automatically. That routine gets and dispatches
    // messages from/to any visible Frame.
  }

};

// the WinMain, every Windows application should have this function
int VACA_MAIN()
{
  try {
    // create the example application
    Example app;

    // run the application (calling Example::main)
    app.run();

    // the destructor of "Example" is called here
  }
  catch (Exception& e) {
    // if some errors come from Vaca show it (this method shows the
    // error as a MsgBox)
    e.show();
  }
  return 0;
}
