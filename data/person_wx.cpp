/**
 *  @example lagoon/example/factories/person_wx.cpp
 *  This example shows the usage of the wxWidets-based automatically
 *  generated polymorphic factory to create instances of the test person
 *  class.
 *
 *  Copyright 2008-2011 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

//
// We want to use Lagoon's polymorphic factories
#define LAGOON_MT_WITH_MAKE_FACTORY 1
//
// We don't need to traverse through namespace members.
// Using this CT switch can greatly improve compile times
// and the resulting executable size if namespace member
// traversal is not needed
#define LAGOON_NO_NAMESPACE_MEMBERS 1

#include <mirror/mirror.hpp>
#include <lagoon/lagoon.hpp>
#include <lagoon/utils/wx_gui_factory.hpp>
#include <stdexcept>

#include "./person.hpp"

class TestFrame : public wxFrame
{
private:
	void Quit(wxCommandEvent& evt)
	{
		Close();
	}

	template <class Product>
	void TestProduct(void)
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
			meta_product->delete_(factory->new_());
		}
		catch(std::exception& error)
		{
			wxMessageBox(wxString(error.what(), wxConvUTF8));
		}
	}

	void Test_person(wxCommandEvent& evt)
	{
		TestProduct<test::person>();
	}

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
		wxT("Example Frame"),
		wxDefaultPosition,
		wxSize(300, 100)
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
			wxID_HIGHEST + __LINE__,
			wxT("person"),
			wxCommandEventHandler(TestFrame::Test_person)
		);
		SetSizer(sizer);
		SetMinSize(GetSize());
		Show();
	}
};

class TestApp : public wxApp
{
public:
	bool OnInit(void)
	{
		SetTopWindow(new TestFrame);
		return true;
	}
};

IMPLEMENT_APP(TestApp)


