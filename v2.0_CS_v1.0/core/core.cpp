/*
	WAVE Audio Playback for Windows
	Version 4.0 (Interop C# version 1.0)

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "globldef.h"
#include "cstrdef.h"
#include "strdef.hpp"

#include "shared.h"

#include <combaseapi.h>

#include "AudioPB.hpp"
#include "AudioPB_i16.hpp"
#include "AudioPB_i24.hpp"

#define __PB_I16 1
#define __PB_I24 2

static __declspec(align(PTR_SIZE_BYTES)) AudioPB *p_audio = NULL;
static __declspec(align(PTR_SIZE_BYTES)) audiopb_params_t pb_params;
static __declspec(align(PTR_SIZE_BYTES)) HANDLE h_filein = INVALID_HANDLE_VALUE;
static __declspec(align(PTR_SIZE_BYTES)) __string filein_dir = TEXT("");
static __declspec(align(PTR_SIZE_BYTES)) __string err_msg = TEXT("");

__declspec(align(4)) const ULONG32 P_PKEY_Device_FriendlyName[] = {0xa45c254e, 0x4efddf1c, 0xd1672080, 0xe050a846, 14u};

__EXTERNC__ __declspec(dllexport) BOOL APIENTRY Initialize(VOID);
__EXTERNC__ __declspec(dllexport) VOID APIENTRY Deinitialize(VOID);
__EXTERNC__ __declspec(dllexport) const TCHAR* APIENTRY GetLastErrorMessage(VOID);
__EXTERNC__ __declspec(dllexport) BOOL APIENTRY SetFileInDirectory(const TCHAR *fdir);
__EXTERNC__ __declspec(dllexport) BOOL APIENTRY LoadFile_CreateAudioObject(VOID);
__EXTERNC__ __declspec(dllexport) BOOL APIENTRY LoadAudioDeviceList(VOID);
__EXTERNC__ __declspec(dllexport) LONG_PTR APIENTRY GetAudioDeviceListEntryCount(VOID);
__EXTERNC__ __declspec(dllexport) const TCHAR* APIENTRY GetAudioDeviceListEntry_FriendlyName(ULONG_PTR index);
__EXTERNC__ __declspec(dllexport) BOOL APIENTRY ChooseDevice(ULONG_PTR index);
__EXTERNC__ __declspec(dllexport) BOOL APIENTRY ChooseDefaultDevice(VOID);
__EXTERNC__ __declspec(dllexport) BOOL APIENTRY InitializeAudioObject(VOID);
__EXTERNC__ __declspec(dllexport) INT APIENTRY GetStatus(VOID);
__EXTERNC__ __declspec(dllexport) BOOL APIENTRY RunPlayback(VOID);
__EXTERNC__ __declspec(dllexport) VOID APIENTRY PausePlayback(VOID);
__EXTERNC__ __declspec(dllexport) VOID APIENTRY ResumePlayback(VOID);
__EXTERNC__ __declspec(dllexport) VOID APIENTRY StopPlayback(VOID);
__EXTERNC__ __declspec(dllexport) LONG64 APIENTRY GetAudioDataSizeFrames(VOID);
__EXTERNC__ __declspec(dllexport) LONG64 APIENTRY GetAudioDataPositionFrames(VOID);
__EXTERNC__ __declspec(dllexport) BOOL APIENTRY SetAudioDataPositionFrames(ULONG64 position);

static BOOL WINAPI filein_open(VOID);
static VOID WINAPI filein_close(VOID);

static INT WINAPI filein_get_params(VOID);
static BOOL WINAPI compare_signature(const CHAR *auth, const UINT8 *buf);

__declspec(dllexport) BOOL APIENTRY Initialize(VOID)
{
	HRESULT _n_ret;

	p_processheap = GetProcessHeap();
	if(p_processheap == NULL)
	{
		err_msg = TEXT("Error: failed to retrieve process heap.");
		goto _l_Initialize_error;
	}

	_n_ret = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if((_n_ret != S_OK) && (_n_ret != S_FALSE))
	{
		err_msg = TEXT("Error: failed to initialize COMBASEAPI.");
		goto _l_Initialize_error;
	}

	return TRUE;

_l_Initialize_error:

	Deinitialize();
	return FALSE;
}

__declspec(dllexport) VOID APIENTRY Deinitialize(VOID)
{
	filein_close();

	if(p_audio != NULL)
	{
		delete p_audio;
		p_audio = NULL;
	}

	CoUninitialize();
	return;
}

__declspec(dllexport) const TCHAR* APIENTRY GetLastErrorMessage(VOID)
{
	return err_msg.c_str();
}

__declspec(dllexport) BOOL APIENTRY SetFileInDirectory(const TCHAR *fdir)
{
	if(fdir == NULL)
	{
		err_msg = TEXT("Error: invalid parameter.");
		return FALSE;
	}

	filein_dir = fdir;
	return TRUE;
}

__declspec(dllexport) BOOL APIENTRY LoadFile_CreateAudioObject(VOID)
{
	INT _n_ret;

	if(!filein_open())
	{
		err_msg = TEXT("Error: failed to open input file.");
		goto _l_LoadFile_CreateAudioObject_error;
	}

	_n_ret = filein_get_params();
	if(_n_ret < 0) goto _l_LoadFile_CreateAudioObject_error;

	if(p_audio != NULL)
	{
		delete p_audio;
		p_audio = NULL;
	}

	pb_params.audiobuffer_size_frames = _get_closest_power2_ceil((ULONG_PTR) pb_params.sample_rate);
	pb_params.streambuffer_segment_size_frames = pb_params.audiobuffer_size_frames/4u;
	pb_params.streambuffer_n_segments = 2u;
	pb_params.file_dir = filein_dir.c_str();

	switch(_n_ret)
	{
		case __PB_I16:
			p_audio = new AudioPB_i16(&pb_params);
			break;

		case __PB_I24:
			p_audio = new AudioPB_i24(&pb_params);
			break;
	}

	if(p_audio != NULL) return TRUE;

	err_msg = TEXT("Error: failed to create audio object instance.");

_l_LoadFile_CreateAudioObject_error:

	filein_close();
	return FALSE;
}

__declspec(dllexport) BOOL APIENTRY LoadAudioDeviceList(VOID)
{
	if(p_audio == NULL)
	{
		err_msg = TEXT("Error: audio object is not ready.");
		return FALSE;
	}

	if(!p_audio->loadAudioDeviceList())
	{
		err_msg = p_audio->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

__declspec(dllexport) LONG_PTR APIENTRY GetAudioDeviceListEntryCount(VOID)
{
	LONG_PTR _count;

	if(p_audio == NULL)
	{
		err_msg = TEXT("Error: audio object is not ready.");
		return -1;
	}

	_count = p_audio->getAudioDeviceListEntryCount();
	if(_count < 0) err_msg = p_audio->getLastErrorMessage();

	return _count;
}

__declspec(dllexport) const TCHAR* APIENTRY GetAudioDeviceListEntry_FriendlyName(ULONG_PTR index)
{
	const TCHAR *_entry = NULL;

	if(p_audio == NULL)
	{
		err_msg = TEXT("Error: audio object is not ready.");
		return NULL;
	}

	_entry = p_audio->getAudioDeviceListEntry(index);
	if(_entry == NULL) err_msg = p_audio->getLastErrorMessage();

	return _entry;
}

__declspec(dllexport) BOOL APIENTRY ChooseDevice(ULONG_PTR index)
{
	if(p_audio == NULL)
	{
		err_msg = TEXT("Error: audio object is not ready.");
		return FALSE;
	}

	if(!p_audio->chooseDevice(index))
	{
		err_msg = p_audio->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

__declspec(dllexport) BOOL APIENTRY ChooseDefaultDevice(VOID)
{
	if(p_audio == NULL)
	{
		err_msg = TEXT("Error: audio object is not ready.");
		return FALSE;
	}

	if(!p_audio->chooseDefaultDevice())
	{
		err_msg = p_audio->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

__declspec(dllexport) BOOL APIENTRY InitializeAudioObject(VOID)
{
	if(p_audio == NULL)
	{
		err_msg = TEXT("Error: audio object is not ready.");
		return FALSE;
	}

	if(!p_audio->initialize())
	{
		err_msg = p_audio->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

__declspec(dllexport) INT APIENTRY GetStatus(VOID)
{
	if(p_audio == NULL)
	{
		err_msg = TEXT("Error: audio object is not ready.");
		return -1;
	}

	return p_audio->getStatus();
}

__declspec(dllexport) BOOL APIENTRY RunPlayback(VOID)
{
	if(p_audio == NULL)
	{
		err_msg = TEXT("Error: audio object is not ready.");
		return FALSE;
	}

	if(!p_audio->runPlayback())
	{
		err_msg = p_audio->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

__declspec(dllexport) VOID APIENTRY PausePlayback(VOID)
{
	if(p_audio != NULL) p_audio->pausePlayback();
	return;
}

__declspec(dllexport) VOID APIENTRY ResumePlayback(VOID)
{
	if(p_audio != NULL) p_audio->resumePlayback();
	return;
}

__declspec(dllexport) VOID APIENTRY StopPlayback(VOID)
{
	if(p_audio != NULL) p_audio->stopPlayback();
	return;
}

__declspec(dllexport) LONG64 APIENTRY GetAudioDataSizeFrames(VOID)
{
	LONG64 _datasize_frames;

	if(p_audio == NULL)
	{
		err_msg = TEXT("Error: audio object is not ready.");
		return -1;
	}

	_datasize_frames = p_audio->getAudioDataSizeFrames();
	if(_datasize_frames < 0) err_msg = p_audio->getLastErrorMessage();

	return _datasize_frames;
}

__declspec(dllexport) LONG64 APIENTRY GetAudioDataPositionFrames(VOID)
{
	LONG64 _datapos_frames;

	if(p_audio == NULL)
	{
		err_msg = TEXT("Error: audio object is not ready.");
		return -1;
	}

	_datapos_frames = p_audio->getAudioDataPositionFrames();
	if(_datapos_frames < 0) err_msg = p_audio->getLastErrorMessage();

	return _datapos_frames;
}

__declspec(dllexport) BOOL APIENTRY SetAudioDataPositionFrames(ULONG64 position)
{
	if(p_audio == NULL)
	{
		err_msg = TEXT("Error: audio object is not ready.");
		return FALSE;
	}

	if(!p_audio->setAudioDataPositionFrames(position))
	{
		err_msg = p_audio->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

static BOOL WINAPI filein_open(VOID)
{
	filein_close();

	h_filein = CreateFile(filein_dir.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, INVALID_HANDLE_VALUE);
	return (h_filein != INVALID_HANDLE_VALUE);
}

static VOID WINAPI filein_close(VOID)
{
	if(h_filein == INVALID_HANDLE_VALUE) return;

	CloseHandle(h_filein);
	h_filein = INVALID_HANDLE_VALUE;
	return;
}

static INT WINAPI filein_get_params(VOID)
{
	const ULONG_PTR _BUFFER_SIZE = 8192u;
	ULONG_PTR _buffer_index;
	UINT8 *_p_headerinfo = NULL;

	DWORD _dummy32;
	UINT32 _u32;
	UINT16 _u16;
	UINT16 _bitdepth;

	if(p_processheap == NULL)
	{
		err_msg = TEXT("Error: p_processheap is NULL.");
		goto _l_filein_get_params_error;
	}

	_p_headerinfo = (UINT8*) HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, _BUFFER_SIZE);
	if(_p_headerinfo == NULL)
	{
		err_msg = TEXT("Error: HeapAlloc failed.");
		goto _l_filein_get_params_error;
	}

	SetFilePointer(h_filein, 0, NULL, FILE_BEGIN);
	ReadFile(h_filein, _p_headerinfo, (DWORD) _BUFFER_SIZE, &_dummy32, NULL);
	filein_close();

	if(!compare_signature("RIFF", _p_headerinfo))
	{
		err_msg = TEXT("Error: file format not supported.");
		goto _l_filein_get_params_error;
	}

	if(!compare_signature("WAVE", (const UINT8*) (((ULONG_PTR) _p_headerinfo) + 8u)))
	{
		err_msg = TEXT("Error: file format not supported.");
		goto _l_filein_get_params_error;
	}

	_buffer_index = 12u;

	while(TRUE)
	{
		if(_buffer_index > (_BUFFER_SIZE - 8u))
		{
			err_msg = TEXT("Error: broken file header.");
			goto _l_filein_get_params_error;
		}

		if(compare_signature("fmt ", (const UINT8*) (((ULONG_PTR) _p_headerinfo) + _buffer_index))) break;

		_u32 = *((UINT32*) (((ULONG_PTR) _p_headerinfo) + _buffer_index + 4u));
		_buffer_index += (ULONG_PTR) (_u32 + 8u);
	}

	if(_buffer_index > (_BUFFER_SIZE - 24u))
	{
		err_msg = TEXT("Error: broken file header.");
		goto _l_filein_get_params_error;
	}

	_u16 = *((UINT16*) (((ULONG_PTR) _p_headerinfo) + _buffer_index + 8u));
	if(_u16 != 1u)
	{
		err_msg = TEXT("Error: audio encoding format not supported.");
		goto _l_filein_get_params_error;
	}

	pb_params.n_channels = *((UINT16*) (((ULONG_PTR) _p_headerinfo) + _buffer_index + 10u));
	pb_params.sample_rate = *((UINT32*) (((ULONG_PTR) _p_headerinfo) + _buffer_index + 12u));
	_bitdepth = *((UINT16*) (((ULONG_PTR) _p_headerinfo) + _buffer_index + 22u));

	pb_params.channel_mask = 0u; /*Feature is reserved for future versions.*/

	_u32 = *((UINT32*) (((ULONG_PTR) _p_headerinfo) + _buffer_index + 4u));
	_buffer_index += (ULONG_PTR) (_u32 + 8u);

	while(TRUE)
	{
		if(_buffer_index > (_BUFFER_SIZE - 8u))
		{
			err_msg = TEXT("Error: broken file header.");
			goto _l_filein_get_params_error;
		}

		if(compare_signature("data", (const UINT8*) (((ULONG_PTR) _p_headerinfo) + _buffer_index))) break;

		_u32 = *((UINT32*) (((ULONG_PTR) _p_headerinfo) + _buffer_index + 4u));
		_buffer_index += (ULONG_PTR) (_u32 + 8u);
	}

	_u32 = *((UINT32*) (((ULONG_PTR) _p_headerinfo) + _buffer_index + 4u));

	pb_params.audio_data_begin = (ULONG64) (_buffer_index + 8u);
	pb_params.audio_data_end = pb_params.audio_data_begin + ((ULONG64) _u32);

	HeapFree(p_processheap, 0u, _p_headerinfo);
	_p_headerinfo = NULL;

	switch(_bitdepth)
	{
		case 16u:
			return __PB_I16;

		case 24u:
			return __PB_I24;
	}

	err_msg = TEXT("Error: audio format not supported.");

_l_filein_get_params_error:

	filein_close();
	if(_p_headerinfo != NULL) HeapFree(p_processheap, 0u, _p_headerinfo);
	return -1;
}

static BOOL WINAPI compare_signature(const CHAR *auth, const UINT8 *buf)
{
	ULONG_PTR _index;

	if(auth == NULL) return FALSE;
	if(buf == NULL) return FALSE;

	for(_index = 0u; _index < 4u; _index++) if(auth[_index] != ((CHAR) buf[_index])) return FALSE;

	return TRUE;
}

__declspec(noreturn) VOID WINAPI app_exit(UINT exit_code, const TCHAR *exit_msg)
{
	Deinitialize();

	if(exit_msg != NULL) MessageBox(NULL, exit_msg, TEXT("PROCESS EXIT CALLED"), (MB_ICONSTOP | MB_OK));

	ExitProcess(exit_code);

	while(TRUE) Sleep(10u);
}
