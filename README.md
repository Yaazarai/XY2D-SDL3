# XY2D-SDL3
Simple 2D game/render library based on SDL3.

Programs can be compiled by opening the ".\BUILDENV" batch file within the folder directory and then running ".\_DEBUG" to compile the executable and then running ".\_SHADERS _DEBUG" to compile debug build for shaders. You can also substitute _DEBUG for _RELEASE to compile for release builds.

Building requires adjusting the compiler path to your version and install path of the LLVM clang-cl compiler and SDL3 and GLM precompiled lbiraries within the _BUILDENV batch script. Note that these should be source directories, for example:

For SDL3:
	C:\lib-includes\SDL3-3.4.2
	C:\lib-includes\SDL3-3.4.2\include\
	C:\lib-includes\SDL3-3.4.2\lib\x64\SDL3.lib

For GLM:
	C:\lib-includes\glm-master
	C:\lib-includes\glm-master\glm