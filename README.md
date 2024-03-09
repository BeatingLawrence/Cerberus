# Cerberus
This is the Cerberus Framework

####Dependencies:
-Boost
-OpenSSL

#### Development Environment Build
1. Open the Cerberus project using CMakeList.txt file
2. Select a NON-QT toolchain and use only "Debug" and "Release" configurations
3. In both configurations edit as follows:
	1. Check "Cerberus" and "install" build steps
	2. Uncheck "all" build step
4. Add "test" configuration starting from "Debug" template (make sure it points to a dedicated empty directory)
5. Set "test" as single target for the test configuration

Compile Debug, Release and test configuration (in this order) to check if setup works 

##### MSVC Important Note:
In MSVC configuration, the gtest and gmock testing libraries are not searched in the system path but in the workspace.

Please copy a pre-compiled gtest version under "gtest" directory at the same level of "CMakeList.txt" file.

inside the gtest directory, "lib" and "include" directories of gtest must be present.

IMPORTANT: Gtest library must have been compiled with RunTime Library set to "SHARED".

Please refer to https://github.com/google/googletest to get the sources of google test library.
