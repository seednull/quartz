#include "wasapi_internal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <functiondiscoverykeys_devpkey.h>

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
