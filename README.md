# Cerberus
This is the Cerberus Framework

####Dependencies:
-Boost (libboost-regex-dev on debian)
-OpenSSL (libssl-dev on debian)

#### Linux/Debian dependencies
Install the system development packages before configuring CMake:

```
sudo apt install libboost-regex-dev libssl-dev
```

Cerberus always asks CMake to use the static Boost.Regex library when using the
system Boost package. On Debian, `libboost-regex-dev` provides the required
headers and static library, so no `CERBERUS_BOOST_ROOT` path is needed for a
normal Linux build.

#### Quick Start
1. Local build for the current machine:
   `cmake --preset gcc -B build/gcc-debug -DCMAKE_BUILD_TYPE=Debug`
   `cmake --build build/gcc-debug --target Cerberus`
2. Local Release build for the current machine:
   `cmake --preset gcc -B build/gcc-release -DCMAKE_BUILD_TYPE=Release`
   `cmake --build build/gcc-release --target Cerberus`
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

#### Windows
MSVC builds use the `Ninja Multi-Config` CMake generator. Install Visual Studio or Visual Studio Build Tools
with the C++ workload, CMake, and Ninja. When building from a shell, run the commands from a Visual Studio
Developer PowerShell/Command Prompt so `cl.exe` can find the MSVC standard library and Windows SDK headers.

Configure and build:

```
cmake --preset msvc -DCERBERUS_OPENSSL_ROOT="C:/Program Files/OpenSSL-Win64" -DCERBERUS_BOOST_ROOT="C:/path/to/boost"
cmake --build --preset msvc-debug
cmake --build --preset msvc-release
```

Configure and build the test executable:

```
cmake --preset msvc-test -DCERBERUS_OPENSSL_ROOT="C:/Program Files/OpenSSL-Win64" -DCERBERUS_BOOST_ROOT="C:/path/to/boost"
cmake --build --preset msvc-test-debug
```

The test executable can then be run directly from `build/msvc-test/Debug/cerberus-test.exe`.

MSVC builds require a Win64 OpenSSL package built for MSVC. The currently tested package is
Win64 OpenSSL v3.0.20 from https://slproweb.com/products/Win32OpenSSL.html.

Set the CMake variable `CERBERUS_OPENSSL_ROOT` to the OpenSSL installation root. The directory must contain:

```
include/openssl/evp.h
lib/VC/x64/MD/libssl.lib
lib/VC/x64/MD/libcrypto.lib
```

Example:

```
-DCERBERUS_OPENSSL_ROOT=C:/Program Files/OpenSSL-Win64
```

Do not use the MinGW OpenSSL package for MSVC builds.

#### Gtest
Gtest library will be automatically fetched from internet, compiled and linked if not present in the system.
This will only happen if GOOGLETEST CMake option is set
