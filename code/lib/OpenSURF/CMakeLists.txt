set(OpenSURF_SOURCES
  fasthessian.cpp
  integral.cpp
  ipoint.cpp
  surf.cpp
  utils.cpp)
set(OpenSURF_HEADERS
  fasthessian.h
  integral.h
  ipoint.h
  kmeans.h
  responselayer.h
  surf.h
  surflib.h
  utils.h)

if(CMAKE_COMPILER_IS_GNUCXX)
	string(REPLACE "-Wall -Wextra -pedantic -Werror" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()


add_library(OpenSURF
  ${OpenSURF_HEADERS}
  ${OpenSURF_SOURCES})
target_link_libraries(OpenSURF ${OpenCV_LIBS})