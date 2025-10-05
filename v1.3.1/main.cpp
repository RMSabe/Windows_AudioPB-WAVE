/*
	WAVE Audio Playback for Windows
	Version 1.3.1

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "globldef.h"
#include "cstrdef.h"
#include "thread.h"
#include "strdef.hpp"

#include "shared.hpp"

#include <combaseapi.h>

#include "AudioPB.hpp"
#include "AudioPB_i16.hpp"
#include "AudioPB_i24.hpp"

#define CUSTOM_GENERIC_WNDCLASS_NAME TEXT("__CUSTOMGENERICWNDCLASS__")

#define CUSTOM_WM_PLAYBACK_FINISHED (WM_USER | 1U)

#define RUNTIME_STATUS_INIT 0
#define RUNTIME_STATUS_IDLE 1
#define RUNTIME_STATUS_CHOOSEFILE 2
#define RUNTIME_STATUS_CHOOSEAUDIODEV 3
#define RUNTIME_STATUS_PLAYBACK_RUNNING 4
#define RUNTIME_STATUS_PLAYBACK_FINISHED 5

#define CUSTOMCOLOR_BLACK 0x00000000
#define CUSTOMCOLOR_WHITE 0x00ffffff
#define CUSTOMCOLOR_LTGRAY 0x00c0c0c0

#define BRUSHINDEX_TRANSPARENT 0U
#define BRUSHINDEX_CUSTOM_SOLID_BLACK 1U
#define BRUSHINDEX_CUSTOM_SOLID_WHITE 2U
#define BRUSHINDEX_CUSTOM_SOLID_LTGRAY 3U

#define PP_BRUSH_LENGTH 4U
#define PP_BRUSH_SIZE (PP_BRUSH_LENGTH*sizeof(HBRUSH))

#define CUSTOMFONT_NORMAL_CHARSET DEFAULT_CHARSET
#define CUSTOMFONT_NORMAL_WIDTH 8
#define CUSTOMFONT_NORMAL_HEIGHT 16
#define CUSTOMFONT_NORMAL_WEIGHT FW_NORMAL

#define CUSTOMFONT_LARGE_CHARSET DEFAULT_CHARSET
#define CUSTOMFONT_LARGE_WIDTH 20
#define CUSTOMFONT_LARGE_HEIGHT 35
#define CUSTOMFONT_LARGE_WEIGHT FW_NORMAL

#define FONTINDEX_CUSTOM_NORMAL 0U
#define FONTINDEX_CUSTOM_LARGE 1U

#define PP_FONT_LENGTH 2U
#define PP_FONT_SIZE (PP_FONT_LENGTH*sizeof(HFONT))

#define CHILDWNDINDEX_TEXT1 0U
#define CHILDWNDINDEX_BUTTON1 1U
#define CHILDWNDINDEX_BUTTON2 2U
#define CHILDWNDINDEX_BUTTON3 3U
#define CHILDWNDINDEX_LISTBOX1 4U

#define PP_CHILDWND_LENGTH 5U
#define PP_CHILDWND_SIZE (PP_CHILDWND_LENGTH*sizeof(HWND))

#define MAINWND_CAPTION TEXT("WAVE Audio Player")

#define MAINWND_BKCOLOR CUSTOMCOLOR_LTGRAY
#define MAINWND_BRUSHINDEX BRUSHINDEX_CUSTOM_SOLID_LTGRAY

#define TEXTWND_TEXTCOLOR CUSTOMCOLOR_BLACK
#define TEXTWND_BKCOLOR MAINWND_BKCOLOR
#define TEXTWND_BRUSHINDEX BRUSHINDEX_TRANSPARENT

#define PB_I16 1
#define PB_I24 2

HANDLE p_audiothread = NULL;
HANDLE h_filein = INVALID_HANDLE_VALUE;

HBRUSH pp_brush[PP_BRUSH_LENGTH] = {NULL};
HFONT pp_font[PP_FONT_LENGTH] = {NULL};
HWND pp_childwnd[PP_CHILDWND_LENGTH] = {NULL};

HWND p_mainwnd = NULL;

AudioPB *p_audio = NULL;

audiopb_params_t pb_params;

__string tstr = TEXT("");

INT runtime_status = -1;
INT prev_status = -1;

WORD custom_generic_wndclass_id = 0u;

extern BOOL WINAPI app_init(VOID);
extern VOID WINAPI app_deinit(VOID);

extern BOOL WINAPI gdiobj_init(VOID);
extern VOID WINAPI gdiobj_deinit(VOID);

extern BOOL WINAPI register_wndclass(VOID);
extern BOOL WINAPI create_mainwnd(VOID);
extern BOOL WINAPI create_childwnd(VOID);

extern INT WINAPI app_get_ref_status(VOID);

extern VOID WINAPI runtime_loop(VOID);

extern VOID WINAPI paintscreen_choosefile(VOID);
extern VOID WINAPI paintscreen_chooseaudiodev(VOID);
extern VOID WINAPI paintscreen_playback_running(VOID);
extern VOID WINAPI paintscreen_playback_finished(VOID);

extern VOID WINAPI text_choose_font(VOID);
extern VOID WINAPI text_align(VOID);
extern VOID WINAPI button_align(VOID);
extern VOID WINAPI listbox_align(VOID);
extern VOID WINAPI ctrls_setup(BOOL redraw_mainwnd);

extern BOOL WINAPI window_get_dimensions(HWND p_wnd, INT *p_xpos, INT *p_ypos, INT *p_width, INT *p_height, INT *p_centerx, INT *p_centery);

extern BOOL WINAPI mainwnd_redraw(VOID);
extern VOID WINAPI childwnd_hide_all(VOID);

extern BOOL WINAPI catch_messages(VOID);

extern LRESULT CALLBACK mainwnd_wndproc(HWND p_wnd, UINT msg, WPARAM wparam, LPARAM lparam);

extern LRESULT CALLBACK mainwnd_event_wmdestroy(HWND p_wnd, WPARAM wparam, LPARAM lparam);
extern LRESULT CALLBACK mainwnd_event_wmcommand(HWND p_wnd, WPARAM wparam, LPARAM lparam);
extern LRESULT CALLBACK mainwnd_event_wmsize(HWND p_wnd, WPARAM wparam, LPARAM lparam);
extern LRESULT CALLBACK mainwnd_event_wmpaint(HWND p_wnd, WPARAM wparam, LPARAM lparam);

extern LRESULT CALLBACK window_event_wmctlcolorstatic(HWND p_wnd, WPARAM wparam, LPARAM lparam);

extern BOOL WINAPI choosefile_proc(VOID);
extern BOOL WINAPI chooseaudiodev_proc(SIZE_T index_sel, BOOL dev_default);
extern BOOL WINAPI initaudioobj_proc(VOID);

extern BOOL WINAPI filein_open(const TCHAR *filein_dir);
extern VOID WINAPI filein_close(VOID);

extern INT WINAPI filein_get_params(VOID);
extern BOOL WINAPI compare_signature(const CHAR *auth, const CHAR *bytebuf);

extern DWORD WINAPI audiothread_proc(VOID *p_args);

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow)
{
	p_instance = hInstance;

	if(!app_init()) return 1;

	runtime_loop();

	app_deinit();
	return 0;
}

BOOL WINAPI app_init(VOID)
{
	HRESULT n_ret = 0;

	if(p_instance == NULL)
	{
		__SPRINTF(textbuf, TEXTBUF_SIZE_CHARS, TEXT("Error: Invalid Instance."));
		goto _l_app_init_error;
	}

	p_processheap = GetProcessHeap();
	if(p_processheap == NULL)
	{
		__SPRINTF(textbuf, TEXTBUF_SIZE_CHARS, TEXT("Error: Invalid Process Heap."));
		goto _l_app_init_error;
	}

	if(!gdiobj_init())
	{
		__SPRINTF(textbuf, TEXTBUF_SIZE_CHARS, TEXT("Error: GDIOBJ Init Failed."));
		goto _l_app_init_error;
	}

	if(!register_wndclass())
	{
		__SPRINTF(textbuf, TEXTBUF_SIZE_CHARS, TEXT("Error: Register WNDCLASS Failed."));
		goto _l_app_init_error;
	}

	if(!create_mainwnd())
	{
		__SPRINTF(textbuf, TEXTBUF_SIZE_CHARS, TEXT("Error: Create MAINWND Failed."));
		goto _l_app_init_error;
	}

	if(!create_childwnd())
	{
		__SPRINTF(textbuf, TEXTBUF_SIZE_CHARS, TEXT("Error: Create CHILDWND Failed."));
		goto _l_app_init_error;
	}

	n_ret = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if(n_ret != S_OK)
	{
		__SPRINTF(textbuf, TEXTBUF_SIZE_CHARS, TEXT("Error: COMBASEAPI Init Failed."));
		goto _l_app_init_error;
	}

	runtime_status = RUNTIME_STATUS_INIT;
	return TRUE;

_l_app_init_error:
	MessageBox(NULL, textbuf, TEXT("INIT ERROR"), (MB_ICONSTOP | MB_OK));
	app_deinit();
	return FALSE;
}

VOID WINAPI app_deinit(VOID)
{
	if(p_audiothread != NULL) thread_stop(&p_audiothread, 0u);

	if(p_audio != NULL)
	{
		delete p_audio;
		p_audio = NULL;
	}

	filein_close();

	if(p_mainwnd != NULL) DestroyWindow(p_mainwnd);

	if(custom_generic_wndclass_id)
	{
		UnregisterClass(CUSTOM_GENERIC_WNDCLASS_NAME, p_instance);
		custom_generic_wndclass_id = 0u;
	}

	gdiobj_deinit();
	CoUninitialize();

	return;
}

__declspec(noreturn) VOID WINAPI app_exit(UINT exit_code, const TCHAR *exit_msg)
{
	app_deinit();

	if(exit_msg != NULL) MessageBox(NULL, exit_msg, TEXT("PROCESS EXIT CALLED"), (MB_ICONSTOP | MB_OK));

	ExitProcess(exit_code);

	while(TRUE) Sleep(1u); /*Not really necessary, but just to be safe.*/
}

BOOL WINAPI gdiobj_init(VOID)
{
	SIZE_T n_obj = 0u;
	LOGFONT logfont;

	pp_brush[BRUSHINDEX_TRANSPARENT] = (HBRUSH) GetStockObject(HOLLOW_BRUSH);
	pp_brush[BRUSHINDEX_CUSTOM_SOLID_BLACK] = CreateSolidBrush(CUSTOMCOLOR_BLACK);
	pp_brush[BRUSHINDEX_CUSTOM_SOLID_WHITE] = CreateSolidBrush(CUSTOMCOLOR_WHITE);
	pp_brush[BRUSHINDEX_CUSTOM_SOLID_LTGRAY] = CreateSolidBrush(CUSTOMCOLOR_LTGRAY);

	for(n_obj = 0u; n_obj < PP_BRUSH_LENGTH; n_obj++) if(pp_brush[n_obj] == NULL) return FALSE;

	ZeroMemory(&logfont, sizeof(LOGFONT));

	logfont.lfCharSet = CUSTOMFONT_NORMAL_CHARSET;
	logfont.lfWidth = CUSTOMFONT_NORMAL_WIDTH;
	logfont.lfHeight = CUSTOMFONT_NORMAL_HEIGHT;
	logfont.lfWeight = CUSTOMFONT_NORMAL_WEIGHT;

	pp_font[FONTINDEX_CUSTOM_NORMAL] = CreateFontIndirect(&logfont);

	logfont.lfCharSet = CUSTOMFONT_LARGE_CHARSET;
	logfont.lfWidth = CUSTOMFONT_LARGE_WIDTH;
	logfont.lfHeight = CUSTOMFONT_LARGE_HEIGHT;
	logfont.lfWeight = CUSTOMFONT_LARGE_WEIGHT;

	pp_font[FONTINDEX_CUSTOM_LARGE] = CreateFontIndirect(&logfont);

	for(n_obj = 0u; n_obj < PP_FONT_LENGTH; n_obj++) if(pp_font[n_obj] == NULL) return FALSE;

	return TRUE;
}

VOID WINAPI gdiobj_deinit(VOID)
{
	SIZE_T n_obj = 0u;

	for(n_obj = 0u; n_obj < PP_BRUSH_LENGTH; n_obj++)
	{
		if(pp_brush[n_obj] != NULL)
		{
			DeleteObject(pp_brush[n_obj]);
			pp_brush[n_obj] = NULL;
		}
	}

	for(n_obj = 0u; n_obj < PP_FONT_LENGTH; n_obj++)
	{
		if(pp_font[n_obj] != NULL)
		{
			DeleteObject(pp_font[n_obj]);
			pp_font[n_obj] = NULL;
		}
	}

	return;
}

BOOL WINAPI register_wndclass(VOID)
{
	WNDCLASS wndclass;

	ZeroMemory(&wndclass, sizeof(WNDCLASS));

	wndclass.style = CS_OWNDC;
	wndclass.lpfnWndProc = &DefWindowProc;
	wndclass.hInstance = p_instance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH) pp_brush[BRUSHINDEX_TRANSPARENT];
	wndclass.lpszClassName = CUSTOM_GENERIC_WNDCLASS_NAME;

	custom_generic_wndclass_id = RegisterClass(&wndclass);

	return (BOOL) custom_generic_wndclass_id;
}

BOOL WINAPI create_mainwnd(VOID)
{
	DWORD style = (WS_CAPTION | WS_VISIBLE | WS_SYSMENU | WS_OVERLAPPED | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX);

	p_mainwnd = CreateWindow(CUSTOM_GENERIC_WNDCLASS_NAME, MAINWND_CAPTION, style, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, p_instance, NULL);
	if(p_mainwnd == NULL) return FALSE;

	if(SetWindowLongPtr(p_mainwnd, GWLP_WNDPROC, (LONG_PTR) &mainwnd_wndproc)) return TRUE;

	return FALSE;

	/*
		Avoid using "return (BOOL) SetWindowLongPtr(...);".

		SetWindowLongPtr() return type is LONG_PTR, casting it to BOOL could cause runtime errors on 64bit Windows.
	*/
}

BOOL WINAPI create_childwnd(VOID)
{
	SIZE_T n_wnd = 0u;
	DWORD style = 0u;

	style = (WS_CHILD | SS_CENTER);
	pp_childwnd[CHILDWNDINDEX_TEXT1] = CreateWindow(TEXT("STATIC"), NULL, style, 0, 0, 0, 0, p_mainwnd, NULL, p_instance, NULL);

	style = (WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON | BS_TEXT | BS_CENTER | BS_VCENTER);

	pp_childwnd[CHILDWNDINDEX_BUTTON1] = CreateWindow(TEXT("BUTTON"), NULL, style, 0, 0, 0, 0, p_mainwnd, NULL, p_instance, NULL);
	pp_childwnd[CHILDWNDINDEX_BUTTON2] = CreateWindow(TEXT("BUTTON"), NULL, style, 0, 0, 0, 0, p_mainwnd, NULL, p_instance, NULL);
	pp_childwnd[CHILDWNDINDEX_BUTTON3] = CreateWindow(TEXT("BUTTON"), TEXT("Choose Default Device"), style, 0, 0, 0, 0, p_mainwnd, NULL, p_instance, NULL);

	style = (WS_CHILD | WS_TABSTOP | WS_VSCROLL | LBS_HASSTRINGS);
	pp_childwnd[CHILDWNDINDEX_LISTBOX1] = CreateWindow(TEXT("LISTBOX"), NULL, style, 0, 0, 0, 0, p_mainwnd, NULL, p_instance, NULL);

	for(n_wnd = 0u; n_wnd < PP_CHILDWND_LENGTH; n_wnd++) if(pp_childwnd[n_wnd] == NULL) return FALSE;

	return TRUE;
}

INT WINAPI app_get_ref_status(VOID)
{
	if(runtime_status == RUNTIME_STATUS_IDLE) return prev_status;

	return runtime_status;
}

VOID WINAPI runtime_loop(VOID)
{
	while(catch_messages())
	{
		switch(runtime_status)
		{
			case RUNTIME_STATUS_IDLE:
				Sleep(10u);
				break;

			case RUNTIME_STATUS_INIT:
				ctrls_setup(TRUE);
				runtime_status = RUNTIME_STATUS_CHOOSEFILE;

			case RUNTIME_STATUS_CHOOSEFILE:
				paintscreen_choosefile();
				goto _l_runtime_loop_runtimestatus_notidle;
				break;

			case RUNTIME_STATUS_CHOOSEAUDIODEV:
				paintscreen_chooseaudiodev();
				goto _l_runtime_loop_runtimestatus_notidle;
				break;

			case RUNTIME_STATUS_PLAYBACK_RUNNING:
				paintscreen_playback_running();
				goto _l_runtime_loop_runtimestatus_notidle;
				break;

			case RUNTIME_STATUS_PLAYBACK_FINISHED:
				paintscreen_playback_finished();
				goto _l_runtime_loop_runtimestatus_notidle;
				break;
		}

_l_runtime_loop_runtimestatus_idle:
		continue;

_l_runtime_loop_runtimestatus_notidle:
		prev_status = runtime_status;
		runtime_status = RUNTIME_STATUS_IDLE;
	}

	return;
}

VOID WINAPI paintscreen_choosefile(VOID)
{
	childwnd_hide_all();

	button_align();

	SendMessage(pp_childwnd[CHILDWNDINDEX_TEXT1], WM_SETTEXT, 0, (LPARAM) TEXT("Choose Audio File"));
	SendMessage(pp_childwnd[CHILDWNDINDEX_BUTTON1], WM_SETTEXT, 0, (LPARAM) TEXT("Browse"));

	ShowWindow(pp_childwnd[CHILDWNDINDEX_TEXT1], SW_SHOW);
	ShowWindow(pp_childwnd[CHILDWNDINDEX_BUTTON1], SW_SHOW);
	return;
}

VOID WINAPI paintscreen_chooseaudiodev(VOID)
{
	childwnd_hide_all();

	button_align();

	SendMessage(pp_childwnd[CHILDWNDINDEX_TEXT1], WM_SETTEXT, 0, (LPARAM) TEXT("Choose Playback Device"));
	SendMessage(pp_childwnd[CHILDWNDINDEX_BUTTON1], WM_SETTEXT, 0, (LPARAM) TEXT("Return"));
	SendMessage(pp_childwnd[CHILDWNDINDEX_BUTTON2], WM_SETTEXT, 0, (LPARAM) TEXT("Choose Selected Device"));

	if(p_audio->loadAudioDeviceList(pp_childwnd[CHILDWNDINDEX_LISTBOX1]))
	{
		ShowWindow(pp_childwnd[CHILDWNDINDEX_BUTTON2], SW_SHOW);
		ShowWindow(pp_childwnd[CHILDWNDINDEX_LISTBOX1], SW_SHOW);
	}
	else
	{
		tstr = TEXT("Error: could not load device list:\r\nExtended Error Info: ");
		tstr += p_audio->getLastErrorMessage();
		MessageBox(NULL, tstr.c_str(), TEXT("ERROR"), (MB_ICONEXCLAMATION | MB_OK));
	}

	ShowWindow(pp_childwnd[CHILDWNDINDEX_TEXT1], SW_SHOW);
	ShowWindow(pp_childwnd[CHILDWNDINDEX_BUTTON1], SW_SHOW);
	ShowWindow(pp_childwnd[CHILDWNDINDEX_BUTTON3], SW_SHOW);

	return;
}

VOID WINAPI paintscreen_playback_running(VOID)
{
	childwnd_hide_all();

	button_align();

	SendMessage(pp_childwnd[CHILDWNDINDEX_TEXT1], WM_SETTEXT, 0, (LPARAM) TEXT("Playback Running"));
	SendMessage(pp_childwnd[CHILDWNDINDEX_BUTTON1], WM_SETTEXT, 0, (LPARAM) TEXT("Stop Playback"));

	ShowWindow(pp_childwnd[CHILDWNDINDEX_TEXT1], SW_SHOW);
	ShowWindow(pp_childwnd[CHILDWNDINDEX_BUTTON1], SW_SHOW);
	return;
}

VOID WINAPI paintscreen_playback_finished(VOID)
{
	childwnd_hide_all();

	button_align();

	SendMessage(pp_childwnd[CHILDWNDINDEX_TEXT1], WM_SETTEXT, 0, (LPARAM) TEXT("Playback Finished"));
	SendMessage(pp_childwnd[CHILDWNDINDEX_BUTTON1], WM_SETTEXT, 0, (LPARAM) TEXT("Start Over"));
	SendMessage(pp_childwnd[CHILDWNDINDEX_BUTTON2], WM_SETTEXT, 0, (LPARAM) TEXT("Quit Application"));

	ShowWindow(pp_childwnd[CHILDWNDINDEX_TEXT1], SW_SHOW);
	ShowWindow(pp_childwnd[CHILDWNDINDEX_BUTTON1], SW_SHOW);
	ShowWindow(pp_childwnd[CHILDWNDINDEX_BUTTON2], SW_SHOW);
	return;
}

VOID WINAPI text_choose_font(VOID)
{
	INT mainwnd_width = 0;
	INT mainwnd_height = 0;

	BOOL small_wnd = FALSE;

	window_get_dimensions(p_mainwnd, NULL, NULL, &mainwnd_width, &mainwnd_height, NULL, NULL);
	small_wnd = ((mainwnd_width <= 640) || (mainwnd_height <= 480));

	if(small_wnd) SendMessage(pp_childwnd[CHILDWNDINDEX_TEXT1], WM_SETFONT, (WPARAM) pp_font[FONTINDEX_CUSTOM_NORMAL], (LPARAM) TRUE);
	else SendMessage(pp_childwnd[CHILDWNDINDEX_TEXT1], WM_SETFONT, (WPARAM) pp_font[FONTINDEX_CUSTOM_LARGE], (LPARAM) TRUE);

	return;
}

VOID WINAPI text_align(VOID)
{
	INT mainwnd_width = 0;
	INT mainwnd_height = 0;

	INT text1_xpos = 0;
	INT text1_ypos = 0;
	INT text1_width = 0;
	INT text1_height = 0;

	BOOL small_wnd = FALSE;

	window_get_dimensions(p_mainwnd, NULL, NULL, &mainwnd_width, &mainwnd_height, NULL, NULL);
	small_wnd = ((mainwnd_width <= 640) || (mainwnd_height <= 480));

	if(small_wnd)
	{
		text1_xpos = 20;
		text1_ypos = 20;
		text1_height = CUSTOMFONT_NORMAL_HEIGHT;
	}
	else
	{
		text1_xpos = 40;
		text1_ypos = 40;
		text1_height = CUSTOMFONT_LARGE_HEIGHT;
	}

	text1_width = mainwnd_width - 2*text1_xpos;

	SetWindowPos(pp_childwnd[CHILDWNDINDEX_TEXT1], NULL, text1_xpos, text1_ypos, text1_width, text1_height, 0u);
	return;
}

VOID WINAPI button_align(VOID)
{
	const INT BUTTON_WIDTH = 200;
	const INT BUTTON_HEIGHT = 20;

	INT mainwnd_width = 0;
	INT mainwnd_height = 0;
	INT mainwnd_centerx = 0;

	INT button1_xpos = 0;
	INT button1_ypos = 0;
	INT button2_xpos = 0;
	INT button2_ypos = 0;
	INT button3_xpos = 0;
	INT button3_ypos = 0;

	INT ref_status = -1;

	BOOL small_wnd = FALSE;

	ref_status = app_get_ref_status();

	window_get_dimensions(p_mainwnd, NULL, NULL, &mainwnd_width, &mainwnd_height, &mainwnd_centerx, NULL);
	small_wnd = ((mainwnd_width <= 640) || (mainwnd_height <= 480));

	switch(ref_status)
	{
		case RUNTIME_STATUS_CHOOSEFILE:
		case RUNTIME_STATUS_PLAYBACK_RUNNING:

			button1_xpos = mainwnd_centerx - BUTTON_WIDTH/2;

			if(small_wnd) button1_ypos = mainwnd_height - BUTTON_HEIGHT - 60;
			else button1_ypos = mainwnd_height - BUTTON_HEIGHT - 100;
			break;

		case RUNTIME_STATUS_CHOOSEAUDIODEV:
			button2_xpos = mainwnd_centerx - BUTTON_WIDTH/2;

			if(small_wnd)
			{
				button1_xpos = button2_xpos;
				button3_xpos = button2_xpos;

				button3_ypos = mainwnd_height - BUTTON_HEIGHT - 60;
				button2_ypos = button3_ypos - BUTTON_HEIGHT - 10;
				button1_ypos = button2_ypos - BUTTON_HEIGHT - 10;
			}
			else
			{
				button1_xpos = button2_xpos - BUTTON_WIDTH - 10;
				button3_xpos = button2_xpos + BUTTON_WIDTH + 10;

				button1_ypos = mainwnd_height - BUTTON_HEIGHT - 100;
				button2_ypos = button1_ypos;
				button3_ypos = button1_ypos;
			}

			break;

		case RUNTIME_STATUS_PLAYBACK_FINISHED:

			if(small_wnd)
			{
				button1_xpos = mainwnd_centerx - BUTTON_WIDTH/2;
				button2_xpos = button1_xpos;

				button2_ypos = mainwnd_height - BUTTON_HEIGHT - 60;
				button1_ypos = button2_ypos - BUTTON_HEIGHT - 10;
			}
			else
			{
				button1_xpos = mainwnd_centerx - BUTTON_WIDTH - 5;
				button2_xpos = mainwnd_centerx + 5;

				button1_ypos = mainwnd_height - BUTTON_HEIGHT - 100;
				button2_ypos = button1_ypos;
			}

			break;
	}

	SetWindowPos(pp_childwnd[CHILDWNDINDEX_BUTTON1], NULL, button1_xpos, button1_ypos, BUTTON_WIDTH, BUTTON_HEIGHT, 0u);
	SetWindowPos(pp_childwnd[CHILDWNDINDEX_BUTTON2], NULL, button2_xpos, button2_ypos, BUTTON_WIDTH, BUTTON_HEIGHT, 0u);
	SetWindowPos(pp_childwnd[CHILDWNDINDEX_BUTTON3], NULL, button3_xpos, button3_ypos, BUTTON_WIDTH, BUTTON_HEIGHT, 0u);

	return;
}

VOID WINAPI listbox_align(VOID)
{
	INT mainwnd_width = 0;
	INT mainwnd_height = 0;

	INT listbox1_width = 0;
	INT listbox1_height = 0;
	INT listbox1_xpos = 0;
	INT listbox1_ypos = 0;

	BOOL small_wnd = FALSE;

	window_get_dimensions(p_mainwnd, NULL, NULL, &mainwnd_width, &mainwnd_height, NULL, NULL);
	small_wnd = ((mainwnd_width <= 640) || (mainwnd_height <= 480));

	if(small_wnd)
	{
		listbox1_xpos = 20;
		listbox1_ypos = 60;
		listbox1_height = 60;
	}
	else
	{
		listbox1_xpos = 40;
		listbox1_ypos = 100;
		listbox1_height = 100;
	}

	listbox1_width = mainwnd_width - 2*listbox1_xpos;

	SetWindowPos(pp_childwnd[CHILDWNDINDEX_LISTBOX1], NULL, listbox1_xpos, listbox1_ypos, listbox1_width, listbox1_height, 0u);
	return;
}

VOID WINAPI ctrls_setup(BOOL redraw_mainwnd)
{
	text_choose_font();
	text_align();
	button_align();
	listbox_align();

	if(redraw_mainwnd) mainwnd_redraw();
	return;
}

BOOL WINAPI window_get_dimensions(HWND p_wnd, INT *p_xpos, INT *p_ypos, INT *p_width, INT *p_height, INT *p_centerx, INT *p_centery)
{
	RECT rect;

	if(p_wnd == NULL) return FALSE;
	if(!GetWindowRect(p_wnd, &rect)) return FALSE;

	if(p_xpos != NULL) *p_xpos = rect.left;
	if(p_ypos != NULL) *p_ypos = rect.top;
	if(p_width != NULL) *p_width = rect.right - rect.left;
	if(p_height != NULL) *p_height = rect.bottom - rect.top;
	if(p_centerx != NULL) *p_centerx = (rect.right - rect.left)/2;
	if(p_centery != NULL) *p_centery = (rect.bottom - rect.top)/2;

	return TRUE;
}

BOOL WINAPI mainwnd_redraw(VOID)
{
	if(p_mainwnd == NULL) return FALSE;
	
	return RedrawWindow(p_mainwnd, NULL, NULL, (RDW_ERASE | RDW_FRAME | RDW_INTERNALPAINT | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN));
}

VOID WINAPI childwnd_hide_all(VOID)
{
	SIZE_T n_wnd = 0u;
	for(n_wnd = 0u; n_wnd < PP_CHILDWND_LENGTH; n_wnd++) ShowWindow(pp_childwnd[n_wnd], SW_HIDE);

	return;
}

BOOL WINAPI catch_messages(VOID)
{
	MSG msg;

	while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if(msg.message == WM_QUIT) return FALSE;

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return TRUE;
}

LRESULT CALLBACK mainwnd_wndproc(HWND p_wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch(msg)
	{
		case WM_DESTROY:
			return mainwnd_event_wmdestroy(p_wnd, wparam, lparam);

		case WM_COMMAND:
			return mainwnd_event_wmcommand(p_wnd, wparam, lparam);

		case WM_SIZE:
			return mainwnd_event_wmsize(p_wnd, wparam, lparam);

		case WM_PAINT:
			return mainwnd_event_wmpaint(p_wnd, wparam, lparam);

		case WM_CTLCOLORSTATIC:
			return window_event_wmctlcolorstatic(p_wnd, wparam, lparam);

		case CUSTOM_WM_PLAYBACK_FINISHED:
			thread_stop(&p_audiothread, 0u);
			if(p_audio != NULL)
			{
				delete p_audio;
				p_audio = NULL;
			}

			runtime_status = RUNTIME_STATUS_PLAYBACK_FINISHED;
			break;
	}

	return DefWindowProc(p_wnd, msg, wparam, lparam);
}

LRESULT CALLBACK mainwnd_event_wmdestroy(HWND p_wnd, WPARAM wparam, LPARAM lparam)
{
	SIZE_T n_wnd = 0u;

	listbox_clear(pp_childwnd[CHILDWNDINDEX_LISTBOX1]);

	p_mainwnd = NULL;

	for(n_wnd = 0u; n_wnd < PP_CHILDWND_LENGTH; n_wnd++) pp_childwnd[n_wnd] = NULL;

	PostQuitMessage(0);
	return 0;
}

LRESULT CALLBACK mainwnd_event_wmcommand(HWND p_wnd, WPARAM wparam, LPARAM lparam)
{
	SSIZE_T _ssize;

	if(p_wnd == NULL) return 0;
	if(!lparam) return 0;

	if(((ULONG_PTR) lparam) == ((ULONG_PTR) pp_childwnd[CHILDWNDINDEX_BUTTON1]))
	{
		switch(prev_status)
		{
			case RUNTIME_STATUS_CHOOSEFILE:
				if(choosefile_proc()) runtime_status = RUNTIME_STATUS_CHOOSEAUDIODEV;
				break;

			case RUNTIME_STATUS_PLAYBACK_RUNNING:
				p_audio->stopPlayback();
				break;

			case RUNTIME_STATUS_CHOOSEAUDIODEV:
				if(p_audio != NULL)
				{
					delete p_audio;
					p_audio = NULL;
				}

			case RUNTIME_STATUS_PLAYBACK_FINISHED:
				runtime_status = RUNTIME_STATUS_CHOOSEFILE;
				break;
		}
	}
	else if(((ULONG_PTR) lparam) == ((ULONG_PTR) pp_childwnd[CHILDWNDINDEX_BUTTON2]))
	{
		switch(prev_status)
		{
			case RUNTIME_STATUS_CHOOSEAUDIODEV:
				_ssize = listbox_get_sel_index(pp_childwnd[CHILDWNDINDEX_LISTBOX1]);
				if(_ssize < 0)
				{
					MessageBox(NULL, TEXT("Error: no item selected"), TEXT("ERROR"), (MB_ICONEXCLAMATION | MB_OK));
					break;
				}
				if(!chooseaudiodev_proc((SIZE_T) _ssize, FALSE)) break;
				if(!initaudioobj_proc()) break;

				p_audiothread = thread_create_default(&audiothread_proc, NULL, NULL);
				runtime_status = RUNTIME_STATUS_PLAYBACK_RUNNING;
				break;

			case RUNTIME_STATUS_PLAYBACK_FINISHED:
				PostQuitMessage(0);
				break;
		}
	}
	else if(((ULONG_PTR) lparam) == ((ULONG_PTR) pp_childwnd[CHILDWNDINDEX_BUTTON3]))
	{
		if(prev_status == RUNTIME_STATUS_CHOOSEAUDIODEV)
		{
			if(chooseaudiodev_proc(0u, TRUE))
			{
				if(initaudioobj_proc())
				{
					p_audiothread = thread_create_default(&audiothread_proc, NULL, NULL);
					runtime_status = RUNTIME_STATUS_PLAYBACK_RUNNING;
				}
			}
		}
	}

	return 0;
}

LRESULT CALLBACK mainwnd_event_wmsize(HWND p_wnd, WPARAM wparam, LPARAM lparam)
{
	if(p_wnd == NULL) return 0;

	ctrls_setup(TRUE);
	return 0;
}

LRESULT CALLBACK mainwnd_event_wmpaint(HWND p_wnd, WPARAM wparam, LPARAM lparam)
{
	HDC p_wnddc = NULL;
	PAINTSTRUCT paint;

	if(p_wnd == NULL) return 0;

	p_wnddc = BeginPaint(p_wnd, &paint);
	if(p_wnddc == NULL) return 0;

	FillRect(paint.hdc, &paint.rcPaint, pp_brush[MAINWND_BRUSHINDEX]);
	EndPaint(p_wnd, &paint);

	return 0;
}

LRESULT CALLBACK window_event_wmctlcolorstatic(HWND p_wnd, WPARAM wparam, LPARAM lparam)
{
	if(p_wnd == NULL) return 0;
	if(!wparam) return 0;
	if(!lparam) return 0;

	SetTextColor((HDC) wparam, TEXTWND_TEXTCOLOR);
	SetBkColor((HDC) wparam, TEXTWND_BKCOLOR);

	return (LRESULT) pp_brush[TEXTWND_BRUSHINDEX];
}

BOOL WINAPI listbox_clear(HWND p_listbox)
{
	SIZE_T n_items = 0u;
	SSIZE_T _ssize = 0;

	if(p_listbox == NULL) return FALSE;

	_ssize = listbox_get_item_count(p_listbox);
	if(_ssize < 0) return FALSE;

	n_items = (SIZE_T) _ssize;

	while(n_items > 0u)
	{
		_ssize = listbox_remove_item(p_listbox, (n_items - 1u));
		if(_ssize < 0) return FALSE;

		n_items--;
	}

	return TRUE;
}

SSIZE_T WINAPI listbox_add_item(HWND p_listbox, const TCHAR *text)
{
	if(p_listbox == NULL) return -1;
	if(text == NULL) return -1;

	return (SSIZE_T) SendMessage(p_listbox, LB_ADDSTRING, 0, (LPARAM) text);
}

SSIZE_T WINAPI listbox_remove_item(HWND p_listbox, SIZE_T index)
{
	if(p_listbox == NULL) return -1;

	return (SSIZE_T) SendMessage(p_listbox, LB_DELETESTRING, (WPARAM) index, 0);
}

SSIZE_T WINAPI listbox_get_item_count(HWND p_listbox)
{
	if(p_listbox == NULL) return -1;

	return (SSIZE_T) SendMessage(p_listbox, LB_GETCOUNT, 0, 0);
}

SSIZE_T WINAPI listbox_get_sel_index(HWND p_listbox)
{
	if(p_listbox == NULL) return -1;

	return (SSIZE_T) SendMessage(p_listbox, LB_GETCURSEL, 0, 0);
}

BOOL WINAPI choosefile_proc(VOID)
{
	SIZE_T textlen = 0u;
	INT n32 = 0;
	OPENFILENAME ofdlg;
	const TCHAR *filters = TEXT("Wave Files\0*.wav;*.WAV\0All Files\0*.*\0\0");

	ZeroMemory(&ofdlg, sizeof(OPENFILENAME));
	ZeroMemory(textbuf, TEXTBUF_SIZE_BYTES);

	ofdlg.lStructSize = sizeof(OPENFILENAME);
	ofdlg.hwndOwner = p_mainwnd;
	ofdlg.lpstrFilter = filters;
	ofdlg.nFilterIndex = 1;
	ofdlg.lpstrFile = textbuf;
	ofdlg.nMaxFile = TEXTBUF_SIZE_CHARS;
	ofdlg.Flags = (OFN_EXPLORER | OFN_ENABLESIZING);
	ofdlg.lpstrDefExt = TEXT(".wav");

	if(!GetOpenFileName(&ofdlg))
	{
		tstr = TEXT("Error: Open File Dialog Failed.");
		goto _l_choosefile_proc_error;
	}

	tstr = textbuf;
	cstr_tolower(textbuf, TEXTBUF_SIZE_CHARS);
	textlen = (SIZE_T) cstr_getlength(textbuf);

	if(textlen >= 5u)
		if(cstr_compare(TEXT(".wav"), &textbuf[textlen - 4u]))
			goto _l_choosefile_proc_fileextcheck_complete;

	n32 = MessageBox(NULL, TEXT("WARNING: Selected file does not have a \".wav\" file extension.\r\nMight be incompatible with this application.\r\nDo you wish to continue?"), TEXT("WARNING: FILE EXTENSION"), (MB_ICONEXCLAMATION | MB_YESNO));

	if(n32 != IDYES)
	{
		tstr = TEXT("Error: Bad File Extension.");
		goto _l_choosefile_proc_error;
	}

_l_choosefile_proc_fileextcheck_complete:

	if(!filein_open(tstr.c_str()))
	{
		tstr = TEXT("Error: Could Not Open File.");
		goto _l_choosefile_proc_error;
	}

	n32 = filein_get_params();
	if(n32 < 0) goto _l_choosefile_proc_error;

	pb_params.file_dir = tstr.c_str();

	if(p_audio != NULL)
	{
		delete p_audio;
		p_audio = NULL;
	}

	switch(n32)
	{
		case PB_I16:
			p_audio = new AudioPB_i16(&pb_params);
			break;

		case PB_I24:
			p_audio = new AudioPB_i24(&pb_params);
			break;
	}

	if(p_audio != NULL) return TRUE;

	tstr = TEXT("Error: Failed To Create Audio Object Instance.");

_l_choosefile_proc_error:
	MessageBox(NULL, tstr.c_str(), TEXT("ERROR"), (MB_ICONEXCLAMATION | MB_OK));
	return FALSE;
}

BOOL WINAPI chooseaudiodev_proc(SIZE_T sel_index, BOOL dev_default)
{
	if(p_audio == NULL) return FALSE;

	if(dev_default) goto _l_chooseaudiodev_proc_default;

_l_chooseaudiodev_proc_index:
	if(p_audio->chooseDevice(sel_index)) return TRUE;

	goto _l_chooseaudiodev_proc_error;

_l_chooseaudiodev_proc_default:
	if(p_audio->chooseDefaultDevice()) return TRUE;

_l_chooseaudiodev_proc_error:
	tstr = TEXT("Error: failed to access audio device\r\nExtended Error Message: ");
	tstr += p_audio->getLastErrorMessage();

	MessageBox(NULL, tstr.c_str(), TEXT("ERROR"), (MB_ICONEXCLAMATION | MB_OK));
	return FALSE;
}

BOOL WINAPI initaudioobj_proc(VOID)
{
	if(p_audio == NULL) return FALSE;

	if(p_audio->initialize()) return TRUE;

	tstr = TEXT("Error: failed to initialize audio object\r\nExtended Error Message: ");
	tstr += p_audio->getLastErrorMessage();

	MessageBox(NULL, tstr.c_str(), TEXT("ERROR"), (MB_ICONEXCLAMATION | MB_OK));
	return FALSE;
}

BOOL WINAPI filein_open(const TCHAR *filein_dir)
{
	filein_close();

	if(filein_dir == NULL) return FALSE;

	h_filein = CreateFile(filein_dir, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	return (h_filein != INVALID_HANDLE_VALUE);
}

VOID WINAPI filein_close(VOID)
{
	if(h_filein == INVALID_HANDLE_VALUE) return;

	CloseHandle(h_filein);
	h_filein = INVALID_HANDLE_VALUE;
	return;
}

INT WINAPI filein_get_params(VOID)
{
	const SIZE_T BUFFER_SIZE = 4096u;
	SIZE_T buffer_index = 0u;

	UINT8 *p_headerinfo = NULL;

	DWORD dummy_32;

	UINT32 u32 = 0u;
	UINT16 u16 = 0u;

	UINT16 bit_depth = 0u;

	p_headerinfo = (UINT8*) HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, BUFFER_SIZE);
	if(p_headerinfo == NULL)
	{
		tstr = TEXT("Error: Memory Allocate Failed.");
		goto _l_filein_get_params_error;
	}

	SetFilePointer(h_filein, 0, NULL, FILE_BEGIN);
	ReadFile(h_filein, p_headerinfo, (DWORD) BUFFER_SIZE, &dummy_32, NULL);
	filein_close();

	if(!compare_signature("RIFF", (const CHAR*) p_headerinfo))
	{
		tstr = TEXT("Error: File Format Not Supported.");
		goto _l_filein_get_params_error;
	}

	if(!compare_signature("WAVE", (const CHAR*) (((SIZE_T) p_headerinfo) + 8u)))
	{
		tstr = TEXT("Error: File Format Not Supported.");
		goto _l_filein_get_params_error;
	}

	buffer_index = 12u;

	while(TRUE)
	{
		if(buffer_index > (BUFFER_SIZE - 8u))
		{
			tstr = TEXT("Error: Broken Header (missing subchunk \"fmt \").\r\nFile is probably corrupted.");
			goto _l_filein_get_params_error;
		}

		if(compare_signature("fmt ", (const CHAR*) (((SIZE_T) p_headerinfo) + buffer_index))) break;

		u32 = *((UINT32*) (((SIZE_T) p_headerinfo) + buffer_index + 4u));

		buffer_index += (SIZE_T) (u32 + 8u);
	}

	if(buffer_index > (BUFFER_SIZE - 24u))
	{
		tstr = TEXT("Error: Broken Header (subchunk \"fmt \" might be corrupted).");
		goto _l_filein_get_params_error;
	}

	u16 = *((UINT16*) (((SIZE_T) p_headerinfo) + buffer_index + 8u));

	if(u16 != 1u)
	{
		tstr = TEXT("Error: Audio Format Encoding Not Supported");
		goto _l_filein_get_params_error;
	}

	pb_params.n_channels = *((UINT16*) (((SIZE_T) p_headerinfo) + buffer_index + 10u));
	pb_params.sample_rate = *((UINT32*) (((SIZE_T) p_headerinfo) + buffer_index + 12u));

	bit_depth = *((UINT16*) (((SIZE_T) p_headerinfo) + buffer_index + 22u));

	u32 = *((UINT32*) (((SIZE_T) p_headerinfo) + buffer_index + 4u));
	buffer_index += (SIZE_T) (u32 + 8u);

	while(TRUE)
	{
		if(buffer_index > (BUFFER_SIZE - 8u))
		{
			tstr = TEXT("Error: Broken Header (missing subchunk \"data\").\r\nFile is probably corrupted.");
			goto _l_filein_get_params_error;
		}

		if(compare_signature("data", (const CHAR*) (((SIZE_T) p_headerinfo) + buffer_index))) break;

		u32 = *((UINT32*) (((SIZE_T) p_headerinfo) + buffer_index + 4u));

		buffer_index += (SIZE_T) (u32 + 8u);
	}

	u32 = *((UINT32*) (((SIZE_T) p_headerinfo) + buffer_index + 4u));

	pb_params.audio_data_begin = (ULONG64) (buffer_index + 8u);
	pb_params.audio_data_end = pb_params.audio_data_begin + ((ULONG64) u32);

	HeapFree(p_processheap, 0u, p_headerinfo);
	p_headerinfo = NULL;

	switch(bit_depth)
	{
		case 16u:
			return PB_I16;

		case 24u:
			return PB_I24;
	}

	tstr = TEXT("Error: Format Not Supported.");

_l_filein_get_params_error:
	filein_close();
	if(p_headerinfo != NULL) HeapFree(p_processheap, 0u, p_headerinfo);
	return -1;
}

BOOL WINAPI compare_signature(const CHAR *auth, const CHAR *bytebuf)
{
	if(auth == NULL) return FALSE;
	if(bytebuf == NULL) return FALSE;

	if(auth[0] != bytebuf[0]) return FALSE;
	if(auth[1] != bytebuf[1]) return FALSE;
	if(auth[2] != bytebuf[2]) return FALSE;
	if(auth[3] != bytebuf[3]) return FALSE;

	return TRUE;
}

DWORD WINAPI audiothread_proc(VOID *p_args)
{
	p_audio->runPlayback();

	PostMessage(p_mainwnd, CUSTOM_WM_PLAYBACK_FINISHED, 0, 0);
	return 0u;
}
