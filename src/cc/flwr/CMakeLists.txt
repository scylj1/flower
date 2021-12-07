# Protobuf file defining flower messages
get_filename_component(trans_proto "../../proto/flwr/proto/transport.proto" ABSOLUTE)
get_filename_component(trans_proto_path "${trans_proto}" PATH)

## gRPC 
# Generated C++ sources and headers from protobuf
set(trans_proto_srcs "transport.pb.cc")
set(trans_proto_hdrs "transport.pb.h")
set(trans_grpc_srcs "transport.grpc.pb.cc")
set(trans_grpc_hdrs "transport.grpc.pb.h")
set(flwr_hdrs "/include/start.h")
set(flwr_srcs "/src/start.cc")

add_custom_command(
  OUTPUT "${trans_proto_srcs}" "${trans_proto_hdrs}" "${trans_grpc_srcs}" "${trans_grpc_hdrs}"
  COMMAND ${_PROTOBUF_PROTOC}
  ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
       --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${trans_proto_path}"
       --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
          "${trans_proto}"
  DEPENDS "${trans_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
include_directories("include")


# Build trans_grpc_proto
add_library(trans_grpc_proto
  STATIC
  ${trans_grpc_srcs}
  ${trans_grpc_hdrs}
  ${trans_proto_srcs}
  ${trans_proto_hdrs})

add_library(flwr 
	STATIC
	${flwr_hdrs}
	${flwr_srcs})

target_include_directories(trans_grpc_proto PUBLIC include)

target_link_libraries(trans_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})
target_link_libraries(flwr
	trans_grpc_proto)
