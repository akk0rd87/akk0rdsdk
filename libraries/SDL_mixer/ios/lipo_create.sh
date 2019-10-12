rm libSDL_mixer.a
xcodebuild -project ../SDL_mixer/Xcode-iOS/SDL_mixer.xcodeproj -target libSDL_mixer-iOS -configuration Release -sdk iphoneos
xcodebuild -project ../SDL_mixer/Xcode-iOS/SDL_mixer.xcodeproj -target libSDL_mixer-iOS -configuration Release -sdk iphonesimulator
lipo -create ../SDL_mixer/Xcode-iOS/build/Release-iphoneos/libSDL_mixer.a ../SDL_mixer/Xcode-iOS/build/Release-iphonesimulator/libSDL_mixer.a -output libSDL_mixer.a
