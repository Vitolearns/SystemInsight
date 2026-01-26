set(PROTO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/system_insight.proto)

set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(PROTO_SRCS ${GENERATED_DIR}/system_insight.pb.cc)
set(PROTO_HDRS ${GENERATED_DIR}/system_insight.pb.h)
set(GRPC_SRCS ${GENERATED_DIR}/system_insight.grpc.pb.cc)
set(GRPC_HDRS ${GENERATED_DIR}/system_insight.grpc.pb.h)

# Find protoc executable
find_program(PROTOC_EXECUTABLE protoc)
if(NOT PROTOC_EXECUTABLE)
    message(FATAL_ERROR "protoc not found")
endif()

# Find grpc_cpp_plugin
find_program(GRPC_PLUGIN_EXECUTABLE grpc_cpp_plugin)
if(NOT GRPC_PLUGIN_EXECUTABLE)
    message(FATAL_ERROR "grpc_cpp_plugin not found")
endif()

add_custom_command(
    OUTPUT ${PROTO_SRCS} ${PROTO_HDRS} ${GRPC_SRCS} ${GRPC_HDRS}
    COMMAND ${PROTOC_EXECUTABLE}
    ARGS --grpc_out=${GENERATED_DIR}
         --cpp_out=${GENERATED_DIR}
         --plugin=protoc-gen-grpc=${GRPC_PLUGIN_EXECUTABLE}
         -I ${CMAKE_CURRENT_SOURCE_DIR}
         ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
)

add_library(system_insight_proto STATIC
    ${PROTO_SRCS}
    ${PROTO_HDRS}
    ${GRPC_SRCS}
    ${GRPC_HDRS}
)

target_include_directories(system_insight_proto PUBLIC ${GENERATED_DIR})
target_link_libraries(system_insight_proto
    PUBLIC
    protobuf::libprotobuf
    grpc::grpc++
)

# Add absl if available
if(absl_FOUND)
    target_link_libraries(system_insight_proto PUBLIC absl::log)
endif()
