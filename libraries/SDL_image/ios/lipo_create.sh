#!/bin/bash

rm *.a
xcodebuild -project ../SDL_Image/Xcode/SDL_image.xcodeproj -target "Static Library" -configuration Release -sdk iphoneos HEADER_SEARCH_PATHS=../../../SDL/SDL/include CONFIGURATION_BUILD_DIR=$(pwd)
