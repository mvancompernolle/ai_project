/******************************************************************
*
*	CyberLink for C++
*
*	Copyright (C) Satoshi Konno 2002-2003
*
*	File: TVMain.cpp
*
*	Revision;
*
*	09/23/03
*		- first revision
*
******************************************************************/

#include <Carbon/Carbon.h>
#include <iostream>

#include "TvDevice.h"

using namespace std;
using namespace CyberLink;

TVDevice *tvDev;

////////////////////////////////////////////////////////// 
//  InitApp
////////////////////////////////////////////////////////// 

void InitApp()
{
	tvDev = new TVDevice();
	tvDev->start();
}

void ExitApp()
{
	tvDev->stop();
	delete tvDev;
}

pascal void TVUpdate(EventLoopTimerRef timerRef, void *userData)
{
	tvDev->update();
	WindowRef window = (WindowRef)userData;
	Rect tvRect;
	SetRect(&tvRect, 0,0,360,230);
	InvalWindowRect(window, &tvRect);
}

/////////////////////////////////////////////////////////////////////////////////
// DrawTV
/////////////////////////////////////////////////////////////////////////////////

pascal OSStatus DrawTV(EventHandlerCallRef myHandler, EventRef event, void* userData)
{
	SetPort(GetWindowPort((WindowRef)userData));
	
	PicHandle tvPict = GetPicture(29876);
	//PicPtr tvPictPtr = *tvPict;
	//Rect TVRect = tvPictPtr->picFrame;
	Rect tvRect;
	SetRect(&tvRect, 0,0,360,230);
	DrawPicture(tvPict, &tvRect);
	
	short int fontNum;
	GetFNum("\pCourier", &fontNum);
	TextFont(fontNum);
	TextFace(bold);
	
	RGBColor col = {0xffff, 0xffff, 0xffff};
	RGBForeColor(&col);
	
	//// Clock String ////
	
	std::string clockStr = tvDev->getClockTime();
	TextSize(16);
	MoveTo(22, 32);
	Str255 pClockStr;
	CopyCStringToPascal(clockStr.c_str(), pClockStr);
	DrawString(pClockStr);

	return noErr;
}

/////////////////////////////////////////////////////////////////////////////////
// main
/////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
    IBNibRef 		nibRef;
    WindowRef 		window;
	
	EventTypeSpec	drawEventType = {kEventClassWindow, kEventWindowDrawContent}; 

    OSStatus		err;
	
    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference(CFSTR("tv"), &nibRef);
    require_noerr( err, CantGetNibRef );
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    err = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
    require_noerr( err, CantSetMenuBar );
    
    // Then create a window. "MainWindow" is the name of the window object. This name is set in 
    // InterfaceBuilder when the nib is created.
    err = CreateWindowFromNib(nibRef, CFSTR("MainWindow"), &window);
    require_noerr( err, CantCreateWindow );

    // We don't need the nib reference anymore.
    DisposeNibReference(nibRef);

	InstallWindowEventHandler( window, NewEventHandlerUPP(DrawTV), 1, &drawEventType, (void *)window, NULL );

	EventLoopTimerRef tref;
	InstallEventLoopTimer(GetCurrentEventLoop(), 0, 1, NewEventLoopTimerUPP(TVUpdate), (void *)window, &tref);
	
	InitApp();
	
   // The window was created hidden so show it.
    ShowWindow( window );
    
	/*
	char cdir[256];
	getcwd(cdir, 255);
	cout << cdir << endl;
	*/
	
    // Call the event loop
    RunApplicationEventLoop();

	ExitApp();
	
	CantCreateWindow:
	CantSetMenuBar:
	CantGetNibRef:
	
	return err;
}

