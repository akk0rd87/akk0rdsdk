#!/bin/bash
# Собирает статическую SDL3 для iOS из исходников SDK (libraries/SDL/SDL) через CMake: <каталог>/lib/libSDL3.a.
# Каталог — в сборке приложения (ci/proj.ios/build/sdl/<sdk>, proj.ios/build/sdl/<sdk> в играх), как на Windows:
# сборки разных приложений не делят один каталог в SDK. Приложение линкует -lSDL3 и системные фреймворки
# SDL3 из <каталог>/sdl3_frameworks.txt: файл ответов компоновщика (OTHER_LDFLAGS @файл, tools/ios/akkord_sdl.yml),
# фреймворки в нем — из sdl3.pc, то есть ровно те, что нужны включенным подсистемам.
#
# Подсистемы SDL, которые нужны не всем играм, — переменные окружения AKKORD_SDL_<ПОДСИСТЕМА>=ON|OFF, по умолчанию
# OFF (задает таргет SDL3 в akkord_sdl.yml, игра переопределяет в своем project.yml). Выключенная подсистема не
# попадает в бинарник: App Store требует строку назначения в Info.plist за каждое API доступа к данным
# пользователя в бинарнике (ITMS-90683), даже если игра его не вызывает.
#   AKKORD_SDL_CAMERA   — камера (AVCaptureDevice: NSCameraUsageDescription)
#   AKKORD_SDL_JOYSTICK — геймпады MFi через GameController, вибрация геймпадов (CoreHaptics), акселерометр как
#                         джойстик (CoreMotion)
#   AKKORD_SDL_HAPTIC   — API SDL_Haptic
#   AKKORD_SDL_SENSOR   — датчики SDL_Sensor (CoreMotion)
#   AKKORD_SDL_HIDAPI   — HIDAPI: геймпады в обход GameController, Steam Controller по Bluetooth LE (CoreBluetooth:
#                         NSBluetoothAlwaysUsageDescription)
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

subsystems=()
for name in CAMERA JOYSTICK HAPTIC SENSOR HIDAPI; do
    var="AKKORD_SDL_$name"
    value="${!var:-OFF}"
    case "$value" in
        ON|OFF) ;;
        *) echo "$var must be ON or OFF, got '$value'" >&2; exit 1 ;;
    esac
    subsystems+=("-DSDL_$name=$value")
done
echo "SDL subsystems: ${subsystems[*]}"

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
    "${subsystems[@]}" \
    -DSDL_OPENGLES="$opengles" > "$out/cmake.log" 2>&1 || { cat "$out/cmake.log" >&2; exit 1; }
cmake --build "$out/cmake" --config Release -- -quiet ONLY_ACTIVE_ARCH=NO
cmake --install "$out/cmake" --config Release > /dev/null

lipo -info "$out/lib/libSDL3.a"

# -Wl,-framework,X и -Wl,-weak_framework,X из Libs в sdl3.pc — в файл ответов компоновщика
grep '^Libs:' "$out/lib/pkgconfig/sdl3.pc" | tr ' ' '\n' | sed -En 's/^-Wl,-(weak_framework|framework),(.*)$/-\1 \2/p' \
    > "$out/sdl3_frameworks.txt"
echo "SDL frameworks: $(tr '\n' ' ' < "$out/sdl3_frameworks.txt")"
