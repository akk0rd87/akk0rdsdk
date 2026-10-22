#!/bin/bash
# Собирает статическую SDL3 для iOS из исходников SDK (libraries/SDL/SDL) через CMake: <каталог>/lib/libSDL3.a.
# Каталог — в сборке приложения (ci/proj.ios/build/sdl/<sdk>, proj.ios/build/sdl/<sdk> в играх), как на Windows:
# сборки разных приложений не делят один каталог в SDK. Приложение линкует -lSDL3 и системные фреймворки
# SDL3 (tools/ios/akkord_sdl.yml).
#
# CMake, а не Xcode-проект SDL: в Xcode-проекте конфигурация iOS (SDL_build_config_ios.h) всегда включает
# OpenGL ES, а SDK рисует на iOS через GPU-рендерер (Metal) — SDL_OPENGLES=OFF убирает GLES и OpenGLES.framework.
# Кроме симулятора: его Metal-устройство — уровня MTLGPUFamilyApple2, а GPU-рендереру SDL нужен Apple3, поэтому
# в симуляторе запасной рендерер — OpenGL ES (videoadapter_ios.mm).
# Минимальная iOS — ios_deployment_target из tools/ios/pods.yml, как у приложений (Metal в симуляторе SDL
# включает только начиная с iOS 13.0).
#
#   AKKORD_SDK_HOME=<корень SDK> build_sdl3.sh iphoneos|iphonesimulator <каталог>
set -euo pipefail

sdk="${1:?usage: build_sdl3.sh iphoneos|iphonesimulator <dir>}"
out="${2:?usage: build_sdl3.sh iphoneos|iphonesimulator <dir>}"
root="${AKKORD_SDK_HOME:?AKKORD_SDK_HOME is not set}"
mkdir -p "$out"
out="$(cd "$out" && pwd)"

case "$sdk" in
    iphoneos) archs="arm64"; opengles=OFF ;;
    # x86_64 — симулятор на Mac с Intel
    iphonesimulator) archs="arm64;x86_64"; opengles=ON ;;
    *) echo "unknown sdk: $sdk" >&2; exit 1 ;;
esac

deployment_target=$(ruby -ryaml -e 'puts YAML.load_file(ARGV[0]).fetch("ios_deployment_target")' "$root/tools/ios/pods.yml")

cmake -S "$root/libraries/SDL/SDL" -B "$out/cmake" -G Xcode \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_OSX_SYSROOT="$sdk" \
    -DCMAKE_OSX_ARCHITECTURES="$archs" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET="$deployment_target" \
    -DCMAKE_INSTALL_PREFIX="$out" \
    -DSDL_SHARED=OFF \
    -DSDL_STATIC=ON \
    -DSDL_TEST_LIBRARY=OFF \
    -DSDL_OPENGLES="$opengles" > "$out/cmake.log"
cmake --build "$out/cmake" --config Release -- -quiet ONLY_ACTIVE_ARCH=NO
cmake --install "$out/cmake" --config Release > /dev/null

lipo -info "$out/lib/libSDL3.a"
