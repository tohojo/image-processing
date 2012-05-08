cmake_minimum_required(VERSION 2.8)
project(image-processing)
find_package(OpenCV REQUIRED)
find_package(Qt4 REQUIRED)

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

option(SEGMENTING "Build segmenting processor" ON)
option(FEATUREPOINTS "Build feature point extraction processor" ON)
option(CALIBRATION "Build calibration processor" ON)
option(DISTORTION "Build distortion removal processor" ON)
option(TESTS "Build tests" ON)



add_subdirectory(lib)
add_subdirectory(src)

if(TESTS)
  message("Building tests")
  add_subdirectory(tests)
endif()
