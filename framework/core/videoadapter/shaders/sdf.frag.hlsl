// SDF-текст для SDL_GPURenderState (GPU-рендерер SDL3).
// Вершинный шейдер — SDL-ный tri_texture.vert. Его собирает shadercross, и в DXIL выходы называются
// TEXCOORD0 (цвет вершины) и TEXCOORD1 (UV атласа), а не COLOR0/TEXCOORD0, как в его HLSL-исходнике:
// входы здесь должны совпадать с ними, иначе Direct3D 12 не создаст pipeline и ничего не нарисуется
// (для SPIR-V и MSL это location 0 и 1 — тот же порядок).
// Ресурсы фрагментного шейдера в SDL_GPU: текстуры и сэмплеры — space2, uniform-буферы — space3.
// Логика та же, что у GLSL-шейдеров в video_opengles.h: расстояние берется из альфа-канала атласа.
// После изменения — пересобрать заголовки: build-shaders.sh

cbuffer Context : register(b0, space3)
{
    float4 font_color;
    float4 outline_color;
    float smooth_param; // полуширина сглаживания края
    float border;       // полуширина обводки
    float outline;      // 1 — с обводкой, 0 — без
    float padding;
};

Texture2D u_texture : register(t0, space2);
SamplerState u_sampler : register(s0, space2);

struct PSInput {
    float4 v_color : TEXCOORD0;
    float2 v_uv : TEXCOORD1;
};

struct PSOutput {
    float4 o_color : SV_Target;
};

// как my_smoothstep в GLSL-версии: допускает edge0 > edge1
float sdf_smoothstep(float edge0, float edge1, float x)
{
    x = saturate((x - edge0) / (edge1 - edge0));
    return x * x * (3.0 - 2.0 * x);
}

PSOutput main(PSInput input)
{
    const float distAlpha = u_texture.Sample(u_sampler, input.v_uv).a;
    float4 rgba = font_color;
    float center = 0.5;

    if (outline > 0.5) {
        const float outlineMaxValue0 = 0.5 - border;
        const float outlineMaxValue1 = 0.5 + border;
        rgba = lerp(rgba, outline_color, sdf_smoothstep(outlineMaxValue1, outlineMaxValue0, distAlpha));
        center = outlineMaxValue0 - border;
    }
    rgba.a *= sdf_smoothstep(center - smooth_param, center + smooth_param, distAlpha);

    PSOutput output;
    output.o_color = rgba * input.v_color;
    return output;
}
