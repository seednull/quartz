#include "wasapi_internal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/*
 */
static Quartz_Result wasapi_instanceEnumerateDevices(Quartz_Instance this, Quartz_DeviceType type, uint32_t *device_count, Quartz_DeviceInfo *infos)
{
	assert(this);
	assert(device_count);

	WASAPI_Instance *instance_ptr = (WASAPI_Instance *)this;
	IMMDeviceEnumerator *wasapi_enumerator = instance_ptr->enumerator;
	assert(wasapi_enumerator);

	IMMDeviceCollection *collection = NULL;

	EDataFlow data_flow = wasapi_helperToDataFlow(type);
	DWORD state_mask = DEVICE_STATE_ACTIVE | DEVICE_STATE_DISABLED;

	HRESULT hr = IMMDeviceEnumerator_EnumAudioEndpoints(wasapi_enumerator, data_flow, state_mask, &collection);
	if (!SUCCEEDED(hr))
		return QUARTZ_WASAPI_ERROR;

	UINT count = 0;
	hr = IMMDeviceCollection_GetCount(collection, &count);
	if (!SUCCEEDED(hr))
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
			hr = IMMDeviceCollection_Item(collection, i, &device);
			if (!SUCCEEDED(hr))
			{
				IMMDeviceCollection_Release(collection);
				return QUARTZ_WASAPI_ERROR;
			}

			Quartz_Result quartz_result = wasapi_helperFillDeviceInfo(device, type, &infos[i]);
			IMMDevice_Release(device);

			if (quartz_result != QUARTZ_SUCCESS)
			{
				IMMDeviceCollection_Release(collection);
				return QUARTZ_WASAPI_ERROR;
			}
		}
	}

	*device_count = (uint32_t)count;

	IMMDeviceCollection_Release(collection);
	return QUARTZ_SUCCESS;
}

static Quartz_Result wasapi_instanceCreateDevice(Quartz_Instance this, Quartz_DeviceType type, uint32_t index, Quartz_Device *device)
{
	assert(this);
	assert(device);

	WASAPI_Instance *instance_ptr = (WASAPI_Instance *)this;
	IMMDeviceEnumerator *wasapi_enumerator = instance_ptr->enumerator;
	assert(wasapi_enumerator);

	IMMDeviceCollection *collection = NULL;

	EDataFlow data_flow = wasapi_helperToDataFlow(type);
	DWORD state_mask = DEVICE_STATE_ACTIVE | DEVICE_STATE_DISABLED;

	HRESULT hr = IMMDeviceEnumerator_EnumAudioEndpoints(wasapi_enumerator, data_flow, state_mask, &collection);
	if (!SUCCEEDED(hr))
		return QUARTZ_WASAPI_ERROR;

	UINT count = 0;
	hr = IMMDeviceCollection_GetCount(collection, &count);
	if (!SUCCEEDED(hr))
	{
		IMMDeviceCollection_Release(collection);
		return QUARTZ_WASAPI_ERROR;
	}

	if (index >= count)
	{
		IMMDeviceCollection_Release(collection);
		return QUARTZ_INVALID_DEVICE_INDEX;
	}

	IMMDevice *wasapi_device = NULL;
	hr = IMMDeviceCollection_Item(collection, index, &wasapi_device);
	IMMDeviceCollection_Release(collection);

	if (!SUCCEEDED(hr))
		return QUARTZ_WASAPI_ERROR;

	WASAPI_Device *device_ptr = (WASAPI_Device *)malloc(sizeof(WASAPI_Device));
	assert(device_ptr);

	Quartz_Result quartz_result = wasapi_deviceInitialize(device_ptr, instance_ptr, wasapi_device, type);
	if (quartz_result != QUARTZ_SUCCESS)
	{
		device_ptr->vtbl->destroyDevice((Quartz_Device)device_ptr);
		return quartz_result;
	}

	*device = (Quartz_Device)device_ptr;
	return QUARTZ_SUCCESS;
}

static Quartz_Result wasapi_instanceCreateDefaultDevice(Quartz_Instance this, Quartz_DeviceType type, Quartz_Device *device)
{
	assert(this);
	assert(device);

	WASAPI_Instance *instance_ptr = (WASAPI_Instance *)this;
	IMMDeviceEnumerator *wasapi_enumerator = instance_ptr->enumerator;
	assert(wasapi_enumerator);

	EDataFlow data_flow = wasapi_helperToDataFlow(type);
	ERole role = eConsole;

	IMMDevice *wasapi_device = NULL;
	HRESULT hr = IMMDeviceEnumerator_GetDefaultAudioEndpoint(wasapi_enumerator, data_flow, role, &wasapi_device);
	if (!SUCCEEDED(hr))
		return QUARTZ_WASAPI_ERROR;

	WASAPI_Device *device_ptr = (WASAPI_Device *)malloc(sizeof(WASAPI_Device));
	assert(device_ptr);

	Quartz_Result quartz_result = wasapi_deviceInitialize(device_ptr, instance_ptr, wasapi_device, type);
	if (quartz_result != QUARTZ_SUCCESS)
	{
		device_ptr->vtbl->destroyDevice((Quartz_Device)device_ptr);
		return quartz_result;
	}

	*device = (Quartz_Device)device_ptr;
	return QUARTZ_SUCCESS;
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
	wasapi_instanceCreateDefaultDevice,

	wasapi_instanceDestroy,
};

/*
 */
Quartz_Result wasapi_createInstance(const Quartz_InstanceDesc *desc, Quartz_Instance *instance)
{
	assert(desc);
	assert(instance);

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (!SUCCEEDED(hr))
		return QUARTZ_WASAPI_ERROR;

	IMMDeviceEnumerator *enumerator = NULL;
	hr = CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, &IID_IMMDeviceEnumerator, &enumerator);
	if (!SUCCEEDED(hr))
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
