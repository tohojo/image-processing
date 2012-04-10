include_directories(../src/processors/)
include_directories(../src/widgets/)
include_directories(../src/)

if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Wno-unused-parameter")
endif()


add_executable(region-test
  region-test.cpp
  ../src/processors/rpoint.h
  ../src/processors/rpoint.cpp
  ../src/processors/region.h
  ../src/processors/region.cpp
)
target_link_libraries(region-test ${QT_LIBRARIES} ${OpenCV_LIBS} QPropertyEditor)

add_executable(rpoint-test
  rpoint-test.cpp
  ../src/processors/rpoint.h
  ../src/processors/rpoint.cpp
)
target_link_libraries(rpoint-test ${OpenCV_LIBS})

add_executable(features-test
  features-test.cpp
  ../src/processors/fast_hessian.h
  ../src/processors/fast_hessian.cpp
  ../src/processors/integral_image.h
  ../src/processors/integral_image.cpp
  ../src/processors/response_layer.h
  ../src/processors/response_layer.cpp
)
target_link_libraries(features-test ${QT_LIBRARIES} ${OpenCV_LIBS})
