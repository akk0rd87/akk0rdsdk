rm libSDL2_image.a
xcodebuild -project ../SDL_mixer/Xcode-iOS/SDL_mixer.xcodeproj -target libSDL_mixer-iOS -configuration Release -sdk iphoneos
xcodebuild -project ../SDL_mixer/Xcode-iOS/SDL_mixer.xcodeproj -target libSDL_mixer-iOS -configuration Release -sdk iphonesimulator
lipo -create ../SDL_mixer/Xcode-iOS/build/Release-iphoneos/libSDL2_image.a ../SDL_mixer/Xcode-iOS/build/Release-iphonesimulator/libSDL2_image.a -output libSDL2_image.a
