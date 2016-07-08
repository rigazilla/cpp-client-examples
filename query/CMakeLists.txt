# CMakeLists files in this project can
# refer to the root source directory of the project as ${HELLO_SOURCE_DIR} and
# to the root binary directory of the project as ${HELLO_BINARY_DIR}.
cmake_minimum_required (VERSION 2.8.11)
project (queryExample)
add_library (hotrod SHARED IMPORTED)
add_library (hotrod_protobuf STATIC IMPORTED)
add_library (protobuf STATIC IMPORTED)
set_property (TARGET hotrod PROPERTY IMPORTED_LOCATION ${HOTROD_CLIENT_PATH}/usr/lib/libhotrod.so)
set_property (TARGET protobuf PROPERTY IMPORTED_LOCATION ${HOTROD_CLIENT_PATH}/usr/lib/libprotobuf.a)
set_property (TARGET hotrod_protobuf PROPERTY IMPORTED_LOCATION ${HOTROD_CLIENT_PATH}/usr/lib/libhotrod_protobuf.a)
include_directories (${HOTROD_CLIENT_PATH}/usr/include include)

add_executable (queryExample bank.pb.cc addressbook.pb.cc queryExample.cpp)
set_target_properties(queryExample PROPERTIES COMPILE_DEFINITIONS "HR_PROTO_EXPORT=")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
target_link_libraries (queryExample hotrod)
target_link_libraries (queryExample ssl)
target_link_libraries (queryExample crypto)
target_link_libraries (queryExample protobuf)
target_link_libraries (queryExample hotrod_protobuf)

