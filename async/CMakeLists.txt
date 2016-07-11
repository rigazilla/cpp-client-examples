# CMakeLists files in this project can
# refer to the root source directory of the project as ${HELLO_SOURCE_DIR} and
# to the root binary directory of the project as ${HELLO_BINARY_DIR}.
cmake_minimum_required (VERSION 2.8.11)
project (async)
add_library (hotrod SHARED IMPORTED)
set_property (TARGET hotrod PROPERTY IMPORTED_LOCATION ${HOTROD_CLIENT_PATH}/usr/lib/libhotrod.so)
include_directories (${HOTROD_CLIENT_PATH}/usr/include)

add_executable (async asyncExample.cpp)
set_target_properties(async PROPERTIES COMPILE_DEFINITIONS "HR_PROTO_EXPORT=")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
target_link_libraries (async hotrod)
target_link_libraries (async ssl)
target_link_libraries (async crypto)

