#include <quartz.h>
#include <cassert>
#include <iostream>

static const char *device_types[] =
{
	"Render",
	"Capture",
};

static void printDeviceInfo(const Quartz_DeviceInfo *info)
{
	std::cout << "Device name: " << info->name << "\n";
	std::cout << "\n";
}

static void testEnumerateDevices(Quartz_Instance instance, Quartz_DeviceType type)
{
	uint32_t device_count = 0;
	Quartz_Result result = quartzEnumerateDevices(instance, type, &device_count, nullptr);
	if (result != QUARTZ_SUCCESS)
		return;

	Quartz_DeviceInfo *infos = (Quartz_DeviceInfo *)malloc(sizeof(Quartz_DeviceInfo) * device_count);
	result = quartzEnumerateDevices(instance, type, &device_count, infos);
	assert(result == QUARTZ_SUCCESS);

	for (uint32_t i = 0; i < device_count; ++i)
	{
		std::cout << " ------ [Device " << i << "] ------\n";
		printDeviceInfo(&infos[i]);
	}

	free(infos);
}

static void testCreateDevice(Quartz_Instance instance, Quartz_DeviceType type, uint32_t index)
{
	Quartz_Device device = QUARTZ_NULL_HANDLE;
	Quartz_Result result = quartzCreateDevice(instance, type, index, &device);
	if (result != QUARTZ_SUCCESS)
		return;

	Quartz_DeviceInfo info = {};
	result = quartzGetDeviceInfo(device, &info);
	assert(result == QUARTZ_SUCCESS);
	std::cout << " ------ [CreateByIndex " << index << "] ------\n";
	printDeviceInfo(&info);

	result = quartzDestroyDevice(device);
	assert(result == QUARTZ_SUCCESS);
}

static void testCreateDefaultDevice(Quartz_Instance instance, Quartz_DeviceType type)
{
	Quartz_Device device = QUARTZ_NULL_HANDLE;
	Quartz_Result result = quartzCreateDefaultDevice(instance, type, &device);
	if (result != QUARTZ_SUCCESS)
		return;

	Quartz_DeviceInfo info = {};
	result = quartzGetDeviceInfo(device, &info);
	assert(result == QUARTZ_SUCCESS);
	std::cout << " ------ [CreateDefault] ------\n";
	printDeviceInfo(&info);

	result = quartzDestroyDevice(device);
	assert(result == QUARTZ_SUCCESS);
}

int main()
{
	Quartz_Instance instance = QUARTZ_NULL_HANDLE;

	Quartz_InstanceDesc instance_desc =
	{
		"01_enumerate_devices",
		"Quartz",
		0,
		0,
	};

	Quartz_Result result = quartzCreateInstance(QUARTZ_API_WASAPI, &instance_desc, &instance);
	assert(result == QUARTZ_SUCCESS);

	Quartz_DeviceType types[] =
	{
		QUARTZ_DEVICE_TYPE_RENDER,
		QUARTZ_DEVICE_TYPE_CAPTURE,
	};

	const size_t count = std::size(types);

	for (size_t i = 0; i < count; ++i)
	{
		Quartz_DeviceType type = types[i];

		std::cout << " ------ [" << device_types[type] << " devices] ------\n";
		testEnumerateDevices(instance, type);
		testCreateDevice(instance, type, 0);
		testCreateDefaultDevice(instance, type);
	}

	result = quartzDestroyInstance(instance);
	assert(result == QUARTZ_SUCCESS);

	return 0;
}
