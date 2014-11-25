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

#include "Vaca/ComboBox.h"
#include "Vaca/Event.h"
#include "Vaca/Debug.h"
#include "Vaca/System.h"
#include "Vaca/WidgetClass.h"

using namespace Vaca;

//////////////////////////////////////////////////////////////////////
// ComboBox

ComboBox::ComboBox(Widget* parent, Style style)
  : Widget(WidgetClassName(WC_COMBOBOX), parent, style)
{
  setBgColor(System::getColor(COLOR_WINDOW));
}

ComboBox::~ComboBox()
{
}

/**
 * (CB_ADDSTRING)
 */
int ComboBox::addItem(const String& text)
{
  int index = sendMessage(CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(text.c_str()));
  if (index == LB_ERR)
    return -1;
  else
    return index;
}

/**
 * (CB_INSERTSTRING)
 * 
 */
void ComboBox::insertItem(int itemIndex, const String& text)
{
  sendMessage(CB_INSERTSTRING, itemIndex, reinterpret_cast<LPARAM>(text.c_str()));
}

/**
 * (CB_DELETESTRING)
 */
void ComboBox::removeItem(int itemIndex)
{
  sendMessage(CB_DELETESTRING, itemIndex, 0);
}

/**
 * (CB_RESETCONTENT)
 */
void ComboBox::removeAllItems()
{
  sendMessage(CB_RESETCONTENT, 0, 0);
}

/**
 * (CB_GETCOUNT)
 */
int ComboBox::getItemCount()
{
  return sendMessage(CB_GETCOUNT, 0, 0);
}

/**
 * (CB_GETLBTEXT and CB_GETLBTEXTLEN)
 */
String ComboBox::getItemText(int itemIndex)
{
  int len = sendMessage(CB_GETLBTEXTLEN, itemIndex, 0);
  if (!len)
    return String("");
  else {
    LPTSTR buf = (LPTSTR)new _TCHAR[len+1];
    sendMessage(CB_GETLBTEXT, itemIndex, reinterpret_cast<LPARAM>(buf));
    String str = String(buf);
    delete buf;
    return str;
  }
}

void ComboBox::setItemText(int itemIndex, const String& text)
{
  bool reselect = getCurrentItem() == itemIndex;

  removeItem(itemIndex);
  insertItem(itemIndex, text);

  if (reselect)
    setCurrentItem(itemIndex);
}

/**
 * Returns the current selected item index (CB_GETCURSEL). Returns -1
 * if there aren't selection at all.
 */
int ComboBox::getCurrentItem()
{
  int index = sendMessage(CB_GETCURSEL, 0, 0);
  if (index != CB_ERR && index >= 0)
    return index;
  else
    return index;
}

/**
 * Changes the current selected item to the @a itemIndex only (CB_SETCURSEL).
 */
void ComboBox::setCurrentItem(int itemIndex)
{
  sendMessage(CB_SETCURSEL, itemIndex, 0);
}

/**
 * Selects the item which its text begins with @a firstText.
 */
void ComboBox::setCurrentItem(const String& firstText)
{
  sendMessage(CB_SELECTSTRING,
	      static_cast<WPARAM>(-1),
	      reinterpret_cast<LPARAM>(firstText.c_str()));
}

void ComboBox::setDropDownVisibile(bool visible)
{
  sendMessage(CB_SHOWDROPDOWN, visible, 0);
}

bool ComboBox::isDropDownVisible()
{
  return sendMessage(CB_GETDROPPEDSTATE, 0, 0);
}

Rect ComboBox::getDropDownBounds()
{
  RECT rc;
  sendMessage(CB_GETDROPPEDCONTROLRECT, 0, reinterpret_cast<LPARAM>(&rc));
  return Rect(&rc);
}

void ComboBox::layout()
{
  Widget::layout();
  
  if ((getStyle().regular & 3) <= 1) { // CBS_SIMPLE = 1
    // do nothing
  }
  else {
    Rect rc = getBounds();
    Rect arc = getAbsoluteBounds();
    Rect wa = System::getWorkAreaBounds();
    int maxHeightTop = arc.y-wa.y;
    int maxHeightBottom = wa.h-(arc.y-wa.y+arc.h);
  
    setBounds(rc.x, rc.y, rc.w,
	      VACA_MIN(rc.h + getHeightForAllItems(),
		       VACA_MAX(maxHeightTop,
				maxHeightBottom)));
  }
}

int ComboBox::getHeightForAllItems()
{
  // TODO CBS_OWNERDRAWVARIABLE, see the MSDN doc of CB_GETITEMHEIGHT
  int height = sendMessage(CB_GETITEMHEIGHT, 0, 0);
  return height*getItemCount()+2;
}

void ComboBox::onPreferredSize(Size& sz)
{
//   sz = Size(4, 4);		// TODO HTHEME stuff
//   int i, n = getItemCount();
//   Rect rc;

//   for (i=0; i<n; ++i) {
//     rc = getItemBounds(i);
//     sz = Size(VACA_MAX(sz.w, rc.w), sz.h+rc.h);
//   }

  if ((getStyle().regular & 3) <= 1) { // CBS_SIMPLE = 1
    // a simple combo-box
    sz = Size(60, 23+getHeightForAllItems()); // TODO
  }
  else {
    sz = Size(60, 23);		// TODO
  }
}

/**
 * When the user press double-click in some item of a Simple combo-box
 * (with the SimpleComboBoxStyle) (Win32 CBN_DBLCLK notification).
 */
void ComboBox::onAction(Event& ev)
{
  Action(ev);
}

/**
 * When the user changes the current selected item (CBN_SELCHANGE).
 */
void ComboBox::onSelChange(Event& ev)
{
  SelChange(ev);
}

/**
 * When the user changes the text of the ComboBox. Only for
 * combo-boxes with SimpleComboBoxStyle or EditComboBoxStyle (Win32
 * CBN_EDITCHANGE notification).
 */
void ComboBox::onEditChange(Event& ev)
{
  EditChange(ev);
}

bool ComboBox::onReflectedCommand(int id, int code, LRESULT& lResult)
{
  if (Widget::onReflectedCommand(id, code, lResult))
    return true;

  switch (code) {

    case CBN_DBLCLK: {
      Event ev(this);
      onAction(ev);
      return true;
    }

    case CBN_SELCHANGE: {
      Event ev(this);
      onSelChange(ev);
      return true;
    }

    case CBN_EDITCHANGE: {
      Event ev(this);
      onEditChange(ev);
      return true;
    }

  }

  return false;
}
