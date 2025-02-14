/*
	Audio Playback application for Windows
	Version 1.0

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "globldef.h"
#include "cstrdef.h"
#include "console.h"

#include "strdef.hpp"

#include "audiopb_shared.hpp"
#include "AudioBaseClass.hpp"
#include "AudioPB_i16_1ch.hpp"
#include "AudioPB_i16_2ch.hpp"
#include "AudioPB_i24_1ch.hpp"
#include "AudioPB_i24_2ch.hpp"

#include <combaseapi.h>

#define PB_I16_1CH 1
#define PB_I16_2CH 2
#define PB_I24_1CH 3
#define PB_I24_2CH 4

AudioBaseClass *p_audio = nullptr;

HANDLE h_file = INVALID_HANDLE_VALUE;

ULONG64 fsize64 = 0u;
ULONG32* const p_fsize_l32 = (ULONG32*) &fsize64;
ULONG32* const p_fsize_h32 = &((ULONG32*) &fsize64)[1];

audiopb_params_t audio_params;

extern BOOL WINAPI app_init(VOID);
extern VOID WINAPI app_deinit(VOID);

extern BOOL WINAPI file_ext_check(VOID);
extern BOOL WINAPI file_open(VOID);
extern VOID WINAPI file_close(VOID);

extern INT WINAPI file_get_params(VOID);
extern BOOL WINAPI compare_signature(const CHAR *auth, const CHAR *buf, SIZE_T offset);

INT main(INT argc, CHAR **argv)
{
	INT n_ret = 0;

	if(!app_init())
	{
		app_deinit();
		return 1;
	}

	if(argc < 2)
	{
		console_wait_keypress(TEXT("Error: missing arguments\r\nThis executable requires an argument: <input file directory>\r\n"));
		goto _l_main_error;
	}

	cstr_char_to_tchar(argv[1], textbuf, TEXTBUF_SIZE_CHARS);
	audio_params.file_dir = textbuf;

	if(!file_ext_check())
	{
		console_wait_keypress(TEXT("Error: file format not supported.\r\n"));
		goto _l_main_error;
	}

	if(!file_open())
	{
		console_wait_keypress(TEXT("Error: could not open file.\r\n"));
		goto _l_main_error;
	}

	n_ret = file_get_params();
	if(n_ret < 0)
	{
		console_wait_keypress(TEXT("Error: audio format not supported or broken wave header.\r\n"));
		goto _l_main_error;
	}

	switch(n_ret)
	{
		case PB_I16_1CH:
			p_audio = new AudioPB_i16_1ch(&audio_params);
			break;

		case PB_I16_2CH:
			p_audio = new AudioPB_i16_2ch(&audio_params);
			break;

		case PB_I24_1CH:
			p_audio = new AudioPB_i24_1ch(&audio_params);
			break;

		case PB_I24_2CH:
			p_audio = new AudioPB_i24_2ch(&audio_params);
			break;
	}

	if(!p_audio->initialize())
	{
		console_wait_keypress(p_audio->getLastErrorMessage().c_str());
		goto _l_main_error;
	}

	if(!p_audio->runPlayback())
	{
		console_wait_keypress(p_audio->getLastErrorMessage().c_str());
		goto _l_main_error;
	}

	app_deinit();
	return 0;

	_l_main_error:
	app_deinit();
	return 1;
}

BOOL WINAPI app_init(VOID)
{
	HRESULT n_ret = 0;

	p_processheap = GetProcessHeap();
	if(p_processheap == nullptr)
	{
		__SPRINTF(textbuf, TEXTBUF_SIZE_CHARS, TEXT("Error: Invalid Process Heap."));
		goto _l_app_init_error;
	}

	if(!console_init())
	{
		__SPRINTF(textbuf, TEXTBUF_SIZE_CHARS, TEXT("Error: Console Init Failed."));
		goto _l_app_init_error;
	}

	n_ret = CoInitialize(NULL);
	if(n_ret != S_OK)
	{
		__SPRINTF(textbuf, TEXTBUF_SIZE_CHARS, TEXT("Error: COMBASEAPI Init Failed."));
		goto _l_app_init_error;
	}

	console_set_title(TEXT("WAVE Audio Playback"));
	return TRUE;

	_l_app_init_error:
	MessageBox(NULL, textbuf, TEXT("INIT ERROR"), (MB_ICONSTOP | MB_OK));
	return FALSE;
}

VOID WINAPI app_deinit(VOID)
{
	if(p_audio != nullptr)
	{
		delete p_audio;
		p_audio = nullptr;
	}

	CoUninitialize();
	console_deinit();

	return;
}

__declspec(noreturn) VOID WINAPI app_exit(UINT exit_code, const TCHAR *exit_msg, BOOL pop_msgbox)
{
	if(exit_msg != nullptr)
	{
		if(pop_msgbox) MessageBox(NULL, exit_msg, TEXT("ERROR OCCURRED"), (MB_ICONSTOP | MB_OK));
		else console_wait_keypress(exit_msg);
	}

	app_deinit();
	ExitProcess(exit_code);

	while(TRUE) Sleep(1u); //Not really necessary, but just to be safe.
}

BOOL WINAPI file_ext_check(VOID)
{
	SIZE_T len = 0u;

	if(audio_params.file_dir == nullptr) return FALSE;

	__SPRINTF(textbuf, TEXTBUF_SIZE_CHARS, TEXT("%s"), audio_params.file_dir);

	cstr_tolower(textbuf, TEXTBUF_SIZE_CHARS);

	len = cstr_getlength(textbuf);

	if(len < 5u) return FALSE;

	return cstr_compare(TEXT(".wav"), &textbuf[len - 4u]);
}

BOOL WINAPI file_open(VOID)
{
	if(audio_params.file_dir == nullptr) return FALSE;

	h_file = CreateFile(audio_params.file_dir, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
	if(h_file == INVALID_HANDLE_VALUE) return FALSE;

	*p_fsize_l32 = (ULONG32) GetFileSize(h_file, (DWORD*) p_fsize_h32);
	return TRUE;
}

VOID WINAPI file_close(VOID)
{
	if(h_file == INVALID_HANDLE_VALUE) return;

	CloseHandle(h_file);
	h_file = INVALID_HANDLE_VALUE;
	fsize64 = 0u;
	return;
}

INT WINAPI file_get_params(VOID)
{	
	const SIZE_T HEADERINFO_SIZE = 4096u;
	CHAR *header_info = nullptr;
	UINT16 *pu16 = nullptr;
	UINT32 *pu32 = nullptr;

	SIZE_T bytepos = 0u;

	DWORD dummy_32;

	UINT16 bit_depth = 0u;
	UINT16 n_channels = 0u;

	header_info = (CHAR*) HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, HEADERINFO_SIZE);
	if(header_info == nullptr) goto _l_file_get_params_error;

	SetFilePointer(h_file, 0, NULL, FILE_BEGIN);
	ReadFile(h_file, header_info, (DWORD) HEADERINFO_SIZE, &dummy_32, NULL);
	file_close();

	if(!compare_signature("RIFF", header_info, 0u)) goto _l_file_get_params_error;

	if(!compare_signature("WAVE", header_info, 8u)) goto _l_file_get_params_error;

	bytepos = 12u;

	while(!compare_signature("fmt ", header_info, bytepos))
	{
		if(bytepos >= (HEADERINFO_SIZE - 256u)) goto _l_file_get_params_error;

		pu32 = (UINT32*) &header_info[bytepos + 4u];
		bytepos += (SIZE_T) (*pu32 + 8u);
	}

	pu16 = (UINT16*) &header_info[bytepos + 8u];

	if(pu16[0] != 1u) goto _l_file_get_params_error;

	n_channels = pu16[1];

	pu32 = (UINT32*) &header_info[bytepos + 12u];
	audio_params.sample_rate = *pu32;

	pu16 = (UINT16*) &header_info[bytepos + 22u];
	bit_depth = *pu16;

	pu32 = (UINT32*) &header_info[bytepos + 4u];
	bytepos += (SIZE_T) (*pu32 + 8u);

	while(!compare_signature("data", header_info, bytepos))
	{
		if(bytepos >= (HEADERINFO_SIZE - 256u)) goto _l_file_get_params_error;

		pu32 = (UINT32*) &header_info[bytepos + 4u];
		bytepos += (SIZE_T) (*pu32 + 8u);
	}

	pu32 = (UINT32*) &header_info[bytepos + 4u];

	audio_params.audio_data_begin = (ULONG64) (bytepos + 8u);
	audio_params.audio_data_end = audio_params.audio_data_begin + ((ULONG64) *pu32);

	HeapFree(p_processheap, 0u, header_info);
	header_info = nullptr;

	if((bit_depth == 16u) && (n_channels == 1u)) return PB_I16_1CH;
	if((bit_depth == 16u) && (n_channels == 2u)) return PB_I16_2CH;
	if((bit_depth == 24u) && (n_channels == 1u)) return PB_I24_1CH;
	if((bit_depth == 24u) && (n_channels == 2u)) return PB_I24_2CH;

	_l_file_get_params_error:
	if(header_info != nullptr) HeapFree(p_processheap, 0u, header_info);
	return -1;
}

BOOL WINAPI compare_signature(const CHAR *auth, const CHAR *buf, SIZE_T offset)
{
	if(auth == nullptr) return FALSE;
	if(buf == nullptr) return FALSE;

	if(auth[0] != buf[offset]) return FALSE;
	if(auth[1] != buf[offset + 1u]) return FALSE;
	if(auth[2] != buf[offset + 2u]) return FALSE;
	if(auth[3] != buf[offset + 3u]) return FALSE;

	return TRUE;
}
