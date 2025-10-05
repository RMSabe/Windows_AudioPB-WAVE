/*
	WAVE Audio Playback for Windows
	Version 1.3 (mono-stereo)

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#ifndef AUDIOPB_I16_1CH_HPP
#define AUDIOPB_I16_1CH_HPP

#include "AudioPB.hpp"

class AudioPB_i16_1ch : public AudioPB {
	public:
		AudioPB_i16_1ch(const audiopb_params_t *p_params);
		~AudioPB_i16_1ch(VOID);

	private:
		SIZE_T BUFFER_SIZE_FRAMES = 0u;
		SIZE_T BUFFER_SIZE_SAMPLES = 0u;
		SIZE_T BUFFER_SIZE_BYTES = 0u;

		INT16 *p_buffer = NULL;

		BOOL WINAPI audio_hw_init(VOID) override;
		BOOL WINAPI buffer_alloc(VOID) override;
		VOID WINAPI buffer_free(VOID) override;
		VOID WINAPI buffer_load(VOID) override;
};

#endif /*AUDIOPB_I16_1CH_HPP*/
