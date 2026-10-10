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

#include "strdef.hpp"

__string WINAPI str_tolower(__string input)
{
	__string output;
	SIZE_T len;
	SIZE_T n_char;

	len = (SIZE_T) input.length();

	output = TEXT("");
	n_char = 0u;
	while(n_char < len)
	{
		if((input[n_char] >= 0x41) && (input[n_char] <= 0x5a)) output += (TCHAR) (((UINT) input[n_char]) | 0x20);
		else output += input[n_char];

		n_char++;
	}

	return output;
}

__string WINAPI str_toupper(__string input)
{
	__string output;
	SIZE_T len;
	SIZE_T n_char;

	len = (SIZE_T) input.length();

	output = TEXT("");
	n_char = 0u;
	while(n_char < len)
	{
		if((input[n_char] >= 0x61) && (input[n_char] <= 0x7a)) output += (TCHAR) (((UINT) input[n_char]) & 0xdf);
		else output += input[n_char];

		n_char++;
	}

	return output;
}
