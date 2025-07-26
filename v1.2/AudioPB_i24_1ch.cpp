/*
	WAVE Audio Playback for Windows
	Version 1.2

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "AudioPB_i24_1ch.hpp"

AudioPB_i24_1ch::AudioPB_i24_1ch(const audiopb_params_t *p_params) : AudioPB(p_params)
{
}

AudioPB_i24_1ch::~AudioPB_i24_1ch(VOID)
{
	this->stopPlayback();
	this->status = this->STATUS_UNINITIALIZED;

	this->filein_close();
	this->audio_hw_deinit_all();
	this->buffer_free();
}

BOOL WINAPI AudioPB_i24_1ch::audio_hw_init(VOID)
{
	HRESULT n_ret;
	UINT32 u32;
	WAVEFORMATEX wavfmt;

	if(this->p_audiodev == NULL)
	{
		this->err_msg = TEXT("AudioPB_i24_1ch::audio_hw_init: Error: p_audiodev is NULL.");
		return FALSE;
	}

	n_ret = this->p_audiodev->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (VOID**) &(this->p_audiomgr));
	if(n_ret != S_OK)
	{
		this->audio_hw_deinit_device();
		this->err_msg = TEXT("AudioPB_i24_1ch::audio_hw_init: Error: IMMDevice::Activate failed.");
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
		this->err_msg = TEXT("AudioPB_i24_1ch::audio_hw_init: Error: audio stream format not supported.");
		return FALSE;
	}

	n_ret = this->p_audiomgr->Initialize(AUDCLNT_SHAREMODE_EXCLUSIVE, 0, 10000000, 0, &wavfmt, NULL);
	if(n_ret != S_OK)
	{
		this->audio_hw_deinit_device();
		this->err_msg = TEXT("AudioPB_i24_1ch::audio_hw_init: Error: IAudioClient::Initialize failed.");
		return FALSE;
	}

	n_ret = this->p_audiomgr->GetBufferSize(&u32);
	if(n_ret != S_OK)
	{
		this->audio_hw_deinit_device();
		this->err_msg = TEXT("AudioPB_i24_1ch::audio_hw_init: Error: IAudioClient::GetBufferSize failed.");
		return FALSE;
	}

	n_ret = this->p_audiomgr->GetService(__uuidof(IAudioRenderClient), (VOID**) &(this->p_audioout));
	if(n_ret != S_OK)
	{
		this->audio_hw_deinit_device();
		this->err_msg = TEXT("AudioPB_i24_1ch::audio_hw_init: Error: IAudioClient::GetService failed.");
		return FALSE;
	}

	this->AUDIOBUFFER_SIZE_FRAMES = (SIZE_T) u32;
	this->AUDIOBUFFER_SIZE_SAMPLES = this->AUDIOBUFFER_SIZE_FRAMES*2u;
	this->AUDIOBUFFER_SIZE_BYTES = this->AUDIOBUFFER_SIZE_SAMPLES*2u;

	this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES = _get_closest_power2_ceil(this->AUDIOBUFFER_SIZE_FRAMES/2u);
	this->AUDIOBUFFER_SEGMENT_SIZE_SAMPLES = this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES*2u;
	this->AUDIOBUFFER_SEGMENT_SIZE_BYTES = this->AUDIOBUFFER_SEGMENT_SIZE_SAMPLES*2u;

	this->BYTEBUF_SIZE = this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES*3u;

	return TRUE;
}

BOOL WINAPI AudioPB_i24_1ch::buffer_alloc(VOID)
{
	this->buffer_free();

	this->p_bytebuf = (UINT8*) HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, this->BYTEBUF_SIZE);

	if(this->p_bytebuf == NULL)
	{
		this->buffer_free();
		return FALSE;
	}

	return TRUE;
}

VOID WINAPI AudioPB_i24_1ch::buffer_free(VOID)
{
	if(this->p_bytebuf != NULL)
	{
		HeapFree(p_processheap, 0u, this->p_bytebuf);
		this->p_bytebuf = NULL;
	}

	return;
}

VOID WINAPI AudioPB_i24_1ch::buffer_load(VOID)
{
	SIZE_T n_sample = 0u;
	SIZE_T n_byte = 0u;

	HRESULT n_ret = 0;
	DWORD dummy_32;

	if((*((ULONG64*) &(this->filein_pos_64))) >= this->AUDIO_DATA_END)
	{
		this->stop_playback = TRUE;
		return;
	}

	n_ret = this->p_audioout->GetBuffer((UINT32) this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES, (BYTE**) &(this->p_audiobuffer));
	if(n_ret != S_OK) app_exit(1u, TEXT("AudioPB_i24_1ch::buffer_load: Error: IAudioRenderClient::GetBuffer failed."));

	ZeroMemory(this->p_bytebuf, this->BYTEBUF_SIZE);

	SetFilePointer(this->h_filein, (LONG) this->filein_pos_64.l32, (LONG*) &(this->filein_pos_64.h32), FILE_BEGIN);
	ReadFile(this->h_filein, this->p_bytebuf, (DWORD) this->BYTEBUF_SIZE, &dummy_32, NULL);
	*((ULONG64*) &(this->filein_pos_64)) += (ULONG64) this->BYTEBUF_SIZE;

	n_byte = 1u;
	for(n_sample = 0u; n_sample < this->AUDIOBUFFER_SEGMENT_SIZE_SAMPLES; n_sample += 2u)
	{
		((INT16*) this->p_audiobuffer)[n_sample] = *((INT16*) (this->p_bytebuf + n_byte));
		((INT16*) this->p_audiobuffer)[n_sample + 1u] = *((INT16*) (this->p_bytebuf + n_byte));

		n_byte += 3u;
	}

	return;
}
