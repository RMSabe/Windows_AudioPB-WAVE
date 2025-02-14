/*
	These are some definitions to ease application development on Windows systems.
	Version 3.2

	config.h is the macro configuration file, where all macro code settings are applied.
	Settings like text format, target system and so on are set in config.h.

	globldef.h is the global definition file. It defines/undefines macros based on the definitions in config.h file.
	It should be the first #include in all subsequent header and source files.

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#ifndef GLOBLDEF_H
#define GLOBLDEF_H

#include "config.h"

#ifdef __DOSWIN
#ifdef __NTWIN
#error "__DOSWIN and __NTWIN must not be defined simultaneously"
#endif
#endif

#ifndef __DOSWIN
#ifndef __NTWIN
#define __NTWIN
#endif
#endif

#ifdef __DOSWIN
#ifdef _WIN64
#error "DOS based Windows does not support 64 bit applications"
#endif
#ifdef __USE_UTF16
#error "DOS based Windows does not support UNICODE UTF-16 text format"
#endif
#endif

#ifdef __USE_UTF16
#ifndef UNICODE
#define UNICODE
#endif
#else
#ifdef UNICODE
#undef UNICODE
#endif
#endif

#include <windows.h>

#define TEXTBUF_SIZE_CHARS 256U
#define TEXTBUF_SIZE_BYTES (TEXTBUF_SIZE_CHARS*sizeof(TCHAR))

extern HINSTANCE p_instance;
extern HANDLE p_processheap;

extern TCHAR textbuf[];

#endif /*GLOBLDEF_H*/
