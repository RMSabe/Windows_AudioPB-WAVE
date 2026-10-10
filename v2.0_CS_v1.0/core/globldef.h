/*
	These are some definitions to ease application development on Windows systems.
	Version 4.0.2

	config.h is the macro configuration file, where all macro code settings are applied.
	Settings like text format, target system and so on are set in config.h.

	globldef.h is the global definition file. It defines/undefines macros based on the definitions in config.h file.
	It should be the first #include in all subsequent header and source files.

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com

	GitHub Repository: https://github.com/RMSabe/WinLib
*/

#ifndef GLOBLDEF_H
#define GLOBLDEF_H 1

#include "config.h"

#ifdef __cplusplus
#define __EXTERNC__ extern "C"
#else
#define __EXTERNC__ extern
#endif

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
#ifdef __TEXTFORMAT_USE_WCHAR
#error "DOS based Windows does not support UNICODE UTF-16 text format"
#endif
#endif

#ifdef __TEXTFORMAT_USE_WCHAR
#ifndef UNICODE
#define UNICODE
#endif
#else
#ifdef UNICODE
#undef UNICODE
#endif
#endif

#include <windows.h>

#define PTR_SIZE_BYTES (sizeof(VOID*))
#define PTR_SIZE_BITS (PTR_SIZE_BYTES*8U)
#define PTR_MAX_VALUE ((ULONG_PTR) ~((ULONG_PTR) 0U))
#define PTR_MSB_VALUE ((ULONG_PTR) (((ULONG_PTR) 1U) << (PTR_SIZE_BITS - 1U)))

#define TEXTBUF_SIZE_BYTES (TEXTBUF_SIZE_CHARS*sizeof(TCHAR))

struct _fileptr64 {
	ULONG32 l32;
	ULONG32 h32;
};

typedef struct _fileptr64 fileptr64_t;

__EXTERNC__ HINSTANCE p_instance;
__EXTERNC__ HANDLE p_processheap;

__EXTERNC__ TCHAR textbuf[];

/*
	Get the closest power of 2 value from an input value 
	_round() will round the result
	_floor() will return the closest power of 2 below the input value
	_ceil() will return the closest power of 2 above the input value 
	These functions are intended primarily for size of buffer allocations (SIZE_T), (ULONG_PTR), (UINT_PTR) 
	If the function fails or if input == 0, it returns 0
*/

__EXTERNC__ ULONG_PTR WINAPI _get_closest_power2_round(ULONG_PTR input);
__EXTERNC__ ULONG_PTR WINAPI _get_closest_power2_floor(ULONG_PTR input);
__EXTERNC__ ULONG_PTR WINAPI _get_closest_power2_ceil(ULONG_PTR input);

/* Check if input is a power of 2 */

__EXTERNC__ BOOL WINAPI _is_power2(ULONG_PTR input);

#endif /*GLOBLDEF_H*/
