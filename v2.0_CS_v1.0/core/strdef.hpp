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

#ifndef STRDEF_HPP
#define STRDEF_HPP 1

#include "globldef.h"
#include <string>

#ifdef __TEXTFORMAT_USE_WCHAR
typedef std::wstring __string;
#define __TOSTRING(val) std::to_wstring(val)
#else
typedef std::string __string;
#define __TOSTRING(val) std::to_string(val)
#endif

/*Return new string with characters converted to upper/lower case.
Might not work well with specific UTF-16 characters.*/

extern __string WINAPI str_tolower(__string input);
extern __string WINAPI str_toupper(__string input);

#endif /*STRDEF_HPP*/
