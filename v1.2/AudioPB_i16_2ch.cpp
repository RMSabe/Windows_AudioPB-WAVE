/*
	WAVE Audio Playback for Windows
	Version 1.2

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "AudioPB_i16_2ch.hpp"

AudioPB_i16_2ch::AudioPB_i16_2ch(const audiopb_params_t *p_params) : AudioPB(p_params)
{
}

AudioPB_i16_2ch::~AudioPB_i16_2ch(VOID)
{
	this->stopPlayback();
	this->status = this->STATUS_UNINITIALIZED;

	this->filein_close();
	this->audio_hw_deinit_all();
	this->buffer_free();
}

BOOL WINAPI AudioPB_i16_2ch::audio_hw_init(VOID)
{
	HRESULT n_ret;
	UINT32 u32;
	WAVEFORMATEX wavfmt;

	if(this->p_audiodev == NULL)
	{
		this->err_msg = TEXT("AudioPB_i16_2ch::audio_hw_init: Error: p_audiodev is NULL.");
		return FALSE;
	}

	n_ret = this->p_audiodev->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (VOID**) &(this->p_audiomgr));
	if(n_ret != S_OK)
	{
		this->audio_hw_deinit_device();
		this->err_msg = TEXT("AudioPB_i16_2ch::audio_hw_init: Error: IMMDevice::Activate failed.");
		return FALSE;
	}

	wavfmt.wFormatTag = WAVE_FORMAT_PCM;
	wavfmt.nChannels = 2u;
	wavfmt.nSamplesPerSec = this->SAMPLE_RATE;
	wavfmt.nAvgBytesPerSec = this->SAMPLE_RATE*4u;
	wavfmt.nBlockAlign = 4u;
	wavfmt.wBitsPerSample = 16u;
	wavfmt.cbSize = 0u;

	n_ret = this->p_audiomgr->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, &wavfmt, NULL);
	if(n_ret != S_OK)
	{
		this->audio_hw_deinit_device();
		this->err_msg = TEXT("AudioPB_i16_2ch::audio_hw_init: Error: audio stream format not supported.");
		return FALSE;
	}

	n_ret = this->p_audiomgr->Initialize(AUDCLNT_SHAREMODE_EXCLUSIVE, 0, 10000000, 0, &wavfmt, NULL);
	if(n_ret != S_OK)
	{
		this->audio_hw_deinit_device();
		this->err_msg = TEXT("AudioPB_i16_2ch::audio_hw_init: Error: IAudioClient::Initialize failed.");
		return FALSE;
	}

	n_ret = this->p_audiomgr->GetBufferSize(&u32);
	if(n_ret != S_OK)
	{
		this->audio_hw_deinit_device();
		this->err_msg = TEXT("AudioPB_i16_2ch::audio_hw_init: Error: IAudioClient::GetBufferSize failed.");
		return FALSE;
	}

	n_ret = this->p_audiomgr->GetService(__uuidof(IAudioRenderClient), (VOID**) &(this->p_audioout));
	if(n_ret != S_OK)
	{
		this->audio_hw_deinit_device();
		this->err_msg = TEXT("AudioPB_i16_2ch::audio_hw_init: Error: IAudioClient::GetService failed.");
		return FALSE;
	}

	this->AUDIOBUFFER_SIZE_FRAMES = (SIZE_T) u32;
	this->AUDIOBUFFER_SIZE_SAMPLES = this->AUDIOBUFFER_SIZE_FRAMES*2u;
	this->AUDIOBUFFER_SIZE_BYTES = this->AUDIOBUFFER_SIZE_SAMPLES*2u;

	this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES = _get_closest_power2_ceil(this->AUDIOBUFFER_SIZE_FRAMES/2u);
	this->AUDIOBUFFER_SEGMENT_SIZE_SAMPLES = this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES*2u;
	this->AUDIOBUFFER_SEGMENT_SIZE_BYTES = this->AUDIOBUFFER_SEGMENT_SIZE_SAMPLES*2u;

	return TRUE;
}

BOOL WINAPI AudioPB_i16_2ch::buffer_alloc(VOID)
{
	/*Unused*/

	return TRUE;
}

VOID WINAPI AudioPB_i16_2ch::buffer_free(VOID)
{
	/*Unused*/

	return;
}

VOID WINAPI AudioPB_i16_2ch::buffer_load(VOID)
{
	HRESULT n_ret = 0;
	DWORD dummy_32;

	if((*((ULONG64*) &(this->filein_pos_64))) >= this->AUDIO_DATA_END)
	{
		this->stop_playback = TRUE;
		return;
	}

	n_ret = this->p_audioout->GetBuffer((UINT32) this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES, (BYTE**) &(this->p_audiobuffer));
	if(n_ret != S_OK) app_exit(1u, TEXT("AudioPB_i16_2ch::buffer_load: Error: IAudioRenderClient::GetBuffer failed."));

	ZeroMemory(this->p_audiobuffer, this->AUDIOBUFFER_SEGMENT_SIZE_BYTES);

	SetFilePointer(this->h_filein, (LONG) this->filein_pos_64.l32, (LONG*) &(this->filein_pos_64.h32), FILE_BEGIN);
	ReadFile(this->h_filein, this->p_audiobuffer, (DWORD) this->AUDIOBUFFER_SEGMENT_SIZE_BYTES, &dummy_32, NULL);
	*((ULONG64*) &(this->filein_pos_64)) += (ULONG64) this->AUDIOBUFFER_SEGMENT_SIZE_BYTES;

	return;
}
