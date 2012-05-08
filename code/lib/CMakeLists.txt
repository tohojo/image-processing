add_subdirectory(QPropertyEditor)

if(FEATUREPOINTS)
  add_subdirectory(OpenSURF)
endif()

if(DISTORTION)
  add_subdirectory(distortion)
endif()