// ----------------------------------------------------------------------------
//	global function to run the test
// ----------------------------------------------------------------------------

#include "TChoicesDlg.h"
#include <v/vnotice.h>
#include <stdio.h>

// ----------------------------------------------------------------------------
//	Menu item tags (not predefined in V)
//	Duplicate these in choicedlg.cpp !!!
// ----------------------------------------------------------------------------
enum {
	M_RunTest1 = 10000,		// run the test without columns
	M_RunTest2,			// run test with columns
	M_RunTest3,			// test not centered
	M_RunTest4,			// test initial selection = item 9
	M_RunTest5,			// test initial selection = "Choice 8"
	M_EndOfList			// last item
};

void RunTest (vApp * const myApp, ItemVal whichTest)
{
	// variables for choices
	char const * const prefix = "Choice";
	char str[10];
	static short const numChoices = 11;

	// build a list of choices
	TChoicesDialog dlg (myApp,
		"Make a choice\nfrom the following:",
		numChoices,
		"Choices Test",
		whichTest == M_RunTest3 ? false : true);
	for (short i = 0; i < numChoices; i++)
	{
		sprintf (str, "%s %d", prefix, i);
		dlg.AddChoice (str);
		// check for errors
		if ( dlg.LastError() != TChoicesDialog::NoError )
		{
			vNoticeDialog nd (myApp, "Error");
			nd.Notice ("Error when attempting to\n"
				   "add a selection.\n\n"
				   "Terminating this test.");
			return;
		}
		if ( i == 5 && whichTest == M_RunTest2 )
			dlg.NextColumn ();
	}
	if ( whichTest == M_RunTest4 )
		dlg.InitialSelection (9);
	if ( whichTest == M_RunTest5 )
	{
		short const i = 2;
		sprintf (str, "%s %d", prefix, i);
		dlg.InitialSelection (str);
	}


	// display list of choices to user and get selection
	const short choice = dlg.GetChoice () - 1;
	vNoticeDialog note (myApp, "Your Choice");
	char msg[81];
	if ( choice >= 0 )
		sprintf (msg, "You selected %s %d",
			 prefix, choice);
	else if ( choice == -1 )
		sprintf (msg, "You pressed Cancel.");
	else
		sprintf (msg, "Unknown choice!");
	note.Notice (msg);
	// check for errors
	if ( dlg.LastError() != TChoicesDialog::NoError )
	{
		vNoticeDialog nd (myApp, "Error");
		nd.Notice ("An error was found after you "
			   "selected your choice.\n");
	}
}
