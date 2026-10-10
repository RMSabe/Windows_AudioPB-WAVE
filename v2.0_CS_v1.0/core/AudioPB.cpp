/*
	WAVE Audio Playback for Windows
	Version 4.0

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "AudioPB.hpp"
#include "cstrdef.h"

#include <combaseapi.h>

AudioPB::AudioPB(const audiopb_params_t *p_params)
{
	this->setParameters(p_params);
}

AudioPB::~AudioPB(VOID)
{
}

BOOL WINAPI AudioPB::setParameters(const audiopb_params_t *p_params)
{
	if(this->status > AudioPB::STATUS_UNINITIALIZED)
	{
		this->err_msg = TEXT("AudioPB::setParameters: Error: cannot run method. Audio object already initialized.");
		return FALSE;
	}

	if(p_params == NULL)
	{
		this->err_msg = TEXT("AudioPB::setParameters: Error: given params object is NULL.");
		return FALSE;
	}

	if(p_params->file_dir == NULL)
	{
		this->err_msg = TEXT("AudioPB::setParameters: Error: invalid input file directory.");
		return FALSE;
	}

	this->status = AudioPB::STATUS_UNINITIALIZED;

	this->AUDIO_DATA_BEGIN = p_params->audio_data_begin;
	this->AUDIO_DATA_END = p_params->audio_data_end;
	this->FILEIN_DIR = p_params->file_dir;
	this->AUDIOBUFFER_SIZE_FRAMES = p_params->audiobuffer_size_frames;
	this->STREAMBUFFER_N_SEGMENTS = p_params->streambuffer_n_segments;
	this->STREAMBUFFER_SEGMENT_SIZE_FRAMES = p_params->streambuffer_segment_size_frames;
	this->N_CHANNELS = (ULONG_PTR) p_params->n_channels;
	this->SAMPLE_RATE = p_params->sample_rate;
	this->CHANNEL_MASK = p_params->channel_mask;

	return TRUE;
}

BOOL WINAPI AudioPB::initialize(VOID)
{
	if(this->status > AudioPB::STATUS_UNINITIALIZED) return TRUE;

	this->status = AudioPB::STATUS_UNINITIALIZED;

	if(!this->SAMPLE_RATE)
	{
		this->status = AudioPB::STATUS_ERROR_INVALIDPARAMS;
		this->err_msg = TEXT("AudioPB::initialize: Error: invalid sample rate value.");
		return FALSE;
	}

	if(this->N_CHANNELS < AudioPB::N_CHANNELS_MIN)
	{
		this->status = AudioPB::STATUS_ERROR_INVALIDPARAMS;
		this->err_msg = TEXT("AudioPB::initialize: Error: invalid number of channels.");
		return FALSE;
	}

	if(this->STREAMBUFFER_N_SEGMENTS < AudioPB::STREAMBUFFER_N_SEGMENTS_MIN)
	{
		this->status = AudioPB::STATUS_ERROR_INVALIDPARAMS;
		this->err_msg = TEXT("AudioPB::initialize: Error: invalid stream buffer segment count.");
		return FALSE;
	}

	if(this->STREAMBUFFER_SEGMENT_SIZE_FRAMES < AudioPB::STREAMBUFFER_SEGMENT_SIZE_FRAMES_MIN)
	{
		this->status = AudioPB::STATUS_ERROR_INVALIDPARAMS;
		this->err_msg = TEXT("AudioPB::initialize: Error: invalid stream buffer segment size.");
		return FALSE;
	}

	if(this->AUDIOBUFFER_SIZE_FRAMES < this->STREAMBUFFER_SEGMENT_SIZE_FRAMES)
	{
		this->status = AudioPB::STATUS_ERROR_INVALIDPARAMS;
		this->err_msg = TEXT("AudioPB::initialize: Error: invalid audio buffer size. (audio buffer is smaller than stream buffer segment).");
		return FALSE;
	}

	this->STREAMBUFFER_SEGMENT_SIZE_SAMPLES = (this->STREAMBUFFER_SEGMENT_SIZE_FRAMES)*(this->N_CHANNELS);
	this->STREAMBUFFER_SEGMENT_SIZE_BYTES = (this->STREAMBUFFER_SEGMENT_SIZE_SAMPLES)*(this->AUDIO_BYTES_PER_SAMPLE);

	this->STREAMBUFFER_SIZE_FRAMES = (this->STREAMBUFFER_N_SEGMENTS)*(this->STREAMBUFFER_SEGMENT_SIZE_FRAMES);
	this->STREAMBUFFER_SIZE_SAMPLES = (this->STREAMBUFFER_SIZE_FRAMES)*(this->N_CHANNELS);
	this->STREAMBUFFER_SIZE_BYTES = (this->STREAMBUFFER_SIZE_SAMPLES)*(this->AUDIO_BYTES_PER_SAMPLE);

	if(!this->filein_open()) return FALSE;

	if(!this->audiodevice_init())
	{
		this->filein_close();
		return FALSE;
	}

	if(!this->buffer_alloc())
	{
		this->filein_close();
		this->audiodevice_deinit();
		return FALSE;
	}

	this->status = AudioPB::STATUS_READY;
	return TRUE;
}

BOOL WINAPI AudioPB::runPlayback(VOID)
{
	if(this->status != AudioPB::STATUS_READY)
	{
		this->err_msg = TEXT("AudioPB::runPlayback: Error: Audio object either not initialized or already running.");
		return FALSE;
	}

	this->playback_proc();
	this->status = AudioPB::STATUS_UNINITIALIZED;

	this->filein_close();
	this->audiodevice_deinit();
	this->buffer_free();

	return TRUE;
}

VOID WINAPI AudioPB::pausePlayback(VOID)
{
	if(this->status == AudioPB::STATUS_RUNNING)
	{
		this->status = AudioPB::STATUS_PAUSED;
		this->audiodevice_wait();
		this->audiodev.p_audioclient->Stop();
	}

	return;
}

VOID WINAPI AudioPB::resumePlayback(VOID)
{
	if(this->status == AudioPB::STATUS_PAUSED)
	{
		this->audiodev.p_audioclient->Start();
		this->status = AudioPB::STATUS_RUNNING;
	}

	return;
}

VOID WINAPI AudioPB::stopPlayback(VOID)
{
	if(this->status > AudioPB::STATUS_READY) this->status = AudioPB::STATUS_STOPPED;
	return;
}

LONG64 WINAPI AudioPB::getAudioDataSizeFrames(VOID)
{
	ULONG_PTR _bytes_per_frame;

	if(this->status < AudioPB::STATUS_READY) return -1;

	_bytes_per_frame = (this->FILE_BYTES_PER_SAMPLE)*(this->N_CHANNELS);

	return (LONG64) ((this->AUDIO_DATA_END - this->AUDIO_DATA_BEGIN)/((ULONG64) _bytes_per_frame));
}

LONG64 WINAPI AudioPB::getAudioDataPositionFrames(VOID)
{
	ULONG_PTR _bytes_per_frame;

	if(this->status < AudioPB::STATUS_READY) return -1;

	_bytes_per_frame = (this->FILE_BYTES_PER_SAMPLE)*(this->N_CHANNELS);

	return (LONG64) ((*((ULONG64*) &(this->filein_pos_64)) - this->AUDIO_DATA_BEGIN)/((ULONG64) _bytes_per_frame));
}

BOOL WINAPI AudioPB::setAudioDataPositionFrames(ULONG64 position)
{
	ULONG64 _audiodata_size_frames;
	ULONG_PTR _bytes_per_frame;

	if(this->status < AudioPB::STATUS_READY) return FALSE;

	_audiodata_size_frames = (ULONG64) this->getAudioDataSizeFrames();
	if(position >= _audiodata_size_frames)
	{
		this->err_msg = TEXT("AudioPB::setAudioDataPositionFrames: Error: invalid position value.");
		return FALSE;
	}

	_bytes_per_frame = (this->FILE_BYTES_PER_SAMPLE)*(this->N_CHANNELS);

	*((ULONG64*) &(this->filein_pos_64)) = this->AUDIO_DATA_BEGIN + position*((ULONG64) _bytes_per_frame);
	return TRUE;
}

BOOL WINAPI AudioPB::loadAudioDeviceList(VOID)
{
	const PROPERTYKEY* const _P_PKEY = (const PROPERTYKEY*) P_PKEY_Device_FriendlyName;
	IMMDevice *_p_dev = NULL;
	IPropertyStore *_p_devprop = NULL;
	ULONG_PTR _audiodevicelist_byteoffset;
	HRESULT _n_ret;
	UINT _n_dev;
	PROPVARIANT _propvar;

	if(this->status > AudioPB::STATUS_UNINITIALIZED)
	{
		this->err_msg = TEXT("AudioPB::loadAudioDeviceList: Error: cannot run method. Audio object already initialized.");
		return FALSE;
	}

	this->audiodevicelist_deinit();

	if(this->p_audiodevenum == NULL)
	{
		_n_ret = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (VOID**) &(this->p_audiodevenum));
		if(_n_ret != S_OK)
		{
			this->status = AudioPB::STATUS_ERROR_AUDIOHW;
			this->err_msg = TEXT("AudioPB::loadAudioDeviceList: Error: CoCreateInstance (IMMDeviceEnumerator) failed.");
			return FALSE;
		}
	}

	if(!this->audiodevicelist_init()) return FALSE;

	_audiodevicelist_byteoffset = 0u;

	for(_n_dev = 0u; _n_dev < (UINT) this->audiodevlist.devlist_n_entries; _n_dev++)
	{
		_n_ret = this->audiodevlist.p_devcoll->Item(_n_dev, &_p_dev);
		if(_n_ret != S_OK)
		{
			this->audiodevicelist_deinit();
			this->status = AudioPB::STATUS_ERROR_AUDIOHW;
			this->err_msg = TEXT("AudioPB::loadAudioDeviceList: Error: IMMDeviceCollection::Item failed.");
			return FALSE;
		}

		_n_ret = _p_dev->OpenPropertyStore(STGM_READ, &_p_devprop);
		if(_n_ret != S_OK)
		{
			_p_dev->Release();
			this->audiodevicelist_deinit();
			this->status = AudioPB::STATUS_ERROR_AUDIOHW;
			this->err_msg = TEXT("AudioPB::loadAudioDeviceList: Error: IMMDevice::OpenPropertyStore failed.");
			return FALSE;
		}

		PropVariantInit(&_propvar);

		_n_ret = _p_devprop->GetValue(*_P_PKEY, &_propvar);
		if(_n_ret != S_OK)
		{
			_p_devprop->Release();
			_p_dev->Release();
			this->audiodevicelist_deinit();
			this->status = AudioPB::STATUS_ERROR_AUDIOHW;
			this->err_msg = TEXT("AudioPB::loadAudioDeviceList: Error: IPropertyStore::GetValue failed.");
			return FALSE;
		}

		if(_propvar.vt == VT_EMPTY) __SPRINTF((TCHAR*) (((ULONG_PTR) (this->audiodevlist.p_devlist)) + _audiodevicelist_byteoffset), AudioPB::AUDIODEVICELIST_ENTRYLENGTH, TEXT("Unknown Audio Device"));
		else cstr_copy_wchar_to_tchar(_propvar.pwszVal, (TCHAR*) (((ULONG_PTR) (this->audiodevlist.p_devlist)) + _audiodevicelist_byteoffset), AudioPB::AUDIODEVICELIST_ENTRYLENGTH);

		PropVariantClear(&_propvar);

		_p_devprop->Release();
		_p_devprop = NULL;

		_p_dev->Release();
		_p_dev = NULL;

		_audiodevicelist_byteoffset += (AudioPB::AUDIODEVICELIST_ENTRYLENGTH)*(sizeof(TCHAR));
	}

	PropVariantClear(&_propvar);

	if(_p_devprop != NULL) _p_devprop->Release();
	if(_p_dev != NULL) _p_dev->Release();

	this->status = AudioPB::STATUS_UNINITIALIZED;
	return TRUE;
}

LONG_PTR WINAPI AudioPB::getAudioDeviceListEntryCount(VOID)
{
	return (LONG_PTR) this->audiodevlist.devlist_n_entries;
}

const TCHAR* WINAPI AudioPB::getAudioDeviceListEntry(ULONG_PTR index)
{
	if(this->audiodevlist.p_devlist == NULL)
	{
		this->err_msg = TEXT("AudioPB::getAudioDeviceListEntry: Error: list is not loaded.");
		return NULL;
	}

	if(index >= this->audiodevlist.devlist_n_entries)
	{
		this->err_msg = TEXT("AudioPB::getAudioDeviceListEntry: Error: given index is out of bounds.");
		return NULL;
	}

	return (const TCHAR*) (((ULONG_PTR) (this->audiodevlist.p_devlist)) + index*sizeof(TCHAR)*(AudioPB::AUDIODEVICELIST_ENTRYLENGTH));
}

BOOL WINAPI AudioPB::chooseDevice(ULONG_PTR index)
{
	HRESULT _n_ret;

	if(this->status > AudioPB::STATUS_UNINITIALIZED)
	{
		this->err_msg = TEXT("AudioPB::chooseDevice: Error: cannot run method. Audio object already initialized.");
		return FALSE;
	}

	if(this->audiodevlist.p_devcoll == NULL)
	{
		this->err_msg = TEXT("AudioPB::chooseDevice: Error: audio device list is not loaded.");
		return FALSE;
	}

	this->audiodevice_deinit();

	_n_ret = this->audiodevlist.p_devcoll->Item((UINT) index, &(this->audiodev.p_device));
	if(_n_ret != S_OK)
	{
		this->status = AudioPB::STATUS_ERROR_AUDIOHW;
		this->err_msg = TEXT("AudioPB::chooseDevice: Error: IMMDeviceCollection::Item failed.");
		return FALSE;
	}

	this->status = AudioPB::STATUS_UNINITIALIZED;
	return TRUE;
}

BOOL WINAPI AudioPB::chooseDefaultDevice(VOID)
{
	HRESULT _n_ret;

	if(this->status > AudioPB::STATUS_UNINITIALIZED)
	{
		this->err_msg = TEXT("AudioPB::chooseDefaultDevice: Error: cannot run method. Audio object already initialized.");
		return FALSE;
	}

	if(this->p_audiodevenum == NULL)
	{
		this->err_msg = TEXT("AudioPB::chooseDefaultDevice: Error: audio device enumerator is not ready.");
		return FALSE;
	}

	this->audiodevice_deinit();

	_n_ret = this->p_audiodevenum->GetDefaultAudioEndpoint(eRender, eMultimedia, &(this->audiodev.p_device));
	if(_n_ret != S_OK)
	{
		this->status = AudioPB::STATUS_ERROR_AUDIOHW;
		this->err_msg = TEXT("AudioPB::chooseDefaultDevice: Error: IMMDeviceEnumerator::GetDefaultAudioEndpoint failed.");
		return FALSE;
	}

	this->status = AudioPB::STATUS_UNINITIALIZED;
	return TRUE;
}

INT WINAPI AudioPB::getStatus(VOID)
{
	return this->status;
}

__string WINAPI AudioPB::getLastErrorMessage(VOID)
{
	if(this->status == AudioPB::STATUS_UNINITIALIZED)
		return (TEXT("Error: Audio object not initialized\r\nExtended error message: ") + this->err_msg);

	return this->err_msg;
}

VOID WINAPI AudioPB::deinitialize(VOID)
{
	this->status = AudioPB::STATUS_UNINITIALIZED;

	this->filein_close();
	this->audiodevice_deinit();
	this->buffer_free();

	this->audiodevicelist_deinit();

	if(this->p_audiodevenum != NULL)
	{
		this->p_audiodevenum->Release();
		this->p_audiodevenum = NULL;
	}

	return;
}

BOOL WINAPI AudioPB::filein_open(VOID)
{
	this->filein_close();

	this->h_filein = CreateFile(this->FILEIN_DIR.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, INVALID_HANDLE_VALUE);
	if(this->h_filein == INVALID_HANDLE_VALUE)
	{
		this->status = AudioPB::STATUS_ERROR_NOFILE;
		this->err_msg = TEXT("AudioPB::filein_open: Error: failed to open input file.");
		return FALSE;
	}

	this->filein_size_64.l32 = (ULONG32) GetFileSize(this->h_filein, (DWORD*) &(this->filein_size_64.h32));
	return TRUE;
}

VOID WINAPI AudioPB::filein_close(VOID)
{
	if(this->h_filein == INVALID_HANDLE_VALUE) return;

	CloseHandle(this->h_filein);
	this->h_filein = INVALID_HANDLE_VALUE;
	*((ULONG64*) &(this->filein_size_64)) = 0u;

	return;
}

BOOL WINAPI AudioPB::audiodevice_init(VOID)
{
	ULONG64 _audiobuffer_time;
	ULONG_PTR _n_channel;
	HRESULT _n_ret;
	DWORD _ch_mask;
	WAVEFORMATEXTENSIBLE _wavfmt;

	if(this->audiodev.p_audioclient != NULL)
	{
		this->audiodev.p_audioclient->Stop();
		this->audiodev.p_audioclient->Release();
		this->audiodev.p_audioclient = NULL;
	}

	if(this->audiodev.p_audioservice != NULL)
	{
		this->audiodev.p_audioservice->Release();
		this->audiodev.p_audioservice = NULL;
	}

	if(this->audiodev.p_device == NULL)
	{
		this->err_msg = TEXT("AudioPB::audiodevice_init: Error: audio device is not ready.");
		return FALSE;
	}

	_n_ret = this->audiodev.p_device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (VOID**) &(this->audiodev.p_audioclient));
	if(_n_ret != S_OK)
	{
		this->audiodevice_deinit();
		this->status = AudioPB::STATUS_ERROR_AUDIOHW;
		this->err_msg = TEXT("AudioPB::audiodevice_init: Error: IMMDevice::Activate failed.");
		return FALSE;
	}

	if(this->CHANNEL_MASK) _ch_mask = (DWORD) this->CHANNEL_MASK;
	else
	{
		_ch_mask = 0u;
		for(_n_channel = 0u; _n_channel < this->N_CHANNELS; _n_channel++) _ch_mask |= (1 << _n_channel);
	}

	ZeroMemory(&_wavfmt, sizeof(WAVEFORMATEXTENSIBLE));

	_wavfmt.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	_wavfmt.Format.nChannels = (WORD) this->N_CHANNELS;
	_wavfmt.Format.wBitsPerSample = (WORD) ((this->AUDIO_BYTES_PER_SAMPLE)*8u);
	_wavfmt.Format.nBlockAlign = (_wavfmt.Format.nChannels)*((WORD) this->AUDIO_BYTES_PER_SAMPLE);
	_wavfmt.Format.nSamplesPerSec = (DWORD) this->SAMPLE_RATE;
	_wavfmt.Format.nAvgBytesPerSec = (_wavfmt.Format.nSamplesPerSec)*((DWORD) _wavfmt.Format.nBlockAlign);
	_wavfmt.Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
	_wavfmt.Samples.wValidBitsPerSample = (WORD) this->AUDIODATA_BITS_PER_SAMPLE;
	_wavfmt.dwChannelMask = _ch_mask;
	_wavfmt.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;

	_n_ret = this->audiodev.p_audioclient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, (WAVEFORMATEX*) &_wavfmt, NULL);
	if(_n_ret != S_OK)
	{
		this->audiodevice_deinit();
		this->status = AudioPB::STATUS_ERROR_AUDIOHW;
		this->err_msg = TEXT("AudioPB::audiodevice_init: Error: audio device does not support audio stream format.");
		return FALSE;
	}

	_audiobuffer_time = ((ULONG64) this->AUDIOBUFFER_SIZE_FRAMES)*10000000/((ULONG64) this->SAMPLE_RATE);

	_n_ret = this->audiodev.p_audioclient->Initialize(AUDCLNT_SHAREMODE_EXCLUSIVE, 0, (REFERENCE_TIME) _audiobuffer_time, 0, (WAVEFORMATEX*) &_wavfmt, NULL);
	if(_n_ret != S_OK)
	{
		this->audiodevice_deinit();
		this->status = AudioPB::STATUS_ERROR_AUDIOHW;
		this->err_msg = TEXT("AudioPB::audiodevice_init: Error: IAudioClient::Initialize failed.");
		return FALSE;
	}

	_n_ret = this->audiodev.p_audioclient->GetService(__uuidof(IAudioRenderClient), (VOID**) &(this->audiodev.p_audioservice));
	if(_n_ret != S_OK)
	{
		this->audiodevice_deinit();
		this->status = AudioPB::STATUS_ERROR_AUDIOHW;
		this->err_msg = TEXT("AudioPB::audiodevice_init: Error: IAudioClient::GetService failed.");
		return FALSE;
	}

	return TRUE;
}

VOID WINAPI AudioPB::audiodevice_deinit(VOID)
{
	if(this->audiodev.p_audioclient != NULL)
	{
		this->audiodev.p_audioclient->Stop();
		this->audiodev.p_audioclient->Release();
		this->audiodev.p_audioclient = NULL;
	}

	if(this->audiodev.p_audioservice != NULL)
	{
		this->audiodev.p_audioservice->Release();
		this->audiodev.p_audioservice = NULL;
	}

	if(this->audiodev.p_device != NULL)
	{
		this->audiodev.p_device->Release();
		this->audiodev.p_device = NULL;
	}

	return;
}

BOOL WINAPI AudioPB::audiodevicelist_init(VOID)
{
	HRESULT _n_ret;
	UINT _devcoll_count;

	if(p_processheap == NULL)
	{
		this->status = AudioPB::STATUS_ERROR_MEMORY;
		this->err_msg = TEXT("AudioPB::audiodevicelist_init: Error: p_processheap is NULL.");
		return FALSE;
	}

	if(this->p_audiodevenum == NULL)
	{
		this->err_msg = TEXT("AudioPB::audiodevicelist_init: Error: audio device enumerator not ready.");
		return FALSE;
	}

	if(!this->audiodevicelist_deinit()) return FALSE;

	_n_ret = this->p_audiodevenum->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &(this->audiodevlist.p_devcoll));
	if(_n_ret != S_OK)
	{
		this->status = AudioPB::STATUS_ERROR_AUDIOHW;
		this->err_msg = TEXT("AudioPB::audiodevicelist_init: Error: IMMDeviceEnumerator::EnumAudioEndpoints failed.");
		return FALSE;
	}

	_n_ret = this->audiodevlist.p_devcoll->GetCount(&_devcoll_count);
	if(_n_ret != S_OK)
	{
		this->audiodevicelist_deinit();
		this->status = AudioPB::STATUS_ERROR_AUDIOHW;
		this->err_msg = TEXT("AudioPB::audiodevicelist_init: Error: IMMDeviceCollection::GetCount failed.");
		return FALSE;
	}

	if(!_devcoll_count)
	{
		this->audiodevicelist_deinit();
		this->err_msg = TEXT("AudioPB::audiodevicelist_init: Error: no audio devices found.");
		return FALSE;
	}

	this->audiodevlist.devlist_n_entries = (ULONG_PTR) _devcoll_count;

	this->audiodevlist.p_devlist = (TCHAR*) HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, (this->audiodevlist.devlist_n_entries)*(AudioPB::AUDIODEVICELIST_ENTRYLENGTH)*sizeof(TCHAR));
	if(this->audiodevlist.p_devlist == NULL)
	{
		this->audiodevicelist_deinit();
		this->status = AudioPB::STATUS_ERROR_MEMORY;
		this->err_msg = TEXT("AudioPB::audiodevicelist_init: Error: HeapAlloc failed.");
		return FALSE;
	}

	return TRUE;
}

BOOL WINAPI AudioPB::audiodevicelist_deinit(VOID)
{
	if(p_processheap == NULL)
	{
		this->status = AudioPB::STATUS_ERROR_MEMORY;
		this->err_msg = TEXT("AudioPB::audiodevicelist_deinit: Error: p_processheap is NULL.");
		return FALSE;
	}

	if(this->audiodevlist.p_devlist != NULL)
	{
		if(!HeapFree(p_processheap, 0u, this->audiodevlist.p_devlist))
		{
			this->status = AudioPB::STATUS_ERROR_MEMORY;
			this->err_msg = TEXT("AudioPB::audiodevicelist_deinit: Error: HeapFree failed.");
			return FALSE;
		}

		this->audiodevlist.p_devlist = NULL;
	}

	this->audiodevlist.devlist_n_entries = 0u;

	if(this->audiodevlist.p_devcoll != NULL)
	{
		this->audiodevlist.p_devcoll->Release();
		this->audiodevlist.p_devcoll = NULL;
	}

	return TRUE;
}

VOID WINAPI AudioPB::playback_proc(VOID)
{
	this->playback_init();
	this->playback_loop();

	this->audiodev.p_audioclient->Stop();
	return;
}

VOID WINAPI AudioPB::playback_init(VOID)
{
	BYTE *_p_audiobuffer = NULL;
	ULONG_PTR _audiobuffer_size_bytes;

	*((ULONG64*) &(this->filein_pos_64)) = this->AUDIO_DATA_BEGIN;

	this->streambuffer_nseg_load = 0u;
	this->streambuffer_nseg_play = (this->STREAMBUFFER_N_SEGMENTS)/2u;

	_audiobuffer_size_bytes = (this->AUDIOBUFFER_SIZE_FRAMES)*(this->N_CHANNELS)*(this->AUDIO_BYTES_PER_SAMPLE);

	((IAudioRenderClient*) (this->audiodev.p_audioservice))->GetBuffer((UINT32) this->AUDIOBUFFER_SIZE_FRAMES, &_p_audiobuffer);
	
	ZeroMemory(_p_audiobuffer, _audiobuffer_size_bytes);

	((IAudioRenderClient*) (this->audiodev.p_audioservice))->ReleaseBuffer((UINT32) this->AUDIOBUFFER_SIZE_FRAMES, 0u);

	this->status = AudioPB::STATUS_RUNNING;
	this->audiodev.p_audioclient->Start();

	this->audiodevice_wait();
	return;
}

VOID WINAPI AudioPB::playback_loop(VOID)
{
	while(TRUE)
	{
		if((this->status < AudioPB::STATUS_READY) || (this->status == AudioPB::STATUS_STOPPED)) break;

		if(this->status == AudioPB::STATUS_PAUSED)
		{
			Sleep(1u);
			continue;
		}

		this->buffer_play();
		this->buffer_load();

		this->streambuffer_nseg_update();
		this->audiodevice_wait();
	}

	return;
}

VOID WINAPI AudioPB::streambuffer_nseg_update(VOID)
{
	this->streambuffer_nseg_load++;
	this->streambuffer_nseg_load %= this->STREAMBUFFER_N_SEGMENTS;

	this->streambuffer_nseg_play++;
	this->streambuffer_nseg_play %= this->STREAMBUFFER_N_SEGMENTS;

	return;
}

VOID WINAPI AudioPB::buffer_play(VOID)
{
	VOID *_p_output = NULL;
	BYTE *_p_audiobuffer = NULL;
	HRESULT _n_ret;

	if(this->status != AudioPB::STATUS_RUNNING) return;

	_p_output = (VOID*) (((ULONG_PTR) (this->p_streambuffer)) + (this->streambuffer_nseg_play)*(this->STREAMBUFFER_SEGMENT_SIZE_BYTES));

	_n_ret = ((IAudioRenderClient*) (this->audiodev.p_audioservice))->GetBuffer((UINT32) this->STREAMBUFFER_SEGMENT_SIZE_FRAMES, &_p_audiobuffer);
	if(_n_ret != S_OK) app_exit((UINT) -1, TEXT("CRITICAL ERROR OCCURRED: AudioPB::buffer_play: Error: IAudioRenderClient::GetBuffer failed."));

	CopyMemory(_p_audiobuffer, _p_output, this->STREAMBUFFER_SEGMENT_SIZE_BYTES);

	_n_ret = ((IAudioRenderClient*) (this->audiodev.p_audioservice))->ReleaseBuffer((UINT32) this->STREAMBUFFER_SEGMENT_SIZE_FRAMES, 0u);
	if(_n_ret != S_OK) app_exit((UINT) -1, TEXT("CRITICAL ERROR OCCURRED: AudioPB::buffer_play: Error: IAudioRenderClient::ReleaseBuffer failed."));

	return;
}

VOID WINAPI AudioPB::audiodevice_wait(VOID)
{
	ULONG_PTR _n_frames_free;
	UINT32 _u32;

	do{
		this->audiodev.p_audioclient->GetCurrentPadding(&_u32);
		_n_frames_free = this->AUDIOBUFFER_SIZE_FRAMES - ((ULONG_PTR) _u32);

		Sleep(1u);
	}while(_n_frames_free < this->STREAMBUFFER_SEGMENT_SIZE_FRAMES);

	return;
}
