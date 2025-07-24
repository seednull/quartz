#pragma once

#include <stdint.h> // TODO: get rid of this dependency later

// Version
#define QUARTZ_VERSION_MAJOR 1
#define QUARTZ_VERSION_MINOR 0
#define QUARTZ_VERSION_PATCH 0
#define QUARTZ_VERSION "1.0.0-dev"

// Platform specific defines
#if defined(_WIN32)
	#define QUARTZ_EXPORT		__declspec(dllexport)
	#define QUARTZ_IMPORT		__declspec(dllimport)
	#define QUARTZ_INLINE		__forceinline
	#define QUARTZ_RESTRICT		__restrict
#else
	#define QUARTZ_EXPORT		__attribute__((visibility("default")))
	#define QUARTZ_IMPORT
	#define QUARTZ_INLINE		__inline__
	#define QUARTZ_RESTRICT		__restrict
#endif

#if defined(QUARTZ_SHARED_LIBRARY)
	#define QUARTZ_APIENTRY QUARTZ_EXPORT extern
#else
	#define QUARTZ_APIENTRY QUARTZ_IMPORT extern
#endif

#if !defined(QUARTZ_NULL_HANDLE)
	#define QUARTZ_NULL_HANDLE 0
#endif

#define QUARTZ_DEFINE_HANDLE(TYPE) typedef uint64_t TYPE;

#ifdef __cplusplus
extern "C" {
#endif

// Constants

// Opaque handles
QUARTZ_DEFINE_HANDLE(Quartz_Instance);
QUARTZ_DEFINE_HANDLE(Quartz_Device);
QUARTZ_DEFINE_HANDLE(Quartz_Buffer);

// Enums
typedef enum Quartz_Result_t
{
	QUARTZ_SUCCESS = 0,
	QUARTZ_NOT_SUPPORTED,
	QUARTZ_INVALID_OUTPUT_ARGUMENT,
	QUARTZ_INVALID_INSTANCE,
	QUARTZ_INVALID_DEVICE,
	QUARTZ_INVALID_DEVICE_INDEX,
	QUARTZ_INVALID_BUFFER_FORMAT,

	// FIXME: add more error codes for internal errors
	QUARTZ_INTERNAL_ERROR,

	// FIXME: add more error codes for wasapi stuff
	QUARTZ_WASAPI_ERROR,

	QUARTZ_RESULT_ENUM_MAX,
	QUARTZ_RESULT_ENUM_FORCE32 = 0x7FFFFFFF,
} Quartz_Result;

typedef enum Quartz_Api_t
{
	QUARTZ_API_WASAPI = 0,
	QUARTZ_API_DIRECTSOUND,
	QUARTZ_API_NULL,

	QUARTZ_API_AUTO,

	QUARTZ_API_ENUM_MAX,
	QUARTZ_API_ENUM_FORCE32 = 0x7FFFFFFF,
} Quartz_Api;

typedef enum Quartz_DeviceType_t
{
	QUARTZ_DEVICE_TYPE_RENDER = 0,
	QUARTZ_DEVICE_TYPE_CAPTURE,

	QUARTZ_DEVICE_TYPE_ENUM_MAX,
	QUARTZ_DEVICE_TYPE_ENUM_FORCE32 = 0x7FFFFFFF,
} Quartz_DeviceType;

typedef enum Quartz_SampleFormat_t
{
	QUARTZ_SAMPLE_FORMAT_UNKNOWN = 0,
	QUARTZ_SAMPLE_FORMAT_UINT8,
	QUARTZ_SAMPLE_FORMAT_SINT16,
	QUARTZ_SAMPLE_FORMAT_SINT24,
	QUARTZ_SAMPLE_FORMAT_SINT32,
	QUARTZ_SAMPLE_FORMAT_FLOAT32,

	QUARTZ_SAMPLE_FORMAT_ENUM_MAX,
	QUARTZ_SAMPLE_FORMAT_ENUM_FORCE32 = 0x7FFFFFFF,
} Quartz_SampleFormat;

typedef enum Quartz_SampleRate_t
{
	QUARTZ_SAMPLE_RATE_48000 = 48000,
	QUARTZ_SAMPLE_RATE_44100 = 44100,

	QUARTZ_SAMPLE_RATE_32000 = 32000,
	QUARTZ_SAMPLE_RATE_24000 = 24000,
	QUARTZ_SAMPLE_RATE_22050 = 22050,

	QUARTZ_SAMPLE_RATE_88200 = 88200,
	QUARTZ_SAMPLE_RATE_96000 = 96000,
	QUARTZ_SAMPLE_RATE_176400 = 176400,
	QUARTZ_SAMPLE_RATE_192000 = 192000,

	QUARTZ_SAMPLE_RATE_MIN = QUARTZ_SAMPLE_RATE_22050,
	QUARTZ_SAMPLE_RATE_MAX = QUARTZ_SAMPLE_RATE_192000,

	QUARTZ_SAMPLE_RATE_ENUM_FORCE32 = 0x7FFFFFFF,
} Quartz_SampleRate;

// Structs
typedef struct Quartz_InstanceDesc_t
{
	const char *application_name;
	const char *engine_name;
	uint32_t application_version;
	uint32_t engine_version;
} Quartz_InstanceDesc;

typedef struct Quartz_DeviceInfo_t
{
	char name[256];
	Quartz_Api api;
	Quartz_DeviceType type;
} Quartz_DeviceInfo;

typedef struct Quartz_BufferDesc_t
{
	uint32_t sample_rate;
	Quartz_SampleFormat format;
	uint32_t duration_milliseconds;
	uint32_t num_channels;
	// TODO: channel mappings (1:0, 2:0, 2:1, 5:1, 5:2, 7:1)
} Quartz_BufferDesc;

// Function pointers
typedef Quartz_Result (*PFN_quartzEnumerateDevices)(Quartz_Instance instance, Quartz_DeviceType type, uint32_t *device_count, Quartz_DeviceInfo *infos);
typedef Quartz_Result (*PFN_quartzCreateDevice)(Quartz_Instance instance, Quartz_DeviceType type, uint32_t index, Quartz_Device *device);
typedef Quartz_Result (*PFN_quartzCreateDefaultDevice)(Quartz_Instance instance, Quartz_DeviceType type, Quartz_Device *device);

typedef Quartz_Result (*PFN_quartzDestroyInstance)(Quartz_Instance instance);

typedef Quartz_Result (*PFN_quartzGetDeviceInfo)(Quartz_Device device, Quartz_DeviceInfo *info);

typedef Quartz_Result (*PFN_quartzCreateBuffer)(Quartz_Device device, const Quartz_BufferDesc *desc, Quartz_Buffer *buffer);

typedef Quartz_Result (*PFN_quartzDestroyBuffer)(Quartz_Device device, Quartz_Buffer buffer);
typedef Quartz_Result (*PFN_quartzDestroyDevice)(Quartz_Device device);

typedef Quartz_Result (*PFN_quartzMapBuffer)(Quartz_Device device, Quartz_Buffer buffer, void **ptr);
typedef Quartz_Result (*PFN_quartzUnmapBuffer)(Quartz_Device device, Quartz_Buffer buffer);


typedef struct Quartz_InstanceTable_t
{
	PFN_quartzEnumerateDevices enumerateDevices;
	PFN_quartzCreateDevice createDevice;
	PFN_quartzCreateDefaultDevice createDefaultDevice;

	PFN_quartzDestroyInstance destroyInstance;
} Quartz_InstanceTable;

typedef struct Quartz_DeviceTable_t
{
	PFN_quartzGetDeviceInfo getDeviceInfo;
	PFN_quartzCreateBuffer createBuffer;

	PFN_quartzDestroyBuffer destroyBuffer;
	PFN_quartzDestroyDevice destroyDevice;
	
	PFN_quartzMapBuffer mapBuffer;
	PFN_quartzUnmapBuffer unmapBuffer;
} Quartz_DeviceTable;

// API
#if !defined(QUARTZ_NO_PROTOTYPES)
QUARTZ_APIENTRY Quartz_Result quartzCreateInstance(Quartz_Api api, const Quartz_InstanceDesc *desc, Quartz_Instance *instance);
QUARTZ_APIENTRY Quartz_Result quartzGetInstanceTable(Quartz_Instance instance, Quartz_InstanceTable *instance_table);
QUARTZ_APIENTRY Quartz_Result quartzGetDeviceTable(Quartz_Device device, Quartz_DeviceTable *device_table);

QUARTZ_APIENTRY Quartz_Result quartzEnumerateDevices(Quartz_Instance instance, Quartz_DeviceType type, uint32_t *device_count, Quartz_DeviceInfo *infos);
QUARTZ_APIENTRY Quartz_Result quartzCreateDevice(Quartz_Instance instance, Quartz_DeviceType type, uint32_t index, Quartz_Device *device);
QUARTZ_APIENTRY Quartz_Result quartzCreateDefaultDevice(Quartz_Instance instance, Quartz_DeviceType type, Quartz_Device *device);

QUARTZ_APIENTRY Quartz_Result quartzDestroyInstance(Quartz_Instance instance);

QUARTZ_APIENTRY Quartz_Result quartzGetDeviceInfo(Quartz_Device device, Quartz_DeviceInfo *info);

QUARTZ_APIENTRY Quartz_Result quartzCreateBuffer(Quartz_Device device, const Quartz_BufferDesc *desc, Quartz_Buffer *buffer);

QUARTZ_APIENTRY Quartz_Result quartzDestroyBuffer(Quartz_Device device, Quartz_Buffer buffer);
QUARTZ_APIENTRY Quartz_Result quartzDestroyDevice(Quartz_Device device);

QUARTZ_APIENTRY Quartz_Result quartzMapBuffer(Quartz_Device device, Quartz_Buffer buffer, void **ptr);
QUARTZ_APIENTRY Quartz_Result quartzUnmapBuffer(Quartz_Device device, Quartz_Buffer buffer);
#endif

#ifdef __cplusplus
}
#endif
