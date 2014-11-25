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

#include "Vaca/CustomButton.h"
#include "Vaca/Debug.h"

using namespace Vaca;

#ifndef ODS_HOTLIGHT
#  define ODS_HOTLIGHT 64
#endif

#ifndef ODS_INACTIVE
#  define ODS_INACTIVE 128
#endif

#ifndef ODS_NOACCEL
#  define ODS_NOACCEL 256
#endif

#ifndef ODS_NOFOCUSRECT
#  define ODS_NOFOCUSRECT 512
#endif

CustomButton::CustomButton(const String& text, Widget* parent, Style style)
  : Button(text, parent, style)
  , m_itemAction(0)
  , m_itemState(0)
{
}

CustomButton::~CustomButton()
{
}

bool CustomButton::onReflectedDrawItem(Graphics& g, LPDRAWITEMSTRUCT lpDrawItem)
{
  assert(lpDrawItem->CtlType == ODT_BUTTON);

  m_itemAction = lpDrawItem->itemState;
  m_itemState = lpDrawItem->itemState;

  return doPaint(g);
}

/**
 * Returns true if you should draw the entire widget.
 */
bool CustomButton::isDrawEntire()
{
  return (m_itemAction & ODA_DRAWENTIRE) == ODA_DRAWENTIRE;
}

/**
 * Returns true if you should draw only the focus change (check
 * CustomButton::drawEntire first). Use CustomButton::isStateFocus to
 * known if the Widget has or not the focus.
 */
bool CustomButton::isFocusChanged()
{
  return (m_itemAction & ODA_FOCUS) == ODA_FOCUS;
}

/**
 * Returns true if you should draw only the selection state change
 * (check CustomButton::drawEntire first). Use
 * CustomButton::isStateSelected to known if the Widget is or not
 * selected.
 */
bool CustomButton::isSelectionChanged()
{
  return (m_itemAction & ODA_SELECT) == ODA_SELECT;
}

// /**
//  * Returns true if the widget is checked. (only for CustomMenuItem)
//  */
// bool CustomButton::isStateChecked()
// {
//   return m_itemState & ODS_CHECKED;
// }

/**
 * Returns true if this button is the default one.
 */
bool CustomButton::hasDefaultOptionVisualAspect()
{
  return (m_itemState & ODS_DEFAULT) == ODS_DEFAULT;
}

/**
 * Returns true if this widget must be drawn disabled.
 */
bool CustomButton::hasDisabledVisualAspect()
{
  return (m_itemState & ODS_DISABLED) == ODS_DISABLED;
}

/**
 * Returns true if this widget must be drawn with the focus.
 */
bool CustomButton::hasFocusVisualAspect()
{
  return (m_itemState & ODS_FOCUS) == ODS_FOCUS;
}

// /**
//  * Returns true if this menu item must be drawn grayed. (Only for CustomMenuItem)
//  */
// bool CustomButton::hasGrayedVisualAspect()
// {
//   return m_itemState & ODS_GRAYED;
// }

// bool CustomButton::hasHotLightVisualAspect()
// {
//   return m_itemState & ODS_HOTLIGHT;
// }

// bool CustomButton::hasInactiveVisualAspect()
// {
//   return m_itemState & ODS_INACTIVE;
// }

/**
 * Indicates that you shouldn't draw the accelerator character
 * underscored.
 */
bool CustomButton::hasNoAccelVisualAspect()
{
  return (m_itemState & ODS_NOACCEL) == ODS_NOACCEL;
}

/**
 * Indicates that you shouldn't draw the focus even when the button
 * has the focus (CustomButton::hasFocusVisualAspect is true).
 */
bool CustomButton::hasNoFocusRectVisualAspect()
{
  return (m_itemState & ODS_NOFOCUSRECT) == ODS_NOFOCUSRECT;
}

bool CustomButton::hasSelectedVisualAspect()
{
  return (m_itemState & ODS_SELECTED) == ODS_SELECTED;
}
