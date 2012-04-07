set(image-processing_SOURCES
  main.cpp
  message-handler.cpp
  processing-gui.cpp
  widgets/graphicsview.cpp
  widgets/imagegraphicsitem.cpp
  widgets/poiitem.h
  widgets/poiitem.cpp
  widgets/resizingimagelabel.cpp
  widgets/signalingdockwidget.cpp
  processors/processor.cpp
  processors/segmenting.cpp
  processors/null_processor.cpp
  processors/processor_model.cpp
  processors/rpoint.h
  processors/rpoint.cpp
  processors/region.h
  processors/region.cpp
  util.cpp
  util.h)
set(image-processing_HEADERS
  message-handler.h
  processing-gui.h
  widgets/graphicsview.h
  widgets/imagegraphicsitem.h
  widgets/resizingimagelabel.h
  widgets/signalingdockwidget.h
  processors/processor.h
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