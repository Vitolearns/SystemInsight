set(PROTO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/system_insight.proto)

set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(PROTO_SRCS ${GENERATED_DIR}/system_insight.pb.cc)
set(PROTO_HDRS ${GENERATED_DIR}/system_insight.pb.h)
set(GRPC_SRCS ${GENERATED_DIR}/system_insight.grpc.pb.cc)
set(GRPC_HDRS ${GENERATED_DIR}/system_insight.grpc.pb.h)

add_custom_command(
    OUTPUT ${PROTO_SRCS} ${PROTO_HDRS} ${GRPC_SRCS} ${GRPC_HDRS}
    COMMAND protobuf::protoc
    ARGS --grpc_out=${GENERATED_DIR}
         --cpp_out=${GENERATED_DIR}
         --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
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
    gRPC::grpc++
    absl::log
)

