cmake_minimum_required(VERSION 2.8)
project(AdaptiveSegment)
find_package(OpenCV REQUIRED)
add_executable(AdaptiveSegment AdaptiveSegment)
target_link_libraries(AdaptiveSegment ${OpenCV_LIBS})


