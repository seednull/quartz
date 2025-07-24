#include <quartz.h>

#define _USE_MATH_DEFINES
#include <cmath>
#include <cassert>

#include <iostream>

static void testBuffers(Quartz_Device device)
{
	Quartz_Buffer buffer = QUARTZ_NULL_HANDLE;

	Quartz_BufferDesc desc =
	{
		QUARTZ_SAMPLE_RATE_48000,
		QUARTZ_SAMPLE_FORMAT_SINT32,
		50,
		2
	};

	Quartz_Result result = quartzCreateBuffer(device, &desc, &buffer);
	assert(result == QUARTZ_SUCCESS);

	result = quartzStart(device, buffer);
	assert(result == QUARTZ_SUCCESS);

	int32_t *frames = nullptr;
	uint32_t frame_count = 0;

	double phase1 = 0.0;
	double phase2 = 0.0;
	const double freq1 = 440.0;
	const double freq2 = 441.0;
	const double amp = 0.5;
	const double inc1 = 2.0 * M_PI * freq1 / desc.sample_rate;
	const double inc2 = 2.0 * M_PI * freq2 / desc.sample_rate;

	while (true)
	{
		result = quartzBeginRender(device, buffer, reinterpret_cast<void **>(&frames), &frame_count);
		assert(result == QUARTZ_SUCCESS);

		if (frame_count > 0)
			std::cout << "Rendering " << frame_count << " frames\n";

		for (uint32_t i = 0; i < frame_count; ++i)
		{
			phase1 += inc1;
			phase2 += inc2;

			double osc1 = sin(phase1) * amp;
			double osc2 = sin(phase2) * amp;

			int32_t sample1 = static_cast<int32_t>(osc1 * INT32_MAX);
			int32_t sample2 = static_cast<int32_t>(osc2 * INT32_MAX);

			frames[2 * i + 0] = sample1;
			frames[2 * i + 1] = sample2;
		}

		result = quartzEndRender(device, buffer, frame_count);
		assert(result == QUARTZ_SUCCESS);
	}

	result = quartzStop(device, buffer);
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
	result = quartzCreateDefaultDevice(instance, QUARTZ_DEVICE_TYPE_RENDER, &device);
	assert(result == QUARTZ_SUCCESS);

	testBuffers(device);

	result = quartzDestroyDevice(device);
	assert(result == QUARTZ_SUCCESS);

	result = quartzDestroyInstance(instance);
	assert(result == QUARTZ_SUCCESS);

	return 0;
}
