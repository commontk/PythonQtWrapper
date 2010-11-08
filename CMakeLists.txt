###########################################################################
#
#  Library:   CTK
#
#  Copyright (c) Kitware Inc.
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.commontk.org/LICENSE
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
###########################################################################

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.2)

SET(project_policies
  CMP0001 # NEW: CMAKE_BACKWARDS_COMPATIBILITY should no longer be used.
  CMP0002 # NEW: Logical target names must be globally unique.
  CMP0003 # NEW: Libraries linked via full path no longer produce linker search paths.
  CMP0004 # NEW: Libraries linked may NOT have leading or trailing whitespace.
  CMP0005 # NEW: Preprocessor definition values are now escaped automatically.
  CMP0006 # NEW: Installing MACOSX_BUNDLE targets requires a BUNDLE DESTINATION.
  CMP0007 # NEW: List command no longer ignores empty elements.
  CMP0008 # NEW: Libraries linked by full-path must have a valid library file name.
  CMP0009 # NEW: FILE GLOB_RECURSE calls should not follow symlinks by default.
  CMP0010 # NEW: Bad variable reference syntax is an error.
  CMP0011 # NEW: Included scripts do automatic cmake_policy PUSH and POP.
  CMP0012 # NEW: if() recognizes numbers and boolean constants.
  CMP0013 # NEW: Duplicate binary directories are not allowed.
  CMP0014 # NEW: Input directories must have CMakeLists.txt
  )
FOREACH(policy ${project_policies})
  IF(POLICY ${policy})
    CMAKE_POLICY(SET ${policy} NEW)
  ENDIF()
ENDFOREACH()

#-----------------------------------------------------------------------------
PROJECT(PythonQtWrapper)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Output directories.
#-----------------------------------------------------------------------------
FOREACH(type LIBRARY RUNTIME ARCHIVE)
  SET(CMAKE_${type}_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin CACHE INTERNAL "Single output directory for building all libraries.")
ENDFOREACH()

#-----------------------------------------------------------------------------
# Qt
#-----------------------------------------------------------------------------
SET(minimum_required_qt_version "4.6")

FIND_PACKAGE(Qt4)

IF(QT4_FOUND)

  IF("${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH}" VERSION_LESS "${minimum_required_qt_version}")
    MESSAGE(FATAL_ERROR "error: CTK requires Qt >= ${minimum_required_qt_version} -- you cannot use Qt ${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH}.")
  ENDIF()
  
  INCLUDE(${QT_USE_FILE})

ELSE(QT4_FOUND)
  MESSAGE(FATAL_ERROR "error: Qt${minimum_required_qt_version} was not found on your system. You probably need to set the QT_QMAKE_EXECUTABLE variable")
ENDIF(QT4_FOUND)

#-----------------------------------------------------------------------------
# PythonQtWrapper version number.  An even minor number corresponds to releases.
#-----------------------------------------------------------------------------
SET(PythonQtWrapper_MAJOR_VERSION 0)
SET(PythonQtWrapper_MINOR_VERSION 0)
SET(PythonQtWrapper_BUILD_VERSION 0)
SET(PythonQtWrapper_VERSION
    "${PythonQtWrapper_MAJOR_VERSION}.${PythonQtWrapper_MINOR_VERSION}.${PythonQtWrapper_BUILD_VERSION}")

#-----------------------------------------------------------------------------
# Set C/CXX Flags
#-----------------------------------------------------------------------------
IF(CMAKE_COMPILER_IS_GNUCXX)
  SET(cflags "-Wall -Wextra -Wpointer-arith -Winvalid-pch -Wcast-align -Wwrite-strings -D_FORTIFY_SOURCE=2")
  SET(CMAKE_C_FLAGS "${cflags} ${CMAKE_C_FLAGS}" CACHE STRING "CMake C Flags" FORCE)
  SET(CMAKE_CXX_FLAGS "${cflags} -Woverloaded-virtual -Wold-style-cast -Wstrict-null-sentinel -Wsign-promo ${CMAKE_CXX_FLAGS}" CACHE STRING "CMake CXX Flags" FORCE)
ENDIF()

#-----------------------------------------------------------------------------
# Subdirectories
#-----------------------------------------------------------------------------

ADD_SUBDIRECTORY(srcs)


