LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/android \
    $(LOCAL_PATH)/lib/include \
    $(LOCAL_PATH)/third_party/highway \
    $(LOCAL_PATH)/third_party/brotli/c/include \
    $(LOCAL_PATH)/third_party/brotli/c/dec \

BROTLI_SRC_FILES := \
    third_party/brotli/c/common/constants.c \
    third_party/brotli/c/common/context.c \
    third_party/brotli/c/common/dictionary.c \
    third_party/brotli/c/common/platform.c \
    third_party/brotli/c/common/shared_dictionary.c \
    third_party/brotli/c/common/transform.c \
    third_party/brotli/c/dec/bit_reader.c \
    third_party/brotli/c/dec/decode.c \
    third_party/brotli/c/dec/huffman.c \
    third_party/brotli/c/dec/state.c \

HIGHWAY_SRC_FILES := \
    third_party/highway/hwy/aligned_allocator.cc \
    third_party/highway/hwy/per_target.cc \
    third_party/highway/hwy/targets.cc \

LOCAL_SRC_FILES :=  \
    $(BROTLI_SRC_FILES) \
    $(HIGHWAY_SRC_FILES) \
    lib/jxl/ac_strategy.cc \
    lib/jxl/alpha.cc \
    lib/jxl/ans_common.cc \
    lib/jxl/aux_out.cc \
    lib/jxl/base/cache_aligned.cc \
    lib/jxl/base/data_parallel.cc \
    lib/jxl/base/padded_bytes.cc \
    lib/jxl/base/random.cc \
    lib/jxl/blending.cc \
    lib/jxl/box_content_decoder.cc \
    lib/jxl/chroma_from_luma.cc \
    lib/jxl/coeff_order.cc \
    lib/jxl/color_encoding_internal.cc \
    lib/jxl/color_management.cc \
    lib/jxl/compressed_dc.cc \
    lib/jxl/convolve_separable5.cc \
    lib/jxl/convolve_separable7.cc \
    lib/jxl/convolve_slow.cc \
    lib/jxl/convolve_symmetric3.cc \
    lib/jxl/convolve_symmetric5.cc \
    lib/jxl/dct_scales.cc \
    lib/jxl/dec_ans.cc \
    lib/jxl/dec_cache.cc \
    lib/jxl/dec_context_map.cc \
    lib/jxl/dec_external_image.cc \
    lib/jxl/dec_frame.cc \
    lib/jxl/dec_group.cc \
    lib/jxl/dec_group_border.cc \
    lib/jxl/dec_huffman.cc \
    lib/jxl/dec_modular.cc \
    lib/jxl/dec_noise.cc \
    lib/jxl/dec_patch_dictionary.cc \
    lib/jxl/dec_xyb.cc \
    lib/jxl/decode.cc \
    lib/jxl/decode_to_jpeg.cc \
    lib/jxl/enc_bit_writer.cc \
    lib/jxl/entropy_coder.cc \
    lib/jxl/epf.cc \
    lib/jxl/fast_dct.cc \
    lib/jxl/fields.cc \
    lib/jxl/frame_header.cc \
    lib/jxl/gauss_blur.cc \
    lib/jxl/headers.cc \
    lib/jxl/huffman_table.cc \
    lib/jxl/icc_codec.cc \
    lib/jxl/icc_codec_common.cc \
    lib/jxl/image.cc \
    lib/jxl/image_bundle.cc \
    lib/jxl/image_metadata.cc \
    lib/jxl/jpeg/dec_jpeg_data.cc \
    lib/jxl/jpeg/dec_jpeg_data_writer.cc \
    lib/jxl/jpeg/jpeg_data.cc \
    lib/jxl/loop_filter.cc \
    lib/jxl/luminance.cc \
    lib/jxl/memory_manager_internal.cc \
    lib/jxl/modular/encoding/dec_ma.cc \
    lib/jxl/modular/encoding/encoding.cc \
    lib/jxl/modular/modular_image.cc \
    lib/jxl/modular/transform/rct.cc \
    lib/jxl/modular/transform/squeeze.cc \
    lib/jxl/modular/transform/transform.cc \
    lib/jxl/opsin_params.cc \
    lib/jxl/passes_state.cc \
    lib/jxl/quant_weights.cc \
    lib/jxl/quantizer.cc \
    lib/jxl/render_pipeline/low_memory_render_pipeline.cc \
    lib/jxl/render_pipeline/render_pipeline.cc \
    lib/jxl/render_pipeline/simple_render_pipeline.cc \
    lib/jxl/render_pipeline/stage_blending.cc \
    lib/jxl/render_pipeline/stage_chroma_upsampling.cc \
    lib/jxl/render_pipeline/stage_epf.cc \
    lib/jxl/render_pipeline/stage_from_linear.cc \
    lib/jxl/render_pipeline/stage_gaborish.cc \
    lib/jxl/render_pipeline/stage_noise.cc \
    lib/jxl/render_pipeline/stage_patches.cc \
    lib/jxl/render_pipeline/stage_splines.cc \
    lib/jxl/render_pipeline/stage_spot.cc \
    lib/jxl/render_pipeline/stage_to_linear.cc \
    lib/jxl/render_pipeline/stage_tone_mapping.cc \
    lib/jxl/render_pipeline/stage_upsampling.cc \
    lib/jxl/render_pipeline/stage_write.cc \
    lib/jxl/render_pipeline/stage_xyb.cc \
    lib/jxl/render_pipeline/stage_ycbcr.cc \
    lib/jxl/splines.cc \
    lib/jxl/toc.cc \

LOCAL_CFLAGS := -DJXL_INTERNAL_LIBRARY_BUILD -DJPEGXL_MAJOR_VERSION=0 -DJPEGXL_MINOR_VERSION=7 -DJPEGXL_PATCH_VERSION=2 -DJPEGXL_ENABLE_TRANSCODE_JPEG=1 -D__STDC_FORMAT_MACROS

LOCAL_MODULE := jxl

include $(BUILD_STATIC_LIBRARY)
