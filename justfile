generate_ios:
    rm -rf ./cmake-build-debug-ios
    cmake -G Xcode -S . -B cmake-build-debug-ios \
      -DCMAKE_SYSTEM_NAME=iOS \
      -DCMAKE_OSX_SYSROOT=iphoneos \
      -DCMAKE_OSX_ARCHITECTURES=arm64
    open cmake-build-debug-ios/3d_renderer.xcodeproj
