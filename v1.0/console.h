/*
	Console utilities for Windows
	Version 3.2

	These utilities are based on the win32 console API. (Compatible with NT based Windows only).
	https://learn.microsoft.com/en-us/windows/console/console-functions

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#ifndef CONSOLE_H
#define CONSOLE_H

#include "globldef.h"

#ifdef __DOSWIN
#error "console.h is not supported on DOS based Windows."
#endif

/*Internal Windows Console API definitions*/

#define __CONSOLE_TEXTCOLOR_BLACK (0U)
#define __CONSOLE_TEXTCOLOR_BLUE (FOREGROUND_BLUE)
#define __CONSOLE_TEXTCOLOR_GREEN (FOREGROUND_GREEN)
#define __CONSOLE_TEXTCOLOR_CYAN (FOREGROUND_GREEN | FOREGROUND_BLUE)
#define __CONSOLE_TEXTCOLOR_RED (FOREGROUND_RED)
#define __CONSOLE_TEXTCOLOR_MAGENTA (FOREGROUND_RED | FOREGROUND_BLUE)
#define __CONSOLE_TEXTCOLOR_BROWN (FOREGROUND_RED | FOREGROUND_GREEN)
#define __CONSOLE_TEXTCOLOR_LTGRAY (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define __CONSOLE_TEXTCOLOR_DKGRAY (FOREGROUND_INTENSITY)
#define __CONSOLE_TEXTCOLOR_LTBLUE (FOREGROUND_INTENSITY | FOREGROUND_BLUE)
#define __CONSOLE_TEXTCOLOR_LTGREEN (FOREGROUND_INTENSITY | FOREGROUND_GREEN)
#define __CONSOLE_TEXTCOLOR_LTCYAN (FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define __CONSOLE_TEXTCOLOR_LTRED (FOREGROUND_INTENSITY | FOREGROUND_RED)
#define __CONSOLE_TEXTCOLOR_LTMAGENTA (FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE)
#define __CONSOLE_TEXTCOLOR_YELLOW (FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN)
#define __CONSOLE_TEXTCOLOR_WHITE (FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)

#define __CONSOLE_BKCOLOR_BLACK (0U)
#define __CONSOLE_BKCOLOR_BLUE (BACKGROUND_BLUE)
#define __CONSOLE_BKCOLOR_GREEN (BACKGROUND_GREEN)
#define __CONSOLE_BKCOLOR_CYAN (BACKGROUND_GREEN | BACKGROUND_BLUE)
#define __CONSOLE_BKCOLOR_RED (BACKGROUND_RED)
#define __CONSOLE_BKCOLOR_MAGENTA (BACKGROUND_RED | BACKGROUND_BLUE)
#define __CONSOLE_BKCOLOR_BROWN (BACKGROUND_RED | BACKGROUND_GREEN)
#define __CONSOLE_BKCOLOR_LTGRAY (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE)
#define __CONSOLE_BKCOLOR_DKGRAY (BACKGROUND_INTENSITY)
#define __CONSOLE_BKCOLOR_LTBLUE (BACKGROUND_INTENSITY | BACKGROUND_BLUE)
#define __CONSOLE_BKCOLOR_LTGREEN (BACKGROUND_INTENSITY | BACKGROUND_GREEN)
#define __CONSOLE_BKCOLOR_LTCYAN (BACKGROUND_INTENSITY | BACKGROUND_GREEN | BACKGROUND_BLUE)
#define __CONSOLE_BKCOLOR_LTRED (BACKGROUND_INTENSITY | BACKGROUND_RED)
#define __CONSOLE_BKCOLOR_LTMAGENTA (BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_BLUE)
#define __CONSOLE_BKCOLOR_YELLOW (BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN)
#define __CONSOLE_BKCOLOR_WHITE (BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE)

#define __CONSOLE_TEXTCOLOR_DEFAULT __CONSOLE_TEXTCOLOR_LTGRAY
#define __CONSOLE_BKCOLOR_DEFAULT __CONSOLE_BKCOLOR_BLACK
#define __CONSOLE_COLOR_DEFAULT (__CONSOLE_BKCOLOR_DEFAULT | __CONSOLE_TEXTCOLOR_DEFAULT)

/*console.h definitions*/

#define CONSOLE_COLOR_BLACK 0U
#define CONSOLE_COLOR_BLUE 1U
#define CONSOLE_COLOR_GREEN 2U
#define CONSOLE_COLOR_CYAN 3U
#define CONSOLE_COLOR_RED 4U
#define CONSOLE_COLOR_MAGENTA 5U
#define CONSOLE_COLOR_BROWN 6U
#define CONSOLE_COLOR_LTGRAY 7U
#define CONSOLE_COLOR_DKGRAY 8U
#define CONSOLE_COLOR_LTBLUE 9U
#define CONSOLE_COLOR_LTGREEN 10U
#define CONSOLE_COLOR_LTCYAN 11U
#define CONSOLE_COLOR_LTRED 12U
#define CONSOLE_COLOR_LTMAGENTA 13U
#define CONSOLE_COLOR_YELLOW 14U
#define CONSOLE_COLOR_WHITE 15U

#define CONSOLE_FOREGROUND_COLOR_DEFAULT CONSOLE_COLOR_LTGRAY
#define CONSOLE_BACKGROUND_COLOR_DEFAULT CONSOLE_COLOR_BLACK

/*Initialize console stdin, stdout and strerr handlers. Must be called before calling any other function in this header.
Returns TRUE is successful, FALSE otherwise.*/

extern BOOL WINAPI console_init(VOID);

/*Closes console stdin, stdout and stderr handlers.*/

extern VOID WINAPI console_deinit(VOID);

/*Basic stdio read/write functions.
Returns TRUE if successful, FALSE otherwise.*/

extern BOOL WINAPI console_stdin_read(TCHAR *p_buffer, SIZE_T length);
extern BOOL WINAPI console_stdout_write(const TCHAR *text);
extern BOOL WINAPI console_stderr_write(const TCHAR *text);

/*Variation of stdin_read()
Does the same as stdin_read, but truncates the text at the first line break.
stdin_read captures the line break characters from command line. stdin_readcmd captures only the text line without line break characters.
Returns TRUE if successful, FALSE otherwise.*/

extern BOOL WINAPI console_stdin_readcmd(TCHAR *p_buffer, SIZE_T length);

/*Color functions using Windows internal __COLOR definitions.
This function DOES NOT affect previously written texts. Use console_fill_screen_color/_console_fill_screen_color for that.
Set Color returns TRUE if successful, FALSE otherwise.
Get Color returns color value if successful, -1 otherwise.*/

extern BOOL WINAPI _console_set_color(BYTE color);
extern INT WINAPI _console_get_color(VOID);

/*Color functions using console.h color definitions.
This function DOES NOT affect previously written texts. Use console_fill_screen_color/_console_fill_screen_color for that.
Set Color returns TRUE if successful, FALSE otherwise.
Get Color returns color value if successful, -1 otherwise.*/

extern BOOL WINAPI console_set_foreground_color(BYTE foreground_color);
extern INT WINAPI console_get_foreground_color(VOID);

extern BOOL WINAPI console_set_background_color(BYTE background_color);
extern INT WINAPI console_get_background_color(VOID);

/*Set/Get Console window title
Returns TRUE if successful, FALSE otherwise.*/

extern BOOL WINAPI console_get_title(TCHAR *p_buffer, SIZE_T buffer_length, SIZE_T *p_title_length);
extern BOOL WINAPI console_set_title(const TCHAR *title);

/*Get Full Console stdout buffer dimensions (chars per line, number of lines).
Returns TRUE if successful, FALSE otherwise.*/

extern BOOL WINAPI console_get_current_dimensions(SHORT *p_width, SHORT *p_height);

/*Get Full Console buffer size in number of characters.
Returns buffer size if successful, -1 otherwise.*/

extern INT WINAPI console_get_current_size_chars(VOID);

/*Get Console window stdout dimensions (chars per line, number of lines).
Returns TRUE if successful, FALSE otherwise.*/

extern BOOL WINAPI console_get_current_window_dimensions(SHORT *p_width, SHORT *p_height);

/*Get Console window size in number of characters.
Returns size if successful, -1 otherwise.*/

extern INT WINAPI console_get_current_window_size_chars(VOID);

/*Get/Set text cursor position.
Returns TRUE if successful, FALSE otherwise.*/

extern BOOL WINAPI console_get_cursor_position(SHORT *p_xpos, SHORT *p_ypos);
extern BOOL WINAPI console_set_cursor_position(SHORT xpos, SHORT ypos);

/*Writes character "fill_char", "length_chars" times, starting from coordinates "xstart" and "ystart"
Returns TRUE if successful, FALSE otherwise.*/

extern BOOL WINAPI console_fill_screen_chars(TCHAR fill_char, SHORT xstart, SHORT ystart, DWORD length_chars);

/*Sets the color for each character, starting at coordinates "xstart" and "ystart" and going as far as "length_chars"
These functions ONLY affects previously written texts. To set color for upcoming texts, use set_color functions.
Returns TRUE if successful, FALSE otherwise.*/

extern BOOL WINAPI _console_fill_screen_color(BYTE color, SHORT xstart, SHORT ystart, DWORD length_chars);
extern BOOL WINAPI console_fill_screen_color(BYTE foreground, BYTE background, SHORT xstart, SHORT ystart, DWORD length_chars);

/*Clear all text in screen, preserving foreground and background colors. Returns cursor position to 0, 0.
Returns TRUE if successful, FALSE otherwise.*/

extern BOOL WINAPI console_clear_text(VOID);

/*Same thing as "console_clear_text()", except this will reset the foreground and background colors to the default console colors.*/

extern BOOL WINAPI console_clear_screen(VOID);

/*Pause runtime, wait user to press any key to continue, display optional message "text".
Returns TRUE if successful, FALSE otherwise.
This function will clear any events in the input buffer
For more accurate event handling, use "console_set_input_event_handlers" and "console_manage_input_events".*/

extern BOOL WINAPI console_wait_keypress(const TCHAR *text);

/*Check if user has pressed any key, without pausing runtime.
Returns TRUE if yes, FALSE if no or error.
peek_key_pressed will not clear the events in the input buffer.
get_key_pressed will clear the events in the input buffer.
For more accurate event handling, use "console_set_input_event_handlers" and "console_manage_input_events".*/

extern BOOL WINAPI console_peek_key_pressed(VOID);
extern BOOL WINAPI console_get_key_pressed(VOID);

/*Does pretty much the same as console_peek_key_pressed() and console_get_key_pressed() respectivelly,
except these will only return TRUE if the pressed key matches the one specified in the arguments.*/

extern BOOL WINAPI console_peek_specific_key_pressed(WORD virtual_key_code);
extern BOOL WINAPI console_get_specific_key_pressed(WORD virtual_key_code);

/*Set the event handler functions.
If one or more event handler functions doesn't exist, set it to NULL.*/

extern VOID WINAPI console_set_input_event_handlers(VOID (WINAPI *on_key_event)(KEY_EVENT_RECORD*), VOID (WINAPI *on_mouse_event)(MOUSE_EVENT_RECORD*), VOID (WINAPI *on_window_buffer_size_event)(WINDOW_BUFFER_SIZE_RECORD*));

/*Check input events, call the respective event handler function set by "console_set_input_event_handlers", and passes the respective struct *EVENT_RECORD as argument.
This function will clear any events in the input buffer.
Returns TRUE if successful, FALSE otherwise.*/

extern BOOL WINAPI console_manage_input_events(VOID);

#endif /*CONSOLE_H*/
