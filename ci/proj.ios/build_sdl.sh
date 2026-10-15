#!/bin/bash
# Собирает SDL2 и SDL2_image для iOS из исходников SDK (libraries/SDL, libraries/SDL_image) теми же целями,
# что libraries/*/ios/lipo_create.sh, но в ci/proj.ios/build/sdl/<sdk>, не трогая libraries/.
# CiTestSDL линкуется с ними (LIBRARY_SEARCH_PATHS), а не с предсобранными libSDL2*.a из git.
#
#   AKKORD_SDK_HOME=<корень SDK> ./build_sdl.sh iphoneos|iphonesimulator
set -euo pipefail

sdk="${1:?usage: build_sdl.sh iphoneos|iphonesimulator}"
root="${AKKORD_SDK_HOME:?AKKORD_SDK_HOME is not set}"
out="$(cd "$(dirname "$0")" && pwd)/build/sdl/$sdk"
mkdir -p "$out"

build() {
    xcodebuild -project "$1" -target "$2" -configuration Release -sdk "$sdk" ONLY_ACTIVE_ARCH=NO \
        CONFIGURATION_BUILD_DIR="$out" SYMROOT="$out/sym" OBJROOT="$out/obj" "${@:3}" | tail -n 5
}

build "$root/libraries/SDL/SDL/Xcode/SDL/SDL.xcodeproj" "Static Library-iOS"
build "$root/libraries/SDL_image/SDL_image/Xcode/SDL_image.xcodeproj" "Static Library" \
    HEADER_SEARCH_PATHS="$root/libraries/SDL/SDL/include"

ls -l "$out"/libSDL2.a "$out"/libSDL2_image.a
