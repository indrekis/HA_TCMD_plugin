/***********************************************************************
  This file is part of HAWCX, a archiver plugin for Windows Commander.
  Copyright (C) 1999 Sergey Zharsky  e-mail:zharik@usa.net
***********************************************************************
  config.cpp
***********************************************************************/

#include "StdAfx.h"

#include "HAEngine.h"
#include "wcxhead.h"
#include "resource.h"
#include "config.h"

#define SEC_NAME			"hadll"
#define INI_NAME			"wcplugin.ini"
#define COMBO_METHOD		"Method"

int gPackMetod=M_HSC;

HINSTANCE hInstance;

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	if(ul_reason_for_call==DLL_PROCESS_ATTACH)
	{
		hInstance=(HINSTANCE)hModule;
		gPackMetod=GetPrivateProfileInt(SEC_NAME,COMBO_METHOD,M_HSC-M_CPY,INI_NAME);
	}
    return TRUE;
}

void __stdcall SetChangeVolProc(int hArcData,tChangeVolProc pChangeVolProc)
{
    int index=hArcData;
    if (hArcData<1 || hArcData>MAX_ARCHIVES)
      index=0;
    ArchiveList[index].ChangeVolProc=pChangeVolProc;
}

void __stdcall SetProcessDataProc(int hArcData,tProcessDataProc pProcessDataProc)
{
    int index=hArcData;
    if (hArcData<1 || hArcData>MAX_ARCHIVES)
      index=0;
    ArchiveList[index].ProcessDataProc=pProcessDataProc;    
}

int __stdcall GetPackerCaps()
{
  return PK_CAPS_NEW | PK_CAPS_MODIFY | PK_CAPS_MULTIPLE | PK_CAPS_DELETE | 
              PK_CAPS_OPTIONS | PK_CAPS_SEARCHTEXT | PK_CAPS_BY_CONTENT;
}

char * GetHaDllVersion(char * lpc)
{
 char modulePath[_MAX_PATH];
 void *pData=NULL;
 DWORD  handle;
 DWORD  dwSize;
 char   szName[512];
 char *lpBuffer;
 char *iRet=NULL;

 if(GetModuleFileName(hInstance,modulePath,_MAX_PATH))
 {
  dwSize = GetFileVersionInfoSize(modulePath, &handle);
  if(dwSize)
  {
	pData=malloc(dwSize);
	GetFileVersionInfo(modulePath, handle, dwSize, pData);
	VerQueryValue(pData, "\\VarFileInfo\\Translation", (void **)&lpBuffer, (unsigned int *)&dwSize);
	if (dwSize!=0)
	{
	 wsprintf(szName, "\\StringFileInfo\\%04X%04X\\FileVersion",*((LPWORD)lpBuffer),*((LPWORD)(lpBuffer)+1));
	 VerQueryValue(pData, szName, (void **)&lpBuffer, (unsigned int*)&dwSize);
	 iRet=lpc;
	 lstrcpy(lpc,lpBuffer);
	}
  }
 }
 if(pData)
 {
  free(pData);
  pData=NULL;
 }
 return iRet;
}


HFONT hFontBold,hFontURL;
HANDLE hBrush;
HCURSOR hCursor;
HWND hwndTT;
char toolsStr[512];

BOOL CALLBACK DialogProc(HWND hwndDlg,UINT  uMsg,WPARAM wParam,LPARAM  lParam)
{
   switch (uMsg) {
     case WM_INITDIALOG:
		 {
			 LOGFONT lf;
			 HFONT hFont;
			 char verStr[40];
			 TOOLINFO ti;    // tool information 

			 SendDlgItemMessage(hwndDlg,IDC_COMBO_METHOD, CB_ADDSTRING,0,(LPARAM)"CPY");
			 SendDlgItemMessage(hwndDlg,IDC_COMBO_METHOD, CB_ADDSTRING,0,(LPARAM)"ASC");
			 SendDlgItemMessage(hwndDlg,IDC_COMBO_METHOD, CB_ADDSTRING,0,(LPARAM)"HSC");

			 SendDlgItemMessage(hwndDlg,IDC_COMBO_METHOD,CB_SETCURSEL,GetPrivateProfileInt(SEC_NAME,COMBO_METHOD,gPackMetod,INI_NAME),0);

			 hFont=(HFONT)SendDlgItemMessage(hwndDlg,IDC_STATIC_NAME,WM_GETFONT,0,0);

			 hBrush=CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
			 hCursor=LoadCursor(hInstance,MAKEINTRESOURCE(IDC_CURSOR));
			 
			 GetObject(hFont,sizeof(LOGFONT),&lf);
			 lf.lfWeight=FW_BOLD;
			 hFontBold=CreateFontIndirect(&lf);
			 lf.lfWeight=FW_NORMAL;
			 lf.lfUnderline=TRUE;
		     hFontURL=CreateFontIndirect(&lf);
             SendDlgItemMessage(hwndDlg,IDC_STATIC_NAME,WM_SETFONT,(WPARAM)hFontBold,0);
			 SendDlgItemMessage(hwndDlg,IDC_STATIC_MAIL,WM_SETFONT,(WPARAM)hFontURL,0);
			 SendDlgItemMessage(hwndDlg,IDC_STATIC_URL,WM_SETFONT,(WPARAM)hFontURL,0);
			 SendDlgItemMessage(hwndDlg,IDC_STATIC_VERSION,WM_SETFONT,(WPARAM)hFontBold,0);
			 SendDlgItemMessage(hwndDlg,IDC_STATIC_MADE_IN,WM_SETFONT,(WPARAM)hFontBold,0);
			 SetClassLong(GetDlgItem(hwndDlg,IDC_STATIC_MAIL),GCL_HCURSOR,(LPARAM)hCursor);

			 SetDlgItemText(hwndDlg,IDC_STATIC_VERSION,GetHaDllVersion(verStr));

			 InitCommonControls();

			 hwndTT=CreateWindowEx(0,TOOLTIPS_CLASS, (LPSTR) NULL,TTS_ALWAYSTIP|TTS_NOPREFIX, 
					0,0,0,0,hwndDlg,(HMENU) NULL, hInstance, NULL);
			 ZeroMemory(&ti,sizeof(TOOLINFO));
			 ti.cbSize = sizeof(TOOLINFO); 
			 ti.uFlags=TTF_IDISHWND|TTF_SUBCLASS;
            ti.hwnd = hwndDlg; 

            ti.uId = (UINT) GetDlgItem(hwndDlg,IDC_STATIC_MAIL); 
            ti.lpszText = LPSTR_TEXTCALLBACK ;
             
            SendMessage(hwndTT, TTM_ADDTOOL, 0,(LPARAM)&ti);

			ti.uId = (UINT) GetDlgItem(hwndDlg,IDC_STATIC_MAIL); 
			SendMessage(hwndTT, TTM_ADDTOOL, 0,(LPARAM)&ti);
			ti.uId = (UINT) GetDlgItem(hwndDlg,IDC_STATIC_URL); 
			SendMessage(hwndTT, TTM_ADDTOOL, 0,(LPARAM)&ti);
			ti.uId = (UINT) GetDlgItem(hwndDlg,IDC_COMBO_METHOD); 
			SendMessage(hwndTT, TTM_ADDTOOL, 0,(LPARAM)&ti);

			SendMessage(hwndTT, TTM_SETMAXTIPWIDTH,0,200);
			SendMessage(hwndTT, TTM_SETDELAYTIME,TTDT_AUTOPOP,MAKELONG(82800,0));
  
		   return TRUE;
		 }
       break;
     case WM_CTLCOLORSTATIC:
			switch(GetDlgCtrlID((HWND)lParam))
			{
			case IDC_STATIC_MAIL:
			case IDC_STATIC_URL:
				SetBkMode((HDC)wParam,TRANSPARENT);
				SetTextColor((HDC)wParam,RGB(0,0,255));
				return (BOOL)hBrush;
			case IDC_STATIC_VERSION:
			case IDC_STATIC_MADE_IN:
				SetBkMode((HDC)wParam,TRANSPARENT);
				SetTextColor((HDC)wParam,RGB(128,0,0));
				return (BOOL)hBrush;
			}
		 break;
			case WM_NOTIFY:
				{
					LPNMHDR lpnmhdr=(LPNMHDR)lParam;
					if(lpnmhdr->code == TTN_NEEDTEXT)
					{
						UINT id;
						LPNMTTDISPINFO lpnmtdi = (LPNMTTDISPINFO)lParam;
						lpnmtdi->hinst=NULL;
						if(lpnmtdi->uFlags & TTF_IDISHWND)
							id=GetWindowLong((HWND)lpnmtdi->hdr.idFrom,GWL_ID);
						else
							id=lpnmtdi->hdr.idFrom;
						lpnmtdi->lpszText=toolsStr;
						LoadString(hInstance,id,toolsStr,512);
						lpnmtdi->uFlags|=TTF_DI_SETITEM;
					}
				}
				break;
     case WM_COMMAND:
       switch (LOWORD(wParam)) {
         case IDOK:    /* OK  */
			 {
				 char num[4];
				 num[1]='\0';
		   DeleteObject(hBrush);
			num[0]=(char)('0'+(gPackMetod=SendDlgItemMessage(hwndDlg,IDC_COMBO_METHOD,CB_GETCURSEL,0,0)));
			WritePrivateProfileString(SEC_NAME,COMBO_METHOD,num,INI_NAME);

			DeleteObject(hFontBold);
			DeleteObject(hFontURL);
			DestroyCursor(hCursor);
			EndDialog(hwndDlg,1);
			break;
			 }
         case IDCANCEL:    /* Cancel */
           DeleteObject(hBrush);
			DeleteObject(hFontBold);
			DeleteObject(hFontURL);
			DestroyCursor(hCursor);
			EndDialog(hwndDlg,0);
			break;
		 case IDC_STATIC_MAIL:
		 case IDC_STATIC_URL:
			 if(HIWORD(wParam) == STN_CLICKED)
			 {
				 char str[255];
				 GetWindowText((HWND)LOWORD(lParam),str,255);
				 ShellExecute(hwndDlg,"open",str,0,0,SW_SHOW);
			 }
			 break;
       }
     break;
   }
   return FALSE;
}

void __stdcall ConfigurePacker(HANDLE ParentHandle, HANDLE hinstance)
{	
   /* You may ask for packing (NOT unpacking!) parameters here */
   DialogBox((HINSTANCE)hinstance,"CONFDLG",(HWND)ParentHandle,(DLGPROC)&DialogProc);
}

// Invoke ConfigureDialog externally
void __stdcall Settings(HWND hWnd, HINSTANCE hInst, LPTSTR lpCmdLine, int nCmdShow)
{  
  ConfigurePacker(GetFocus(), hInstance);
}

// = end of file config.cpp =