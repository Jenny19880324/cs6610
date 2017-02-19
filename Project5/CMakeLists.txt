cmake_minimum_required(VERSION 3.1.1)

#to supress generation of ZERO_CHECK
set(CMAKE_SUPPRESS_GENERATION TRUE)
set(PROJ RenderBuffers)

project(${PROJ})

#GLFW
if (MSVC)
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "C:/Program Files/GLFW/lib/cmake")
    set(GLFW_INCLUDE_DIR "C:/Program Files/GLFW/include")
elseif (APPLE)
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "/usr/locla/lib/cmake")
    set(GLFW_INCLUDE_DIR "/usr/local/include/GLFW")
endif (MSVC)

find_package(glfw3 3.2 REQUIRED)

#EXECUTABLE
add_executable(${PROJ}
src/main.cpp
src/glad.cpp
src/lodepng.cpp)

#INCLUDE
target_include_directories(${PROJ} PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${GLFW_INCLUDE_DIR})


#LINKING
target_link_libraries(${PROJ} glfw)
