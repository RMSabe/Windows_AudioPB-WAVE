/*
	WAVE Audio Playback for Windows
	Version 1.3 (mono-stereo)

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#ifndef AUDIOPB_I24_2CH_HPP
#define AUDIOPB_I24_2CH_HPP

#include "AudioPB.hpp"

class AudioPB_i24_2ch : public AudioPB {
	public:
		AudioPB_i24_2ch(const audiopb_params_t *p_params);
		~AudioPB_i24_2ch(VOID);

	private:
		SIZE_T BYTEBUF_SIZE = 0u;

		UINT8 *p_bytebuf = NULL;

		BOOL WINAPI audio_hw_init(VOID) override;
		BOOL WINAPI buffer_alloc(VOID) override;
		VOID WINAPI buffer_free(VOID) override;
		VOID WINAPI buffer_load(VOID) override;
};

#endif /*AUDIOPB_I24_2CH_HPP*/
