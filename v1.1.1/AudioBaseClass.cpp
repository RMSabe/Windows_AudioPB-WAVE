/*
	WAVE Audio Playback for Windows
	Version 1.1.1

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "AudioBaseClass.hpp"
#include "cstrdef.h"
#include <combaseapi.h>

AudioBaseClass::AudioBaseClass(const audiopb_params_t *p_params)
{
	this->setParameters(p_params);
}

BOOL WINAPI AudioBaseClass::setParameters(const audiopb_params_t *p_params)
{
	if(p_params == NULL) return FALSE;
	if(p_params->p_audiodevlistbox == NULL) return FALSE;
	if(p_params->file_dir == NULL) return FALSE;

	this->p_audiodevlistbox = p_params->p_audiodevlistbox;
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
		this->status = this->STATUS_ERROR_NOFILE;
		this->err_msg = TEXT("INITIALIZE: Error: could not open file.");
		return FALSE;
	}

	if(!this->audio_hw_init())
	{
		this->status = this->STATUS_ERROR_AUDIOHW;
		this->filein_close();
		return FALSE;
	}

	if(!this->buffer_alloc())
	{
		this->status = this->STATUS_ERROR_MEMALLOC;
		this->err_msg = TEXT("INITIALIZE: Error: memory allocate failed.");
		this->filein_close();
		this->audio_hw_deinit_all();
		return FALSE;
	}

	this->status = this->STATUS_READY;
	return TRUE;
}

BOOL WINAPI AudioBaseClass::runPlayback(VOID)
{
	if(this->status < 1)
	{
		this->err_msg = TEXT("RUN PLAYBACK: Error: audio object has not been initialized.");
		return FALSE;
	}

	this->status = this->STATUS_PLAYING;
	this->playback_proc();

	this->filein_close();
	this->audio_hw_deinit_all();
	this->buffer_free();

	this->status = this->STATUS_UNINITIALIZED;
	return TRUE;
}

VOID WINAPI AudioBaseClass::stopPlayback(VOID)
{
	this->stop = TRUE;
	return;
}

BOOL WINAPI AudioBaseClass::loadAudioDeviceList(VOID)
{
	const PROPERTYKEY *p_pkey = (const PROPERTYKEY*) P_PKEY_Device_FriendlyName;
	IMMDevice *p_dev = NULL;
	IPropertyStore *p_devprop = NULL;
	HRESULT n_ret = 0;
	UINT devcoll_count = 0u;
	UINT n_dev = 0u;
	PROPVARIANT propvar;

	if(this->p_audiodevlistbox == NULL)
	{
		this->err_msg = TEXT("LOAD AUDIO DEVICE LIST: Error: List Box Object is invalid.");
		return FALSE;
	}

	listbox_clear(this->p_audiodevlistbox);

	this->audio_hw_deinit_all();

	n_ret = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (VOID**) &this->p_audiodevenum);
	if(n_ret != S_OK)
	{
		this->err_msg = TEXT("LOAD AUDIO DEVICE LIST: Error: audio device enumerator instance failed.");
		return FALSE;
	}

	n_ret = this->p_audiodevenum->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &this->p_audiodevcoll);
	if(n_ret != S_OK)
	{
		this->audio_hw_deinit_all();
		this->err_msg = TEXT("LOAD AUDIO DEVICE LIST: Error: audio device collection instance failed.");
		return FALSE;
	}

	n_ret = this->p_audiodevcoll->GetCount(&devcoll_count);
	if(n_ret != S_OK)
	{
		this->audio_hw_deinit_all();
		this->err_msg = TEXT("LOAD AUDIO DEVICE LIST: Error: audio device collection get count failed.");
		return FALSE;
	}

	for(n_dev = 0u; n_dev < devcoll_count; n_dev++)
	{
		n_ret = this->p_audiodevcoll->Item(n_dev, &p_dev);
		if(n_ret != S_OK)
		{
			this->audio_hw_deinit_all();
			this->err_msg = TEXT("LOAD AUDIO DEVICE LIST: Error: load audio device handle failed.");
			return FALSE;
		}

		n_ret = p_dev->OpenPropertyStore(STGM_READ, &p_devprop);
		if(n_ret != S_OK)
		{
			p_dev->Release();
			this->audio_hw_deinit_all();
			this->err_msg = TEXT("LOAD AUDIO DEVICE LIST: Error: load audio device properties failed.");
			return FALSE;
		}

		PropVariantInit(&propvar);

		n_ret = p_devprop->GetValue(*p_pkey, &propvar);
		if(n_ret != S_OK)
		{
			p_devprop->Release();
			p_dev->Release();
			this->audio_hw_deinit_all();
			this->err_msg = TEXT("LOAD AUDIO DEVICE LIST: Error: get audio device property value failed.");
			return FALSE;
		}

		if(propvar.vt == VT_EMPTY) __SPRINTF(textbuf, TEXTBUF_SIZE_CHARS, TEXT("Unknown Audio Device"));
		else cstr_wchar_to_tchar(propvar.pwszVal, textbuf, TEXTBUF_SIZE_CHARS);

		listbox_add_item(this->p_audiodevlistbox, textbuf);

		PropVariantClear(&propvar);

		p_devprop->Release();
		p_devprop = NULL;
		p_dev->Release();
		p_dev = NULL;
	}

	PropVariantClear(&propvar);

	if(p_devprop != NULL) p_devprop->Release();
	if(p_dev != NULL) p_dev->Release();

	return TRUE;
}

BOOL WINAPI AudioBaseClass::chooseDevice(SIZE_T index)
{
	HRESULT n_ret = 0;

	if(this->p_audiodevcoll == NULL)
	{
		this->err_msg = TEXT("CHOOSE DEVICE: Error: device collection invalid.");
		return FALSE;
	}

	if(this->p_audiodev != NULL) this->p_audiodev->Release();

	n_ret = this->p_audiodevcoll->Item((UINT) index, &this->p_audiodev);
	if(n_ret != S_OK)
	{
		this->err_msg = TEXT("CHOOSE DEVICE: Error: could not retrieve device handle.");
		return FALSE;
	}

	return TRUE;
}

BOOL WINAPI AudioBaseClass::chooseDefaultDevice(VOID)
{
	HRESULT n_ret = 0;

	if(this->p_audiodevenum == NULL)
	{
		this->err_msg = TEXT("CHOOSE DEFAULT DEVICE: Error: device enumerator invalid.");
		return FALSE;
	}

	if(this->p_audiodev != NULL) this->p_audiodev->Release();

	n_ret = this->p_audiodevenum->GetDefaultAudioEndpoint(eRender, eMultimedia, &this->p_audiodev);
	if(n_ret != S_OK)
	{
		this->err_msg = TEXT("CHOOSE DEFAULT DEVICE: Error: could not retrieve device handle.");
		return FALSE;
	}

	return TRUE;
}

__string WINAPI AudioBaseClass::getLastErrorMessage(VOID)
{
	return this->err_msg;
}

BOOL WINAPI AudioBaseClass::filein_open(VOID)
{
	if(this->h_filein != INVALID_HANDLE_VALUE) CloseHandle(this->h_filein);

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

VOID WINAPI AudioBaseClass::audio_hw_deinit_device(VOID)
{
	if(this->p_audiomgr != NULL) this->p_audiomgr->Stop();

	if(this->p_audioout != NULL)
	{
		this->p_audioout->Release();
		this->p_audioout = NULL;
	}

	if(this->p_audiomgr != NULL)
	{
		this->p_audiomgr->Release();
		this->p_audiomgr = NULL;
	}

	if(this->p_audiodev != NULL)
	{
		this->p_audiodev->Release();
		this->p_audiodev = NULL;
	}

	return;
}

VOID WINAPI AudioBaseClass::audio_hw_deinit_all(VOID)
{
	this->audio_hw_deinit_device();

	if(this->p_audiodevcoll != NULL)
	{
		this->p_audiodevcoll->Release();
		this->p_audiodevcoll = NULL;
	}

	if(this->p_audiodevenum != NULL)
	{
		this->p_audiodevenum->Release();
		this->p_audiodevenum = NULL;
	}

	return;
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

	this->filein_pos_64 = this->audio_data_begin;
	this->stop = FALSE;

	n_ret = this->p_audioout->GetBuffer((UINT32) this->AUDIOBUFFER_SIZE_FRAMES, (BYTE**) &this->p_audiobuffer);
	if(n_ret != S_OK) app_exit(1u, TEXT("PLAYBACK INIT: Error: AudioRenderClient::GetBuffer failed."));

	ZeroMemory(this->p_audiobuffer, this->AUDIOBUFFER_SIZE_BYTES);

	n_ret = this->p_audioout->ReleaseBuffer((UINT32) this->AUDIOBUFFER_SIZE_FRAMES, 0u);
	if(n_ret != S_OK) app_exit(1u, TEXT("PLAYBACK INIT: Error: AudioRenderClient::ReleaseBuffer failed."));

	n_ret = this->p_audiomgr->Start();
	if(n_ret != S_OK) app_exit(1u, TEXT("PLAYBACK INIT: Error: AudioClient::Start failed."));

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
	if(n_ret != S_OK) app_exit(1u, TEXT("PLAYBACK LOOP: Error: AudioClient::Stop failed."));

	return;
}

VOID WINAPI AudioBaseClass::buffer_play(VOID)
{
	HRESULT n_ret = 0;

	n_ret = this->p_audioout->ReleaseBuffer((UINT32) this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES, 0u);
	if(n_ret != S_OK) app_exit(1u, TEXT("BUFFER PLAY: Error: AudioRenderClient::ReleaseBuffer failed."));

	return;
}

VOID WINAPI AudioBaseClass::audio_hw_wait(VOID)
{
	SIZE_T n_frames_free = 0u;
	HRESULT n_ret = 0;
	UINT32 u32 = 0u;

	do{
		n_ret = this->p_audiomgr->GetCurrentPadding(&u32);
		if(n_ret != S_OK) app_exit(1u, TEXT("AUDIO HW WAIT: Error: AudioClient::GetCurrentPadding failed."));
		n_frames_free = this->AUDIOBUFFER_SIZE_FRAMES - ((SIZE_T) u32);
		Sleep(1u);
	}while(n_frames_free < this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES);

	return;
}
