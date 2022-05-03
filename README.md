# Cerberus
This is the Cerberus Framework

#### Development Environment Build
1. Open the Cerberus project using CMakeList.txt file
2. Select a NON-QT toolchain and use only "Debug" and "Release" configurations
3. In both configurations edit as follows:
	1. CMAKE_INSTALL_PREFIX parameter equal to "../Dist"
	2. Check "Cerberus" and "install" build steps
	3. Uncheck "all" build step
4. Add "test" configuration starting from "Debug" template (make sure it points to a dedicated empty directory)
5. Set "test" as single target for the test configuration

Compile Debug, Release and test configuration (in this order) to check if setup works 
