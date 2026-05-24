# Cerberus
This is the Cerberus Framework

####Dependencies:
-Boost (libboost-regex-dev on debian)
-OpenSSL (libssl-dev on debian)

#### Quick Start
1. Local build for the current machine:
   `cmake --preset local -B build/local-debug -DCMAKE_BUILD_TYPE=Debug`
   `cmake --build build/local-debug --target Cerberus`
2. Local Release build for the current machine:
   `cmake --preset local -B build/local-release -DCMAKE_BUILD_TYPE=Release`
   `cmake --build build/local-release --target Cerberus`
3. Remote ARM64 build with remote sysroot bootstrap during configure:
   `export CERBERUS_REMOTE_SSH_TARGET="user@target-host"`
   `cmake --preset remote-arm64 -B build/remote-arm64-debug -DCMAKE_BUILD_TYPE=Debug`
   `cmake --build build/remote-arm64-debug --target Cerberus`
4. Remote ARM64 build with extra synced target paths:
   `export CERBERUS_REMOTE_SSH_TARGET="user@target-host"`
   `export CERBERUS_REMOTE_EXTRA_SYNC_PATHS="/usr/local;/opt/vendor"`
   `cmake --preset remote-arm64 -B build/remote-arm64-release -DCMAKE_BUILD_TYPE=Release`
   `cmake --build build/remote-arm64-release --target Cerberus`

#### Development Environment Build (suggested for QTCreator)
1. Open the Cerberus project using CMakeList.txt file
2. Select a NON-QT toolchain and use only "Debug" and "Release" configurations
3. In both configurations edit as follows:
	1. Check "Cerberus" build step
	2. Uncheck "all" build step
4. Add "test" configuration starting from "Debug" template (make sure it points to a dedicated empty directory)
5. Set "cerberus-test" as single target for the test configuration
6. In Run tab add a deploy step of type "CMake Install"

#### Gtest
Gtest library will be automatically fetched from internet, compiled and linked if not present in the system.
This will only happen if GOOGLETEST CMake option is set

Important note: on Windows, Gtest library must have been compiled with RunTime Library set to "SHARED".
Please refer to https://github.com/google/googletest to get the sources of google test library.
