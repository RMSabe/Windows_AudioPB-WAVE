/*
	WAVE Audio Playback for Windows
	Version 4.0

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "AudioPB_i16.hpp"

AudioPB_i16::AudioPB_i16(const audiopb_params_t *p_params) : AudioPB(p_params)
{
	this->AUDIO_BYTES_PER_SAMPLE = 2u;
	this->FILE_BYTES_PER_SAMPLE = 2u;
	this->AUDIODATA_BITS_PER_SAMPLE = 16u;
}

AudioPB_i16::~AudioPB_i16(VOID)
{
	this->deinitialize();
}

BOOL WINAPI AudioPB_i16::buffer_alloc(VOID)
{
	if(!this->buffer_free()) return FALSE;

	if(p_processheap == NULL)
	{
		this->status = AudioPB::STATUS_ERROR_MEMORY;
		this->err_msg = TEXT("AudioPB_i16::buffer_alloc: Error: p_processheap is NULL.");
		return FALSE;
	}

	this->p_streambuffer = HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, this->STREAMBUFFER_SIZE_BYTES);

	if(this->p_streambuffer == NULL)
	{
		this->status = AudioPB::STATUS_ERROR_MEMORY;
		this->err_msg = TEXT("AudioPB_i16::buffer_alloc: Error: HeapAlloc failed.");
		return FALSE;
	}

	return TRUE;
}

BOOL WINAPI AudioPB_i16::buffer_free(VOID)
{
	if(p_processheap == NULL)
	{
		this->status = AudioPB::STATUS_ERROR_MEMORY;
		this->err_msg = TEXT("AudioPB_i16::buffer_free: Error: p_processheap is NULL.");
		return FALSE;
	}

	if(this->p_streambuffer != NULL)
	{
		if(!HeapFree(p_processheap, 0u, this->p_streambuffer))
		{
			this->status = AudioPB::STATUS_ERROR_MEMORY;
			this->err_msg = TEXT("AudioPB_i16::buffer_free: Error: HeapFree failed.");
			return FALSE;
		}

		this->p_streambuffer = NULL;
	}

	return TRUE;
}

VOID WINAPI AudioPB_i16::buffer_load(VOID)
{
	VOID *_p_input = NULL;
	DWORD _dummy32;

	if(*((ULONG64*) &(this->filein_pos_64)) >= this->AUDIO_DATA_END)
	{
		this->status = AudioPB::STATUS_STOPPED;
		return;
	}

	_p_input = (VOID*) (((ULONG_PTR) (this->p_streambuffer)) + (this->streambuffer_nseg_load)*(this->STREAMBUFFER_SEGMENT_SIZE_BYTES));
	ZeroMemory(_p_input, this->STREAMBUFFER_SEGMENT_SIZE_BYTES);

	SetFilePointer(this->h_filein, (LONG) this->filein_pos_64.l32, (LONG*) &(this->filein_pos_64.h32), FILE_BEGIN);
	ReadFile(this->h_filein, _p_input, (DWORD) this->STREAMBUFFER_SEGMENT_SIZE_BYTES, &_dummy32, NULL);
	*((ULONG64*) &(this->filein_pos_64)) += (ULONG64) this->STREAMBUFFER_SEGMENT_SIZE_BYTES;

	return;
}
