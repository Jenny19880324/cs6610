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
    set(FREEGLUT_DIR "C:\\Users\\Lijin\\Downloads\\freeglut-MSVC-3.0.0-2.mp\\freeglut\\")
    set(FREEGLUT_INCLUDE_DIR "${FREEGLUT_DIR}\\include")
    set(FREEGLUT_DLL "${FREEGLUT_DIR}/bin/x64/freeglut.dll")
    set(FREEGLUT_LIB_DIR "${FREEGLUT_DIR}\\lib\\x64")
    include_directories(${FREEGLUT_INCLUDE_DIR})
    link_directories(${FREEGLUT_LIB_DIR})
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

#Find GLEW
set(GLEW_DIR "C:\\Users\\Lijin\\Downloads\\glew-2.0.0-win32\\glew-2.0.0\\")
set(GLEW_INCLUDE_DIR "${GLEW_DIR}\\include")
set(GLEW_DLL "${GLEW_DIR}\\bin\\Release\\x64\\glew32.dll")
set(GLEW_LIB_DIR "${GLEW_DIR}\\lib\\Release\\x64")
set(GLEW_LIB "glew32.lib")
include_directories(${GLEW_INCLUDE_DIR})
link_directories(${GLEW_LIB_DIR})

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
target_link_libraries(${PROJ} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${GLEW_LIB})

#COPY DLL to exe folder
if (MSVC)
    add_custom_command(TARGET Transformations POST_BUILD
	    COMMAND ${CMAKE_COMMAND} -E copy_if_different
	    ${FREEGLUT_DLL} ${GLEW_DLL}
	    $<TARGET_FILE_DIR:Transformations>)
endif (MSVC)
