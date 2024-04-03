#!/bin/bash

rm *.a
xcodebuild -project ../SDL/Xcode/SDL/SDL.xcodeproj -target "Static Library-iOS" -configuration Release -sdk iphoneos CONFIGURATION_BUILD_DIR=$(pwd)
