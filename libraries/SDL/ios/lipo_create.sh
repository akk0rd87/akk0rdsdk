rm libSDL2.a
xcodebuild -project ../SDL/Xcode-iOS/SDL/SDL.xcodeproj -target libSDL-iOS -configuration Release -sdk iphoneos
xcodebuild -project ../SDL/Xcode-iOS/SDL/SDL.xcodeproj -target libSDL-iOS -configuration Release -sdk iphonesimulator
lipo -create ../SDL/Xcode-iOS/SDL/build/Release-iphoneos/libSDL2_image.a ../SDL/Xcode-iOS/SDL/build/Release-iphonesimulator/libSDL2_image.a -output libSDL2.a