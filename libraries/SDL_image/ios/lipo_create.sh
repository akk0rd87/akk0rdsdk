rm libSDL2_image.a
xcodebuild -project ../SDL_Image/Xcode-iOS/SDL_image.xcodeproj -target libSDL_image-iOS -configuration Release -sdk iphoneos
#xcodebuild -project ../SDL_Image/Xcode-iOS/SDL_image.xcodeproj -target libSDL_image-iOS -configuration Release -sdk iphonesimulator
#lipo -create ../SDL_Image/Xcode-iOS/build/Release-iphoneos/libSDL2_image.a ../SDL_Image/Xcode-iOS/build/Release-iphonesimulator/libSDL2_image.a -output libSDL2_image.a
cp ../SDL_Image/Xcode-iOS/build/Release-iphoneos/libSDL2_image.a libSDL2_image.a