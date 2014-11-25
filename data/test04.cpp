/**
 *  Copyright 2009-2011 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 *  WARNING: this file is here for development / testing purposes only
 *  and its contents and input / output can change without any prior
 *  notice.
 */

#define LAGOON_NO_NAMESPACE_MEMBERS 1
#define LAGOON_MT_WITH_MAKE_FACTORY 1
#define LAGOON_FACT_WITH_CREATE 1

#include <lagoon/utils/wx_gui_factory.hpp>

#include <stdexcept>

#include <mirror/mirror.hpp>
#include <lagoon/lagoon.hpp>

#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/choicebk.h>
#include <wx/notebook.h>
#include <wx/listbook.h>
#include <wx/aui/auibook.h>

#include "./test.hpp"

LAGOON_NAMESPACE_BEGIN

LAGOON_NAMESPACE_END

class TestFrame : public wxFrame
{
private:
	void Quit(wxCommandEvent& evt)
	{
		Close();
	}

	template <class BookCtrl, class Product>
	void TestBookCtrl(void)
	{
		try
		{
			using namespace lagoon;
			auto meta_product = reflected_class<Product>();
			wx_gui_factory_builder builder;
			wx_gui_factory_data data(
				(wxWindow*)this,
				(wxSizer*)nullptr
			);
			auto factory = meta_product->make_factory(
				builder,
				raw_ptr(&data)
			);
#if LAGOON_FACT_WITH_CREATE
			boost::any y = factory->create();
#else
			meta_product->delete_(factory->new_());
#endif
		}
		catch(std::exception& error)
		{
			wxMessageBox(wxString(
				error.what(),
				wxConvUTF8
			));
		}
	}

	void TestChoicebook_y(wxCommandEvent& evt)
	{
		TestBookCtrl<wxChoicebook, test::y>();
	}

	void TestNotebook_y(wxCommandEvent& evt)
	{
		TestBookCtrl<wxNotebook, test::y>();
	}

	void TestListbook_y(wxCommandEvent& evt)
	{
		TestBookCtrl<wxListbook, test::y>();
	}

	void TestAuiNotebook_y(wxCommandEvent& evt)
	{
		TestBookCtrl<wxAuiNotebook, test::y>();
	}

	void TestChoicebook_person(wxCommandEvent& evt)
	{
		TestBookCtrl<wxChoicebook, test::person>();
	}

	enum {
		IdActionTestChoicebook_y = wxID_HIGHEST + __LINE__,
		IdActionTestNotebook_y,
		IdActionTestListbook_y,
		IdActionTestAuiNotebook_y,
		IdActionTestChoicebook_person
	};

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
	TestFrame(void)
	 : wxFrame(
		(wxWindow*)nullptr,
		wxID_ANY,
		wxT("Test Frame"),
		wxDefaultPosition,
		wxSize(400, 300)
	)
	{
		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		make_button(
			sizer,
			wxID_CANCEL,
			wxT("Quit"),
			wxCommandEventHandler(TestFrame::Quit)
		);
		sizer->AddStretchSpacer(1);
		make_button(
			sizer,
			IdActionTestChoicebook_y,
			wxT("Test wxChoicebook y"),
			wxCommandEventHandler(TestFrame::TestChoicebook_y)
		);
/*
		make_button(
			sizer,
			IdActionTestNotebook_y,
			wxT("Test wxNotebook y"),
			wxCommandEventHandler(TestFrame::TestNotebook_y)
		);
		make_button(
			sizer,
			IdActionTestListbook_y,
			wxT("Test wxListbook y"),
			wxCommandEventHandler(TestFrame::TestListbook_y)
		);
		make_button(
			sizer,
			IdActionTestAuiNotebook_y,
			wxT("Test wxAuiNotebook y"),
			wxCommandEventHandler(TestFrame::TestAuiNotebook_y)
		);
*/
		make_button(
			sizer,
			IdActionTestChoicebook_person,
			wxT("Test wxChoicebook person"),
			wxCommandEventHandler(TestFrame::TestChoicebook_person)
		);
		SetSizer(sizer);
		SetMinSize(GetSize());
		Show();
	}
};

class TestApp : public wxApp
{
private:
	wxLocale appLocale;
public:
	bool OnInit(void)
	{
/*
		wxArrayString lcids;
		lcids.Add(wxT("en_US"));
		lcids.Add(wxT("sk_SK"));
		wxString lcid(
			wxGetSingleChoice(
				wxT("Pick a locale"),
				wxT("Locale"),
				lcids
			)
		);
		const wxLanguageInfo* lci = wxLocale::FindLanguageInfo(lcid);
		if((lci != nullptr) && appLocale.Init(lci->Language, 0))
		{
			wxStandardPaths stdPaths;
			wxFileName localePath(stdPaths.GetExecutablePath());
			localePath.SetName(wxEmptyString);
			localePath.AppendDir(wxT("locale"));
			appLocale.AddCatalogLookupPathPrefix(
				localePath.GetFullPath()
			);
			appLocale.AddCatalog(wxT("test2"));
		}
*/
		SetTopWindow(new TestFrame);
		return true;
	}
};

IMPLEMENT_APP(TestApp)

