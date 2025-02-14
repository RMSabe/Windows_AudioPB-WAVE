/*
	Audio Playback application for Windows
	Version 1.0

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "AudioPB_i24_2ch.hpp"

#include <combaseapi.h>

AudioPB_i24_2ch::AudioPB_i24_2ch(const audiopb_params_t *p_params) : AudioBaseClass(p_params)
{
}

AudioPB_i24_2ch::~AudioPB_i24_2ch(VOID)
{
	this->stop_thread(&this->p_userthread, 0u);
	this->filein_close();
	this->audio_hw_deinit();
	this->buffer_free();
}

BOOL WINAPI AudioPB_i24_2ch::audio_hw_init(VOID)
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

	this->BYTEBUF_SIZE = this->AUDIOBUFFER_SEGMENT_SIZE_SAMPLES*3u;

	return TRUE;
}

BOOL WINAPI AudioPB_i24_2ch::buffer_alloc(VOID)
{
	this->p_bytebuf = (UINT8*) HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, this->BYTEBUF_SIZE);

	return (this->p_bytebuf != nullptr);
}

VOID WINAPI AudioPB_i24_2ch::buffer_free(VOID)
{
	if(this->p_bytebuf != nullptr)
	{
		HeapFree(p_processheap, 0u, this->p_bytebuf);
		this->p_bytebuf = nullptr;
	}

	return;
}

VOID WINAPI AudioPB_i24_2ch::buffer_load(VOID)
{
	SIZE_T n_sample = 0u;
	SIZE_T n_byte = 0u;
	HRESULT n_ret = 0;
	DWORD dummy_32;

	if(this->filein_pos_64 >= this->audio_data_end)
	{
		this->stop = TRUE;
		return;
	}

	n_ret = this->p_audioout->GetBuffer((UINT32) this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES, (BYTE**) &this->p_audiobuffer);
	if(n_ret != S_OK) app_exit(1u, TEXT("IAudioRenderClient->GetBuffer failed"), TRUE);

	ZeroMemory(this->p_bytebuf, this->BYTEBUF_SIZE);

	SetFilePointer(this->h_filein, (LONG) *this->p_filein_pos_l32, (LONG*) this->p_filein_pos_h32, FILE_BEGIN);
	ReadFile(this->h_filein, this->p_bytebuf, (DWORD) this->BYTEBUF_SIZE, &dummy_32, NULL);
	this->filein_pos_64 += (ULONG64) this->BYTEBUF_SIZE;

	n_byte = 1u;
	for(n_sample = 0u; n_sample < this->AUDIOBUFFER_SEGMENT_SIZE_SAMPLES; n_sample++)
	{
		((INT16*) this->p_audiobuffer)[n_sample] = *((INT16*) &this->p_bytebuf[n_byte]);
		n_byte += 3u;
	}

	return;
}
