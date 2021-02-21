cmake_minimum_required(VERSION 3.0)
set ( CMAKE_CXX_STANDARD 11 )
project( drawtown )
find_package( OpenCV REQUIRED )

include_directories( ${OpenCV_INCLUDE_DIRS} )
include_directories( include/ )
file( GLOB SOURCES src/*.cpp )

add_executable( drawtown ${SOURCES} )
target_link_libraries( drawtown ${OpenCV_LIBS} )
