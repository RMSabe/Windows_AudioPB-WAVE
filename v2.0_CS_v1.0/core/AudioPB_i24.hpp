/*
	WAVE Audio Playback for Windows
	Version 4.0

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#ifndef AUDIOPB_I24_HPP
#define AUDIOPB_I24_HPP

#include "AudioPB.hpp"

class AudioPB_i24 : public AudioPB {
	public:
		AudioPB_i24(const audiopb_params_t *p_params);
		~AudioPB_i24(VOID) override;

	private:
		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR INPUTBUFFER_SIZE_BYTES = 0u;
		__declspec(align(PTR_SIZE_BYTES)) UINT8 *p_inputbuffer = NULL;

		BOOL WINAPI buffer_alloc(VOID) override;
		BOOL WINAPI buffer_free(VOID) override;
		VOID WINAPI buffer_load(VOID) override;
};

#endif /*AUDIOPB_I24_HPP*/
