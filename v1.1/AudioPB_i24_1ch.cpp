/*
	WAVE Audio Playback for Windows
	Version 1.1

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "AudioPB_i24_1ch.hpp"

AudioPB_i24_1ch::AudioPB_i24_1ch(const audiopb_params_t *p_params) : AudioBaseClass(p_params)
{
}

AudioPB_i24_1ch::~AudioPB_i24_1ch(VOID)
{
	this->stopPlayback();
	this->filein_close();
	this->audio_hw_deinit();
	this->buffer_free();
}

BOOL WINAPI AudioPB_i24_1ch::audio_hw_init(VOID)
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

	this->BYTEBUF_SIZE = this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES*3u;

	return TRUE;
}

BOOL WINAPI AudioPB_i24_1ch::buffer_alloc(VOID)
{
	this->buffer_free(); /*Clear any previous memory allocation*/

	this->p_bytebuf = (UINT8*) HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, this->BYTEBUF_SIZE);

	return (this->p_bytebuf != nullptr);
}

VOID WINAPI AudioPB_i24_1ch::buffer_free(VOID)
{
	if(this->p_bytebuf != nullptr)
	{
		HeapFree(p_processheap, 0u, this->p_bytebuf);
		this->p_bytebuf = nullptr;
	}

	return;
}

VOID WINAPI AudioPB_i24_1ch::buffer_load(VOID)
{
	SIZE_T n_frame = 0u;
	SIZE_T n_byte = 0u;
	HRESULT n_ret = 0;
	DWORD dummy_32;

	if(this->filein_pos_64 >= this->audio_data_end)
	{
		this->stop = TRUE;
		return;
	}

	n_ret = this->p_audioout->GetBuffer((UINT32) this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES, (BYTE**) &this->p_audiobuffer);
	if(n_ret != S_OK) app_exit(1u, TEXT("BUFFER LOAD: Error: AudioRenderClient::GetBuffer failed."));

	ZeroMemory(this->p_bytebuf, this->BYTEBUF_SIZE);

	SetFilePointer(this->h_filein, (LONG) *this->p_filein_pos_l32, (LONG*) this->p_filein_pos_h32, FILE_BEGIN);
	ReadFile(this->h_filein, this->p_bytebuf, (DWORD) this->BYTEBUF_SIZE, &dummy_32, NULL);
	this->filein_pos_64 += (ULONG64) this->BYTEBUF_SIZE;

	n_byte = 1u;
	for(n_frame = 0u; n_frame < this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES; n_frame++)
	{
		((INT16*) this->p_audiobuffer)[2u*n_frame] = *((INT16*) &this->p_bytebuf[n_byte]);
		((INT16*) this->p_audiobuffer)[2u*n_frame + 1u] = *((INT16*) &this->p_bytebuf[n_byte]);
		n_byte += 3u;
	}

	return;
}
