What I have implemented
	a Blin shading teapot
	ctrl + left click rotate the light around the teapot
	right click to adjust the distance to camera
	left click to adjust the angles of camera

What I could not implemented
	None

Additional functionalities beyond project requirements
	both for OSX Yosemite and Windows 10

How to use my implementation
OSX Yosemite
1.install cmake 3.1.1 or above
2. Install GLFW
2.1 Download GLFW from http://www.glfw.org/ and extract glfw-3.2.1.zip
2.2 in glfw-3.2.1 mkdir glfw-build, cd glfw-build, cmake ..
2.3 make install, then GLFW is installed in /usr/local/lib
    and the include is in /usr/local/include/GLFW
3. make a build folder in Project3 folder, cd to the build folder and type cmake..
then type make
then type ./Shading ../obj/teapot.obj

Windows10
1. Make sure you have Visual Studio 2015
2. Install GLFW
2.1 Download GLFW from http://www.glfw.org/ and extract glfw-3.2.1.zip
2.2 In glfw-3.2.1 mkdir glfw-build, cd glfw-build, cmake .. -G "Visual Studio 14 Win64"
2.3 cmake --build .  --target INSTALL --config Release
    then you can see that GLFW is installed in C:/Program Files/GLFW
3. make a build foder in Project3, cd to the build folder and type cmake .. -G "Visual Studio 14 Win64"
4. Open Transformations.sln in the build folder using VS2015
5. Set Shading as start project
6. In project properties -> Debugging set Command Arguments to "../obj/teapot.obj"
7. Build Transformations
8. run Transformations from VS

