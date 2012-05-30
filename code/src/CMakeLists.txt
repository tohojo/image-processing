

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
  widgets/qfileinfo_property.cpp
  widgets/custom_types.h
  widgets/custom_types.cpp
  widgets/file_edit_widget.cpp
  processors/processor.cpp
  processors/null_processor.cpp
  processors/two_image_processor.cpp
  processors/processor_model.cpp
  util.cpp
  util.h)
set(image-processing_HEADERS
  message-handler.h
  processing-gui.h
  widgets/graphicsview.h
  widgets/imagegraphicsitem.h
  widgets/resizingimagelabel.h
  widgets/signalingdockwidget.h
  widgets/qfileinfo_property.h
  widgets/file_edit_widget.h
  processors/processor.h
  processors/null_processor.h
  processors/two_image_processor.h
  processors/processor_model.h
  )
set(image-processing_LIBS
 QPropertyEditor )

if(SEGMENTING)
  message(STATUS "Building segmenting")
  add_definitions(-DSEGMENTING)
  set(image-processing_SOURCES ${image-processing_SOURCES}
    processors/rpoint.h
    processors/rpoint.cpp
    processors/region.h
    processors/region.cpp
    processors/threshold_segmenter.h
    processors/threshold_segmenter.cpp
    processors/segmenting.cpp)
  set(image-processing_HEADERS ${image-processing_HEADERS}
    processors/segmenting.h)
endif()

if(FEATUREPOINTS)
  message(STATUS "Building feature point extraction")
  add_definitions(-DFEATURE_POINTS)
  set(image-processing_SOURCES ${image-processing_SOURCES}
    processors/fast_hessian.h
    processors/fast_hessian.cpp
    processors/response_layer.h
    processors/response_layer.cpp
    processors/integral_image.h
    processors/integral_image.cpp
    processors/feature_points.cpp)
  set(image-processing_HEADERS ${image-processing_HEADERS}
    processors/feature_points.h)
  set(image-processing_LIBS ${image-processing_LIBS}
    OpenSURF)
endif()

if(CALIBRATION)
  message(STATUS "Building calibration")
  add_definitions(-DCALIBRATION)
  set(image-processing_SOURCES ${image-processing_SOURCES}
    processors/calibration_processor.cpp
    processors/camera_calibrator.h
    processors/camera_calibrator.cpp
    processors/calibration_object.h
    processors/calibration_object.cpp
# We need the thresholding and feature point stuff for calibration as well...
    processors/threshold_segmenter.h
    processors/threshold_segmenter.cpp
    processors/fast_hessian.h
    processors/fast_hessian.cpp
    processors/response_layer.h
    processors/response_layer.cpp
    processors/integral_image.h
    processors/integral_image.cpp)
  set(image-processing_HEADERS ${image-processing_HEADERS}
    processors/calibration_processor.h)
endif()

if(DISTORTION)
  message(STATUS "Building distortion removal")
  add_definitions(-DDISTORTION)
  set(image-processing_SOURCES ${image-processing_SOURCES}
    processors/distortion-removal.cpp)
  set(image-processing_HEADERS ${image-processing_HEADERS}
    processors/distortion-removal.h)
  set(image-processing_LIBS ${image-processing_LIBS}
    distortion)
endif()

if(RECTIFICATION)
  message(STATUS "Building rectification")
  add_definitions(-DRECTIFICATION)
  set(image-processing_SOURCES ${image-processing_SOURCES}
    processors/rectification_processor.cpp)
  set(image-processing_HEADERS ${image-processing_HEADERS}
    processors/rectification_processor.h)
endif()

if(RESIZING)
  message(STATUS "Building resizer")
  add_definitions(-DRESIZING)
  set(image-processing_SOURCES ${image-processing_SOURCES}
    processors/resizing_processor.cpp)
  set(image-processing_HEADERS ${image-processing_HEADERS}
    processors/resizing_processor.h)
endif()

if(STEREO)
  message(STATUS "Building stereo-matching")
  add_definitions(-DSTEREO)
  set(image-processing_SOURCES ${image-processing_SOURCES}
#    processors/stereomatch.cpp
    processors/stereo_processor.cpp)
  set(image-processing_HEADERS ${image-processing_HEADERS}
#    processors/stereomatch.h
    processors/stereo_processor.h)
endif()

if(PCA_773)
  message(STATUS "Building PCA training")
  add_definitions(-DPCA_773)
  set(image-processing_SOURCES ${image-processing_SOURCES}
  processors/pca_training_processor.cpp)
  set(image-processing_HEADERS ${image-processing_HEADERS}
  processors/pca_training_processor.h)
endif()

if(FACE_NORMAL)
  message(STATUS "Building face normalisation")
  add_definitions(-DFACE_NORMAL)
  set(image-processing_SOURCES ${image-processing_SOURCES}
  processors/face_normalisation_processor.cpp)
  set(image-processing_HEADERS ${image-processing_HEADERS}
  processors/face_normalisation_processor.h)
endif()

set(image-processing_FORMS ui/processing-gui.ui ui/file-edit-widget.ui)

# QT4-specific preprocessor headers
qt4_wrap_cpp(image-processing_HEADERS_MOC ${image-processing_HEADERS})
qt4_wrap_ui(image-processing_FORMS_HEADERS ${image-processing_FORMS})

# make sure the compiler can find the generated _ui* files
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(processors/)
include_directories(widgets/)
include_directories(./)

include_directories (${image-processing_SOURCE_DIR}/lib/QPropertyEditor)
include_directories (${image-processing_SOURCE_DIR}/lib/OpenSURF)
include_directories (${image-processing_SOURCE_DIR}/lib/distortion)
link_directories (${image-processing_BINARY_DIR}/lib/QPropertyEditor)
link_directories (${image-processing_BINARY_DIR}/lib/OpenSURF)
link_directories (${image-processing_BINARY_DIR}/lib/distortion)

add_executable(image-processing
  ${image-processing_HEADERS_MOC}
  ${image-processing_FORMS_HEADERS}
  ${image-processing_SOURCES})
target_link_libraries(image-processing ${QT_LIBRARIES} ${OpenCV_LIBS} ${image-processing_LIBS})
