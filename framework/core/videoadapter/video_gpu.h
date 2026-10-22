#ifndef __AKK0RD_SDK_VIDEOADAPTER_GPU_H__
#define __AKK0RD_SDK_VIDEOADAPTER_GPU_H__

// SDF и градиент для GPU-рендерера SDL3 (драйвер "gpu": Metal, Direct3D 12, Vulkan).
// SDF рисуется через SDL_RenderGeometry со своим фрагментным шейдером (SDL_GPURenderState) — в общей очереди
// команд SDL, поэтому не нужны ни FlushRenderer, ни сохранение и восстановление состояния, а viewport, масштаб,
// клип, render target и режим смешивания (у текстуры атласа) учитывает сам SDL.
// Вершины — в координатах рендерера (пиксели BWrapper::GetScreenSize()).

#include <cstring>
#include "video_interface.h"
#include "video_gpu_formats.h"

#if AKKORD_SDK_GPU_SHADERS_SPIRV
#include "shaders/sdf.frag.spv.h"
#endif
#if AKKORD_SDK_GPU_SHADERS_DXIL
#include "shaders/sdf.frag.dxil.h"
#endif
#if AKKORD_SDK_GPU_SHADERS_MSL
#include "shaders/sdf.frag.msl.h"
#endif

class VideoBuffer_GPU;

class VideoAdapter_GPU : public VideoAdapter {
public:
    static bool IsGPURenderer(SDL_Renderer* Renderer) {
        const char* name = Renderer ? SDL_GetRendererName(Renderer) : nullptr;
        return name && 0 == std::strcmp(name, SDL_GPU_RENDERER);
    }

    virtual void PreInit() override {};
    virtual void PostInit() override {};

    virtual void InitSDFPlain() override { InitSDFState(); };
    virtual void InitSDFOutline() override { InitSDFState(); }; // обводка — тот же шейдер, флаг в uniform-буфере
    virtual void InitGradient() override {}; // градиент — цвета вершин, свой шейдер не нужен

    virtual void ShutDown() override {
        if (sdfState) {
            SDL_DestroyGPURenderState(sdfState);
            sdfState = nullptr;
        }
        if (sdfShader) {
            SDL_ReleaseGPUShader(device, sdfShader);
            sdfShader = nullptr;
        }
        device = nullptr;
    };

    virtual std::unique_ptr<VideoBuffer> CreateVideoBuffer() override;

    virtual void DrawLinearGradientRect(const AkkordRect& Rect, const AkkordColor& X0Y0, const AkkordColor& X1Y0, const AkkordColor& X1Y1, const AkkordColor& X0Y1) override {
        const float x0 = static_cast<float>(Rect.GetX());
        const float y0 = static_cast<float>(Rect.GetY());
        const float x1 = static_cast<float>(Rect.GetX() + Rect.GetW());
        const float y1 = static_cast<float>(Rect.GetY() + Rect.GetH());
        const SDL_Vertex vertices[4] = {
            { { x0, y0 }, ToFColor(X0Y0), { 0.0F, 0.0F } },
            { { x1, y0 }, ToFColor(X1Y0), { 0.0F, 0.0F } },
            { { x1, y1 }, ToFColor(X1Y1), { 0.0F, 0.0F } },
            { { x0, y1 }, ToFColor(X0Y1), { 0.0F, 0.0F } }
        };
        const int indices[6] = { 0, 1, 3, 1, 2, 3 };
        if (!SDL_RenderGeometry(BWrapper::GetActiveRenderer(), nullptr, vertices, 4, indices, 6)) {
            logError("Gradient SDL_RenderGeometry error: %s", SDL_GetError());
        }
    };

private:
    // раскладка — как cbuffer Context в shaders/sdf.frag.hlsl
    struct SDFUniforms {
        float fontColor[4];
        float outlineColor[4];
        float smooth;
        float border;
        float outline;
        float padding;
    };

    SDL_GPUDevice* device = nullptr;
    SDL_GPUShader* sdfShader = nullptr;
    SDL_GPURenderState* sdfState = nullptr;

    static SDL_FColor ToFColor(const AkkordColor& Color) {
        return { Color.GetR() / 255.0F, Color.GetG() / 255.0F, Color.GetB() / 255.0F, Color.GetA() / 255.0F };
    }

    static void ToFloat4(const AkkordColor& Color, float* Result) {
        const auto c = ToFColor(Color);
        Result[0] = c.r; Result[1] = c.g; Result[2] = c.b; Result[3] = c.a;
    }

    void InitSDFState() {
        if (sdfState) {
            return;
        }
        auto renderer = BWrapper::GetActiveRenderer();
        device = SDL_GetGPURendererDevice(renderer);
        if (!device) {
            logError("SDL_GetGPURendererDevice error: %s", SDL_GetError());
            return;
        }

        SDL_GPUShaderCreateInfo info;
        SDL_zero(info);
        const auto formats = SDL_GetGPUShaderFormats(device);
#if AKKORD_SDK_GPU_SHADERS_SPIRV
        if (!info.code && (formats & SDL_GPU_SHADERFORMAT_SPIRV)) {
            info.format = SDL_GPU_SHADERFORMAT_SPIRV;
            info.code = sdf_frag_spv;
            info.code_size = sdf_frag_spv_len;
        }
#endif
#if AKKORD_SDK_GPU_SHADERS_DXIL
        if (!info.code && (formats & SDL_GPU_SHADERFORMAT_DXIL)) {
            info.format = SDL_GPU_SHADERFORMAT_DXIL;
            info.code = sdf_frag_dxil;
            info.code_size = sdf_frag_dxil_len;
        }
#endif
#if AKKORD_SDK_GPU_SHADERS_MSL
        if (!info.code && (formats & SDL_GPU_SHADERFORMAT_MSL)) {
            info.format = SDL_GPU_SHADERFORMAT_MSL;
            info.code = sdf_frag_msl;
            info.code_size = sdf_frag_msl_len;
        }
#endif
        if (!info.code) {
            logError("No SDF shader for GPU device %s (shader formats 0x%x): see shaders/build-shaders.sh", SDL_GetGPUDeviceDriver(device), static_cast<unsigned>(formats));
            return;
        }
        info.stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
        info.num_samplers = 1;
        info.num_uniform_buffers = 1;

        sdfShader = SDL_CreateGPUShader(device, &info);
        if (!sdfShader) {
            logError("SDF SDL_CreateGPUShader error: %s", SDL_GetError());
            return;
        }

        SDL_GPURenderStateCreateInfo stateInfo;
        SDL_zero(stateInfo);
        stateInfo.fragment_shader = sdfShader;
        sdfState = SDL_CreateGPURenderState(renderer, &stateInfo);
        if (!sdfState) {
            logError("SDF SDL_CreateGPURenderState error: %s", SDL_GetError());
            return;
        }
        logDebug("SDF GPU shader: %s", SDL_GetGPUDeviceDriver(device));
    }

    friend class VideoBuffer_GPU;
    void DrawSDFBuffer(const VideoBuffer_GPU& Buffer, const VideoSDFBufferDrawParams& Params);
};

class VideoBuffer_GPU : public VideoBuffer {
public:
    friend class VideoAdapter_GPU;
    virtual void Clear() override {
        vertices.clear();
        indices.clear();
    };

    virtual void Reserve(unsigned Count) override {
        vertices.reserve(Count * 4);
        indices.reserve(Count * 6);
    };

    virtual void Append(const VideoBufferAppendParams& Params) override {
        const float u1 = Params.SrcRect->x / Params.TextureW;
        const float u2 = (Params.SrcRect->x + Params.SrcRect->w) / Params.TextureW;
        const float v1 = Params.SrcRect->y / Params.TextureH;
        const float v2 = (Params.SrcRect->y + Params.SrcRect->h) / Params.TextureH;

        const float x1 = Params.DestRect->x;
        const float x2 = Params.DestRect->x + Params.DestRect->w;
        const float y1 = Params.DestRect->y;
        const float y2 = Params.DestRect->y + Params.DestRect->h;

        // цвет — в uniform-буфере (VideoSDFBufferDrawParams), вершины белые
        constexpr SDL_FColor white{ 1.0F, 1.0F, 1.0F, 1.0F };
        const int first = static_cast<int>(vertices.size());
        vertices.push_back({ { x1, y1 }, white, { u1, v1 } });
        vertices.push_back({ { x2, y1 }, white, { u2, v1 } });
        vertices.push_back({ { x1, y2 }, white, { u1, v2 } });
        vertices.push_back({ { x2, y2 }, white, { u2, v2 } });
        indices.insert(indices.cend(), { first, first + 1, first + 2, first + 1, first + 2, first + 3 });
    };

    virtual void DrawSDF(const VideoSDFBufferDrawParams& Params) override {
        if (!indices.empty()) {
            videoAdapter->DrawSDFBuffer(*this, Params);
        }
    };

    VideoBuffer_GPU(VideoAdapter_GPU* VideoAdapter) : videoAdapter(VideoAdapter) {}
private:
    std::vector<SDL_Vertex> vertices;
    std::vector<int> indices;
    VideoAdapter_GPU* videoAdapter;
};

std::unique_ptr<VideoBuffer> VideoAdapter_GPU::CreateVideoBuffer() {
    return std::make_unique<VideoBuffer_GPU>(this);
}

void VideoAdapter_GPU::DrawSDFBuffer(const VideoBuffer_GPU& Buffer, const VideoSDFBufferDrawParams& Params) {
    if (!sdfState) {
        logError("SDF GPU render state is not initialized!");
        return;
    }

    SDFUniforms uniforms;
    ToFloat4(*Params.Color, uniforms.fontColor);
    ToFloat4(*Params.OutlineColor, uniforms.outlineColor);
    uniforms.smooth = Params.GetSmoothness();
    uniforms.border = Params.GetShaderBorder();
    uniforms.outline = Params.Outline ? 1.0F : 0.0F;
    uniforms.padding = 0.0F;

    // данные копируются; если состояние уже стоит в очереди команд, SDL сам выполнит очередь перед заменой
    if (!SDL_SetGPURenderStateFragmentUniforms(sdfState, 0, &uniforms, sizeof(uniforms))) {
        logError("SDL_SetGPURenderStateFragmentUniforms error: %s", SDL_GetError());
        return;
    }

    auto renderer = BWrapper::GetActiveRenderer();
    SDL_SetGPURenderState(renderer, sdfState);
    if (!SDL_RenderGeometry(renderer, Params.Texture, Buffer.vertices.data(), static_cast<int>(Buffer.vertices.size()), Buffer.indices.data(), static_cast<int>(Buffer.indices.size()))) {
        logError("SDF SDL_RenderGeometry error: %s", SDL_GetError());
    }
    SDL_SetGPURenderState(renderer, nullptr);
}

#endif // __AKK0RD_SDK_VIDEOADAPTER_GPU_H__
