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

uint32_t wasapi_helperToBitDepth(Quartz_SampleFormat format)
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
