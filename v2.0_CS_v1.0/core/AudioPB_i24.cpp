/*
	WAVE Audio Playback for Windows
	Version 4.0

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "AudioPB_i24.hpp"

AudioPB_i24::AudioPB_i24(const audiopb_params_t *p_params) : AudioPB(p_params)
{
	this->AUDIO_BYTES_PER_SAMPLE = 4u;
	this->FILE_BYTES_PER_SAMPLE = 3u;
	this->AUDIODATA_BITS_PER_SAMPLE = 24u;
}

AudioPB_i24::~AudioPB_i24(VOID)
{
	this->deinitialize();
}

BOOL WINAPI AudioPB_i24::buffer_alloc(VOID)
{
	if(!this->buffer_free()) return FALSE;

	if(p_processheap == NULL)
	{
		this->status = AudioPB::STATUS_ERROR_MEMORY;
		this->err_msg = TEXT("AudioPB_i24::buffer_alloc: Error: p_processheap is NULL.");
		return FALSE;
	}

	this->INPUTBUFFER_SIZE_BYTES = (this->STREAMBUFFER_SEGMENT_SIZE_SAMPLES)*(this->FILE_BYTES_PER_SAMPLE);

	this->p_streambuffer = HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, this->STREAMBUFFER_SIZE_BYTES);
	this->p_inputbuffer = (UINT8*) HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, this->INPUTBUFFER_SIZE_BYTES);

	if(this->p_streambuffer == NULL)
	{
		this->buffer_free();
		this->status = AudioPB::STATUS_ERROR_MEMORY;
		this->err_msg = TEXT("AudioPB_i24::buffer_alloc: Error: HeapAlloc failed.");
		return FALSE;
	}

	if(this->p_inputbuffer == NULL)
	{
		this->buffer_free();
		this->status = AudioPB::STATUS_ERROR_MEMORY;
		this->err_msg = TEXT("AudioPB_i24::buffer_alloc: Error: HeapAlloc failed.");
		return FALSE;
	}

	return TRUE;
}

BOOL WINAPI AudioPB_i24::buffer_free(VOID)
{
	if(p_processheap == NULL)
	{
		this->status = AudioPB::STATUS_ERROR_MEMORY;
		this->err_msg = TEXT("AudioPB_i24::buffer_free: Error: p_processheap is NULL.");
		return FALSE;
	}

	if(this->p_streambuffer != NULL)
	{
		if(!HeapFree(p_processheap, 0u, this->p_streambuffer))
		{
			this->status = AudioPB::STATUS_ERROR_MEMORY;
			this->err_msg = TEXT("AudioPB_i24::buffer_free: Error: HeapFree failed.");
			return FALSE;
		}

		this->p_streambuffer = NULL;
	}

	if(this->p_inputbuffer != NULL)
	{
		if(!HeapFree(p_processheap, 0u, this->p_inputbuffer))
		{
			this->status = AudioPB::STATUS_ERROR_MEMORY;
			this->err_msg = TEXT("AudioPB_i24::buffer_free: Error: HeapFree failed.");
			return FALSE;
		}

		this->p_inputbuffer = NULL;
	}

	return TRUE;
}

VOID WINAPI AudioPB_i24::buffer_load(VOID)
{
	INT32 *_p_input = NULL;
	ULONG_PTR _nsample;
	ULONG_PTR _nbyte;
	
	INT32 _i32;
	DWORD _dummy32;

	if(*((ULONG64*) &(this->filein_pos_64)) >= this->AUDIO_DATA_END)
	{
		this->status = AudioPB::STATUS_STOPPED;
		return;
	}

	_p_input = (INT32*) (((ULONG_PTR) (this->p_streambuffer)) + (this->streambuffer_nseg_load)*(this->STREAMBUFFER_SEGMENT_SIZE_BYTES));

	ZeroMemory(this->p_inputbuffer, this->INPUTBUFFER_SIZE_BYTES);

	SetFilePointer(this->h_filein, (LONG) this->filein_pos_64.l32, (LONG*) &(this->filein_pos_64.h32), FILE_BEGIN);
	ReadFile(this->h_filein, this->p_inputbuffer, (DWORD) this->INPUTBUFFER_SIZE_BYTES, &_dummy32, NULL);
	*((ULONG64*) &(this->filein_pos_64)) += (ULONG64) this->INPUTBUFFER_SIZE_BYTES;

	_nbyte = 0u;
	for(_nsample = 0u; _nsample < this->STREAMBUFFER_SEGMENT_SIZE_SAMPLES; _nsample++)
	{
		_i32 = ((this->p_inputbuffer[_nbyte + 2u] << 16) | (this->p_inputbuffer[_nbyte + 1u] << 8) | (this->p_inputbuffer[_nbyte]));

		if(_i32 & 0x00800000) _i32 |= 0xff800000;

		_p_input[_nsample] = (_i32 << 8);

		_nbyte += this->FILE_BYTES_PER_SAMPLE;
	}

	return;
}
