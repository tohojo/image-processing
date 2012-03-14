set(image-processing_SOURCES
  main.cpp
  processing-gui.cpp
  resizingimagelabel.cpp
  util.cpp
  headers/util.h)
set(image-processing_HEADERS
  headers/processing-gui.h
  headers/resizingimagelabel.h)
set(image-processing_FORMS ui/processing-gui.ui)

# QT4-specific preprocessor headers
qt4_wrap_cpp(image-processing_HEADERS_MOC ${image-processing_HEADERS})
qt4_wrap_ui(image-processing_FORMS_HEADERS ${image-processing_FORMS})

# make sure the compiler can find the generated _ui* files
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(headers/)

include_directories (${image-processing_SOURCE_DIR}/lib/QPropertyEditor)
link_directories (${image-processing_BINARY_DIR}/lib/QPropertyEditor) 

add_executable(image-processing
  ${image-processing_HEADERS_MOC}
  ${image-processing_FORMS_HEADERS}
  ${image-processing_SOURCES})
target_link_libraries(image-processing ${QT_LIBRARIES} ${OpenCV_LIBS} QPropertyEditor)