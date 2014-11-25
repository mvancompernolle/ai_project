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

#include <Vaca/Vaca.h>
#include <memory>
#include "Document.h"
#include "Editor.h"

using namespace Vaca;

enum {
  ID_NEW_DOCUMENT,
  ID_CLOSE_DOCUMENT,
  ID_EXIT,
  ID_SELECT_DOCUMENT
};

//////////////////////////////////////////////////////////////////////
// Example class definition

class Document;
class MainFrame;

class Example : public Application
	      , public CommandsClient
{
  typedef std::vector<Document*> Documents;

  int m_docCounter;
  Documents m_docs;
  Document* m_currentDoc;
  std::auto_ptr<MainFrame> m_mainFrame;

public:
  Example();
  virtual ~Example();
  virtual void main();

  void onNew();
  void onClose();

  bool hasCurrentDoc();
  void setCurrentDocument(Document* doc);

  Documents getDocuments() { return m_docs; };
  Document* getCurrentDocument() { return m_currentDoc; };

  Signal1<void, Document*> DocumentChange;
  Signal1<void, Document*> NewDocument;
  Signal1<void, Document*> CloseDocument;
};

Example* GetApp()
{
  return static_cast<Example*>(Application::getInstance());
}

//////////////////////////////////////////////////////////////////////
// DocMenuItem

class DocMenuItem : public MenuItem
{
  Document* m_doc;
public:
  DocMenuItem(Document* doc)
    : MenuItem(doc->getName(), 0)
    , m_doc(doc) {
  }

  Document* getDoc() {
    return m_doc;
  }

  virtual void onAction(MenuItemEvent& ev) {
    MenuItem::onAction(ev);
    GetApp()->setCurrentDocument(m_doc);
  }
};

//////////////////////////////////////////////////////////////////////
// MainFrame

class MainFrame : public Frame
{
  Editor m_editor;
  Menu* m_listMenu;

public:
  MainFrame()
    : Frame("Undo (WIP)")
    , m_editor(this)
  {
    setMenuBar(createMenuBar());
    setLayout(new BoxLayout(Orientation::Horizontal, false));
    m_editor.setConstraint(new BoxConstraint(true));

    GetApp()->DocumentChange.connect(&MainFrame::onDocumentChange, this);
    GetApp()->NewDocument.connect(&MainFrame::onNewDocument, this);
    GetApp()->CloseDocument.connect(&MainFrame::onCloseDocument, this);
  }

private:
  MenuBar* createMenuBar()
  {
    MenuBar* menuBar = new MenuBar();
    Menu* docMenu = new Menu("Document");
    m_listMenu = new Menu("List");

    docMenu->add("New", ID_NEW_DOCUMENT);
    docMenu->add("Close", ID_CLOSE_DOCUMENT);
    docMenu->addSeparator();
    docMenu->add("Exit", ID_EXIT);

    menuBar->add(docMenu);
    menuBar->add(m_listMenu);
    return menuBar;
  }

  void onDocumentChange(Document* doc)
  {
    m_editor.setDocument(doc);
  }

  void onNewDocument(Document* doc)
  {
    m_listMenu->add(new DocMenuItem(doc));
    updateListMenuIds();
  }

  void onCloseDocument(Document* doc)
  {
    int i, count = m_listMenu->getItemCount();
    for (i=0; i<count; ++i) {
      DocMenuItem* item = static_cast<DocMenuItem*>(m_listMenu->getMenuItemByIndex(i));
      if (item->getDoc() == doc) {
	m_listMenu->remove(item);
	updateListMenuIds();
	delete item;
	break;
      }
    }
  }

  // Important: After we modified the "m_listMenu" we have to call
  // this routine to setup the IDs of each menu-item in the menu.
  // See TN013
  void updateListMenuIds()
  {
    int i, count = m_listMenu->getItemCount();
    for (i=0; i<count; ++i) {
      MenuItem* item = m_listMenu->getMenuItemByIndex(i);
      item->setId(ID_SELECT_DOCUMENT+i);
    }
  }

};

//////////////////////////////////////////////////////////////////////
// Example implementation

Example::Example()
  : m_docCounter(0)
  , m_currentDoc(NULL)
{
  // Warning: the "Example::m_mainFrame" must be constructed after the
  // "Example::DocumentChange" member is constructed. See TN012.
  m_mainFrame.reset(new MainFrame());

  // commands
  addCommand(new SignalCommand(ID_NEW_DOCUMENT, &Example::onNew, this));
  addCommand(new SignalCommand(ID_EXIT, Bind(&MainFrame::setVisible, m_mainFrame.get(), false)));
  
  SignalCommand* cmd = new SignalCommand(ID_CLOSE_DOCUMENT);
  cmd->Execute.connect(&Example::onClose, this);
  cmd->Enabled.connect(&Example::hasCurrentDoc, this);
  addCommand(cmd);
}

Example::~Example()
{
  for (Documents::iterator
	 it=m_docs.begin(); it!=m_docs.end(); ++it) {
    delete *it;
  }
}

void Example::main()
{
  m_mainFrame->setVisible(true);
}

void Example::onNew()
{
  m_docCounter++;

  // create the new document
  Document* doc = new Document(String("Document #" + String::fromInt(m_docCounter)));
  doc->add(0, "Text " + String::fromInt(m_docCounter));

  // add the document to the list
  m_docs.push_back(doc);

  // new document signal, and set this document as the current one
  NewDocument(doc);
  DocumentChange(m_currentDoc = doc);
}

void Example::onClose()
{
  CloseDocument(m_currentDoc);
  DocumentChange(m_currentDoc = NULL);
}

bool Example::hasCurrentDoc()
{
  return m_currentDoc != NULL;
}

void Example::setCurrentDocument(Document* doc)
{
  DocumentChange(m_currentDoc = doc);
}

//////////////////////////////////////////////////////////////////////

int VACA_MAIN()
{
  Example app;
  app.run();
  return 0;
}
