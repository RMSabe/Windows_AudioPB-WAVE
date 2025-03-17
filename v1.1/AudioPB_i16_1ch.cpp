/*
	WAVE Audio Playback for Windows
	Version 1.1

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "AudioPB_i16_1ch.hpp"

AudioPB_i16_1ch::AudioPB_i16_1ch(const audiopb_params_t *p_params) : AudioBaseClass(p_params)
{
}

AudioPB_i16_1ch::~AudioPB_i16_1ch(VOID)
{
	this->stopPlayback();
	this->filein_close();
	this->audio_hw_deinit();
	this->buffer_free();
}

BOOL WINAPI AudioPB_i16_1ch::audio_hw_init(VOID)
{
	HRESULT n_ret;
	UINT32 u32;
	WAVEFORMATEX wavfmt;

	if(this->p_audiodev == nullptr)
	{
		this->err_msg = TEXT("AUDIO HW INIT: Error: audio device handle invalid.");
		return FALSE;
	}

	n_ret = this->p_audiodev->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (VOID**) &this->p_audiomgr);
	if(n_ret != S_OK)
	{
		this->audio_hw_deinit();
		this->err_msg = TEXT("AUDIO HW INIT: Error: audio device activation failed.");
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
		this->audio_hw_deinit();
		this->err_msg = TEXT("AUDIO HW INIT: Error: audio format not supported.");
		return FALSE;
	}

	n_ret = this->p_audiomgr->Initialize(AUDCLNT_SHAREMODE_EXCLUSIVE, 0, 10000000, 0, &wavfmt, NULL);
	if(n_ret != S_OK)
	{
		this->audio_hw_deinit();
		this->err_msg = TEXT("AUDIO HW INIT: Error: audio engine init failed.");
		return FALSE;
	}

	n_ret = this->p_audiomgr->GetBufferSize(&u32);
	if(n_ret != S_OK)
	{
		this->audio_hw_deinit();
		this->err_msg = TEXT("AUDIO HW INIT: Error: get audio buffer size failed.");
		return FALSE;
	}

	n_ret = this->p_audiomgr->GetService(__uuidof(IAudioRenderClient), (VOID**) &this->p_audioout);
	if(n_ret != S_OK)
	{
		this->audio_hw_deinit();
		this->err_msg = TEXT("AUDIO HW INIT: Error: get audio device handle failed.");
		return FALSE;
	}

	this->AUDIOBUFFER_SIZE_FRAMES = (SIZE_T) u32;
	this->AUDIOBUFFER_SIZE_SAMPLES = this->AUDIOBUFFER_SIZE_FRAMES*2u;
	this->AUDIOBUFFER_SIZE_BYTES = this->AUDIOBUFFER_SIZE_SAMPLES*2u;

	this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES = _get_closest_power2_ceil(this->AUDIOBUFFER_SIZE_FRAMES/2u);
	this->AUDIOBUFFER_SEGMENT_SIZE_SAMPLES = this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES*2u;
	this->AUDIOBUFFER_SEGMENT_SIZE_BYTES = this->AUDIOBUFFER_SEGMENT_SIZE_SAMPLES*2u;

	this->BUFFERIN_SIZE_FRAMES = this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES;
	this->BUFFERIN_SIZE_SAMPLES = this->BUFFERIN_SIZE_FRAMES;
	this->BUFFERIN_SIZE_BYTES = this->BUFFERIN_SIZE_SAMPLES*2u;

	return TRUE;
}

BOOL WINAPI AudioPB_i16_1ch::buffer_alloc(VOID)
{
	this->buffer_free(); /*Clear any previous memory allocations*/

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
	if(n_ret != S_OK) app_exit(1u, TEXT("BUFFER LOAD: Error: AudioRenderClient::GetBuffer failed."));

	ZeroMemory(this->p_bufferin, this->BUFFERIN_SIZE_BYTES);

	SetFilePointer(this->h_filein, (LONG) *this->p_filein_pos_l32, (LONG*) this->p_filein_pos_h32, FILE_BEGIN);
	ReadFile(this->h_filein, this->p_bufferin, (DWORD) this->BUFFERIN_SIZE_BYTES, &dummy_32, NULL);
	this->filein_pos_64 += (ULONG64) this->BUFFERIN_SIZE_BYTES;

	for(n_frame = 0u; n_frame < this->BUFFERIN_SIZE_FRAMES; n_frame++)
	{
		((INT16*) this->p_audiobuffer)[2u*n_frame] = this->p_bufferin[n_frame];
		((INT16*) this->p_audiobuffer)[2u*n_frame + 1u] = this->p_bufferin[n_frame];
	}

	return;
}
