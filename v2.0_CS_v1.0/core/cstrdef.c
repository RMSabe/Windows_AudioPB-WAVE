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

#include "cstrdef.h"

SSIZE_T WINAPI cstr_getlength(const TCHAR *str)
{
	SIZE_T len;

	if(str == NULL) return -1;

	len = 0u;
	while(str[len] != '\0') len++;

	return (SSIZE_T) len;
}

SSIZE_T WINAPI cstr_locatechar(const TCHAR *str, TCHAR c)
{
	SIZE_T len;
	SIZE_T n_char;

	if(str == NULL) return -1;

	len = (SIZE_T) cstr_getlength(str);

	n_char = 0u;
	while(n_char < len)
	{
		if(str[n_char] == c) return (SSIZE_T) n_char;
		n_char++;
	}

	return -1;
}

BOOL WINAPI cstr_compare(const TCHAR *str1, const TCHAR *str2)
{
	SIZE_T num1;
	SIZE_T num2;

	if(str1 == NULL) return FALSE;
	if(str2 == NULL) return FALSE;

	num1 = (SIZE_T) cstr_getlength(str1);
	num2 = (SIZE_T) cstr_getlength(str2);

	if(num1 != num2) return FALSE;

	num1 = 0u;
	while(num1 < num2)
	{
		if(str1[num1] != str2[num1]) return FALSE;
		num1++;
	}

	return TRUE;
}

BOOL WINAPI cstr_compare_upto_len(const TCHAR *str1, const TCHAR *str2, SIZE_T stop_index, BOOL fail_if_nolen)
{
	SIZE_T num1;
	SIZE_T num2;

	if(str1 == NULL) return FALSE;
	if(str2 == NULL) return FALSE;

	num1 = (SIZE_T) cstr_getlength(str1);
	num2 = (SIZE_T) cstr_getlength(str2);

	if((num1 < stop_index) && (num2 < stop_index))
	{
		if(fail_if_nolen) return FALSE;

		return cstr_compare(str1, str2);
	}

	if((num1 < stop_index) || (num2 < stop_index)) return FALSE;

	num1 = 0u;
	while(num1 < stop_index)
	{
		if(str1[num1] != str2[num1]) return FALSE;
		num1++;
	}

	return TRUE;
}

BOOL WINAPI cstr_compare_upto_char(const TCHAR *str1, const TCHAR *str2, TCHAR stop_char, BOOL fail_if_nochar)
{
	SSIZE_T num1;
	SSIZE_T num2;

	if(str1 == NULL) return FALSE;
	if(str2 == NULL) return FALSE;

	num1 = cstr_locatechar(str1, stop_char);
	num2 = cstr_locatechar(str2, stop_char);

	if((num1 < 0) && (num2 < 0))
	{
		if(fail_if_nochar) return FALSE;

		return cstr_compare(str1, str2);
	}

	if(num1 < 0) goto _l_cstr_compare_upto_char_compareloop;

	if((num2 < 0) || (num1 < num2)) num2 = num1;

_l_cstr_compare_upto_char_compareloop:

	num1 = 0u;
	while(num1 < num2)
	{
		if(str1[num1] != str2[num1]) return FALSE;
		num1++;
	}

	return TRUE;
}

BOOL WINAPI cstr_copy(const TCHAR *input_str, TCHAR *output_str, SIZE_T bufferout_length)
{
	SIZE_T stop_index;

	if(input_str == NULL) return FALSE;
	if(output_str == NULL) return FALSE;
	if(!bufferout_length) return FALSE;

	stop_index = (SIZE_T) cstr_getlength(input_str);

	if(stop_index >= bufferout_length) stop_index = bufferout_length - 1u;

	CopyMemory(output_str, input_str, (stop_index*sizeof(TCHAR)));

	output_str[stop_index] = (TCHAR) '\0';

	return TRUE;
}

BOOL WINAPI cstr_copy_upto_len(const TCHAR *input_str, TCHAR *output_str, SIZE_T bufferout_length, SIZE_T stop_index, BOOL append_nullchar)
{
	SIZE_T input_len;

	if(input_str == NULL) return FALSE;
	if(output_str == NULL) return FALSE;
	if(!bufferout_length) return FALSE;

	output_str[bufferout_length - 1u] = (TCHAR) '\0'; /*Write null char terminator to the end of output buffer, for safety*/

	input_len = (SIZE_T) cstr_getlength(input_str);
	if(input_len < stop_index) stop_index = input_len;

	if(stop_index >= bufferout_length) stop_index = bufferout_length - 1u;

	CopyMemory(output_str, input_str, (stop_index*sizeof(TCHAR)));

	if(append_nullchar) output_str[stop_index] = (TCHAR) '\0';

	return TRUE;
}

BOOL WINAPI cstr_copy_upto_char(const TCHAR *input_str, TCHAR *output_str, SIZE_T bufferout_length, TCHAR stop_char, BOOL append_nullchar)
{
	SSIZE_T num1;

	if(input_str == NULL) return FALSE;
	if(output_str == NULL) return FALSE;
	if(!bufferout_length) return FALSE;

	num1 = cstr_locatechar(input_str, stop_char);

	if(num1 < 0) num1 = cstr_getlength(input_str);

	return cstr_copy_upto_len(input_str, output_str, bufferout_length, (SIZE_T) num1, append_nullchar);
}

BOOL WINAPI cstr_tolower(TCHAR *str, SIZE_T buffer_length)
{
	SIZE_T len;
	SIZE_T n_char;

	if(str == NULL) return FALSE;
	if(!buffer_length) return FALSE;

	str[buffer_length - 1u] = (TCHAR) '\0';

	len = (SIZE_T) cstr_getlength(str);

	n_char = 0u;
	while(n_char < len)
	{
		if((str[n_char] >= 0x41) && (str[n_char] <= 0x5a)) str[n_char] |= 0x20;
		n_char++;
	}

	return TRUE;
}

BOOL WINAPI cstr_toupper(TCHAR *str, SIZE_T buffer_length)
{
	SIZE_T len;
	SIZE_T n_char;

	if(str == NULL) return FALSE;
	if(!buffer_length) return FALSE;

	str[buffer_length - 1u] = (TCHAR) '\0';

	len = (SIZE_T) cstr_getlength(str);

	n_char = 0u;
	while(n_char < len)
	{
		if((str[n_char] >= 0x61) && (str[n_char] <= 0x7a)) str[n_char] &= 0xdf;
		n_char++;
	}

	return TRUE;
}

BOOL WINAPI cstr_copy_text8_to_text16(const UINT8 *input_str, UINT16 *output_str, SIZE_T bufferout_length)
{
	SIZE_T stop_index;
	SIZE_T n_char;

	if(input_str == NULL) return FALSE;
	if(output_str == NULL) return FALSE;
	if(!bufferout_length) return FALSE;

	stop_index = 0u;
	while(input_str[stop_index] != '\0') stop_index++;

	if(stop_index >= bufferout_length) stop_index = bufferout_length - 1u;

	n_char = 0u;
	while(n_char < stop_index)
	{
		output_str[n_char] = (UINT16) input_str[n_char];
		n_char++;
	}

	output_str[stop_index] = '\0';

	return TRUE;
}

BOOL WINAPI cstr_copy_text8_to_text32(const UINT8 *input_str, UINT32 *output_str, SIZE_T bufferout_length)
{
	SIZE_T stop_index;
	SIZE_T n_char;

	if(input_str == NULL) return FALSE;
	if(output_str == NULL) return FALSE;
	if(!bufferout_length) return FALSE;

	stop_index = 0u;
	while(input_str[stop_index] != '\0') stop_index++;

	if(stop_index >= bufferout_length) stop_index = bufferout_length - 1u;

	n_char = 0u;
	while(n_char < stop_index)
	{
		output_str[n_char] = (UINT32) input_str[n_char];
		n_char++;
	}

	output_str[stop_index] = '\0';

	return TRUE;
}

BOOL WINAPI cstr_copy_text16_to_text8(const UINT16 *input_str, UINT8 *output_str, SIZE_T bufferout_length)
{
	SIZE_T stop_index;
	SIZE_T n_char;

	if(input_str == NULL) return FALSE;
	if(output_str == NULL) return FALSE;
	if(!bufferout_length) return FALSE;

	stop_index = 0u;
	while(input_str[stop_index] != '\0') stop_index++;

	if(stop_index >= bufferout_length) stop_index = bufferout_length - 1u;

	n_char = 0u;
	while(n_char < stop_index)
	{
		output_str[n_char] = (UINT8) input_str[n_char];
		n_char++;
	}

	output_str[stop_index] = '\0';

	return TRUE;
}

BOOL WINAPI cstr_copy_text16_to_text32(const UINT16 *input_str, UINT32 *output_str, SIZE_T bufferout_length)
{
	SIZE_T stop_index;
	SIZE_T n_char;

	if(input_str == NULL) return FALSE;
	if(output_str == NULL) return FALSE;
	if(!bufferout_length) return FALSE;

	stop_index = 0u;
	while(input_str[stop_index] != '\0') stop_index++;

	if(stop_index >= bufferout_length) stop_index = bufferout_length - 1u;

	n_char = 0u;
	while(n_char < stop_index)
	{
		output_str[n_char] = (UINT32) input_str[n_char];
		n_char++;
	}

	output_str[stop_index] = '\0';

	return TRUE;
}

BOOL WINAPI cstr_copy_text32_to_text8(const UINT32 *input_str, UINT8 *output_str, SIZE_T bufferout_length)
{
	SIZE_T stop_index;
	SIZE_T n_char;

	if(input_str == NULL) return FALSE;
	if(output_str == NULL) return FALSE;
	if(!bufferout_length) return FALSE;

	stop_index = 0u;
	while(input_str[stop_index] != '\0') stop_index++;

	if(stop_index >= bufferout_length) stop_index = bufferout_length - 1u;

	n_char = 0u;
	while(n_char < stop_index)
	{
		output_str[n_char] = (UINT8) input_str[n_char];
		n_char++;
	}

	output_str[stop_index] = '\0';

	return TRUE;
}

BOOL WINAPI cstr_copy_text32_to_text16(const UINT32 *input_str, UINT16 *output_str, SIZE_T bufferout_length)
{
	SIZE_T stop_index;
	SIZE_T n_char;

	if(input_str == NULL) return FALSE;
	if(output_str == NULL) return FALSE;
	if(!bufferout_length) return FALSE;

	stop_index = 0u;
	while(input_str[stop_index] != '\0') stop_index++;

	if(stop_index >= bufferout_length) stop_index = bufferout_length - 1u;

	n_char = 0u;
	while(n_char < stop_index)
	{
		output_str[n_char] = (UINT16) input_str[n_char];
		n_char++;
	}

	output_str[stop_index] = '\0';

	return TRUE;
}
