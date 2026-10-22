#!/usr/bin/env bash
# Сборка шейдеров GPU-бэкенда (video_gpu.h) в заголовки с массивами байтов, как в SDL (src/render/gpu/shaders).
#   shadercross (SDL_shadercross) — SPIR-V, MSL и DXIL;
#   без него — только DXIL через dxc (на Windows — из Windows SDK).
# Какие форматы собраны, video_gpu_formats.h определяет по наличию заголовков.

set -e
cd "$(dirname "$0")"

make-header() {
    xxd -i "$1" | sed \
        -e 's/^unsigned /const unsigned /g' \
        -e 's,^const,static const,' \
        > "$1.h"
    rm -f "$1"
}

find-dxc() {
    if command -v dxc > /dev/null 2>&1; then
        command -v dxc
        return
    fi
    ls "/c/Program Files (x86)/Windows Kits/10/bin/"*/x64/dxc.exe 2>/dev/null | sort -V | tail -1
}

for filename in *.hlsl; do
    echo "$filename"
    base="${filename%.hlsl}"
    if command -v shadercross > /dev/null 2>&1; then
        shadercross "$filename" -o "$base.spv" && make-header "$base.spv"
        shadercross "$filename" -o "$base.msl" && make-header "$base.msl"
        shadercross "$filename" -o "$base.dxil" && make-header "$base.dxil"
    else
        dxc="$(find-dxc)"
        if [ -z "$dxc" ]; then
            echo "Neither shadercross nor dxc found" >&2
            exit 1
        fi
        # шейдерная модель 6.0 — как у DXIL-шейдеров самого SDL
        "$dxc" -T ps_6_0 -E main -Fo "$base.dxil" "$filename"
        make-header "$base.dxil"
    fi
done
