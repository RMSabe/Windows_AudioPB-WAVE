/*
	WAVE Audio Playback for Windows
	Version 1.2.1

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#ifndef AUDIOPB_I16_2CH_HPP
#define AUDIOPB_I16_2CH_HPP

#include "AudioPB.hpp"

class AudioPB_i16_2ch : public AudioPB {
	public:
		AudioPB_i16_2ch(const audiopb_params_t *p_params);
		~AudioPB_i16_2ch(VOID);

	private:
		BOOL WINAPI audio_hw_init(VOID) override;
		BOOL WINAPI buffer_alloc(VOID) override;
		VOID WINAPI buffer_free(VOID) override;
		VOID WINAPI buffer_load(VOID) override;
};

#endif /*AUDIOPB_I16_2CH_HPP*/
