// ----------------------------------------------------------------------------
//	File:	datagrid.cpp
//
//	Main V classes for test of class TChoicesDialog.
//
//	Requires the V GUI library, version 1.22 or higher.
//
//	Author: Thomas E. Hilinski, tehilinski@sourceforge.net
//
//	Copyright (c) 1999, T.E. Hilinski. All rights reserved.
//	This software is made available under the terms of the
//	GNU LIBRARY GENERAL PUBLIC LICENSE.
//	This copyright notice must remain intact and in place.
// ----------------------------------------------------------------------------

#include <v/vapp.h>
#include <v/vcmdwin.h>
#include <v/vtextcnv.h>
#include <v/vutil.h>
#include <cstdio>
#include <sstream>
using namespace std;

// ----------------------------------------------------------------------------
//	global function to run the test (You MUST supply this.)
// ----------------------------------------------------------------------------
extern void RunTest (vApp *const myApp, ItemVal whichTest);

// ----------------------------------------------------------------------------
//	class TApp
// ----------------------------------------------------------------------------
class TApp : public vApp
{
private:
	friend int AppMain (int argc, char** argv);	// allow AppMain access
public:
	TApp (char const * appName = "") : vApp (appName) {}
	~TApp () {}

	// overridden methods
	vWindow* NewAppWin (vWindow *win, char const * name, int h, int w,
			    vAppWinInfo *winInfo);
	void Exit (void);
};

// ----------------------------------------------------------------------------
//	class TCmdWindow
// ----------------------------------------------------------------------------
class TCmdWindow : public vCmdWindow
{
public:
	//--- constructors and destructor
	TCmdWindow (char const * name);
	~TCmdWindow ();
	//--- overridden functions
	void WindowCommand (ItemVal itemId, ItemVal val, CmdType cType);
	//--- functions
	void Write (const char* text)
	  { mainTextCanvas->DrawText(text); }
	void DisplayStartMsg ( short const testNumber );
	void DisplayEndMsg ( short const testNumber );
private:
	//--- window display items
	vMenuPane *mainMenuPane;		// for the main menu
	vTextCanvasPane* mainTextCanvas;	// For the main text window
};

// ----------------------------------------------------------------------------
//	local variables
// ----------------------------------------------------------------------------

static const char *thisAppName = "TChoicesDialog Test of " __DATE__;
TApp myApp ((char *)thisAppName);		// myApp class
TCmdWindow* msgWindow;				// message window

// ----------------------------------------------------------------------------
//	Menu item tags (not predefined in V)
//	Duplicate these in runtest.cpp !!!
// ----------------------------------------------------------------------------
enum {
	M_RunTest1 = 10000,		// run the test without columns
	M_RunTest2,			// run test with columns
	M_RunTest3,			// test not centered
	M_RunTest4,			// test initial selection = item 9
	M_RunTest5,			// test initial selection = "Choice 8"
	M_EndOfList			// last item
};

// ----------------------------------------------------------------------------
//	Menu
// ----------------------------------------------------------------------------
static vMenu actionMenu[] =
{
	{"&1 - Run test without columns",
		M_RunTest1, isSens, notUsed, notUsed, noKey, noSub},
	{"&2 - Run test with columns",
		M_RunTest2, isSens, notUsed, notUsed, noKey, noSub},
	{"&3 - Run test without dialog centered",
		M_RunTest3, isSens, notUsed, notUsed, noKey, noSub},
	{"&4 - Run test with initial selection = item 9",
		M_RunTest4, isSens, notUsed, notUsed, noKey, noSub},
	{"&5 - Run test with initial selection = \"Choice 2\"",
		M_RunTest5, isSens, notUsed, notUsed, noKey, noSub},
	{"E&xit", M_Exit, isSens, notUsed, notUsed, noKey, noSub},
	{NULL}
};
static vMenu menuBar[] =
{
	{"&Tests", 1111, isSens, notUsed, notUsed, noKey, &actionMenu[0]},
	{0, 0}
};

// ----------------------------------------------------------------------------
//	class TCmdWindow methods
// ----------------------------------------------------------------------------

TCmdWindow::TCmdWindow (char const * name) : vCmdWindow (name)
{
	mainMenuPane = new vMenuPane ((vMenu*)menuBar);
	AddPane (mainMenuPane);
	mainTextCanvas = new vTextCanvasPane;
	AddPane(mainTextCanvas);
	ShowWindow ();
}

TCmdWindow::~TCmdWindow ()
{
	delete mainMenuPane;
	delete mainTextCanvas;
}

void TCmdWindow::WindowCommand (ItemVal itemId, ItemVal val, CmdType cType)
{
	// Default event processing
	vCmdWindow::WindowCommand (itemId, val, cType);

	if ( itemId >= M_RunTest1 && itemId < M_EndOfList )
	{
		DisplayStartMsg (itemId - M_RunTest1 + 1);
		::RunTest (&myApp, itemId);
		DisplayEndMsg (itemId - M_RunTest1 + 1);
	}
	else if ( itemId == M_Exit )
		myApp.Exit();
}

void TCmdWindow::DisplayStartMsg ( short const testNumber )
{
	// get info on application startup
	char timeStr[21], dateStr[21];
	vGetLocalTime (timeStr);		// start time
	vGetLocalDate (dateStr);		// start date
	std::ostringstream msg;
	msg << "Test " << testNumber << " started at "
	    << timeStr << " on " << dateStr
	    << std::endl;
	mainTextCanvas->DrawText ( msg.str().c_str() );
}

void TCmdWindow::DisplayEndMsg ( short const testNumber )
{
	// display time/data of test end
	char timeStr[21], dateStr[21];
	vGetLocalTime (timeStr);		// start time
	vGetLocalDate (dateStr);		// start date
	std::ostringstream msg;
	msg << "Test " << testNumber << " ended at "
	    << timeStr << " on " << dateStr
	    << std::endl;
	mainTextCanvas->DrawText ( msg.str().c_str() );
}

// ----------------------------------------------------------------------------
//	TApp overridden methods
// ----------------------------------------------------------------------------

vWindow* TApp::NewAppWin (vWindow* win, char const * name, int h, int w,
			  vAppWinInfo* winInfo)
{
	// Window name
	char const * myName = ( (!name || !(*name)) ? "Untitled" : name );

	// Create a new window
	TCmdWindow *thisWin = (TCmdWindow*) win;	// local copy
	if (!thisWin)
		thisWin = new TCmdWindow (myName);

	// Save info about window
	vAppWinInfo* myWinInfo = winInfo;
	if (!myWinInfo)
		myWinInfo = new vAppWinInfo (myName);

	// Register and return
	return vApp::NewAppWin (thisWin, myName, h, w, myWinInfo);
}

void TApp::Exit (void)
{
	vApp::Exit ();
}

// ----------------------------------------------------------------------------
//	instance of this app
// ----------------------------------------------------------------------------

int AppMain(int argc, char** argv)
{
	::msgWindow = (TCmdWindow *)::myApp.NewAppWin (
				NULL, "Messages", 0, 0, 0);
	if (!::msgWindow)
	{
		// cannot continue - no window can be created!
		return -1;
	}

	// display startup information

	// V has not yet forced an initial redraw or paint of the canvas!
	// So do it now with a platform-dependent message.
#if defined(V_VersionWindows) || defined(V_VersionWin95)
	// activate the first window
	// tile the windows vertically
	::SendMessage ( myApp.winClientHwnd(), WM_MDITILE,
			(WPARAM)(UINT)MDITILE_VERTICAL, 0 );
			//(WPARAM)(UINT)MDITILE_HORIZONTAL, 0 );
#else
	// X11
#endif
	::msgWindow->Write("");
	char msg[81], timeStr[21], dateStr[21];
	vGetLocalTime (timeStr);		// startup time
	vGetLocalDate (dateStr);		// startup date
	sprintf (msg, "%s\n", ::thisAppName);
	::msgWindow->Write(msg);
	sprintf (msg, "Session started at %s on %s \n\n", timeStr, dateStr);
	::msgWindow->Write(msg);
	::msgWindow->Write("Ready.\n\n");

	return 0;
}
