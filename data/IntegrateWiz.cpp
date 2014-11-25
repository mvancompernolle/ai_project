///
///  Copyright (C) 2004-2011 Andrej Vodopivec <andrej.vodopivec@gmail.com>
///
///  This program is free software; you can redistribute it and/or modify
///  it under the terms of the GNU General Public License as published by
///  the Free Software Foundation; either version 2 of the License, or
///  (at your option) any later version.
///
///  This program is distributed in the hope that it will be useful,
///  but WITHOUT ANY WARRANTY; without even the implied warranty of
///  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
///  GNU General Public License for more details.
///
///
///  You should have received a copy of the GNU General Public License
///  along with this program; if not, write to the Free Software
///  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
///

#include "IntegrateWiz.h"

#include <wx/config.h>

enum {
  definite_id,
  special_from,
  special_to,
  numeric_id
};

IntegrateWiz::IntegrateWiz(wxWindow* parent, int id,
                           const wxString& title, const wxPoint& pos,
                           const wxSize& size, long style):
    wxDialog(parent, id, title, pos, size, wxDEFAULT_DIALOG_STYLE)
{
  label_2 = new wxStaticText(this, -1, _("Expression:"));
  text_ctrl_1 = new BTextCtrl(this, -1, wxEmptyString, wxDefaultPosition,
                              wxSize(230, -1));
  label_3 = new wxStaticText(this, -1, _("Variable:"));
  text_ctrl_2 = new BTextCtrl(this, -1, wxT("x"), wxDefaultPosition,
                              wxSize(110, -1));
  checkbox_1 = new wxCheckBox(this, definite_id, _("&Definite integration"));
  label_4 = new wxStaticText(this, -1, _("From:"));
  text_ctrl_3 = new BTextCtrl(this, -1, wxT("0"), wxDefaultPosition,
                              wxSize(110, -1));
  button_3 = new wxButton(this, special_from, _("Special"));
  label_5 = new wxStaticText(this, -1, _("To:"));
  text_ctrl_4 = new BTextCtrl(this, -1, wxT("1"), wxDefaultPosition,
                              wxSize(110, -1));
  button_4 = new wxButton(this, special_to, _("Special"));
  checkbox_2 = new wxCheckBox(this, numeric_id, _("&Numerical integration"));
  label_6 = new wxStaticText(this, -1, _("Method:"));
  wxString numeric_methods[] = { wxT("quadpack"), wxT("romberg") };
  combobox_1 = new wxComboBox(this, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize,
                              2, numeric_methods, wxCB_DROPDOWN | wxCB_READONLY);
  static_line_1 = new wxStaticLine(this, -1);
#if defined __WXMSW__
  button_1 = new wxButton(this, wxID_OK, _("OK"));
  button_2 = new wxButton(this, wxID_CANCEL, _("Cancel"));
#else
  button_1 = new wxButton(this, wxID_CANCEL, _("Cancel"));
  button_2 = new wxButton(this, wxID_OK, _("OK"));
#endif

  set_properties();
  do_layout();
}


void IntegrateWiz::set_properties()
{
  SetTitle(_("Integrate"));
#if defined __WXMSW__
  button_1->SetDefault();
#else
  button_2->SetDefault();
#endif

  text_ctrl_3->Enable(false);
  button_3->Enable(false);
  text_ctrl_4->Enable(false);
  button_4->Enable(false);
  checkbox_2->Enable(false);
  combobox_1->Enable(false);

  int num_sel = 0;
  wxConfig::Get()->Read(wxT("Wiz/Int/numericSelection"), &num_sel);
  combobox_1->SetSelection(num_sel);

  text_ctrl_1->SetFocus();
}


void IntegrateWiz::do_layout()
{
  wxFlexGridSizer* grid_sizer_3 = new wxFlexGridSizer(3, 1, 0, 0);
  wxBoxSizer* sizer_3 = new wxBoxSizer(wxHORIZONTAL);
  wxFlexGridSizer* grid_sizer_4 = new wxFlexGridSizer(7, 2, 0, 0);
  wxFlexGridSizer* grid_sizer_6 = new wxFlexGridSizer(1, 2, 0, 0);
  wxFlexGridSizer* grid_sizer_5 = new wxFlexGridSizer(1, 2, 0, 0);
  grid_sizer_4->Add(label_2, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 5);
  grid_sizer_4->Add(text_ctrl_1, 0, wxALL, 5);
  grid_sizer_4->Add(label_3, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 5);
  grid_sizer_4->Add(text_ctrl_2, 0, wxALL, 5);
  grid_sizer_4->Add(20, 20, 0, 0);
  grid_sizer_4->Add(checkbox_1, 0, wxALL, 5);
  grid_sizer_4->Add(label_4, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 5);
  grid_sizer_5->Add(text_ctrl_3, 0, wxALL | wxEXPAND, 5);
  grid_sizer_5->Add(button_3, 0, wxALL, 5);
  grid_sizer_5->AddGrowableCol(0);
  grid_sizer_4->Add(grid_sizer_5, 1, 0, 0);
  grid_sizer_4->Add(label_5, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 5);
  grid_sizer_6->Add(text_ctrl_4, 0, wxALL | wxEXPAND, 5);
  grid_sizer_6->Add(button_4, 0, wxALL, 5);
  grid_sizer_6->AddGrowableCol(0);
  grid_sizer_4->Add(grid_sizer_6, 1, 0, 0);
  grid_sizer_4->Add(20, 20, 0, 0);
  grid_sizer_4->Add(checkbox_2, 0, wxALL, 5);
  grid_sizer_4->Add(label_6, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 5);
  grid_sizer_4->Add(combobox_1, 0, wxALL, 5);
  grid_sizer_3->Add(grid_sizer_4, 1, wxEXPAND, 0);
  grid_sizer_3->Add(static_line_1, 0, wxEXPAND | wxLEFT | wxRIGHT, 2);
  sizer_3->Add(button_1, 0, wxALL, 5);
  sizer_3->Add(button_2, 0, wxALL, 5);
  grid_sizer_3->Add(sizer_3, 1, wxALIGN_RIGHT, 0);
  SetAutoLayout(true);
  SetSizer(grid_sizer_3);
  grid_sizer_3->Fit(this);
  grid_sizer_3->SetSizeHints(this);
  Layout();
}

wxString IntegrateWiz::GetValue()
{
  wxString s;
  if (checkbox_2->GetValue())
  {
    if (combobox_1->GetValue() == wxT("romberg"))
    {
      wxConfig::Get()->Write(wxT("Wiz/Int/numericSelection"), 1);
      s = wxT("romberg(") +
          text_ctrl_1->GetValue() +
          wxT(", ") +
          text_ctrl_2->GetValue() +
          wxT(", ") +
          text_ctrl_3->GetValue() +
          wxT(", ") +
          text_ctrl_4->GetValue() +
          wxT(");");
    }
    else
    {
      wxConfig::Get()->Write(wxT("Wiz/Int/numericSelection"), 0);
      wxString from = text_ctrl_3->GetValue();
      wxString to = text_ctrl_4->GetValue();
      if (from == wxT("minf") && to == wxT("inf"))
        s = wxT("quad_qagi(") +
            text_ctrl_1->GetValue() + wxT(", ") +
            text_ctrl_2->GetValue() + wxT(", 0, 'both);");
      else if (from == wxT("minf"))
        s = wxT("quad_qagi(") +
            text_ctrl_1->GetValue() + wxT(", ") +
            text_ctrl_2->GetValue() + wxT(", ") +
            to + wxT(", minf);");
      else if (to == wxT("inf"))
        s = wxT("quad_qagi(") +
            text_ctrl_1->GetValue() + wxT(", ") +
            text_ctrl_2->GetValue() + wxT(", ") +
            from + wxT(", inf);");
      else
        s = wxT("quad_qags(") +
            text_ctrl_1->GetValue() + wxT(", ") +
            text_ctrl_2->GetValue() + wxT(", ") +
            from + wxT(", ") + to + wxT(");");
    }
  }
  else
  {
    s = wxT("integrate(") +
        text_ctrl_1->GetValue() +
        wxT(", ") +
        text_ctrl_2->GetValue();
    if (checkbox_1->GetValue())
    {
      s += wxT(", ") +
           text_ctrl_3->GetValue() +
           wxT(", ") +
           text_ctrl_4->GetValue();
    }
    s += wxT(");");
  }

  return s;
}

void IntegrateWiz::OnCheckbox(wxCommandEvent& event)
{
  bool enable = checkbox_1->GetValue();

  text_ctrl_3->Enable(enable);
  button_3->Enable(enable);
  text_ctrl_4->Enable(enable);
  button_4->Enable(enable);
  checkbox_2->Enable(enable);

  enable = enable && checkbox_2->GetValue();
  combobox_1->Enable(enable);
}

void IntegrateWiz::OnButton(wxCommandEvent& event)
{
  switch (event.GetId())
  {
  case special_from:
    {
      wxString choices[] = {wxT("Pi"), wxT("E"), _("Infinity"),
                            _("- Infinity")};
      wxString choice = wxGetSingleChoice(_("Select a constant"),
                                          _("Constant"), 4, choices, this);
      if (choice.Length())
      {
        if (choice == wxT("Pi"))
          text_ctrl_3->SetValue(wxT("%pi"));
        else if (choice == wxT("E"))
          text_ctrl_3->SetValue(wxT("%e"));
        else if (choice == _("Infinity"))
          text_ctrl_3->SetValue(wxT("inf"));
        else if (choice == _("- Infinity"))
          text_ctrl_3->SetValue(wxT("minf"));
      }
    }
    break;
  case special_to:
    {
      wxString choices[] = {wxT("Pi"), wxT("E"), _("Infinity"),
                            _("- Infinity")};
      wxString choice = wxGetSingleChoice(_("Select a constant"),
                                          _("Constant"), 4, choices, this);
      if (choice.Length())
      {
        if (choice == wxT("Pi"))
          text_ctrl_4->SetValue(wxT("%pi"));
        else if (choice == wxT("E"))
          text_ctrl_4->SetValue(wxT("%e"));
        else if (choice == _("Infinity"))
          text_ctrl_4->SetValue(wxT("inf"));
        else if (choice == _("- Infinity"))
          text_ctrl_4->SetValue(wxT("minf"));
      }
    }
    break;
  }
}

BEGIN_EVENT_TABLE(IntegrateWiz, wxDialog)
  EVT_BUTTON(special_from, IntegrateWiz::OnButton)
  EVT_BUTTON(special_to, IntegrateWiz::OnButton)
  EVT_CHECKBOX(definite_id, IntegrateWiz::OnCheckbox)
  EVT_CHECKBOX(numeric_id, IntegrateWiz::OnCheckbox)
END_EVENT_TABLE()
