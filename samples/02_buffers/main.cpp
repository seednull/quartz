#include <quartz.h>
#include <cassert>
#include <iostream>

static void testBuffers(Quartz_Device device)
{
	Quartz_Buffer buffer = QUARTZ_NULL_HANDLE;
	void *ptr = nullptr;

	Quartz_BufferDesc desc =
	{
		QUARTZ_SAMPLE_RATE_48000,
		QUARTZ_SAMPLE_FORMAT_FLOAT32,
		1000,
		2
	};

	Quartz_Result result = quartzCreateBuffer(device, &desc, &buffer);
	assert(result == QUARTZ_SUCCESS);

	result = quartzMapBuffer(device, buffer, &ptr);
	assert(result == QUARTZ_SUCCESS);

	result = quartzUnmapBuffer(device, buffer);
	assert(result == QUARTZ_SUCCESS);

	result = quartzDestroyBuffer(device, buffer);
	assert(result == QUARTZ_SUCCESS);
}

int main()
{
	Quartz_Instance instance = QUARTZ_NULL_HANDLE;

	Quartz_InstanceDesc instance_desc =
	{
		"02_buffers",
		"Quartz",
		0,
		0,
	};

	Quartz_Result result = quartzCreateInstance(QUARTZ_API_WASAPI, &instance_desc, &instance);
	assert(result == QUARTZ_SUCCESS);

	Quartz_Device device = QUARTZ_NULL_HANDLE;
	result = quartzCreateDefaultDevice(instance, QUARTZ_DEVICE_TYPE_PLAYBACK, &device);
	assert(result == QUARTZ_SUCCESS);

	testBuffers(device);

	result = quartzDestroyDevice(device);
	assert(result == QUARTZ_SUCCESS);

	result = quartzDestroyInstance(instance);
	assert(result == QUARTZ_SUCCESS);

	return 0;
}
