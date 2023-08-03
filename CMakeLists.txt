cmake_minimum_required(VERSION 3.5.1)
project(cmake_protobuf)

# Find protobuf
find_package(Protobuf REQUIRED)
if (PROTOBUF_FOUND)
    message("protobuf found")
else () 
    message(FATAL_ERROR "Cannot find Protobuf")
endif ()
if(NOT ZLIB_FOUND)
  find_package(ZLIB)
endif()
if(NOT TARGET absl::strings)
  find_package(absl CONFIG)
endif()
if(NOT TARGET utf8_range)
  find_package(utf8_range CONFIG)
endif()

# Compile the .proto file into .cpp and .h file
set(PROTO_FILES CanalProtocol.proto EntryProtocol.proto)
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})
message("PROTO_SRCS = ${PROTO_SRCS}")
message("PROTO_HDRS = ${PROTO_HDRS}")

set(CMAKE_CXX_STANDARD 17)

# include_directories(${PROJECT_SOURCE_DIR}/include)
# include_directories(${PROJECT_SOURCE_DIR}/src)

set(CMAKE_CXX_FLAGS "-g -Wextra")

add_executable(cmake_protobuf
        test.cpp
        protobuf.cpp
        ${PROTO_SRCS}
        ${PROTO_HDRS})

# Include dependent
target_include_directories(${PROJECT_NAME}
PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
PUBLIC ${PROTOBUF_INCLUDE_DIRS})

# Library dependent
target_link_libraries(${PROJECT_NAME} ${PROTOBUF_LIBRARIES} 
absl_die_if_null
absl_statusor
absl_log_internal_check_op
absl_log_internal_message
absl_log_internal_nullguard
absl_hash
absl_raw_hash_set
absl_status
absl_cord
absl_cordz_info
absl_synchronization
absl_time
absl_time_zone
absl_str_format_internal
absl_strings
absl_throw_delegate
absl_raw_logging_internal
absl_spinlock_wait
    )
