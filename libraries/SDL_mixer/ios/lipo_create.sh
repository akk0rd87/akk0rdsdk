rm libSDL2_mixer.a
xcodebuild -project ../SDL_mixer/Xcode/SDL_mixer.xcodeproj -target "Static Library" -configuration Release -sdk iphoneos
# xcodebuild -project ../SDL_mixer/Xcode/SDL_mixer.xcodeproj -target "Static Library" -configuration Release -sdk iphonesimulator

#lipo -create ../SDL_mixer/Xcode-iOS/build/Release-iphoneos/libSDL2_mixer.a ../SDL_mixer/Xcode-iOS/build/Release-iphonesimulator/libSDL2_mixer.a -output libSDL2_mixer.a
cp ../SDL_mixer/Xcode/build/Release-iphoneos/libSDL2_mixer.a libSDL2_mixer.a