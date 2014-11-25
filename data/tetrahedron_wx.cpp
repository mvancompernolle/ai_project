/**
 *  @example mirror/example/factories/tetrahedron_wx.cpp
 *
 *  This example shows how to use a factory created by the factory generator
 *  with a wxWidgets-based user input interface implementation to generically
 *  construct instances of classes with non default constructors.
 *
 *  Copyright 2008-2010 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */


#include <mirror/factory.hpp>
#include <mirror/meta_class.hpp>

#include <mirror/utils/wx_gui_factory.hpp>
#include <mirror/utils/wx_gui_factory/default_traits.hpp>
#include "./tetrahedron.hpp"

#include <wx/wx.h>
#include <wx/choicebk.h>
#include <wx/notebook.h>
#include <wx/listbook.h>
#include <wx/toolbook.h>
#include <wx/aui/auibook.h>

class TetrahedronTestFrame : public wxFrame
{
private:
	// Handler of the Quit-button press event
	void Quit(wxCommandEvent& evt)
	{
		Close();
	}

	// generic function using the factory generator to create
	// a factory using the passed wxWidgets' BookCtrl type
	// to create an instance of the Product type
	template <class BookCtrl, class Product>
	void TestBookCtrl(void)
	{
		// use the wxWidgets-based dialog and other necessary
		// components from the Mirror's utils
		using mirror::wx_gui_factory_dialog;
		using mirror::wx_input_gui_widget_traits_templ;
		using mirror::wx_input_gui_default_text_traits;
		using mirror::wx_input_gui_traits_templ;
		//
		// Now use the wxWidgets-based factory dialog to create
		// an instance of the Product type. Use the trait template
		// class with some reasonable parameters to configure
		// the dialog, use the default text traits provided by Mirror
		//
		// Delete the new instance right after it is created and
		// returned because we don't need it anymore
		delete wx_gui_factory_dialog<
			Product,
			wx_input_gui_traits_templ<
				wx_input_gui_widget_traits_templ<
					BookCtrl,
					8, 8, 8, 8, 8, 4
				>,
				wx_input_gui_default_text_traits
			>
		>(wxT("Factory Test Dialog")).new_();
	}

	void TestChoicebook(wxCommandEvent& evt)
	{
		TestBookCtrl<wxChoicebook, test::tetrahedron>();
	}

	void TestNotebook(wxCommandEvent& evt)
	{
		TestBookCtrl<wxNotebook, test::tetrahedron>();
	}

	void TestListbook(wxCommandEvent& evt)
	{
		TestBookCtrl<wxListbook, test::tetrahedron>();
	}

	void TestToolbook(wxCommandEvent& evt)
	{
		TestBookCtrl<wxToolbook, test::tetrahedron>();
	}

	void TestAuiNotebook(wxCommandEvent& evt)
	{
		TestBookCtrl<wxAuiNotebook, test::tetrahedron>();
	}

	// Event identifiers for the buttons
	enum {
		IdActionTestChoicebook = wxID_HIGHEST + __LINE__,
		IdActionTestNotebook,
		IdActionTestListbook,
		IdActionTestToolbook,
		IdActionTestAuiNotebook
	};

	// Make and add a new button with the given title, event id and
	// event handler function
	void make_button(
		wxSizer* sizer,
		int action_id,
		const wxString& title,
		void (wxObject::* handler)(wxEvent&)
	)
	{
		sizer->Add(
			new wxButton(this, action_id, title),
			0,
			wxEXPAND
		);
		Connect(
			action_id,
			wxEVT_COMMAND_BUTTON_CLICKED,
			handler
		);
	}
public:
	TetrahedronTestFrame(void)
	 : wxFrame(
		(wxWindow*)nullptr,
		wxID_ANY,
		wxT("Factory Test Frame"),
		wxDefaultPosition,
		wxSize(400, 300)
	)
	{
		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		make_button(
			sizer,
			wxID_CANCEL,
			wxT("Quit"),
			wxCommandEventHandler(TetrahedronTestFrame::Quit)
		);
		sizer->AddStretchSpacer(1);
		make_button(
			sizer,
			IdActionTestChoicebook,
			wxT("Test wxChoicebook"),
			wxCommandEventHandler(TetrahedronTestFrame::TestChoicebook)
		);
		make_button(
			sizer,
			IdActionTestNotebook,
			wxT("Test wxNotebook"),
			wxCommandEventHandler(TetrahedronTestFrame::TestNotebook)
		);
		make_button(
			sizer,
			IdActionTestListbook,
			wxT("Test wxListbook"),
			wxCommandEventHandler(TetrahedronTestFrame::TestListbook)
		);
		make_button(
			sizer,
			IdActionTestToolbook,
			wxT("Test wxToolbook"),
			wxCommandEventHandler(TetrahedronTestFrame::TestToolbook)
		);
		make_button(
			sizer,
			IdActionTestAuiNotebook,
			wxT("Test wxAuiNotebook"),
			wxCommandEventHandler(TetrahedronTestFrame::TestAuiNotebook)
		);
		SetSizer(sizer);
		SetMinSize(GetSize());
		Show();
	}
};

class TetrahedronTestApp : public wxApp
{
public:
	bool OnInit(void)
	{
		// Just create a new test frame and
		// make it a Top-Level window of this application
		SetTopWindow(new TetrahedronTestFrame);
		return true;
	}
};

IMPLEMENT_APP(TetrahedronTestApp)
