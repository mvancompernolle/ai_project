// Project1
//

#include <windows.h>
#include <commctrl.h>
#include "Project1_res.h"

LRESULT CALLBACK MainDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

HINSTANCE hInstance;

// WinMain

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
   hInstance=hInst;
	InitCommonControls();
   DialogBox(hInstance, (LPCTSTR)IDD_MAINWINDOW, HWND_DESKTOP, (DLGPROC)MainDlgProc);

   return 0;
}

// MainDlgProc

LRESULT CALLBACK MainDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   switch (message)
   {
      case WM_INITDIALOG:
      {
         RECT r, r2;
         GetWindowRect(hDlg,&r);
         GetWindowRect(GetDesktopWindow(),&r2);
         MoveWindow(hDlg,(r2.right/2)-((r.right-r.left)/2),(r2.bottom/2)-((r.bottom-r.top)/2),r.right-r.left,r.bottom-r.top,TRUE);
         break;
      }

      case WM_COMMAND:
      {
         if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
         {
            EndDialog(hDlg, LOWORD(wParam));
            return TRUE;
         }
         break;
      }
   }
   return FALSE;
}

