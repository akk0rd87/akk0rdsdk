#ifndef __AKK0RD_SDK_VIDEOADAPTER_GPU_FORMATS_H__
#define __AKK0RD_SDK_VIDEOADAPTER_GPU_FORMATS_H__

// Форматы, в которые собран SDF-шейдер GPU-бэкенда (shaders/build-shaders.sh): определяются по наличию заголовков.
// BWrapper::CreateRenderer сообщает их GPU-рендереру, и SDL выбирает устройство, которое их понимает
// (только DXIL — Direct3D 12, MSL — Metal, SPIR-V — Vulkan)

#if __has_include("shaders/sdf.frag.spv.h")
#define AKKORD_SDK_GPU_SHADERS_SPIRV 1
#else
#define AKKORD_SDK_GPU_SHADERS_SPIRV 0
#endif

#if __has_include("shaders/sdf.frag.dxil.h")
#define AKKORD_SDK_GPU_SHADERS_DXIL 1
#else
#define AKKORD_SDK_GPU_SHADERS_DXIL 0
#endif

#if __has_include("shaders/sdf.frag.msl.h")
#define AKKORD_SDK_GPU_SHADERS_MSL 1
#else
#define AKKORD_SDK_GPU_SHADERS_MSL 0
#endif

#endif // __AKK0RD_SDK_VIDEOADAPTER_GPU_FORMATS_H__
