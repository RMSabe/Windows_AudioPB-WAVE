/*
	WAVE Audio Playback for Windows
	Version 4.0

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#ifndef AUDIOPB_I16_HPP
#define AUDIOPB_I16_HPP

#include "AudioPB.hpp"

class AudioPB_i16 : public AudioPB {
	public:
		AudioPB_i16(const audiopb_params_t *p_params);
		~AudioPB_i16(VOID) override;

	private:
		BOOL WINAPI buffer_alloc(VOID) override;
		BOOL WINAPI buffer_free(VOID) override;
		VOID WINAPI buffer_load(VOID) override;
};

#endif /*AUDIOPB_I16_HPP*/
