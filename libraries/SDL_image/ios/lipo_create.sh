#!/bin/bash

rm *.a
xcodebuild -project ../SDL_Image/Xcode-iOS/SDL_image.xcodeproj -target libSDL_image-iOS -configuration Release -sdk iphoneos HEADER_SEARCH_PATHS=../../../SDL/SDL/include CONFIGURATION_BUILD_DIR=$(pwd)
