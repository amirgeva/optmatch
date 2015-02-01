cmake_minimum_required(VERSION 2.8)
include_directories(../../../include)

find_package( OpenCV REQUIRED )
set(OpenCV_LIBS opencv_core opencv_imgproc opencv_calib3d opencv_video opencv_features2d opencv_ml opencv_highgui opencv_objdetect opencv_contrib opencv_legacy opencv_gpu)

IF(CMAKE_COMPILER_IS_GNUCXX)
add_definitions("-std=c++11")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

IF (WIN32)
# Use windows native library. No external dependency
set(ftlibs)
ELSE (WIN32)
find_package(Freetype REQUIRED)
include_directories(${FREETYPE_INCLUDE_DIRS})
set(ftlibs freetype)
ENDIF (WIN32)


add_executable(ocr main.cpp)
target_link_libraries(ocr chrmatch ${OpenCV_LIBS} ${ftlibs})
