#include <windows.h>
#include <string.h>
#include <stdio.h>

#define YES       1
#define NO        0

#define ID_CUTSIZE    8000
#define ID_CUT        8001
#define ID_SIZE       8002
#define ID_INFO       8003
#define ID_START      8004
#define ID_STOP       8005

char szClassName[ ] = "WindowsFFmpegHelper";

#define SMS(_s_) SetWindowText(hCmd,_s_)
WNDPROC OldEditProc; WNDPROC OldCmdEditProc;

typedef struct _TIMES
{
    HWND hHours;
    HWND hMinutes;
    HWND hSeconds;
    int Hours;
    int Minutes;
    int Seconds;  
}TIMES;

HINSTANCE ins;
HWND hWnd;
HWND hCmd;
TIMES hStart;
TIMES hEnd;

UCHAR dropped[MAX_PATH];
UCHAR fname[MAX_PATH];
UCHAR *tmp;

UCHAR SelectInit = NO;
UCHAR ThreadIsRunnning = NO;

STARTUPINFO startupinfo;
PROCESS_INFORMATION process;

void SetE(BOOL b)
{
      EnableWindow(GetDlgItem(hWnd,ID_CUT),b);
      EnableWindow(GetDlgItem(hWnd,ID_SIZE),b);
      EnableWindow(GetDlgItem(hWnd,ID_INFO),b);
      EnableWindow(GetDlgItem(hWnd,ID_CUTSIZE),b);
}

     
void GoNow()
{
    int i,len;
    memset (&process, 0, sizeof (PROCESS_INFORMATION)) ;
	memset (&startupinfo, 0, sizeof (STARTUPINFO)) ; 
	
	startupinfo.cb = sizeof(STARTUPINFO);
    startupinfo.wShowWindow = SW_SHOW;
    startupinfo.dwFlags = STARTF_USESHOWWINDOW;
    len = GetWindowTextLength(hCmd);
    if(len < 1)
    {
        SMS("!This can't be empty");
        return;
     }
     memset(dropped, 0, MAX_PATH);
     if(!GetWindowText(hCmd,dropped,MAX_PATH))
     {
        SMS("!Get CMD Text");
        return;
     } 
               
     if(!CreateProcess(NULL, dropped,  NULL, NULL,FALSE,0,NULL, NULL, &startupinfo, &process))
	 {      //    if (!CreateProcess(NULL, dropped,  NULL, NULL,FALSE,NORMAL_PRIORITY_CLASS|CREATE_NEW_CONSOLE	,NULL, NULL, &startupinfo, &process))

        SMS("!CreateProcess"); 
        return;
     }
     WaitForSingleObject(process.hProcess, INFINITE); 
     TerminateProcess(process.hProcess,process.dwProcessId);
     TerminateThread(process.hThread,process.dwThreadId);
	 CloseHandle (process.hProcess);
     CloseHandle (process.hThread); 
}

void StopProc()
{   
     TerminateProcess(process.hProcess,process.dwProcessId);
     TerminateThread(process.hThread,process.dwThreadId);
	 CloseHandle (process.hProcess);
     CloseHandle (process.hThread);
}

void StartProc()
{
     unsigned char buff[1024];
     DWORD nRead = 0;

     EnableWindow(GetDlgItem(hWnd,ID_START),0);
     EnableWindow(GetDlgItem(hWnd,ID_STOP),1);
     SetE(NO);  
     ThreadIsRunnning = YES;
     GoNow();
     EnableWindow(GetDlgItem(hWnd,ID_START),1);
     EnableWindow(GetDlgItem(hWnd,ID_STOP),0); 
     SetE(YES);  
     ThreadIsRunnning = NO;   
}

void MakeCutOrSize(unsigned char *AddThis)
{
     unsigned char temp[4];
     memset(dropped,0,MAX_PATH);
     unsigned int a,b;
     memset(temp,0,4);
     if(!GetWindowText(hStart.hHours,temp,3)) return;
     hStart.Hours = atoi(temp);
     memset(temp,0,4);
     if(!GetWindowText(hStart.hMinutes,temp,3)) return;
     hStart.Minutes = atoi(temp);
     memset(temp,0,4);
     if(!GetWindowText(hStart.hSeconds,temp,3)) return;
     hStart.Seconds = atoi(temp);

     memset(temp,0,4);
     if(!GetWindowText(hEnd.hHours,temp,3)) return;
     hEnd.Hours = atoi(temp);
     memset(temp,0,4);
     if(!GetWindowText(hEnd.hMinutes,temp,3)) return;
     hEnd.Minutes = atoi(temp);
     memset(temp,0,4);
     if(!GetWindowText(hEnd.hSeconds,temp,3)) return;
     hEnd.Seconds = atoi(temp);
     
     //some math claculation
     //validity
     a = (hStart.Hours * 1000) + (hStart.Minutes * 100) + hStart.Seconds;
     b = (hEnd.Hours * 1000) + (hEnd.Minutes * 100) + hEnd.Seconds;
     if(a > b)
     {
          SMS("!Time Intervals");
          return;
     }
     //seconds
     hEnd.Seconds -= hStart.Seconds;
     if(hEnd.Seconds < 0){ hEnd.Seconds += 60; hEnd.Minutes--; }
     //minutes
     hEnd.Minutes -= hStart.Minutes;
     if(hEnd.Minutes < 0){ hEnd.Minutes += 60; hEnd.Hours--; }
     //hours
     hEnd.Hours -= hStart.Hours;
     if(hEnd.Hours < 0)
     {
          SMS("#BUG: Time Intervals");
          return;
     }
   
     
   sprintf(dropped,"ffmpeg -ss %02i:%02i:%02i.000 -i \"%s\" -t %02i:%02i:%02i.000 %s\"%d_%s\"",
                   hStart.Hours,
                   hStart.Minutes,
                   hStart.Seconds,
                   fname,
                   hEnd.Hours,
                   hEnd.Minutes,
                   hEnd.Seconds, 
                   AddThis,                  
                   GetTickCount(),fname);
     //sprintf(dropped,"ffmpeg -ss 00:00:00.000 -i \"%s\" -t 00:00:00.000 -acodec copy -vf scale=1920:960 \"%d_%s\"",fname,GetTickCount(),fname);
     SMS(dropped);
     EnableWindow(GetDlgItem(hWnd,ID_START),1); 
}

void SetCutAndSize()
{
   MakeCutOrSize("-acodec copy -vf scale=1920:960 ");
}

void SetJustCut()
{
  MakeCutOrSize("-vcodec copy -acodec copy ");
}                         

LRESULT CALLBACK CmdEditProc(HWND hnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if(message == WM_CHAR) 
    {
       if((wParam == '\r') || (wParam == '\n'))
             return 0;		
    }
	return CallWindowProc(OldCmdEditProc, hnd, message, wParam, lParam);
}

LRESULT CALLBACK EditProc(HWND hnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static unsigned char bfText[4];
    static unsigned char bText[4];
	switch (message) 
	{
		case WM_LBUTTONDOWN:
		{
			SetWindowText(hnd,"");
			memset(bfText,0,4);
		}
		break;
    	case WM_KEYUP: 
        {
			bfText[0] = 0;
			bfText[1] = 0;
			bfText[2] = 0;
			if(GetWindowText(hnd,bfText,3))
			{                          
			   if(atoi(bfText) >= 60)
			   {
			      SetWindowText(hnd,"");
			   }
			}	
		}
		break;  			
	case WM_KILLFOCUS:
	{
        memset(bText,0,4); 
	    if(GetWindowText(hnd,bText,3))
	    {
	        if(bText[1] == 0)
	        {
               bText[1] = bText[0];
               bText[0] = '0'; 
               SetWindowText(hnd,bText);        
            }
        }
        else
        {
            SetWindowText(hnd,"00");
        }
	}
		break;

	default:
		break;
	}
	return CallWindowProc(OldEditProc, hnd, message, wParam, lParam);
}
LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)                  
    {
         case WM_CREATE:
         {
              HFONT hFont;
              HWND s1;
              hWnd = hwnd;  //
            //  SetPriorityClass (GetCurrentProcess (), REALTIME_PRIORITY_CLASS);
            //  SetThreadPriority (GetCurrentThread (), THREAD_PRIORITY_TIME_CRITICAL);
              hFont = CreateFont(15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Comic Sans MS");
			
              s1 = CreateWindow("BUTTON","Cut",WS_CHILD|WS_VISIBLE,2,2,47,18,hwnd,(HMENU)ID_CUT,ins,NULL);
              SNDMSG(s1, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(1, 0));
              s1=CreateWindow("BUTTON","Size",WS_CHILD|WS_VISIBLE,2,22,47,18,hwnd,(HMENU)ID_SIZE,ins,NULL);
              SNDMSG(s1, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(1, 0));
              s1=CreateWindow("BUTTON","Cut &&\r\nSize",WS_CHILD|WS_VISIBLE|BS_MULTILINE,53,2,47,38,hwnd,(HMENU)ID_CUTSIZE,ins,NULL);
              SNDMSG(s1, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(1, 0));
              s1=CreateWindow("STATIC","Start:",WS_CHILD|WS_VISIBLE,107,2,32,15,hwnd,0,ins,NULL);
              SNDMSG(s1, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(1, 0));
              s1=CreateWindow("STATIC","End:",WS_CHILD|WS_VISIBLE,107,22,32,15,hwnd,0,ins,NULL);
              SNDMSG(s1, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(1, 0));
              
              hStart.hHours =CreateWindow("EDIT", "00",WS_CHILD|WS_VISIBLE|ES_NUMBER,142, 2, 17, 15, hwnd, NULL,ins, NULL);  	
              SNDMSG(hStart.hHours, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(1, 0));
              SNDMSG(hStart.hHours, EM_SETLIMITTEXT,(WPARAM)2,(LPARAM)0); 
              OldEditProc= (WNDPROC) SetWindowLong(hStart.hHours, GWL_WNDPROC, (LPARAM)EditProc);
	
              hStart.hMinutes =CreateWindow("EDIT", "00",WS_CHILD|WS_VISIBLE|ES_NUMBER,164, 2, 17, 15, hwnd, NULL,ins, NULL);  	
              SNDMSG(hStart.hMinutes, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(1, 0));
              SNDMSG(hStart.hMinutes, EM_SETLIMITTEXT,(WPARAM)2,(LPARAM)0); 
              OldEditProc= (WNDPROC) SetWindowLong(hStart.hMinutes, GWL_WNDPROC, (LPARAM)EditProc);

              hStart.hSeconds =CreateWindow("EDIT", "00",WS_CHILD|WS_VISIBLE|ES_NUMBER,186, 2, 17, 15, hwnd, NULL,ins, NULL);  	
              SNDMSG(hStart.hSeconds, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(1, 0));
              SNDMSG(hStart.hSeconds, EM_SETLIMITTEXT,(WPARAM)2,(LPARAM)0); 
              OldEditProc= (WNDPROC) SetWindowLong(hStart.hSeconds, GWL_WNDPROC, (LPARAM)EditProc);

              hEnd.hHours =CreateWindow("EDIT", "00",WS_CHILD|WS_VISIBLE|ES_NUMBER,142, 22, 17, 15, hwnd, NULL,ins, NULL);  	
              SNDMSG(hEnd.hHours, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(1, 0));
              SNDMSG(hEnd.hHours, EM_SETLIMITTEXT,(WPARAM)2,(LPARAM)0); 
              OldEditProc= (WNDPROC) SetWindowLong(hEnd.hHours, GWL_WNDPROC, (LPARAM)EditProc);
	
              hEnd.hMinutes =CreateWindow("EDIT", "00",WS_CHILD|WS_VISIBLE|ES_NUMBER,164, 22, 17, 15, hwnd, NULL,ins, NULL);  	
              SNDMSG(hEnd.hMinutes, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(1, 0));
              SNDMSG(hEnd.hMinutes, EM_SETLIMITTEXT,(WPARAM)2,(LPARAM)0); 
              OldEditProc= (WNDPROC) SetWindowLong(hEnd.hMinutes, GWL_WNDPROC, (LPARAM)EditProc);

              hEnd.hSeconds =CreateWindow("EDIT", "00",WS_CHILD|WS_VISIBLE|ES_NUMBER,186, 22, 17, 15, hwnd, NULL,ins, NULL);  	
              SNDMSG(hEnd.hSeconds, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(1, 0));
              SNDMSG(hEnd.hSeconds, EM_SETLIMITTEXT,(WPARAM)2,(LPARAM)0); 
              OldEditProc= (WNDPROC) SetWindowLong(hEnd.hSeconds, GWL_WNDPROC, (LPARAM)EditProc);
              
              s1=CreateWindow("BUTTON","Info",WS_CHILD|WS_VISIBLE,208,2,47,38,hwnd,(HMENU)ID_INFO,ins,NULL);
              SNDMSG(s1, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(1, 0));
              s1=CreateWindow("BUTTON","START",WS_CHILD|WS_VISIBLE,260,2,47,38,hwnd,(HMENU)ID_START,ins,NULL);
              SNDMSG(s1, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(1, 0));
              s1=CreateWindow("BUTTON","STOP",WS_CHILD|WS_VISIBLE|WS_DISABLED,312,2,47,38,hwnd,(HMENU)ID_STOP,ins,NULL);
              SNDMSG(s1, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(1, 0));
                            	
              hCmd=CreateWindow("EDIT", "",WS_CHILD|WS_VISIBLE| ES_MULTILINE ,2, 42, 357, 98, hwnd, NULL,ins, NULL);  	
              SNDMSG(hCmd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(1, 0));
			  OldCmdEditProc= (WNDPROC) SetWindowLong(hCmd, GWL_WNDPROC, (LPARAM)CmdEditProc);
              
              SetWindowPos(hwnd, HWND_TOPMOST,0,0,0,0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);						
              AllocConsole();
              DragAcceptFiles(hwnd,1);   
         }
         break;
         case WM_DROPFILES:
         {
              HDROP hDrop;
              if(ThreadIsRunnning == NO)
              {
	              memset(dropped,0,MAX_PATH);
                  memset(fname,0,MAX_PATH);
                  hDrop=(HDROP)wParam;
                  DragQueryFile(hDrop,0,dropped,MAX_PATH);
                  DragFinish(hDrop);
                  if((GetFileAttributes(dropped) & FILE_ATTRIBUTE_DIRECTORY)==FILE_ATTRIBUTE_DIRECTORY)
                    break;
                  tmp = strrchr(dropped,'\\');
                  if(tmp)
                  {
                       tmp++;    
                       sprintf(fname,"%s",tmp);
                       SMS(fname);
                       SelectInit = YES;
                       SetE(YES);
                  }
              }
              
         }
         break; 
         case WM_COMMAND:
         {
              switch(LOWORD(wParam))
              { 
 				  case ID_CUTSIZE:
                  {     
                       if(SelectInit == YES && ThreadIsRunnning == NO)
                       {
                        CreateThread(0,0,(LPTHREAD_START_ROUTINE)SetCutAndSize,0,0,0); 
                       }
                  }
                  break; 
                      
 				  case ID_CUT:
                  {     
                       if(SelectInit == YES && ThreadIsRunnning == NO)
                       {
                           CreateThread(0,0,(LPTHREAD_START_ROUTINE)SetJustCut,0,0,0); 
                        }
                  }
                  break; 
 				  case ID_SIZE:
                  {     
                       if(SelectInit == YES && ThreadIsRunnning == NO)
                       {
                           memset(dropped,0,MAX_PATH);
                           sprintf(dropped,"ffmpeg -i \"%s\" -acodec copy -vf scale=1920:960 \"%d_%s\"",fname,GetTickCount(),fname);
                           SMS(dropped);
                           EnableWindow(GetDlgItem(hWnd,ID_START),1);
                        }   
                  }
                  break;  
 				  case ID_INFO:
                  {     
                       if(SelectInit == YES && ThreadIsRunnning == NO)
                       {
                           memset(dropped,0,MAX_PATH);
                           sprintf(dropped,"ffmpeg -i \"%s\"",fname);
                           SetWindowText(hCmd,dropped);
                           EnableWindow(GetDlgItem(hWnd,ID_START),1);
                        }
                  }
                  break;  
 				  case ID_START:
                  {     
                        CreateThread(0,0,(LPTHREAD_START_ROUTINE)StartProc,0,0,0); 
                  }
                  break; 
                  case ID_STOP:
                  {
                       CreateThread(0,0,(LPTHREAD_START_ROUTINE)StopProc,0,0,0); 
                  }   
                  break;
              }//switch
         }
         break;

        case WM_DESTROY:
        {
              TerminateProcess(process.hProcess,process.dwProcessId);
              TerminateThread(process.hThread,process.dwThreadId);
              CloseHandle (process.hProcess);
              CloseHandle (process.hThread);
              FreeConsole();
              PostQuitMessage (0); 
        } 
        break;
        default:         
            return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}

int WINAPI WinMain (HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nFunsterStil)
{
            
    MSG messages;    
    WNDCLASSEX wincl; 
    HWND hwnd;    
    ins=hThisInstance;

    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;
    wincl.style = CS_DBLCLKS;  
    wincl.cbSize = sizeof (WNDCLASSEX);


    wincl.hIcon = LoadIcon (ins,MAKEINTRESOURCE(200));
    wincl.hIconSm = LoadIcon (ins,MAKEINTRESOURCE(200));
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;  
    wincl.cbClsExtra = 0;  
    wincl.cbWndExtra = 0;      

    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;


    if (!RegisterClassEx (&wincl))
        return 0;

    hwnd = CreateWindowEx(WS_EX_TOPMOST,szClassName,"ffmpeg HELPER",WS_OVERLAPPED|WS_SYSMENU,CW_USEDEFAULT,CW_USEDEFAULT,
    368,174,HWND_DESKTOP,NULL,hThisInstance,NULL );
    
    ShowWindow (hwnd, nFunsterStil);

    while (GetMessage (&messages, NULL, 0, 0))
    {
         TranslateMessage(&messages);
         DispatchMessage(&messages);
    }

     return messages.wParam;
}

