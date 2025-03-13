#include "wasapi_internal.h"

#include <functiondiscoverykeys_devpkey.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/*
 */
static Quartz_Result wasapi_instanceEnumerateDevices(Quartz_Instance this, uint32_t *device_count, Quartz_DeviceInfo *infos)
{
	assert(this);
	assert(device_count);

	WASAPI_Instance *instance_ptr = (WASAPI_Instance *)this;
	IMMDeviceEnumerator *wasapi_enumerator = instance_ptr->enumerator;
	assert(wasapi_enumerator);

	IMMDeviceCollection *collection = NULL;

	EDataFlow data_flow = eRender; // TODO: determine from input argument
	DWORD state_mask = DEVICE_STATE_ACTIVE | DEVICE_STATE_DISABLED;

	HRESULT result = IMMDeviceEnumerator_EnumAudioEndpoints(wasapi_enumerator, data_flow, state_mask, &collection);
	if (!SUCCEEDED(result))
		return QUARTZ_WASAPI_ERROR;

	UINT count = 0;
	result = IMMDeviceCollection_GetCount(collection, &count);
	if (!SUCCEEDED(result))
	{
		IMMDeviceCollection_Release(collection);
		return QUARTZ_WASAPI_ERROR;
	}

	if (infos != NULL)
	{
		memset(infos, 0, sizeof(Quartz_DeviceInfo) * count);
		for (UINT i = 0; i < count; ++i)
		{
			IMMDevice *device = NULL;
			result = IMMDeviceCollection_Item(collection, i, &device);
			if (!SUCCEEDED(result))
			{
				IMMDeviceCollection_Release(collection);
				return QUARTZ_WASAPI_ERROR;
			}

			IPropertyStore *store = NULL;
			result = IMMDevice_OpenPropertyStore(device, STGM_READ, &store);
			if (!SUCCEEDED(result))
			{
				IMMDevice_Release(device);
				IMMDeviceCollection_Release(collection);
				return QUARTZ_WASAPI_ERROR;
			}

			PROPVARIANT friendly_name = {0};
			PropVariantInit(&friendly_name);

			result = IPropertyStore_GetValue(store, &PKEY_Device_FriendlyName, &friendly_name);
			if (!SUCCEEDED(result))
			{
				IPropertyStore_Release(store);
				IMMDevice_Release(device);
				IMMDeviceCollection_Release(collection);
				return QUARTZ_WASAPI_ERROR;
			}

			Quartz_DeviceInfo *info = &infos[i];
			info->api = QUARTZ_API_WASAPI;

			if (friendly_name.vt != VT_EMPTY)
				WideCharToMultiByte(CP_UTF8, 0, friendly_name.pwszVal, -1, info->name, 256, NULL, NULL);

			PropVariantClear(&friendly_name);
			IPropertyStore_Release(store);
			IMMDevice_Release(device);
		}
	}

	*device_count = (uint32_t)count;

	IMMDeviceCollection_Release(collection);
	return QUARTZ_SUCCESS;
}

static Quartz_Result wasapi_instanceCreateDevice(Quartz_Instance this, uint32_t index, Quartz_Device *device)
{
	assert(this);
	assert(device);

	QUARTZ_UNUSED(this);
	QUARTZ_UNUSED(index);
	QUARTZ_UNUSED(device);

	return QUARTZ_NOT_SUPPORTED;
}

static Quartz_Result wasapi_instanceDestroy(Quartz_Instance this)
{
	assert(this);

	WASAPI_Instance *ptr = (WASAPI_Instance *)this;

	IMMDeviceEnumerator_Release(ptr->enumerator);
	free(ptr);

	CoUninitialize();

	return QUARTZ_SUCCESS;
}

/*
 */
static Quartz_InstanceTable instance_vtbl =
{
	wasapi_instanceEnumerateDevices,
	wasapi_instanceCreateDevice,
	wasapi_instanceDestroy,
};

/*
 */
Quartz_Result wasapi_createInstance(const Quartz_InstanceDesc *desc, Quartz_Instance *instance)
{
	assert(desc);
	assert(instance);

	HRESULT result = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (!SUCCEEDED(result))
		return QUARTZ_WASAPI_ERROR;

	IMMDeviceEnumerator *enumerator = NULL;
	result = CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, &IID_IMMDeviceEnumerator, &enumerator);
	if (!SUCCEEDED(result))
		return QUARTZ_WASAPI_ERROR;

	WASAPI_Instance *ptr = (WASAPI_Instance *)malloc(sizeof(WASAPI_Instance));
	assert(ptr);

	// vtable
	ptr->vtbl = &instance_vtbl;

	// info
	ptr->enumerator = enumerator;

	*instance = (Quartz_Instance)ptr;
	return QUARTZ_SUCCESS;
}
