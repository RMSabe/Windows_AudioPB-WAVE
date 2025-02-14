/*
	Audio Playback application for Windows
	Version 1.0

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "AudioBaseClass.hpp"

#include "cstrdef.h"
#include "console.h"

#include <combaseapi.h>

AudioBaseClass::AudioBaseClass(const audiopb_params_t *p_params)
{
	this->setParameters(p_params);
}

BOOL WINAPI AudioBaseClass::setParameters(const audiopb_params_t *p_params)
{
	if(p_params == nullptr) return FALSE;
	if(p_params->file_dir == nullptr) return FALSE;

	this->filein_dir = p_params->file_dir;
	this->audio_data_begin = p_params->audio_data_begin;
	this->audio_data_end = p_params->audio_data_end;
	this->sample_rate = p_params->sample_rate;

	return TRUE;
}

BOOL WINAPI AudioBaseClass::initialize(VOID)
{
	if(!this->filein_open())
	{
		this->status = this->RUNTIME_STATUS_ERROR_NOFILE;
		return FALSE;
	}

	if(!this->audio_hw_init())
	{
		this->filein_close();
		this->status = this->RUNTIME_STATUS_ERROR_AUDIOHW;
		return FALSE;
	}

	if(!this->buffer_alloc())
	{
		this->filein_close();
		this->audio_hw_deinit();
		this->status = this->RUNTIME_STATUS_ERROR_MEMALLOC;
		return FALSE;
	}

	this->status = this->RUNTIME_STATUS_INITIALIZED;
	return TRUE;
}

BOOL WINAPI AudioBaseClass::runPlayback(VOID)
{
	if(this->status < 1) return FALSE;

	console_stdout_write(TEXT("Playback started\r\n"));
	this->playback_proc();
	console_stdout_write(TEXT("Playback finished\r\n"));

	this->stop_thread(&this->p_userthread, 0u);
	this->filein_close();
	this->audio_hw_deinit();
	this->buffer_free();

	this->status = this->RUNTIME_STATUS_UNINITIALIZED;
	return TRUE;
}

__string WINAPI AudioBaseClass::getLastErrorMessage(VOID)
{
	switch(this->status)
	{
		case this->RUNTIME_STATUS_ERROR_MEMALLOC:
			return TEXT("Error: Memory Allocation Failed");

		case this->RUNTIME_STATUS_ERROR_NOFILE:
			return TEXT("Error: Could Not Open File");

		case this->RUNTIME_STATUS_ERROR_AUDIOHW:
			return TEXT("Error: Audio Hardware Init Failed\r\nExtended Message: ") + this->err_msg;

		case this->RUNTIME_STATUS_ERROR_GENERIC:
			return TEXT("Error: Something Went Wrong\r\nExtended Message: ") + this->err_msg;

		case this->RUNTIME_STATUS_UNINITIALIZED:
			return TEXT("Error: Audio Object Not Initialized");
	}

	return this->err_msg;
}

BOOL WINAPI AudioBaseClass::wait_thread(HANDLE *pp_thread)
{
	INT32 n_ret = 0;

	if(pp_thread == nullptr) return FALSE;
	if(*pp_thread == nullptr) return FALSE;

	n_ret = (INT32) WaitForSingleObject(*pp_thread, INFINITE);

	if(n_ret < 0) return FALSE;

	*pp_thread = nullptr;
	return TRUE;
}

BOOL WINAPI AudioBaseClass::stop_thread(HANDLE *pp_thread, DWORD exit_code)
{
	if(pp_thread == nullptr) return FALSE;
	if(*pp_thread == nullptr) return FALSE;
	if(!TerminateThread(*pp_thread, exit_code)) return FALSE;

	*pp_thread = nullptr;
	return TRUE;
}

BOOL WINAPI AudioBaseClass::filein_open(VOID)
{
	this->h_filein = CreateFile(this->filein_dir.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
	if(this->h_filein == INVALID_HANDLE_VALUE) return FALSE;

	*this->p_filein_size_l32 = (ULONG32) GetFileSize(this->h_filein, (DWORD*) this->p_filein_size_h32);
	return TRUE;
}

VOID WINAPI AudioBaseClass::filein_close(VOID)
{
	if(this->h_filein == INVALID_HANDLE_VALUE) return;

	CloseHandle(this->h_filein);
	this->h_filein = INVALID_HANDLE_VALUE;
	this->filein_size_64 = 0u;
	return;
}

VOID WINAPI AudioBaseClass::audio_hw_deinit(VOID)
{
	if(this->p_audiomgr != nullptr) this->p_audiomgr->Stop();

	if(this->p_audioout != nullptr)
	{
		this->p_audioout->Release();
		this->p_audioout = nullptr;
	}

	if(this->p_audiomgr != nullptr)
	{
		this->p_audiomgr->Release();
		this->p_audiomgr = nullptr;
	}

	if(this->p_audiodev != nullptr)
	{
		this->p_audiodev->Release();
		this->p_audiodev = nullptr;
	}

	return;
}

BOOL WINAPI AudioBaseClass::prompt_user_choose_device(IMMDeviceEnumerator *p_devenum, IMMDeviceCollection *p_devcoll)
{
	HRESULT n_ret = 0;
	UINT devcoll_count = 0u;
	INT device_index = 0;

	if(p_devenum == nullptr) return FALSE;
	if(p_devcoll == nullptr) return FALSE;

	n_ret = p_devcoll->GetCount(&devcoll_count);
	if(n_ret != S_OK) return FALSE;

	this->stop = FALSE;

	while(TRUE)
	{
		console_clear_text();
		console_stdout_write(TEXT("Choose Audio Playback Device\r\n\r\n"));
		console_stdout_write(TEXT("Enter \"default\" to use default playback device\r\n"));
		console_stdout_write(TEXT("Enter device index number to select a specific device\r\n"));
		console_stdout_write(TEXT("Enter \"quit\" to quit application\r\n"));
		this->print_audio_device_list(p_devcoll, devcoll_count);

		console_stdin_readcmd(textbuf, TEXTBUF_SIZE_CHARS);
		console_clear_text();

		cstr_tolower(textbuf, TEXTBUF_SIZE_CHARS);

		if(cstr_compare(TEXT("quit"), textbuf))
		{
			this->stop = TRUE;
			return FALSE;
		}

		if(cstr_compare(TEXT("default"), textbuf))
		{
			n_ret = p_devenum->GetDefaultAudioEndpoint(eRender, eConsole, &this->p_audiodev);
			break;
		}

		try
		{
			device_index = std::stoi(textbuf);
		}
		catch(...)
		{
			console_wait_keypress(TEXT("Error: invalid command entered...\r\n"));
			continue;
		}

		if(device_index < 0)
		{
			console_wait_keypress(TEXT("Error: invalid value entered...\r\n"));
			continue;
		}

		if(((UINT) device_index) >= devcoll_count)
		{
			console_wait_keypress(TEXT("Error: invalid value entered...\r\n"));
			continue;
		}

		n_ret = p_devcoll->Item((UINT) device_index, &this->p_audiodev);
		break;
	}

	return (n_ret == S_OK);
}

BOOL WINAPI AudioBaseClass::print_audio_device_list(IMMDeviceCollection *p_devcoll, UINT devcoll_count)
{
	const PROPERTYKEY *p_pkey = (const PROPERTYKEY*) P_PKEY_Device_FriendlyName;
	IMMDevice *p_dev = nullptr;
	IPropertyStore *p_devprop = nullptr;
	HRESULT n_ret = 0;
	UINT n_dev = 0u;
	PROPVARIANT propvar;

	if(p_devcoll == nullptr) return FALSE;
	if(devcoll_count < 1u) return FALSE;

	console_stdout_write(TEXT("Device List:\r\n\r\n"));

	for(n_dev = 0u; n_dev < devcoll_count; n_dev++)
	{
		n_ret = p_devcoll->Item(n_dev, &p_dev);
		if(n_ret != S_OK) return FALSE;

		__SPRINTF(textbuf, TEXTBUF_SIZE_CHARS, TEXT("Device Index %u: "), n_dev);
		console_stdout_write(textbuf);

		n_ret = p_dev->OpenPropertyStore(STGM_READ, &p_devprop);
		if(n_ret != S_OK)
		{
			p_dev->Release();
			return FALSE;
		}

		PropVariantInit(&propvar);

		n_ret = p_devprop->GetValue(*p_pkey, &propvar);
		if(n_ret != S_OK)
		{
			p_devprop->Release();
			p_dev->Release();
			return FALSE;
		}

		if(propvar.vt != VT_EMPTY) cstr_wchar_to_tchar(propvar.pwszVal, textbuf, TEXTBUF_SIZE_CHARS);
		else __SPRINTF(textbuf, TEXTBUF_SIZE_CHARS, TEXT("No Description Found"));

		console_stdout_write(textbuf);
		console_stdout_write(TEXT("\r\n"));

		PropVariantClear(&propvar);

		p_devprop->Release();
		p_devprop = nullptr;
		p_dev->Release();
		p_dev = nullptr;
	}

	console_stdout_write(TEXT("\r\n\r\n"));

	PropVariantClear(&propvar);
	if(p_devprop != nullptr) p_devprop->Release();
	if(p_dev != nullptr) p_dev->Release();
	return TRUE;
}

VOID WINAPI AudioBaseClass::playback_proc(VOID)
{
	this->playback_init();
	this->playback_loop();
	return;
}

VOID WINAPI AudioBaseClass::playback_init(VOID)
{
	HRESULT n_ret = 0;

	this->p_userthread = CreateThread(NULL, 0u, (LPTHREAD_START_ROUTINE) &AudioBaseClass::userthread_proc, this, 0u, NULL);

	this->filein_pos_64 = this->audio_data_begin;
	this->stop = FALSE;

	n_ret = this->p_audioout->GetBuffer((UINT32) this->AUDIOBUFFER_SIZE_FRAMES, (BYTE**) &this->p_audiobuffer);
	if(n_ret != S_OK) app_exit(1u, TEXT("IAudioRenderClient->GetBuffer() failed"), TRUE);

	ZeroMemory(this->p_audiobuffer, this->AUDIOBUFFER_SIZE_BYTES);

	n_ret = this->p_audioout->ReleaseBuffer((UINT32) this->AUDIOBUFFER_SIZE_FRAMES, 0u);
	if(n_ret != S_OK) app_exit(1u, TEXT("IAudioRenderClient->ReleaseBuffer() failed"), TRUE);

	n_ret = this->p_audiomgr->Start();
	if(n_ret != S_OK) app_exit(1u, TEXT("IAudioClient->Start() failed"), TRUE);

	this->audio_hw_wait();
	this->buffer_load();

	return;
}

VOID WINAPI AudioBaseClass::playback_loop(VOID)
{
	HRESULT n_ret = 0;

	while(!this->stop)
	{
		this->buffer_play();
		this->audio_hw_wait();
		this->buffer_load();
	}

	n_ret = this->p_audiomgr->Stop();
	if(n_ret != S_OK) app_exit(1u, TEXT("IAudioClient->Stop() failed"), TRUE);

	return;
}

VOID WINAPI AudioBaseClass::buffer_play(VOID)
{
	HRESULT n_ret = 0;

	n_ret = this->p_audioout->ReleaseBuffer((UINT32) this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES, 0u);
	if(n_ret != S_OK) app_exit(1u, TEXT("IAudioRenderClient->ReleaseBuffer failed"), TRUE);

	return;
}

VOID WINAPI AudioBaseClass::audio_hw_wait(VOID)
{
	SIZE_T n_frames_avail = 0u;
	HRESULT n_ret = 0;
	UINT32 u32 = 0u;

	do{
		n_ret = this->p_audiomgr->GetCurrentPadding(&u32);
		if(n_ret != S_OK) app_exit(1u, TEXT("IAudioClient->GetCurrentPadding() failed"), TRUE);
		n_frames_avail = this->AUDIOBUFFER_SIZE_FRAMES - ((SIZE_T) u32);
		Sleep(1u);
	}while(n_frames_avail < this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES);

	return;
}

DWORD WINAPI AudioBaseClass::userthread_proc(VOID *args)
{
	console_stdout_write(TEXT("Enter \"stop\" to stop playback and quit application\r\n"));

	while(TRUE)
	{
		console_stdin_readcmd(textbuf, TEXTBUF_SIZE_CHARS);
		this->cmd_decode();
	}

	return 0u;
}

VOID WINAPI AudioBaseClass::cmd_decode(VOID)
{
	cstr_tolower(textbuf, TEXTBUF_SIZE_CHARS);

	if(cstr_compare(TEXT("stop"), textbuf)) this->stop = TRUE;
	else console_stdout_write(TEXT("Error: invalid command entered\r\n"));

	return;
}
