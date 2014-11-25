// Application

#include "wx/wx.h"
#include "Project1.h"

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
	int iLeft, iTop, iWidth, iHeight;
	MyFrame *frame;
	wxScrolledWindow *myCanvas;

	iLeft=50;
	iTop=50;
	iWidth=500;
	iHeight=350;

   frame = new MyFrame("Project1",wxPoint(iLeft,iTop),wxSize(iWidth,iHeight));
	myCanvas = new wxScrolledWindow(frame);

	frame->Centre(wxBOTH);

   frame->Show(TRUE);
	return TRUE;
}

MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
       : wxFrame((wxFrame *)NULL, -1, title, pos, size)
{
   //SetIcon(wxICON(IDI_ICON1));
}

