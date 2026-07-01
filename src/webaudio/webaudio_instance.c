#include "webaudio_internal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

QUARTZ_DEFINE_JS_SOURCE(js_webaudio_processor,
	class QuartzProcessor extends AudioWorkletProcessor {
		constructor()
		{
			super();
		}

		process(inputs, outputs)
		{
			return true;
		}
	}

	registerProcessor("quartz-processor", QuartzProcessor);
);

EM_ASYNC_JS(int, js_webaudio_createDevice, (const char *id, int id_size, const char *processor_source, int processor_source_size),
{
	assert(id);
	assert(Module);
	
	const options = {
		sinkId: UTF8ToString(id, id_size)
	};

	const processorSource = UTF8ToString(processor_source, processor_source_size);
	const blob = new Blob([processorSource], { type: "text/javascript" });
	const url = URL.createObjectURL(blob);

	const device = {};

	try
	{
		device.audio = new AudioContext(options);
		await device.audio.audioWorklet.addModule(url);
	}
	catch (e)
	{
		return -1;
	}
	finally
	{
		URL.revokeObjectURL(url);
	}

	const newId = Module.quartz.deviceCounter++;
	Module.quartz.devices.set(newId, device);
	return newId;
});

EM_ASYNC_JS(int, webaudio_getDefaultDeviceInfoSync, (Quartz_DeviceType type, WebAudio_DeviceInfo *info),
{
	assert(info);
	const kind = (type === 0) ? "audiooutput" : "audioinput";

	if (!navigator || !navigator.mediaDevices)
		return -1;

	try
	{
		const devices = await navigator.mediaDevices.enumerateDevices();
		const reduced = devices.filter(d => d.kind === kind);

		let index = 0;
		
		if (reduced.length > 1)
			index = reduced.findIndex(d => d.deviceId === "default");

		if (index < 0 || index >= reduced.length)
			return -2;

		stringToUTF8(reduced[index].deviceId, info, 256);
		stringToUTF8(reduced[index].label, info + 256, 256);
		return index;
	}
	catch (e)
	{
		return -1;
	}
});

EM_ASYNC_JS(int, webaudio_getDeviceInfoSync, (Quartz_DeviceType type, int index, WebAudio_DeviceInfo *info),
{
	assert(info);
	const kind = (type === 0) ? "audiooutput" : "audioinput";

	if (!navigator || !navigator.mediaDevices)
		return -1;

	try
	{
		const devices = await navigator.mediaDevices.enumerateDevices();
		const reduced = devices.filter(d => d.kind === kind);

		if (index < 0 || index >= reduced.length)
			return -2;

		stringToUTF8(reduced[index].deviceId, info, 256);
		stringToUTF8(reduced[index].label, info + 256, 256);
		return index;
	}
	catch (e)
	{
		console.log(e);
		return -1;
	}
});

EM_ASYNC_JS(int, js_webaudio_enumerateDevicesSync, (Quartz_DeviceType type, Quartz_DeviceInfo *infos, int stride),
{
	const kind = (type === 0) ? "audiooutput" : "audioinput";

	if (!navigator || !navigator.mediaDevices)
		return -1;

	try
	{
		const devices = await navigator.mediaDevices.enumerateDevices();
		const reduced = devices.filter(d => d.kind === kind);

		if (infos)
		{
			for (var i = 0; i < reduced.length; ++i)
			{
				const device = reduced[i];
				const dst = infos + stride * i;
				stringToUTF8(device.label, dst, 256);
			}
		}
		return reduced.length;
	}
	catch (e)
	{
		return -1;
	}
});

static Quartz_Result webaudio_initialize()
{
	static int once = 0;

	if (!once)
	{
		EM_ASM(
		{
			assert(Module);

			Module.quartz = {};
			Module.quartz.deviceCounter = 0;
			Module.quartz.devices = new Map();
		});

		once = 1;
	}

	return QUARTZ_SUCCESS;
}

/*
 */
static Quartz_Result webaudio_instanceEnumerateDevices(Quartz_Instance this, Quartz_DeviceType type, uint32_t *device_count, Quartz_DeviceInfo *infos)
{
	assert(this);
	assert(device_count);

	QUARTZ_UNUSED(this);

	int result = js_webaudio_enumerateDevicesSync(type, infos, sizeof(Quartz_DeviceInfo));
	if (result < 0)
		return QUARTZ_WEBAUDIO_ERROR;

	if (infos)
	{
		for (int i = 0; i < result; ++i)
		{
			Quartz_DeviceInfo *info = &infos[i];
			info->api = QUARTZ_API_WASAPI;
			info->type = type;
		}
	}

	*device_count = result;
	return QUARTZ_SUCCESS;
}

static Quartz_Result webaudio_instanceCreateDevice(Quartz_Instance this, Quartz_DeviceType type, uint32_t index, Quartz_Device *device)
{
	assert(this);
	assert(device);

	WebAudio_Instance *instance_ptr = (WebAudio_Instance *)this;
	WebAudio_DeviceInfo webaudio_info = {0};

	int result = webaudio_getDeviceInfoSync(type, index, &webaudio_info);
	if (result == -1)
		return QUARTZ_WEBAUDIO_ERROR;

	if (result == -2)
		return QUARTZ_INVALID_DEVICE_INDEX;

	assert((uint32_t)result == index);

	int javascript_result = js_webaudio_createDevice(webaudio_info.id, 256, js_webaudio_processor, js_webaudio_processor_size);
	if (javascript_result < 0)
		return QUARTZ_WEBAUDIO_ERROR;

	WebAudio_Device *device_ptr = (WebAudio_Device *)malloc(sizeof(WebAudio_Device));
	assert(device_ptr);

	uint32_t id = (uint32_t)javascript_result;
	Quartz_Result quartz_result = webaudio_deviceInitialize(device_ptr, instance_ptr, type, &webaudio_info, id);
	if (quartz_result != QUARTZ_SUCCESS)
	{
		device_ptr->vtbl->destroyDevice((Quartz_Device)device_ptr);
		return quartz_result;
	}

	*device = (Quartz_Device)device_ptr;
	return QUARTZ_SUCCESS;
}

static Quartz_Result webaudio_instanceCreateDefaultDevice(Quartz_Instance this, Quartz_DeviceType type, Quartz_Device *device)
{
	assert(this);
	assert(device);

	WebAudio_Instance *instance_ptr = (WebAudio_Instance *)this;
	WebAudio_DeviceInfo webaudio_info = {0};

	int result = webaudio_getDefaultDeviceInfoSync(type, &webaudio_info);
	if (result == -1)
		return QUARTZ_WEBAUDIO_ERROR;

	if (result == -2)
		return QUARTZ_INVALID_DEVICE_INDEX;

	int javascript_result = js_webaudio_createDevice(webaudio_info.id, 256, js_webaudio_processor, js_webaudio_processor_size);
	if (javascript_result < 0)
		return QUARTZ_WEBAUDIO_ERROR;

	WebAudio_Device *device_ptr = (WebAudio_Device *)malloc(sizeof(WebAudio_Device));
	assert(device_ptr);

	uint32_t id = (uint32_t)javascript_result;
	Quartz_Result quartz_result = webaudio_deviceInitialize(device_ptr, instance_ptr, type, &webaudio_info, id);
	if (quartz_result != QUARTZ_SUCCESS)
	{
		device_ptr->vtbl->destroyDevice((Quartz_Device)device_ptr);
		return quartz_result;
	}

	*device = (Quartz_Device)device_ptr;
	return QUARTZ_SUCCESS;
}

static Quartz_Result webaudio_instanceDestroy(Quartz_Instance this)
{
	assert(this);

	WebAudio_Instance *ptr = (WebAudio_Instance *)this;

	free(ptr);
	return QUARTZ_SUCCESS;
}

/*
 */
static Quartz_InstanceTable instance_vtbl =
{
	webaudio_instanceEnumerateDevices,
	webaudio_instanceCreateDevice,
	webaudio_instanceCreateDefaultDevice,

	webaudio_instanceDestroy,
};

Quartz_Result webaudio_quartzCreateInstance(const Quartz_InstanceDesc *desc, Quartz_Instance *instance)
{
	assert(desc);
	assert(instance);

	QUARTZ_UNUSED(desc);

	Quartz_Result quartz_result = webaudio_initialize();
	if (quartz_result != QUARTZ_SUCCESS)
		return quartz_result;

	WebAudio_Instance *ptr = (WebAudio_Instance *)malloc(sizeof(WebAudio_Instance));
	assert(ptr);

	// vtable
	ptr->vtbl = &instance_vtbl;

	*instance = (Quartz_Instance)ptr;
	return QUARTZ_SUCCESS;
}
