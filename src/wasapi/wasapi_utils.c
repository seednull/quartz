#include "wasapi_internal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <functiondiscoverykeys_devpkey.h>

static GUID subtype_pcm = { STATIC_KSDATAFORMAT_SUBTYPE_PCM };
static GUID subtype_ieee_float = { STATIC_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT };

EDataFlow wasapi_helperToDataFlow(Quartz_DeviceType type)
{
	static EDataFlow wasapi_flows[] =
	{
		eRender,
		eCapture,
	};

	return wasapi_flows[type];
}

uint32_t wasapi_helperToContainerBitDepth(Quartz_SampleFormat format)
{
	static uint32_t wasapi_bit_depths[] =
	{
		0,
		8,
		16,
		32,
		32,
		32,
	};

	return wasapi_bit_depths[format];
}

uint32_t wasapi_helperToActualBitDepth(Quartz_SampleFormat format)
{
	static uint32_t wasapi_bit_depths[] =
	{
		0,
		8,
		16,
		24,
		32,
		32,
	};

	return wasapi_bit_depths[format];
}

DWORD wasapi_helperToSpeakerMask(Quartz_ChannelMapping mapping)
{
	static DWORD speakers[] =
	{
		// Unknown
		0,

		// Common
		SPEAKER_FRONT_LEFT,
		SPEAKER_FRONT_RIGHT,
		SPEAKER_FRONT_CENTER,

		SPEAKER_LOW_FREQUENCY,

		SPEAKER_BACK_LEFT,
		SPEAKER_BACK_RIGHT,
		SPEAKER_FRONT_LEFT_OF_CENTER,
		SPEAKER_FRONT_RIGHT_OF_CENTER,
		SPEAKER_BACK_CENTER,

		SPEAKER_SIDE_LEFT,
		SPEAKER_SIDE_RIGHT,

		SPEAKER_TOP_CENTER,
		SPEAKER_TOP_FRONT_LEFT,
		SPEAKER_TOP_FRONT_RIGHT,
		SPEAKER_TOP_FRONT_CENTER,
		SPEAKER_TOP_BACK_LEFT,
		SPEAKER_TOP_BACK_RIGHT,
		SPEAKER_TOP_BACK_CENTER,

		// Aux
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
	};

	return speakers[mapping];
}

GUID wasapi_helperToSubFormat(Quartz_SampleFormat format)
{
	static GUID subformats[] =
	{
		{0},
		STATIC_KSDATAFORMAT_SUBTYPE_PCM,
		STATIC_KSDATAFORMAT_SUBTYPE_PCM,
		STATIC_KSDATAFORMAT_SUBTYPE_PCM,
		STATIC_KSDATAFORMAT_SUBTYPE_PCM,
		STATIC_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT,
	};

	return subformats[format];
}

Quartz_DeviceFormat wasapi_helperToDeviceFormat(const WAVEFORMATEXTENSIBLE *format)
{
	assert(format);

	typedef struct WASAPI_Channel_t
	{
		DWORD speaker;
		Quartz_ChannelMapping mapping;
	} WASAPI_Channel;

	static WASAPI_Channel channel_map[] =
	{
		{ SPEAKER_FRONT_LEFT, QUARTZ_CHANNEL_MAPPING_FRONT_LEFT },
		{ SPEAKER_FRONT_RIGHT, QUARTZ_CHANNEL_MAPPING_FRONT_RIGHT },
		{ SPEAKER_FRONT_CENTER, QUARTZ_CHANNEL_MAPPING_FRONT_CENTER },
		{ SPEAKER_LOW_FREQUENCY, QUARTZ_CHANNEL_MAPPING_LFE },
		{ SPEAKER_BACK_LEFT, QUARTZ_CHANNEL_MAPPING_BACK_LEFT },
		{ SPEAKER_BACK_RIGHT, QUARTZ_CHANNEL_MAPPING_BACK_RIGHT },
		{ SPEAKER_FRONT_LEFT_OF_CENTER, QUARTZ_CHANNEL_MAPPING_FRONT_LEFT_OF_CENTER },
		{ SPEAKER_FRONT_RIGHT_OF_CENTER, QUARTZ_CHANNEL_MAPPING_FRONT_RIGHT_OF_CENTER },
		{ SPEAKER_BACK_CENTER, QUARTZ_CHANNEL_MAPPING_BACK_CENTER },
		{ SPEAKER_SIDE_LEFT, QUARTZ_CHANNEL_MAPPING_SIDE_LEFT },
		{ SPEAKER_SIDE_RIGHT, QUARTZ_CHANNEL_MAPPING_SIDE_RIGHT },
		{ SPEAKER_TOP_CENTER, QUARTZ_CHANNEL_MAPPING_TOP_CENTER },
		{ SPEAKER_TOP_FRONT_LEFT, QUARTZ_CHANNEL_MAPPING_TOP_FRONT_LEFT },
		{ SPEAKER_TOP_FRONT_CENTER, QUARTZ_CHANNEL_MAPPING_TOP_FRONT_CENTER },
		{ SPEAKER_TOP_FRONT_RIGHT, QUARTZ_CHANNEL_MAPPING_TOP_FRONT_RIGHT },
		{ SPEAKER_TOP_BACK_LEFT, QUARTZ_CHANNEL_MAPPING_TOP_BACK_LEFT },
		{ SPEAKER_TOP_BACK_CENTER, QUARTZ_CHANNEL_MAPPING_TOP_BACK_CENTER },
		{ SPEAKER_TOP_BACK_RIGHT, QUARTZ_CHANNEL_MAPPING_TOP_BACK_RIGHT },
	};

	Quartz_DeviceFormat result = {0};
	result.sample_rate = format->Format.nSamplesPerSec;
	result.channel_count = format->Format.nChannels;

	WORD bit_depth = format->Format.wBitsPerSample;
	WORD tag = format->Format.wFormatTag;

	if (tag == WAVE_FORMAT_EXTENSIBLE)
	{
		bit_depth = format->Samples.wValidBitsPerSample;

		if (IsEqualGUID(&format->SubFormat, &subtype_pcm))
			tag = WAVE_FORMAT_PCM;

		if (IsEqualGUID(&format->SubFormat, &subtype_ieee_float))
			tag = WAVE_FORMAT_IEEE_FLOAT;

		DWORD mask = format->dwChannelMask;
		uint32_t index = 0;
		for (uint32_t i = 0; i < ARRAYSIZE(channel_map); ++i)
		{
			if (mask & channel_map[i].speaker)
			{
				result.channel_mappings[index++] = channel_map[i].mapping;
			}
		}

		assert(index <= result.channel_count);
	}

	if (tag == WAVE_FORMAT_PCM)
	{
		switch (bit_depth)
		{
			case 8: result.sample_format = QUARTZ_SAMPLE_FORMAT_UINT8; break;
			case 16: result.sample_format = QUARTZ_SAMPLE_FORMAT_SINT16; break;
			case 24: result.sample_format = QUARTZ_SAMPLE_FORMAT_SINT24; break;
			case 32: result.sample_format = QUARTZ_SAMPLE_FORMAT_SINT32; break;
			default: result.sample_format = QUARTZ_SAMPLE_FORMAT_UNKNOWN; break;
		}
	}
	else if (tag == WAVE_FORMAT_IEEE_FLOAT)
	{
		switch (bit_depth)
		{
			case 32: result.sample_format = QUARTZ_SAMPLE_FORMAT_FLOAT32; break;
			default: result.sample_format = QUARTZ_SAMPLE_FORMAT_UNKNOWN; break;
		}
	}

	return result;
}

WAVEFORMATEXTENSIBLE wasapi_helperToWaveFormatExtensible(const Quartz_DeviceFormat *format)
{
	assert(format);

	uint32_t container_bit_depth = wasapi_helperToContainerBitDepth(format->sample_format);
	uint32_t audio_frame_size = format->channel_count * container_bit_depth / 8;

	WAVEFORMATEXTENSIBLE result = {0};
	result.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	result.Format.nChannels = (WORD)format->channel_count;
	result.Format.nSamplesPerSec = format->sample_rate;
	result.Format.nAvgBytesPerSec = format->sample_rate * audio_frame_size;
	result.Format.nBlockAlign = (WORD)audio_frame_size;
	result.Format.wBitsPerSample = (WORD)container_bit_depth;
	result.Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);

	result.Samples.wValidBitsPerSample = (WORD)wasapi_helperToActualBitDepth(format->sample_format);
	for (uint32_t i = 0; i < format->channel_count; ++i)
		result.dwChannelMask |= wasapi_helperToSpeakerMask(format->channel_mappings[i]);

	result.SubFormat = wasapi_helperToSubFormat(format->sample_format);

	return result;
}

Quartz_Result wasapi_helperFillDeviceInfo(IMMDevice *device, Quartz_DeviceType type, Quartz_DeviceInfo *info)
{
	assert(device);
	assert(info);

	IPropertyStore *store = NULL;
	HRESULT hr = IMMDevice_OpenPropertyStore(device, STGM_READ, &store);
	if (!SUCCEEDED(hr))
	{
		IMMDevice_Release(device);
		return QUARTZ_WASAPI_ERROR;
	}

	PROPVARIANT friendly_name = {0};
	PropVariantInit(&friendly_name);

	hr = IPropertyStore_GetValue(store, &PKEY_Device_FriendlyName, &friendly_name);
	if (!SUCCEEDED(hr))
	{
		IPropertyStore_Release(store);
		return QUARTZ_WASAPI_ERROR;
	}

	memset(info, 0, sizeof(Quartz_DeviceInfo));
	info->api = QUARTZ_API_WASAPI;
	info->type = type;

	if (friendly_name.vt != VT_EMPTY)
		WideCharToMultiByte(CP_UTF8, 0, friendly_name.pwszVal, -1, info->name, 256, NULL, NULL);

	PropVariantClear(&friendly_name);
	IPropertyStore_Release(store);

	return QUARTZ_SUCCESS;
}
