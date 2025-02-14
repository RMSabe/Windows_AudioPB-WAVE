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

#include "globldef.h"

HINSTANCE p_instance = NULL;
HANDLE p_processheap = NULL;

TCHAR textbuf[TEXTBUF_SIZE_CHARS] = {'\0'};
