set(distortion_SOURCES
  ami_pol.cpp
  ami_tif.cpp
  lens_distortion.cpp
  lens_distortion_estimation.cpp)
set(distortion_HEADERS
  ami_pol.h
  ami_tif.h
  lens_distortion.h
  lens_distortion_estimation.h)

if(CMAKE_COMPILER_IS_GNUCXX)
	string(REPLACE "-Wall -Wextra -pedantic -Werror" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()


add_library(distortion
  ${distortion_HEADERS}
  ${distortion_SOURCES})
target_link_libraries(distortion)