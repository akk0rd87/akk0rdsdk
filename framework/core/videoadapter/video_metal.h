#ifndef __AKK0RD_SDK_VIDEOADAPTER_METAL_H__
#define __AKK0RD_SDK_VIDEOADAPTER_METAL_H__

#include "video_interface.h"
#import <Metal/Metal.h>

#define METAL_CHECK_CREATE(obj) (nil == obj ? logError("Error create %s", #obj) : logDebug("Success create %s", #obj))

struct MetalProgram {
    id<MTLFunction> vertFunc, fragFunc;
};
struct MetalGradientProgram : public MetalProgram {
};
struct MetalSDFProgram : public MetalProgram {
};

class VideoBuffer_Metal;

class VideoAdapter_Metal : public VideoAdapter {
public:
    virtual void PreInit() override {
        mtldevice = MTLCreateSystemDefaultDevice();
        METAL_CHECK_CREATE(mtldevice);

        mtlcmdqueue = [mtldevice newCommandQueue];
        METAL_CHECK_CREATE(mtlcmdqueue);

        mtlcmdbuffer = [mtlcmdqueue commandBuffer];
        METAL_CHECK_CREATE(mtlcmdbuffer);

        mtlpassdesc = [MTLRenderPassDescriptor renderPassDescriptor];
        METAL_CHECK_CREATE(mtlpassdesc);

        /*
        mtlcmdencoder = [mtlcmdbuffer renderCommandEncoderWithDescriptor:mtlpassdesc];
        if(nil == mtlcmdencoder) {
            logError("Error create renderCommandEncoderWithDescriptor");
            return;
        }
        else {
            logDebug("Success create renderCommandEncoderWithDescriptor");
        }
        */

       CompileLibrary();
    };
    virtual void PostInit() override {};

    virtual void InitSDFPlain() override {
        if (!InitProgram(&SDFProgramPlain, @"vert_sdf_plain", @"frag_sdf_plain")) {
            logError("InitSDFPlain error");
        }
    };

    virtual void InitSDFOutline() override {
        if (!InitProgram(&SDFProgramOutline, @"vert_sdf_outln", @"frag_sdf_outln")) {
            logError("InitSDFPlain error");
        }
    };
    virtual void InitGradient() override {
        if (!InitProgram(&GradientProgram, @"vert_gradient", @"frag_gradient")) {
            logError("GradientProgram error");
        }
    };

    virtual std::unique_ptr<VideoBuffer> CreateVideoBuffer() override;
    virtual void DrawLinearGradientRect(const AkkordRect& Rect, const AkkordColor& X0Y0, const AkkordColor& X1Y0, const AkkordColor& X1Y1, const AkkordColor& X0Y1) override {};

private:
    id<MTLDevice> mtldevice;
    id<MTLCommandQueue> mtlcmdqueue;
    id<MTLCommandBuffer> mtlcmdbuffer;
    MTLRenderPassDescriptor *mtlpassdesc;
    id<MTLRenderCommandEncoder> mtlcmdencoder;
    id<MTLLibrary> library;

    MetalSDFProgram SDFProgramPlain, SDFProgramOutline;
    MetalGradientProgram GradientProgram;

    bool LoadFunction(id<MTLFunction>* Func, NSString* FuncName) {
        *Func = nil;
        *Func = [library newFunctionWithName:FuncName];
        if(nil == *Func) {
            NSLog(@"Error load function %@", FuncName);
        }
        else {
            NSLog(@"Success load function %@", FuncName);
        }
        return true;
    }

    bool InitProgram(MetalProgram* Program, NSString* VertFuncName, NSString* FragFuncName) {
        return LoadFunction(&Program->vertFunc, VertFuncName) && LoadFunction(&Program->fragFunc, FragFuncName);
    }

    void CompileLibrary(){
        __autoreleasing NSError *error = nil;

        NSString* librarySrc =
@"#include <metal_stdlib> \n"
"using namespace metal; \n"
" \n"
"struct Vertex \n"
"{ \n"
"    packed_float4 position; \n"
"    packed_float2 texCoords; \n"
"}; \n"
" \n"
"struct TransformedVertex \n"
"{ \n"
"    float4 position [[position]]; \n"
"    float2 texCoords; \n"
"}; \n"
" \n"
"struct Uniforms \n"
"{ \n"
"    float4x4 modelMatrix; \n"
"    float4x4 viewProjectionMatrix; \n"
"    float4 foregroundColor; \n"
"}; \n"
" \n"
"vertex TransformedVertex vert_sdf_plain(constant Vertex *vertices [[buffer(0)]], \n"
"                                      constant Uniforms &uniforms [[buffer(1)]], \n"
"                                      uint vid [[vertex_id]]) \n"
"{ \n"
"    TransformedVertex outVert; \n"
"    outVert.position = uniforms.viewProjectionMatrix * uniforms.modelMatrix * float4(vertices[vid].position); \n"
"    outVert.texCoords = vertices[vid].texCoords; \n"
"    return outVert; \n"
"} \n"
" \n"
"fragment half4 frag_sdf_plain(TransformedVertex vert [[stage_in]], \n"
"                              constant Uniforms &uniforms [[buffer(0)]], \n"
"                              sampler samplr [[sampler(0)]], \n"
"                              texture2d<float, access::sample> texture [[texture(0)]]) \n"
"{ \n"
"    float4 color = uniforms.foregroundColor; \n"
"    // Outline of glyph is the isocontour with value 50% \n"
"    float edgeDistance = 0.5; \n"
"    // Sample the signed-distance field to find distance from this fragment to the glyph outline \n"
"    float sampleDistance = texture.sample(samplr, vert.texCoords).r; \n"
"    // Use local automatic gradients to find anti-aliased anisotropic edge width, cf. Gustavson 2012 \n"
"    float edgeWidth = 0.75 * length(float2(dfdx(sampleDistance), dfdy(sampleDistance))); \n"
"    // Smooth the glyph edge by interpolating across the boundary in a band with the width determined above \n"
"    float insideness = smoothstep(edgeDistance - edgeWidth, edgeDistance + edgeWidth, sampleDistance); \n"
"    return half4(color.r, color.g, color.b, insideness); \n"
"} \n"
"vertex TransformedVertex vert_sdf_outln(constant Vertex *vertices [[buffer(0)]], \n"
"                                      constant Uniforms &uniforms [[buffer(1)]], \n"
"                                      uint vid [[vertex_id]]) \n"
"{ \n"
"    TransformedVertex outVert; \n"
"    return outVert; \n"
"} \n"
" \n"
"fragment half4 frag_sdf_outln(TransformedVertex vert [[stage_in]], \n"
"                              constant Uniforms &uniforms [[buffer(0)]], \n"
"                              sampler samplr [[sampler(0)]], \n"
"                              texture2d<float, access::sample> texture [[texture(0)]]) \n"
"{ \n"
"    float4 color = uniforms.foregroundColor; \n"
"    return half4(color.r, color.g, color.b, color.a); \n"
"} \n"
"vertex TransformedVertex vert_gradient(constant Vertex *vertices [[buffer(0)]], \n"
"                                      constant Uniforms &uniforms [[buffer(1)]], \n"
"                                      uint vid [[vertex_id]]) \n"
"{ \n"
"    TransformedVertex outVert; \n"
"    return outVert; \n"
"} \n"
" \n"
"fragment half4 frag_gradient(TransformedVertex vert [[stage_in]], \n"
"                              constant Uniforms &uniforms [[buffer(0)]], \n"
"                              sampler samplr [[sampler(0)]], \n"
"                              texture2d<float, access::sample> texture [[texture(0)]]) \n"
"{ \n"
"    float4 color = uniforms.foregroundColor; \n"
"    return half4(color.r, color.g, color.b, color.a); \n"
"} \n";

        library = [mtldevice newLibraryWithSource:librarySrc options:nil error:&error];
        if(nil != error) {
            logError("newLibraryWithSource with error object");
            NSLog(@"create library: %@", error);
        }

        if(nil == library) {
            logError("Error create newLibraryWithSource");
            return;
        }
        else {
            logDebug("Success create newLibraryWithSource");
        }
    };

    friend class VideoBuffer_Metal;
    void DrawSDFBuffer(const VideoBuffer_Metal& Buffer, const VideoSDFBufferDrawParams& Params);
};

class VideoBuffer_Metal : public VideoBuffer {
public:
    friend class VideoAdapter_Metal;
    virtual void Clear() override {};
    virtual void Reserve(unsigned Count) override {};
    virtual void Append(const VideoBufferAppendParams& Params) override {};
    virtual void DrawSDF(const VideoSDFBufferDrawParams& Params) override {
        // check is not empty
        videoAdapter->DrawSDFBuffer(*this, Params);
    };

    VideoBuffer_Metal(VideoAdapter_Metal* VideoAdapter) : videoAdapter(VideoAdapter) {}
private:
    VideoAdapter_Metal* videoAdapter;
};

std::unique_ptr<VideoBuffer> VideoAdapter_Metal::CreateVideoBuffer() {
    return std::make_unique<VideoBuffer_Metal>(this);
};

void VideoAdapter_Metal::DrawSDFBuffer(const VideoBuffer_Metal& Buffer, const VideoSDFBufferDrawParams& Params) {

}

#endif //__AKK0RD_SDK_VIDEOADAPTER_METAL_H__
