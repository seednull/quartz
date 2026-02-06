#include <quartz.h>

#define _USE_MATH_DEFINES
#include <cmath>
#include <cassert>

#include <iostream>

#define INT24_MAX 8388607i32

static void testBuffers(Quartz_Device device)
{
	Quartz_Buffer buffer = QUARTZ_NULL_HANDLE;

	Quartz_DeviceFormat preferred_format = {};
	Quartz_Result result = quartzGetPreferredFormat(device, &preferred_format);
	assert(result == QUARTZ_SUCCESS);

	Quartz_BufferDesc desc =
	{
		preferred_format,
		50,
	};

	result = quartzCreateBuffer(device, &desc, &buffer);
	assert(result == QUARTZ_SUCCESS);

	result = quartzStart(device, buffer);
	assert(result == QUARTZ_SUCCESS);

	void *frames = nullptr;
	uint32_t frame_count = 0;

	double phase1 = 0.0;
	double phase2 = 0.0;
	const double freq1 = 440.0;
	const double freq2 = 441.0;
	const double amp = 0.5;
	const double inc1 = 2.0 * M_PI * freq1 / preferred_format.sample_rate;
	const double inc2 = 2.0 * M_PI * freq2 / preferred_format.sample_rate;

	while (true)
	{
		result = quartzBeginRender(device, buffer, &frames, &frame_count);
		assert(result == QUARTZ_SUCCESS);

		if (frame_count > 0)
			std::cout << "Rendering " << frame_count << " frames\n";

		for (uint32_t i = 0; i < frame_count; ++i)
		{
			float osc1 = static_cast<float>(sin(phase1) * amp);
			float osc2 = static_cast<float>(sin(phase2) * amp);

			phase1 = fmod(phase1 + inc1, 2.0 * M_PI);
			phase2 = fmod(phase2 + inc2, 2.0 * M_PI);

			switch (preferred_format.sample_format)
			{
				case QUARTZ_SAMPLE_FORMAT_UINT8:
				{
					uint8_t *typed_frames = reinterpret_cast<uint8_t *>(frames);

					typed_frames[2 * i + 0] = static_cast<uint8_t>((osc1 * 0.5f + 0.5f) * UINT8_MAX);
					typed_frames[2 * i + 1] = static_cast<uint8_t>((osc2 * 0.5f + 0.5f) * UINT8_MAX);
				}
				break;

				case QUARTZ_SAMPLE_FORMAT_SINT16:
				{
					int16_t *typed_frames = reinterpret_cast<int16_t *>(frames);

					typed_frames[2 * i + 0] = static_cast<int16_t>(osc1 * INT16_MAX);
					typed_frames[2 * i + 1] = static_cast<int16_t>(osc2 * INT16_MAX);
				}
				break;

				case QUARTZ_SAMPLE_FORMAT_SINT24:
				{
					int32_t *typed_frames = reinterpret_cast<int32_t *>(frames);

					typed_frames[2 * i + 0] = static_cast<int32_t>(osc1 * INT24_MAX);
					typed_frames[2 * i + 1] = static_cast<int32_t>(osc2 * INT24_MAX);
				}
				break;

				case QUARTZ_SAMPLE_FORMAT_SINT32:
				{
					int32_t *typed_frames = reinterpret_cast<int32_t *>(frames);

					typed_frames[2 * i + 0] = static_cast<int32_t>(osc1 * INT32_MAX);
					typed_frames[2 * i + 1] = static_cast<int32_t>(osc2 * INT32_MAX);
				}
				break;

				case QUARTZ_SAMPLE_FORMAT_FLOAT32:
				{
					float *typed_frames = reinterpret_cast<float *>(frames);

					typed_frames[2 * i + 0] = osc1;
					typed_frames[2 * i + 1] = osc2;
				}
				break;
			}
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
