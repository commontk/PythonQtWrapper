
CONFIGURE_FILE(
  ctkPythonQtWrapperVersion.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/ctkPythonQtWrapperVersion.h
  @ONLY
  )

SET(KIT_SRCS
  ctkCommandLineParser.cpp
  ctkCommandLineParser.h
  ctkPythonQtWrapper.cpp
  ctkPythonQtWrapper.h
  main.cpp
  )

SOURCE_GROUP("Generated" FILES
  ${CMAKE_CURRENT_BINARY_DIR}/ctkPythonQtWrapperVersion.h
  )
  
INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )
  
ADD_EXECUTABLE(${PROJECT_NAME} ${KIT_SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${QT_LIBRARIES})
