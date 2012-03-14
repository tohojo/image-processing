set(QPropertyEditor_SOURCES
  ColorCombo.cpp
  EnumProperty.cpp
  Property.cpp
  QPropertyEditorWidget.cpp
  QPropertyModel.cpp
  QVariantDelegate.cpp)
set(QPropertyEditor_HEADERS
  ColorCombo.h
  EnumProperty.h
  Property.h
  QPropertyEditorWidget.h
  QPropertyModel.h
  QVariantDelegate.h)

if(CMAKE_COMPILER_IS_GNUCXX)
	string(REPLACE "-Wall -Wextra -pedantic -Werror" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()


# QT4-specific preprocessor headers
qt4_wrap_cpp(QPropertyEditor_HEADERS_MOC ${QPropertyEditor_HEADERS})

add_library(QPropertyEditor
  ${QPropertyEditor_HEADERS_MOC}
  ${QPropertyEditor_SOURCES})
target_link_libraries(QPropertyEditor ${QT_LIBRARIES})