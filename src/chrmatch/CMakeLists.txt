cmake_minimum_required(VERSION 2.8)

find_package( OpenCV REQUIRED )

MACRO(ADD_MSVC_PRECOMPILED_HEADER PrecompiledHeader PrecompiledSource SourcesVar)
  IF(MSVC)
    GET_FILENAME_COMPONENT(PrecompiledBasename ${PrecompiledHeader} NAME_WE)
    SET(PrecompiledBinary "${CMAKE_CURRENT_BINARY_DIR}/${PrecompiledBasename}.pch")
    SET(Sources ${${SourcesVar}})

    SET_SOURCE_FILES_PROPERTIES(${PrecompiledSource}
                                PROPERTIES COMPILE_FLAGS "/Yc\"${PrecompiledHeader}\" /Fp\"${PrecompiledBinary}\""
                                           OBJECT_OUTPUTS "${PrecompiledBinary}")
    SET_SOURCE_FILES_PROPERTIES(${Sources}
                                PROPERTIES COMPILE_FLAGS "/Yu\"${PrecompiledHeader}\" /FI\"${PrecompiledBinary}\" /Fp\"${PrecompiledBinary}\""
                                           OBJECT_DEPENDS "${PrecompiledBinary}")  
    # Add precompiled header to SourcesVar
    LIST(APPEND ${SourcesVar} ${PrecompiledSource})
  ENDIF(MSVC)
ENDMACRO(ADD_MSVC_PRECOMPILED_HEADER)

include_directories(../../include)
SET(SOURCES chrmatch.cpp generator.cpp winfont.cpp ftfont.cpp)
ADD_MSVC_PRECOMPILED_HEADER("stdafx.h" "stdafx.cpp" SOURCES)
IF (MSVC)
add_definitions( "/wd4005 /wd4996 /nologo" )
ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)
ENDIF(MSVC)

IF (WIN32)
# Use windows native library. No external dependency
ELSE (WIN32)
find_package(Freetype REQUIRED)
include_directories(${FREETYPE_INCLUDE_DIRS})
ENDIF (WIN32)

IF(CMAKE_COMPILER_IS_GNUCXX)
add_definitions("-std=c++11")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)
add_library(chrmatch ${SOURCES})
