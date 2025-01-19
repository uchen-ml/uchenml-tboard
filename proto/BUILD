load("@protobuf//bazel:proto_library.bzl", "proto_library")
load("@protobuf//bazel:cc_proto_library.bzl", "cc_proto_library")

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