/*
	WAVE Audio Playback for Windows
	Version 1.3.1

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#ifndef AUDIOPB_HPP
#define AUDIOPB_HPP

#include "globldef.h"
#include "strdef.hpp"
#include "shared.hpp"

#include <mmdeviceapi.h>
#include <audioclient.h>

struct _audiopb_params {
	const TCHAR *file_dir;
	ULONG64 audio_data_begin;
	ULONG64 audio_data_end;
	UINT32 sample_rate;
	UINT16 n_channels;
};

typedef struct _audiopb_params audiopb_params_t;

class AudioPB {
	public:
		AudioPB(const audiopb_params_t *p_params);

		BOOL WINAPI setParameters(const audiopb_params_t *p_params);
		BOOL WINAPI initialize(VOID);
		BOOL WINAPI runPlayback(VOID);
		VOID WINAPI stopPlayback(VOID);

		BOOL WINAPI loadAudioDeviceList(HWND p_listbox);
		BOOL WINAPI chooseDevice(SIZE_T index);
		BOOL WINAPI chooseDefaultDevice(VOID);

		__string WINAPI getLastErrorMessage(VOID);

		enum Status {
			STATUS_ERROR_MEMALLOC = -4,
			STATUS_ERROR_NOFILE = -3,
			STATUS_ERROR_AUDIOHW = -2,
			STATUS_ERROR_GENERIC = -1,
			STATUS_UNINITIALIZED = 0,
			STATUS_READY = 1,
			STATUS_PLAYING = 2
		};

	protected:
		HANDLE h_filein = INVALID_HANDLE_VALUE;

		fileptr64_t filein_size_64 = {
			.l32 = 0u,
			.h32 = 0u
		};

		fileptr64_t filein_pos_64 = {
			.l32 = 0u,
			.h32 = 0u
		};

		ULONG64 AUDIO_DATA_BEGIN = 0u;
		ULONG64 AUDIO_DATA_END = 0u;

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

		SIZE_T N_CHANNELS = 0u;

		__string FILEIN_DIR = TEXT("");

		__string err_msg = TEXT("");

		UINT32 SAMPLE_RATE = 0u;

		INT status = this->STATUS_UNINITIALIZED;

		BOOL stop_playback = FALSE;

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

#endif /*AUDIOPB_HPP*/
