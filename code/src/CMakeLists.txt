set(image-processing_SOURCES
  main.cpp
  processing-gui.cpp
  widgets/resizingimagelabel.cpp
  processors/processor.cpp
  processors/feature_points.cpp
  processors/segmenting.cpp
  processors/null_processor.cpp
  processors/processor_model.cpp
  processors/integral_image.h
  processors/integral_image.cpp
  processors/rpoint.h
  processors/rpoint.cpp
  processors/region.h
  processors/region.cpp
  util.cpp
  util.h)
set(image-processing_HEADERS
  processing-gui.h
  widgets/resizingimagelabel.h
  processors/processor.h
  processors/feature_points.h
  processors/segmenting.h
  processors/null_processor.h
  processors/processor_model.h)
set(image-processing_FORMS ui/processing-gui.ui)

# QT4-specific preprocessor headers
qt4_wrap_cpp(image-processing_HEADERS_MOC ${image-processing_HEADERS})
qt4_wrap_ui(image-processing_FORMS_HEADERS ${image-processing_FORMS})

# make sure the compiler can find the generated _ui* files
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(processors/)
include_directories(widgets/)
include_directories(./)

include_directories (${image-processing_SOURCE_DIR}/lib/QPropertyEditor)
link_directories (${image-processing_BINARY_DIR}/lib/QPropertyEditor) 

add_executable(image-processing
  ${image-processing_HEADERS_MOC}
  ${image-processing_FORMS_HEADERS}
  ${image-processing_SOURCES})
target_link_libraries(image-processing ${QT_LIBRARIES} ${OpenCV_LIBS} QPropertyEditor)