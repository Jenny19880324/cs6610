cmake_minimum_required(VERSION 2.8.9)
project(helloWorld)

#to supress generation of ZERO_CHECK
set(CMAKE_SUPPRESS_GENERATION true)

#FIND OPENGL
find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIRS})
link_directories(${OpenGL_DEFINITIONS})
if(NOT OPENGL_FOUND)
    message(ERROR "OpenGL not found!")
endif(NOT OPENGL_FOUND)


#Find FREEGLUT
set(GLUT_INCLUDE_DIR ./dependencies/freeglut/include)
set(GLUT_glut_LIBRARY ./dependencies/freeglut/lib/x64)
set(GLUT_DLL_DIR ./dependencies/freeglut/bin/x64)
set(GLUT_LIBRARIES freeglut)
include_directories(${GLUT_INCLUDE_DIR})
link_directories(${GLUT_glut_LIBRARY})
#DISABLE WARNINGS FROM GL3
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-deprecated-declarations")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

#EXECUTABLE
add_executable(helloWorld
    src/main.cpp)

#LINKING
target_link_libraries(helloWorld ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})

#COPY DLL to exe folder
add_custom_command(TARGET helloWorld POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${PROJECT_SOURCE_DIR}/${GLUT_DLL_DIR}/freeglut.dll"
	$<TARGET_FILE_DIR:helloWorld>)
