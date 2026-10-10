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

#ifndef CSTRDEF_H
#define CSTRDEF_H 1

#include "globldef.h"
#include <stdio.h>

#ifdef __TEXTFORMAT_USE_WCHAR
#define __SPRINTF(output, size, input, ...) swprintf(output, size, input, ## __VA_ARGS__)
#define __CSTRTOINT32(str) _wtol(str)
#define __CSTRTOINT64(str) _wtoi64(str)
#define __CSTRTODOUBLE(str) _wtof(str)
#else
#define __SPRINTF(output, size, input, ...) snprintf(output, size, input, ## __VA_ARGS__)
#define __CSTRTOINT32(str) atol(str)
#define __CSTRTODOUBLE(str) atof(str)
#ifdef __NTWIN
#define __CSTRTOINT64(str) _atoi64(str)
#endif
#endif

/*
	cstr_getlength()
	get the length of a null-terminated C string.

	returns length of string or -1 if error.
*/

__EXTERNC__ SSIZE_T WINAPI cstr_getlength(const TCHAR *str);

/*
	cstr_locatechar()
	locates a character within a null terminated C string.

	returns index of the first matching character, or -1 if character is not found or error.
*/

__EXTERNC__ SSIZE_T WINAPI cstr_locatechar(const TCHAR *str, TCHAR c);

/*
	cstr_compare()
	compare 2 null terminated C strings.

	returns TRUE if they're equal, FALSE if not equal or error.
*/

__EXTERNC__ BOOL WINAPI cstr_compare(const TCHAR *str1, const TCHAR *str2);

/*
	cstr_compare_upto_len()
	compare 2 null terminated C strings up to a given length (stop_index).

	The value in fail_if_nolen defines the outcome of this function if both strings are shorter than stop_index:
	If both strings are shorter than stop_index and fail_if_nolen is set to TRUE, then function will return FALSE, even if the strings are equal.
	If both strings are shorter than stop_index and fail_if_nolen is set to FALSE, then function will perform normal cstr_compare between the 2 strings.

	returns TRUE if they're equal up to stop_index, FALSE if not equal or error.
*/

__EXTERNC__ BOOL WINAPI cstr_compare_upto_len(const TCHAR *str1, const TCHAR *str2, SIZE_T stop_index, BOOL fail_if_nolen);

/*
	cstr_compare_upto_char()
	compare 2 null terminated C strings up to a given character (stop_char).

	The value in fail_if_nochar defines the outcome of this function if neither strings have stop_char.
	If neither strings have stop_char and fail_if_nochar is set to TRUE, then function will return FALSE, even if the strings are equal.
	If neither strings have stop_char and fail_if_nochar is set to FALSE, then function will perform normal cstr_compare between the 2 strings.

	returns TRUE if they're equal up to stop_char, FALSE if not equal or error.
*/

__EXTERNC__ BOOL WINAPI cstr_compare_upto_char(const TCHAR *str1, const TCHAR *str2, TCHAR stop_char, BOOL fail_if_nochar);

/*
	cstr_copy()
	Copy a null terminated C string to an output buffer, appends null terminator character.

	If output buffer is shorter than input string, the input characters past (bufferout_length - 1) will be truncated in the output.

	Returns TRUE if successful, FALSE if error.
*/

__EXTERNC__ BOOL WINAPI cstr_copy(const TCHAR *input_str, TCHAR *output_str, SIZE_T bufferout_length);

/*
	cstr_copy_upto_len()
	Copy a null terminated C string to an output buffer up to a specified length (stop_index).

	If input_str is shorter than stop_index, then function will copy the whole input_str.
	If stop_index is longer than output buffer length, the input characters past (bufferout_length - 1) will be truncated in the output.

	if append_nullchar is set to TRUE, then function will append a null terminator to the copied content in the output buffer.
	for safety, function will always write a null terminator at the very end of output buffer, even if append_nullchar is set to FALSE.

	returns TRUE if successful, FALSE if error.
*/

__EXTERNC__ BOOL WINAPI cstr_copy_upto_len(const TCHAR *input_str, TCHAR *output_str, SIZE_T bufferout_length, SIZE_T stop_index, BOOL append_nullchar);

/*
	cstr_copy_upto_char()
	Copy a null terminated C string to an output buffer up to a specified character (stop_char).

	If input_str does not have stop_char, then function will copy the whole input_str.
	If input_str content is longer than output buffer, all input characters past (bufferout_length - 1) will be truncated in the output.

	if append_nullchar is set to TRUE, then function will append a null terminator to the copied content in the output buffer.
	for safety, function will always write a null terminator at the very end of output buffer, even if append_nullchar is set to FALSE.

	returns TRUE if successful, FALSE if error.
*/

__EXTERNC__ BOOL WINAPI cstr_copy_upto_char(const TCHAR *input_str, TCHAR *output_str, SIZE_T bufferout_length, TCHAR stop_char, BOOL append_nullchar);

/*
	cstr_tolower() & cstr_toupper()
	converts all letters in a null terminated C string to upper/lower case.

	returns TRUE if successful, FALSE otherwise.
	Might not work too well with some specific UTF-16 characters.
*/

__EXTERNC__ BOOL WINAPI cstr_tolower(TCHAR *str, SIZE_T buffer_length);
__EXTERNC__ BOOL WINAPI cstr_toupper(TCHAR *str, SIZE_T buffer_length);

/*
	cstr_copy_..._to_...()
	Converts a null terminated C string into different text formats. Copy the converted text into the output buffer.

	returns TRUE if successful, FALSE if error.
*/

__EXTERNC__ BOOL WINAPI cstr_copy_text8_to_text16(const UINT8 *input_str, UINT16 *output_str, SIZE_T bufferout_length);
__EXTERNC__ BOOL WINAPI cstr_copy_text8_to_text32(const UINT8 *input_str, UINT32 *output_str, SIZE_T bufferout_length);

__EXTERNC__ BOOL WINAPI cstr_copy_text16_to_text8(const UINT16 *input_str, UINT8 *output_str, SIZE_T bufferout_length);
__EXTERNC__ BOOL WINAPI cstr_copy_text16_to_text32(const UINT16 *input_str, UINT32 *output_str, SIZE_T bufferout_length);

__EXTERNC__ BOOL WINAPI cstr_copy_text32_to_text8(const UINT32 *input_str, UINT8 *output_str, SIZE_T bufferout_length);
__EXTERNC__ BOOL WINAPI cstr_copy_text32_to_text16(const UINT32 *input_str, UINT16 *output_str, SIZE_T bufferout_length);

static inline BOOL cstr_copy_char_to_wchar(const CHAR *input_str, WCHAR *output_str, SIZE_T bufferout_length)
{
	return cstr_copy_text8_to_text16((const UINT8*) input_str, (UINT16*) output_str, bufferout_length);
}

static inline BOOL cstr_copy_wchar_to_char(const WCHAR *input_str, CHAR *output_str, SIZE_T bufferout_length)
{
	return cstr_copy_text16_to_text8((const UINT16*) input_str, (UINT8*) output_str, bufferout_length);
}

static inline BOOL cstr_copy_char_to_tchar(const CHAR *input_str, TCHAR *output_str, SIZE_T bufferout_length)
{
#ifdef __TEXTFORMAT_USE_WCHAR
	return cstr_copy_char_to_wchar(input_str, (WCHAR*) output_str, bufferout_length);
#else
	return cstr_copy((const TCHAR*) input_str, output_str, bufferout_length);
#endif
}

static inline BOOL cstr_copy_tchar_to_char(const TCHAR *input_str, CHAR *output_str, SIZE_T bufferout_length)
{
#ifdef __TEXTFORMAT_USE_WCHAR
	return cstr_copy_wchar_to_char((const WCHAR*) input_str, output_str, bufferout_length);
#else
	return cstr_copy(input_str, (TCHAR*) output_str, bufferout_length);
#endif
}

static inline BOOL cstr_copy_wchar_to_tchar(const WCHAR *input_str, TCHAR *output_str, SIZE_T bufferout_length)
{
#ifdef __TEXTFORMAT_USE_WCHAR
	return cstr_copy((const TCHAR*) input_str, output_str, bufferout_length);
#else
	return cstr_copy_wchar_to_char(input_str, (CHAR*) output_str, bufferout_length);
#endif
}

static inline BOOL cstr_copy_tchar_to_wchar(const TCHAR *input_str, WCHAR *output_str, SIZE_T bufferout_length)
{
#ifdef __TEXTFORMAT_USE_WCHAR
	return cstr_copy(input_str, (TCHAR*) output_str, bufferout_length);
#else
	return cstr_copy_char_to_wchar((const CHAR*) input_str, output_str, bufferout_length);
#endif
}

#endif /*CSTRDEF_H*/
