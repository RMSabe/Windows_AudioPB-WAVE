/*
	WAVE Audio Playback for Windows
	Version 1.1.1

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#ifndef AUDIOBASECLASS_HPP
#define AUDIOBASECLASS_HPP

#include "globldef.h"
#include "strdef.hpp"
#include "shared.hpp"

#include <mmdeviceapi.h>
#include <audioclient.h>

class AudioBaseClass {
	public:
		AudioBaseClass(const audiopb_params_t *p_params);

		BOOL WINAPI setParameters(const audiopb_params_t *p_params);
		BOOL WINAPI initialize(VOID);
		BOOL WINAPI runPlayback(VOID);
		VOID WINAPI stopPlayback(VOID);

		BOOL WINAPI loadAudioDeviceList(VOID);
		BOOL WINAPI chooseDevice(SIZE_T index);
		BOOL WINAPI chooseDefaultDevice(VOID);

		__string WINAPI getLastErrorMessage(VOID);

	protected:

		enum Status {
			STATUS_ERROR_MEMALLOC = -4,
			STATUS_ERROR_NOFILE = -3,
			STATUS_ERROR_AUDIOHW = -2,
			STATUS_ERROR_GENERIC = -1,
			STATUS_UNINITIALIZED = 0,
			STATUS_READY = 1,
			STATUS_PLAYING = 2,
			STATUS_STOPPED = 3
		};

		HANDLE h_filein = INVALID_HANDLE_VALUE;

		ULONG64 filein_size_64 = 0u;
		ULONG32* const p_filein_size_l32 = (ULONG32*) &filein_size_64;
		ULONG32* const p_filein_size_h32 = &((ULONG32*) &filein_size_64)[1];

		ULONG64 filein_pos_64 = 0u;
		ULONG32* const p_filein_pos_l32 = (ULONG32*) &filein_pos_64;
		ULONG32* const p_filein_pos_h32 = &((ULONG32*) &filein_pos_64)[1];

		ULONG64 audio_data_begin = 0u;
		ULONG64 audio_data_end = 0u;

		IMMDeviceEnumerator *p_audiodevenum = NULL;
		IMMDeviceCollection *p_audiodevcoll = NULL;

		IMMDevice *p_audiodev = NULL;
		IAudioClient *p_audiomgr = NULL;
		IAudioRenderClient *p_audioout = NULL;

		SIZE_T AUDIOBUFFER_SIZE_FRAMES = 0u;
		SIZE_T AUDIOBUFFER_SIZE_SAMPLES = 0u;
		SIZE_T AUDIOBUFFER_SIZE_BYTES = 0u;

		SIZE_T AUDIOBUFFER_SEGMENT_SIZE_FRAMES = 0u;
		SIZE_T AUDIOBUFFER_SEGMENT_SIZE_SAMPLES = 0u;
		SIZE_T AUDIOBUFFER_SEGMENT_SIZE_BYTES = 0u;

		VOID *p_audiobuffer = NULL;

		HWND p_audiodevlistbox = NULL;

		__string filein_dir = TEXT("");
		__string err_msg = TEXT("");

		UINT32 sample_rate = 0u;

		INT status = this->STATUS_UNINITIALIZED;

		BOOL stop = FALSE;

		BOOL WINAPI filein_open(VOID);
		VOID WINAPI filein_close(VOID);

		virtual BOOL WINAPI audio_hw_init(VOID) = 0;

		VOID WINAPI audio_hw_deinit_device(VOID);
		VOID WINAPI audio_hw_deinit_all(VOID);

		virtual BOOL WINAPI buffer_alloc(VOID) = 0;
		virtual VOID WINAPI buffer_free(VOID) = 0;

		VOID WINAPI playback_proc(VOID);
		VOID WINAPI playback_init(VOID);
		VOID WINAPI playback_loop(VOID);

		virtual VOID WINAPI buffer_load(VOID) = 0;
		VOID WINAPI buffer_play(VOID);
		VOID WINAPI audio_hw_wait(VOID);
};

#endif /*AUDIOBASECLASS_HPP*/
