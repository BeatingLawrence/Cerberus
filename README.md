# Cerberus
This is the Cerberus Framework

####Dependencies:
-Boost
-OpenSSL

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
