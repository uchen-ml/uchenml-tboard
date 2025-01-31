package(default_visibility = ["//visibility:public"])

proto_library(
    name = "tensor_proto",
    srcs = ["event.proto",
            "histogram.proto",
            "resource_handle.proto",
            "summary.proto",
            "tensor_shape.proto",
            "tensor.proto",
            "types.proto"],
)

cc_proto_library(
    name = "tensor_cc_proto",
    deps = [":tensor_proto"],
)