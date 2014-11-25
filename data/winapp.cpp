// Project1
//

#include <windows.h>
#include "Project1_res.h"

LRESULT WINAPI MainWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

HINSTANCE hInstance;
HWND hWindow;

// WinMain

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
   WNDCLASS wc;
   MSG msg;
   //HACCEL hAccelTable;

   if (!hPrevInst)
   {
      wc.lpszClassName="Project1Class";
      wc.lpfnWndProc=MainWndProc;
      wc.style=CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
      wc.hInstance=hInst;
      wc.hIcon=LoadIcon(hInst,MAKEINTRESOURCE(IDI_ICON1));
      wc.hCursor=LoadCursor(NULL, IDC_ARROW);
      wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
      wc.lpszMenuName=MAKEINTRESOURCE(IDR_MAINMENU);
      wc.cbClsExtra=0;
      wc.cbWndExtra=0;
      RegisterClass(&wc);
   }

   hWindow=CreateWindow("Project1Class","Application",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,500,350,NULL,NULL,hInst,NULL);
   hInstance=hInst;

   ShowWindow(hWindow,nCmdShow);
   UpdateWindow(hWindow);

   //hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_ACCELATOR1);

   while(GetMessage(&msg,NULL,0,0))
   {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
   }

   return msg.wParam;
}

// MainWndProc

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam,LPARAM lParam)
{
   static int wmId, wmEvent;

   switch(msg)
   {
      // WM_COMMAND

      case WM_COMMAND:
         wmId    = LOWORD(wParam);
         wmEvent = HIWORD(wParam);
         switch(wmId)
         {
            // About

            case ID_FILE_EXIT:
               PostQuitMessage(0);
               break;

            case ID_HELP_ABOUT:
               DialogBox(hInstance, (LPSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)AboutDlgProc);
               break;

            default:
               return DefWindowProc(hWnd, msg, wParam, lParam);
         }
         break;

      // WM_DESTROY

      case WM_DESTROY:
         PostQuitMessage(0);
         break;

		// DEFAULT

		default:
         return(DefWindowProc(hWnd, msg, wParam, lParam));
   }
   return 0;
}

// AboutDlgProc

LRESULT CALLBACK AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   switch (message)
   {
      case WM_INITDIALOG:
         break;

      case WM_COMMAND:
         if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
         {
            EndDialog(hDlg, LOWORD(wParam));
            return TRUE;
         }
         break;
   }
   return FALSE;
}

