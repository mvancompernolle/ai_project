/**
 *  Copyright 2009-2010 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 *  WARNING: this file is here for development / testing purposes only
 *  and its contents and input / output can change without any prior
 *  notice.
 */

#include <mirror/mirror_base.hpp>
#include <mirror/pre_registered/full.hpp>

#include <mirror/utils/wx_gui_factory.hpp>
#include <mirror/utils/wx_gui_factory/datetime.hpp>
#include <mirror/utils/wx_gui_factory/default_traits.hpp>

#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/choicebk.h>
#include <wx/notebook.h>
#include <wx/listbook.h>
#include <wx/treebook.h>
#include <wx/toolbook.h>
#include <wx/aui/auibook.h>

#include "./test.hpp"

#include <mirror/utils/enum_val_by_name.hpp>

enum class test_enum
{
	a, b, c, d, e, f, g, h
};

namespace test {

struct q
{
	test_enum r;
	test_enum s;
	gender g;
};

} // namespace test

MIRROR_NAMESPACE_BEGIN

MIRROR_NAMESPACE_END

MIRROR_REG_BEGIN

MIRROR_REG_GLOBAL_SCOPE_ENUM_BEGIN(test_enum)
MIRROR_REG_ENUM_VALUE(a)
MIRROR_REG_ENUM_VALUE(b)
MIRROR_REG_ENUM_VALUE(c)
MIRROR_REG_ENUM_VALUE(d)
MIRROR_REG_ENUM_VALUE(e)
MIRROR_REG_ENUM_VALUE(f)
MIRROR_REG_ENUM_VALUE(g)
MIRROR_REG_ENUM_VALUE(h)
MIRROR_REG_ENUM_END

MIRROR_REG_CLASS_BEGIN(struct, test, q)
MIRROR_REG_CLASS_MEM_VARS_BEGIN
MIRROR_REG_CLASS_MEM_VAR(_, _, _, r)
MIRROR_REG_CLASS_MEM_VAR(_, _, _, s)
MIRROR_REG_CLASS_MEM_VAR(_, _, _, g)
MIRROR_REG_CLASS_MEM_VARS_END
MIRROR_REG_CLASS_END

MIRROR_REG_END

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
		using mirror::wx_gui_factory_dialog;
		using mirror::wx_input_gui_widget_traits_templ;
		using mirror::wx_input_gui_default_text_traits;
		using mirror::wx_input_gui_traits_templ;
		delete wx_gui_factory_dialog<
			Product,
			wx_input_gui_traits_templ<
				wx_input_gui_widget_traits_templ<
					BookCtrl,
					8, 8, 8, 8, 8, 4
				>,
				wx_input_gui_default_text_traits
			>
		>(wxT("Test Factory Dialog")).new_();
	}

	void temp_test(void)
	{
		using namespace mirror;
	}

	void TestChoicebook_y(wxCommandEvent& evt)
	{
		temp_test();
		TestBookCtrl<
			wxChoicebook,
			std::vector<std::pair<int, std::string> >
			//boost::optional<std::string>
			//std::tuple<int, bool, std::string, double>
			//test::z
		>();
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
			appLocale.AddCatalog(wxT("test"));
		}
		SetTopWindow(new TestFrame);
		return true;
	}
};

IMPLEMENT_APP(TestApp)
