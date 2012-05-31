cmake_minimum_required(VERSION 2.8)
project(image-processing)
find_package(OpenCV REQUIRED)
find_package(Qt4 REQUIRED)

add_definitions("-DOPENCV_VERSION=${OpenCV_VERSION_MAJOR}${OpenCV_VERSION_MINOR}")

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

option(SEGMENTING "Build segmenting processor" ON)
option(FEATUREPOINTS "Build feature point extraction processor" ON)
option(CALIBRATION "Build calibration processor" ON)
option(DISTORTION "Build distortion removal processor" ON)
option(RECTIFICATION "Build rectification processor" ON)
option(RESIZING "Build resizing processor" ON)
option(STEREO "Build stereo matching processor" ON)
option(PCA_773 "Build PCA training processor" ON)
option(FACE_NORMAL "Build face normalisation processor" ON)
option(TESTS "Build tests" OFF)



add_subdirectory(lib)
add_subdirectory(src)

if(TESTS)
  message(STATUS "Building tests")
  add_subdirectory(tests)
endif()
