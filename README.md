# XY2D-SDL3
Simple 2D game/render library based on SDL3.

Programs can be compiled by opening the _BUILDENV batch file within the folder directory and then running ".\_DEBUG" to compile the executable and then running ".\_SHADERS _DEBUG" to compile debug build for shaders. You can also substitute _DEBUG for _RELEASE to compile for release builds. If you have VSCODE setup in your MS Window's environment path variables you can click on and open the _CODE batch script to open VSCODE directly into this project folder. In order to COMPILE the program and shaders you must first open _BUILDENV script and run the build scripts from the associated command prompt as the _BUILDENV script setups the temporary environment build paths:
<img width="1689" height="1163" alt="image" src="https://github.com/user-attachments/assets/849fbaa4-9e14-49fd-9150-64fcd7690860" />


Building requires adjusting the compiler path to your version and install path of the LLVM clang-cl compiler and SDL3 and GLM precompiled libraries within the _BUILDENV batch script. Note that these should be source directories, for example:

For SDL3:

	C:\lib-includes\SDL3-3.4.2
	
	C:\lib-includes\SDL3-3.4.2\include\
	
	C:\lib-includes\SDL3-3.4.2\lib\x64\SDL3.lib

For GLM:

	C:\lib-includes\glm-master
	
	C:\lib-includes\glm-master\glm

Upon successful compilation you should get an SDL window and debug console (debug only) running a simple UV compute shader. Left or Right clicking updates stored mouse click positions:

<img width="1660" height="1122" alt="image" src="https://github.com/user-attachments/assets/86ca7ee8-596d-4820-bab2-ce5013ef20d3" />

For rendering the orthographic projection matrix is based into the **Default Vertex Shader** for 2D camera/world transforms.
