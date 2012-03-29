cmake_minimum_required(VERSION 2.8)
project(image-processing)
find_package(OpenCV REQUIRED)
find_package(Qt4 REQUIRED)

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Werror")
endif()

set(CMAKE_BUILD_TYPE Debug)

add_subdirectory(lib)
add_subdirectory(src)

