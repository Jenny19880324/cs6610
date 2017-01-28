cmake_minimum_required(VERSION 2.8.9)

#to supress generation of ZERO_CHECK
set(CMAKE_SUPPRESS_GENERATION true)
set(SRCS src/main.cpp)
set(PROJ Transformations)
set(CY_INCLUDE_DIR dependencies/cyCodeBase)

project(${PROJ})

#FIND OPENGL
find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIRS})
link_directories(${OpenGL_DEFINITIONS})
if(NOT OPENGL_FOUND)
    message(ERROR "OpenGL not found!")
endif(NOT OPENGL_FOUND)


#Find FREEGLUT
if (MSVC)
    set(GLUT_INCLUDE_DIR ./dependencies/freeglut/include)
    set(GLUT_glut_LIBRARY ./dependencies/freeglut/lib/x64)
    set(GLUT_DLL_DIR ./dependencies/freeglut/bin/x64)
    set(GLUT_LIBRARY freeglut)
    include_directories(${GLUT_INCLUDE_DIR})
    link_directories(${GLUT_glut_LIBRARY})
#Find GLUT
elseif (APPLE)
    find_package(GLUT REQUIRED)
    include_directories(${GLUT_INCLUDE_DIRS})
    link_directories(${GLUT_LIBRSRY_DIRS})
    add_definitions(${GLUT_DEFINITIONS})
    if(NOT GLUT_FOUND)
        message(ERROR "GLUT not found!")
    endif(NOT GLUT_FOUND)
endif (MSVC)

#to use the cyTriMesh code release for parsing the obj file
include_directories(${CY_INCLUDE_DIR})

#DISABLE WARNINGS FROM GL3
if (APPLE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-deprecated-declarations")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
endif (APPLE)

#EXECUTABLE
add_executable(${PROJ}
    ${SRCS})

#LINKING
target_link_libraries(${PROJ} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})

#COPY DLL to exe folder
if (MSVC)
    add_custom_command(TARGET ${PROJ} POST_BUILD
	    COMMAND ${CMAKE_COMMAND} -E copy_if_different
	    "${PROJECT_SOURCE_DIR}/${GLUT_DLL_DIR}/freeglut.dll"
	    $<TARGET_FILE_DIR:helloWorld>)
endif (MSVC)
