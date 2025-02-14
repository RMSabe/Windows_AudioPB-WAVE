/*
	Console utilities for Windows
	Version 3.2

	These utilities are based on the win32 console API. (Compatible with NT based Windows only).
	https://learn.microsoft.com/en-us/windows/console/console-functions

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "console.h"
#include "cstrdef.h"

HANDLE _h_console_stdin = INVALID_HANDLE_VALUE;
HANDLE _h_console_stdout = INVALID_HANDLE_VALUE;
HANDLE _h_console_stderr = INVALID_HANDLE_VALUE;

VOID *_p_console_on_key_event = NULL;
VOID *_p_console_on_mouse_event = NULL;
VOID *_p_console_on_window_buffer_size_event = NULL;

BOOL WINAPI console_init(VOID)
{
	_h_console_stdin = GetStdHandle(STD_INPUT_HANDLE);
	_h_console_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
	_h_console_stderr = GetStdHandle(STD_ERROR_HANDLE);

	if((_h_console_stdin == INVALID_HANDLE_VALUE) || (_h_console_stdin == NULL))
	{
		console_deinit();
		return FALSE;
	}

	if((_h_console_stdout == INVALID_HANDLE_VALUE) || (_h_console_stdout == NULL))
	{
		console_deinit();
		return FALSE;
	}

	if((_h_console_stderr == INVALID_HANDLE_VALUE) || (_h_console_stderr == NULL))
	{
		console_deinit();
		return FALSE;
	}

	_console_set_color(__CONSOLE_COLOR_DEFAULT);
	return TRUE;
}

VOID WINAPI console_deinit(VOID)
{
	_console_set_color(__CONSOLE_COLOR_DEFAULT);

	_p_console_on_key_event = NULL;
	_p_console_on_mouse_event = NULL;
	_p_console_on_window_buffer_size_event = NULL;

	if((_h_console_stdin != INVALID_HANDLE_VALUE) && (_h_console_stdin != NULL))
	{
		FlushConsoleInputBuffer(_h_console_stdin);
		CloseHandle(_h_console_stdin);
		_h_console_stdin = INVALID_HANDLE_VALUE;
	}

	if((_h_console_stdout != INVALID_HANDLE_VALUE) && (_h_console_stdout != NULL))
	{
		CloseHandle(_h_console_stdout);
		_h_console_stdout = INVALID_HANDLE_VALUE;
	}

	if((_h_console_stderr != INVALID_HANDLE_VALUE) && (_h_console_stderr != NULL))
	{
		CloseHandle(_h_console_stderr);
		_h_console_stderr = INVALID_HANDLE_VALUE;
	}

	return;
}

/*console.c internal function*/
BOOL WINAPI _console_read(HANDLE h_input, TCHAR *p_buffer, SIZE_T length)
{
	SIZE_T size = length*sizeof(TCHAR);
	DWORD n_read = 0u;

	if((h_input == INVALID_HANDLE_VALUE) || (h_input == NULL)) return FALSE;
	if(p_buffer == NULL) return FALSE;

	if(!ReadConsole(h_input, p_buffer, (DWORD) length, &n_read, NULL)) return FALSE;

	p_buffer[length - 1u] = (TCHAR) '\0';
	return TRUE;
}

/*console.c internal function*/
BOOL WINAPI _console_write(HANDLE h_output, const TCHAR *text)
{
	SIZE_T len = 0u;

	if((h_output == INVALID_HANDLE_VALUE) || (h_output == NULL)) return FALSE;
	if(text == NULL) return FALSE;

	len = (SIZE_T) cstr_getlength(text);

	return WriteConsole(h_output, text, len, NULL, NULL);
}

BOOL WINAPI console_stdin_read(TCHAR *p_buffer, SIZE_T length)
{
	return _console_read(_h_console_stdin, p_buffer, length);
}

BOOL WINAPI console_stdout_write(const TCHAR *text)
{
	return _console_write(_h_console_stdout, text);
}

BOOL WINAPI console_stderr_write(const TCHAR *text)
{
	return _console_write(_h_console_stderr, text);
}

BOOL WINAPI console_stdin_readcmd(TCHAR *p_buffer, SIZE_T length)
{
	SIZE_T nchar = 0u;

	if(p_buffer == NULL) return FALSE;

	if(!console_stdin_read(p_buffer, length)) return FALSE;

	for(nchar = 0u; nchar < length; nchar++)
	{
		if((p_buffer[nchar] == '\r') || (p_buffer[nchar] == '\n'))
		{
			while(nchar < length)
			{
				p_buffer[nchar] = (TCHAR) '\0';
				nchar++;
			}

			break;
		}
	}

	return TRUE;
}

BOOL WINAPI _console_set_color(BYTE color)
{
	CONSOLE_SCREEN_BUFFER_INFO stdout_info;
	WORD text_attr = 0u;

	if((_h_console_stdout == INVALID_HANDLE_VALUE) || (_h_console_stdout == NULL)) return FALSE;

	if(!GetConsoleScreenBufferInfo(_h_console_stdout, &stdout_info)) return FALSE;

	text_attr = stdout_info.wAttributes;
	text_attr &= 0xff00;
	text_attr |= (WORD) color;

	return SetConsoleTextAttribute(_h_console_stdout, text_attr);
}

INT WINAPI _console_get_color(VOID)
{
	CONSOLE_SCREEN_BUFFER_INFO stdout_info;
	BYTE color = 0u;

	if((_h_console_stdout == INVALID_HANDLE_VALUE) || (_h_console_stdout == NULL)) return -1;

	if(!GetConsoleScreenBufferInfo(_h_console_stdout, &stdout_info)) return -1;

	color = (stdout_info.wAttributes & 0xff);

	return (INT) color;
}

BOOL WINAPI console_set_foreground_color(BYTE foreground_color)
{
	CONSOLE_SCREEN_BUFFER_INFO stdout_info;
	WORD text_attr = 0u;

	if((_h_console_stdout == INVALID_HANDLE_VALUE) || (_h_console_stdout == NULL)) return FALSE;

	if(!GetConsoleScreenBufferInfo(_h_console_stdout, &stdout_info)) return FALSE;

	foreground_color &= 0xf;

	text_attr = stdout_info.wAttributes;
	text_attr &= 0xfff0;
	text_attr |= (WORD) foreground_color;

	return SetConsoleTextAttribute(_h_console_stdout, text_attr);
}

INT WINAPI console_get_foreground_color(VOID)
{
	INT color = _console_get_color();
	if(color < 0) return -1;

	return (color & 0xf);
}

BOOL WINAPI console_set_background_color(BYTE background_color)
{
	CONSOLE_SCREEN_BUFFER_INFO stdout_info;
	WORD text_attr = 0u;

	if((_h_console_stdout == INVALID_HANDLE_VALUE) || (_h_console_stdout == NULL)) return FALSE;

	if(!GetConsoleScreenBufferInfo(_h_console_stdout, &stdout_info)) return FALSE;

	background_color &= 0xf;
	background_color = (background_color << 4);

	text_attr = stdout_info.wAttributes;
	text_attr &= 0xff0f;
	text_attr |= (WORD) background_color;

	return SetConsoleTextAttribute(_h_console_stdout, text_attr);
}

INT WINAPI console_get_background_color(VOID)
{
	INT color = _console_get_color();
	if(color < 0) return -1;

	return ((color >> 4) & 0xf);
}

BOOL WINAPI console_get_title(TCHAR *p_buffer, SIZE_T buffer_length, SIZE_T *p_title_length)
{
	DWORD n_ret = 0u;

	if(p_buffer == NULL) return FALSE;

	n_ret = GetConsoleTitle(p_buffer, (DWORD) buffer_length);

	if(n_ret == 0u) return FALSE;

	if(p_title_length != NULL) *p_title_length = (SIZE_T) n_ret;

	if(((SIZE_T) n_ret) < (buffer_length - 1u)) p_buffer[n_ret] = '\0';
	else p_buffer[buffer_length - 1u] = '\0';

	return TRUE;
}

BOOL WINAPI console_set_title(const TCHAR *title)
{
	if(title == NULL) return FALSE;

	return SetConsoleTitle(title);
}

BOOL WINAPI console_get_current_dimensions(SHORT *p_width, SHORT *p_height)
{
	CONSOLE_SCREEN_BUFFER_INFO stdout_info;

	if((_h_console_stdout == INVALID_HANDLE_VALUE) || (_h_console_stdout == NULL)) return FALSE;

	if(!GetConsoleScreenBufferInfo(_h_console_stdout, &stdout_info)) return FALSE;

	if(p_width != NULL) *p_width = stdout_info.dwSize.X;
	if(p_height != NULL) *p_height = stdout_info.dwSize.Y;

	return TRUE;
}

INT WINAPI console_get_current_size_chars(VOID)
{
	SHORT width = 0;
	SHORT height = 0;

	if(!console_get_current_dimensions(&width, &height)) return -1;

	return ((INT) width)*((INT) height);
}

BOOL WINAPI console_get_current_window_dimensions(SHORT *p_width, SHORT *p_height)
{
	CONSOLE_SCREEN_BUFFER_INFO stdout_info;

	if((_h_console_stdout == INVALID_HANDLE_VALUE) || (_h_console_stdout == NULL)) return FALSE;

	if(!GetConsoleScreenBufferInfo(_h_console_stdout, &stdout_info)) return FALSE;

	if(p_width != NULL) *p_width = stdout_info.dwMaximumWindowSize.X;
	if(p_height != NULL) *p_height = stdout_info.dwMaximumWindowSize.Y;

	return TRUE;
}

INT WINAPI console_get_current_window_size_chars(VOID)
{
	SHORT width = 0;
	SHORT height = 0;

	if(!console_get_current_window_dimensions(&width, &height)) return -1;

	return ((INT) width)*((INT) height);
}

BOOL WINAPI console_get_cursor_position(SHORT *p_xpos, SHORT *p_ypos)
{
	CONSOLE_SCREEN_BUFFER_INFO stdout_info;

	if((_h_console_stdout == INVALID_HANDLE_VALUE) || (_h_console_stdout == NULL)) return FALSE;

	if(!GetConsoleScreenBufferInfo(_h_console_stdout, &stdout_info)) return FALSE;

	if(p_xpos != NULL) *p_xpos = stdout_info.dwCursorPosition.X;
	if(p_ypos != NULL) *p_ypos = stdout_info.dwCursorPosition.Y;

	return TRUE;
}

BOOL WINAPI console_set_cursor_position(SHORT xpos, SHORT ypos)
{
	COORD pos;
	pos.X = xpos;
	pos.Y = ypos;

	if((_h_console_stdout == INVALID_HANDLE_VALUE) || (_h_console_stdout == NULL)) return FALSE;

	return SetConsoleCursorPosition(_h_console_stdout, pos);
}

BOOL WINAPI console_fill_screen_chars(TCHAR fill_char, SHORT xstart, SHORT ystart, DWORD length_chars)
{
	DWORD filled = 0u;
	COORD pos;

	pos.X = xstart;
	pos.Y = ystart;

	if((_h_console_stdout == INVALID_HANDLE_VALUE) || (_h_console_stdout == NULL)) return FALSE;

	return FillConsoleOutputCharacter(_h_console_stdout, fill_char, length_chars, pos, &filled);
}

BOOL WINAPI _console_fill_screen_color(BYTE color, SHORT xstart, SHORT ystart, DWORD length_chars)
{
	CONSOLE_SCREEN_BUFFER_INFO stdout_info;
	DWORD filled = 0u;
	COORD pos;
	WORD text_attr = 0u;

	pos.X = xstart;
	pos.Y = ystart;

	if((_h_console_stdout == INVALID_HANDLE_VALUE) || (_h_console_stdout == NULL)) return FALSE;

	if(!GetConsoleScreenBufferInfo(_h_console_stdout, &stdout_info)) return FALSE;

	text_attr = stdout_info.wAttributes;
	text_attr &= 0xff00;
	text_attr |= (WORD) color;

	return FillConsoleOutputAttribute(_h_console_stdout, text_attr, length_chars, pos, &filled);
}

BOOL WINAPI console_fill_screen_color(BYTE foreground, BYTE background, SHORT xstart, SHORT ystart, DWORD length_chars)
{
	foreground &= 0xf;
	background &= 0xf;

	return _console_fill_screen_color(((background << 4) | (foreground)), xstart, ystart, length_chars);
}

BOOL WINAPI console_clear_text(VOID)
{
	if(!console_fill_screen_chars('\0', 0, 0, console_get_current_size_chars())) return FALSE;
	if(!console_set_cursor_position(0, 0)) return FALSE;

	return TRUE;
}

BOOL WINAPI console_clear_screen(VOID)
{
	if(!_console_fill_screen_color(__CONSOLE_COLOR_DEFAULT, 0, 0, console_get_current_size_chars())) return FALSE;
	if(!console_fill_screen_chars('\0', 0, 0, console_get_current_size_chars())) return FALSE;
	if(!console_set_cursor_position(0, 0)) return FALSE;

	return TRUE;
}

/*console.c internal function*/
BOOL WINAPI _console_peek_stdin_record(INPUT_RECORD *p_input_record_struct_array, DWORD array_length, DWORD *p_structswritten)
{
	DWORD events_written = 0u;

	if((_h_console_stdin == INVALID_HANDLE_VALUE) || (_h_console_stdin == NULL)) return FALSE;

	if(p_input_record_struct_array == NULL) return FALSE;

	if(!PeekConsoleInput(_h_console_stdin, p_input_record_struct_array, array_length, &events_written)) return FALSE;

	if(p_structswritten != NULL) *p_structswritten = events_written;
	return TRUE;
}

/*console.c internal function*/
BOOL WINAPI _console_read_stdin_record(INPUT_RECORD *p_input_record_struct_array, DWORD array_length, DWORD *p_structswritten)
{
	DWORD events_written = 0u;

	if((_h_console_stdin == INVALID_HANDLE_VALUE) || (_h_console_stdin == NULL)) return FALSE;

	if(p_input_record_struct_array == NULL) return FALSE;

	if(!ReadConsoleInput(_h_console_stdin, p_input_record_struct_array, array_length, &events_written)) return FALSE;

	if(p_structswritten != NULL) *p_structswritten = events_written;
	return TRUE;
}

BOOL WINAPI console_wait_keypress(const TCHAR *text)
{
	KEY_EVENT_RECORD *p_event = NULL;
	INPUT_RECORD input_data;
	DWORD n_events = 0u;
	BOOL loop_stop = FALSE;

	if((_h_console_stdin == INVALID_HANDLE_VALUE) || (_h_console_stdin == NULL)) return FALSE;

	if(text != NULL)
	{
		console_stdout_write(text);
		console_stdout_write(TEXT("\r\n"));
	}

	console_stdout_write(TEXT("Press any key to continue...\r\n"));
	FlushConsoleInputBuffer(_h_console_stdin);

	do{
		GetNumberOfConsoleInputEvents(_h_console_stdin, &n_events);

		if(n_events > 0u)
		{
			_console_read_stdin_record(&input_data, 1, NULL);
			if(input_data.EventType == KEY_EVENT)
			{
				p_event = (KEY_EVENT_RECORD*) &input_data.Event;
				if(p_event->bKeyDown)
					loop_stop = TRUE;
			}
		}

		Sleep(1u);

	}while(!loop_stop);

	return TRUE;
}

/*console.c internal function*/
BOOL WINAPI _console_key_pressed(BOOL peek, BOOL specific, WORD specific_virtual_key_code)
{
	KEY_EVENT_RECORD *p_event = NULL;
	INPUT_RECORD *p_input_data = NULL;
	DWORD n_event = 0u;
	DWORD n_events = 0u;
	BOOL key_pressed = FALSE;

	if((_h_console_stdin == INVALID_HANDLE_VALUE) || (_h_console_stdin == NULL)) return FALSE;
	if(p_processheap == NULL) return FALSE;

	GetNumberOfConsoleInputEvents(_h_console_stdin, &n_events);
	if(n_events == 0u) return FALSE;

	p_input_data = (INPUT_RECORD*) HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, (n_events*sizeof(INPUT_RECORD)));

	if(peek) _console_peek_stdin_record(p_input_data, n_events, NULL);
	else _console_read_stdin_record(p_input_data, n_events, NULL);

	for(n_event = 0u; n_event < n_events; n_event++)
	{
		if(p_input_data[n_event].EventType == KEY_EVENT)
		{
			p_event = (KEY_EVENT_RECORD*) &p_input_data[n_event].Event;
			if(p_event->bKeyDown)
			{
				if(!specific || (p_event->wVirtualKeyCode == specific_virtual_key_code))
				{
					key_pressed = TRUE;
					break;
				}
			}
		}
	}

	if(p_input_data != NULL)
	{
		HeapFree(p_processheap, 0, p_input_data);
		p_input_data = NULL;
		p_event = NULL;
	}

	return key_pressed;
}

BOOL WINAPI console_peek_key_pressed(VOID)
{
	return _console_key_pressed(TRUE, FALSE, 0u);
}

BOOL WINAPI console_get_key_pressed(VOID)
{
	return _console_key_pressed(FALSE, FALSE, 0u);
}

BOOL WINAPI console_peek_specific_key_pressed(WORD virtual_key_code)
{
	return _console_key_pressed(TRUE, TRUE, virtual_key_code);
}

BOOL WINAPI console_get_specific_key_pressed(WORD virtual_key_code)
{
	return _console_key_pressed(FALSE, TRUE, virtual_key_code);
}

VOID WINAPI console_set_input_event_handlers(VOID (WINAPI *on_key_event)(KEY_EVENT_RECORD*), VOID (WINAPI *on_mouse_event)(MOUSE_EVENT_RECORD*), VOID (WINAPI *on_window_buffer_size_event)(WINDOW_BUFFER_SIZE_RECORD*))
{
	_p_console_on_key_event = (VOID*) on_key_event;
	_p_console_on_mouse_event = (VOID*) on_mouse_event;
	_p_console_on_window_buffer_size_event = (VOID*) on_window_buffer_size_event;

	return;
}

BOOL WINAPI console_manage_input_events(VOID)
{
	VOID *p_event = NULL;
	INPUT_RECORD *p_input_data = NULL;
	DWORD n_event = 0u;
	DWORD n_events = 0u;

	if((_h_console_stdin == INVALID_HANDLE_VALUE) || (_h_console_stdin == NULL)) return FALSE;
	if(p_processheap == NULL) return FALSE;

	GetNumberOfConsoleInputEvents(_h_console_stdin, &n_events);
	if(n_events == 0u) return TRUE;

	p_input_data = (INPUT_RECORD*) HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, (n_events*sizeof(INPUT_RECORD)));

	_console_read_stdin_record(p_input_data, n_events, NULL);

	for(n_event = 0u; n_event < n_events; n_event++)
	{
		p_event = &p_input_data[n_event].Event;

		switch(p_input_data[n_event].EventType)
		{
			case KEY_EVENT:
				if(_p_console_on_key_event != NULL) (*((VOID (WINAPI*)(KEY_EVENT_RECORD*)) _p_console_on_key_event))((KEY_EVENT_RECORD*) p_event);
				break;

			case MOUSE_EVENT:
				if(_p_console_on_mouse_event != NULL) (*((VOID (WINAPI*)(MOUSE_EVENT_RECORD*)) _p_console_on_mouse_event))((MOUSE_EVENT_RECORD*) p_event);
				break;

			case WINDOW_BUFFER_SIZE_EVENT:
				if(_p_console_on_window_buffer_size_event != NULL) (*((VOID (WINAPI*)(WINDOW_BUFFER_SIZE_RECORD*)) _p_console_on_window_buffer_size_event))((WINDOW_BUFFER_SIZE_RECORD*) p_event);
				break;
		}
	}

	if(p_input_data != NULL)
	{
		HeapFree(p_processheap, 0, p_input_data);
		p_input_data = NULL;
		p_event = NULL;
	}

	return TRUE;
}
