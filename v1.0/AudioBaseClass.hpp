/*
	Audio Playback application for Windows
	Version 1.0

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#ifndef AUDIOBASECLASS_HPP
#define AUDIOBASECLASS_HPP

#include "globldef.h"

#include "strdef.hpp"
#include "audiopb_shared.hpp"

#include <mmdeviceapi.h>
#include <audioclient.h>

class AudioBaseClass {
	public:
		AudioBaseClass(const audiopb_params_t *p_params);

		BOOL WINAPI setParameters(const audiopb_params_t *p_params);
		BOOL WINAPI initialize(VOID);
		BOOL WINAPI runPlayback(VOID);

		__string WINAPI getLastErrorMessage(VOID);

	protected:

		enum Status {
			RUNTIME_STATUS_ERROR_MEMALLOC = -4,
			RUNTIME_STATUS_ERROR_NOFILE = -3,
			RUNTIME_STATUS_ERROR_AUDIOHW = -2,
			RUNTIME_STATUS_ERROR_GENERIC = -1,
			RUNTIME_STATUS_UNINITIALIZED = 0,
			RUNTIME_STATUS_INITIALIZED = 1
		};

		HANDLE p_userthread = nullptr;

		HANDLE h_filein = INVALID_HANDLE_VALUE;

		ULONG64 filein_size_64 = 0u;
		ULONG32* const p_filein_size_l32 = (ULONG32*) &this->filein_size_64;
		ULONG32* const p_filein_size_h32 = &((ULONG32*) &this->filein_size_64)[1];

		ULONG64 filein_pos_64 = 0u;
		ULONG32* const p_filein_pos_l32 = (ULONG32*) &this->filein_pos_64;
		ULONG32* const p_filein_pos_h32 = &((ULONG32*) &this->filein_pos_64)[1];

		ULONG64 audio_data_begin = 0u;
		ULONG64 audio_data_end = 0u;

		IMMDevice *p_audiodev = nullptr;
		IAudioClient *p_audiomgr = nullptr;
		IAudioRenderClient *p_audioout = nullptr;

		const SIZE_T AUDIOBUFFER_N_SEGMENTS = 2u;

		SIZE_T AUDIOBUFFER_SIZE_FRAMES = 0u;
		SIZE_T AUDIOBUFFER_SIZE_SAMPLES = 0u;
		SIZE_T AUDIOBUFFER_SIZE_BYTES = 0u;

		SIZE_T AUDIOBUFFER_SEGMENT_SIZE_FRAMES = 0u;
		SIZE_T AUDIOBUFFER_SEGMENT_SIZE_SAMPLES = 0u;
		SIZE_T AUDIOBUFFER_SEGMENT_SIZE_BYTES = 0u;

		VOID *p_audiobuffer = nullptr;

		__string filein_dir = TEXT("");
		__string err_msg = TEXT("");

		INT status = this->RUNTIME_STATUS_UNINITIALIZED;

		UINT32 sample_rate = 0u;

		BOOL stop = FALSE;

		BOOL WINAPI wait_thread(HANDLE *pp_thread);
		BOOL WINAPI stop_thread(HANDLE *pp_thread, DWORD exit_code);

		BOOL WINAPI filein_open(VOID);
		VOID WINAPI filein_close(VOID);

		virtual BOOL WINAPI audio_hw_init(VOID) = 0;
		VOID WINAPI audio_hw_deinit(VOID);

		virtual BOOL WINAPI buffer_alloc(VOID) = 0;
		virtual VOID WINAPI buffer_free(VOID) = 0;

		BOOL WINAPI prompt_user_choose_device(IMMDeviceEnumerator *p_devenum, IMMDeviceCollection *p_devcoll);
		BOOL WINAPI print_audio_device_list(IMMDeviceCollection *p_devcoll, UINT devcoll_count);

		VOID WINAPI playback_proc(VOID);
		VOID WINAPI playback_init(VOID);
		VOID WINAPI playback_loop(VOID);

		virtual VOID WINAPI buffer_load(VOID) = 0;
		VOID WINAPI buffer_play(VOID);
		VOID WINAPI audio_hw_wait(VOID);

		DWORD WINAPI userthread_proc(VOID *args);
		VOID WINAPI cmd_decode(VOID);
};

#endif //AUDIOBASECLASS_HPP
