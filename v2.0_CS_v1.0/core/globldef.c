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

#include "globldef.h"

__declspec(align(PTR_SIZE_BYTES)) HINSTANCE p_instance = NULL;
__declspec(align(PTR_SIZE_BYTES)) HANDLE p_processheap = NULL;
__declspec(align(PTR_SIZE_BYTES)) TCHAR textbuf[TEXTBUF_SIZE_CHARS] = {'\0'};

ULONG_PTR WINAPI _get_closest_power2_round(ULONG_PTR input)
{
	ULONG_PTR _numptr1;
	ULONG_PTR _numptr2;

	if(!input) return 0u;
	if(_is_power2(input)) return input;

	if(input > PTR_MSB_VALUE)
	{
		_numptr1 = input - PTR_MSB_VALUE;
		_numptr2 = PTR_MAX_VALUE - input;

		if(_numptr1 < _numptr2) return PTR_MSB_VALUE;
		return 0u;
	}

	_numptr2 = _get_closest_power2_ceil(input);
	_numptr1 = (_numptr2 >> 1);

	if((input - _numptr1) < (_numptr2 - input)) return _numptr1;

	return _numptr2;
}

ULONG_PTR WINAPI _get_closest_power2_floor(ULONG_PTR input)
{
	ULONG_PTR _numptr;

	if(!input) return 0u;
	if(_is_power2(input)) return input;

	if(input > PTR_MSB_VALUE) return PTR_MSB_VALUE;

	_numptr = _get_closest_power2_ceil(input);
	_numptr = (_numptr >> 1);

	return _numptr;
}

ULONG_PTR WINAPI _get_closest_power2_ceil(ULONG_PTR input)
{
	ULONG_PTR _numptr;

	if(!input) return 0u;
	if(_is_power2(input)) return input;

	_numptr = 1u;

	while(_numptr)
	{
		if(_numptr > input) break;
		_numptr = (_numptr << 1);
	}

	return _numptr;
}

BOOL WINAPI _is_power2(ULONG_PTR input)
{
	ULONG_PTR _numptr;

	if(!input) return FALSE;

	_numptr = 1u;

	while(_numptr)
	{
		if(input == _numptr) return TRUE;
		_numptr = (_numptr << 1);
	}

	return FALSE;
}
