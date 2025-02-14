/*
	Audio Playback application for Windows
	Version 1.0

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "AudioPB_i16_1ch.hpp"

#include <combaseapi.h>

AudioPB_i16_1ch::AudioPB_i16_1ch(const audiopb_params_t *p_params) : AudioBaseClass(p_params)
{
}

AudioPB_i16_1ch::~AudioPB_i16_1ch(VOID)
{
	this->stop_thread(&this->p_userthread, 0u);
	this->filein_close();
	this->audio_hw_deinit();
	this->buffer_free();
}

BOOL WINAPI AudioPB_i16_1ch::audio_hw_init(VOID)
{
	IMMDeviceEnumerator *p_devenum = nullptr;
	IMMDeviceCollection *p_devcoll = nullptr;
	HRESULT n_ret = 0;
	UINT32 u32 = 0u;
	WAVEFORMATEX wavfmt;

	n_ret = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (VOID**) &p_devenum);
	if(n_ret != S_OK)
	{
		this->err_msg = TEXT("Device Enumerator Instance Failed");
		return FALSE;
	}

	n_ret = p_devenum->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &p_devcoll);
	if(n_ret != S_OK)
	{
		this->err_msg = TEXT("Device Collection Instance Failed");
		p_devenum->Release();
		return FALSE;
	}

	if(!this->prompt_user_choose_device(p_devenum, p_devcoll))
	{
		if(this->stop) this->err_msg = TEXT("User Requested Stop");
		else this->err_msg = TEXT("Open Playback Device Failed");

		p_devenum->Release();
		p_devcoll->Release();

		return FALSE;
	}

	p_devenum->Release();
	p_devenum = nullptr;
	p_devcoll->Release();
	p_devcoll = nullptr;

	n_ret = this->p_audiodev->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (VOID**) &this->p_audiomgr);
	if(n_ret != S_OK)
	{
		this->err_msg = TEXT("Activate Playback Device Failed");
		return FALSE;
	}

	wavfmt.wFormatTag = WAVE_FORMAT_PCM;
	wavfmt.nChannels = 2u;
	wavfmt.nSamplesPerSec = this->sample_rate;
	wavfmt.nAvgBytesPerSec = 4u*this->sample_rate;
	wavfmt.nBlockAlign = 4u;
	wavfmt.wBitsPerSample = 16u;
	wavfmt.cbSize = 0u;

	n_ret = this->p_audiomgr->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, &wavfmt, NULL);
	if(n_ret != S_OK)
	{
		this->err_msg = TEXT("Audio Device Does Not Support Specified Format");
		return FALSE;
	}

	n_ret = this->p_audiomgr->Initialize(AUDCLNT_SHAREMODE_EXCLUSIVE, 0, 10000000, 0, &wavfmt, NULL);
	if(n_ret != S_OK)
	{
		this->err_msg = TEXT("Initialize Playback Engine Failed");
		return FALSE;
	}

	n_ret = this->p_audiomgr->GetBufferSize(&u32);
	if(n_ret != S_OK)
	{
		this->err_msg = TEXT("Get Playback Buffer Size Failed");
		return FALSE;
	}

	n_ret = this->p_audiomgr->GetService(__uuidof(IAudioRenderClient), (VOID**) &this->p_audioout);
	if(n_ret != S_OK)
	{
		this->err_msg = TEXT("Get Playback Device Handle Failed");
		return FALSE;
	}

	this->AUDIOBUFFER_SIZE_FRAMES = (SIZE_T) u32;
	this->AUDIOBUFFER_SIZE_SAMPLES = this->AUDIOBUFFER_SIZE_FRAMES*2u;
	this->AUDIOBUFFER_SIZE_BYTES = this->AUDIOBUFFER_SIZE_SAMPLES*2u;

	this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES = this->AUDIOBUFFER_SIZE_FRAMES/this->AUDIOBUFFER_N_SEGMENTS;
	this->AUDIOBUFFER_SEGMENT_SIZE_SAMPLES = this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES*2u;
	this->AUDIOBUFFER_SEGMENT_SIZE_BYTES = this->AUDIOBUFFER_SEGMENT_SIZE_SAMPLES*2u;

	this->BUFFERIN_SIZE_FRAMES = this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES;
	this->BUFFERIN_SIZE_SAMPLES = this->BUFFERIN_SIZE_FRAMES;
	this->BUFFERIN_SIZE_BYTES = this->BUFFERIN_SIZE_SAMPLES*2u;

	return TRUE;
}

BOOL WINAPI AudioPB_i16_1ch::buffer_alloc(VOID)
{
	this->p_bufferin = (INT16*) HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, this->BUFFERIN_SIZE_BYTES);

	return (this->p_bufferin != nullptr);
}

VOID WINAPI AudioPB_i16_1ch::buffer_free(VOID)
{
	if(this->p_bufferin != nullptr)
	{
		HeapFree(p_processheap, 0u, this->p_bufferin);
		this->p_bufferin = nullptr;
	}

	return;
}

VOID WINAPI AudioPB_i16_1ch::buffer_load(VOID)
{
	SIZE_T n_frame = 0u;
	HRESULT n_ret = 0;
	DWORD dummy_32;

	if(this->filein_pos_64 >= this->audio_data_end)
	{
		this->stop = TRUE;
		return;
	}

	n_ret = this->p_audioout->GetBuffer((UINT32) this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES, (BYTE**) &this->p_audiobuffer);
	if(n_ret != S_OK) app_exit(1u, TEXT("IAudioRenderClient->GetBuffer failed"), TRUE);

	ZeroMemory(this->p_bufferin, this->BUFFERIN_SIZE_BYTES);

	SetFilePointer(this->h_filein, (LONG) *this->p_filein_pos_l32, (LONG*) this->p_filein_pos_h32, FILE_BEGIN);
	ReadFile(this->h_filein, this->p_bufferin, (DWORD) this->BUFFERIN_SIZE_BYTES, &dummy_32, NULL);
	this->filein_pos_64 += (ULONG64) this->BUFFERIN_SIZE_BYTES;

	for(n_frame = 0u; n_frame < this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES; n_frame++)
	{
		((INT16*) this->p_audiobuffer)[2u*n_frame] = this->p_bufferin[n_frame];
		((INT16*) this->p_audiobuffer)[2u*n_frame + 1u] = this->p_bufferin[n_frame];
	}

	return;
}
