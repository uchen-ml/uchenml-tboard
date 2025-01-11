# TensorBoard Support for UchenML

## Libraries

Language level: C++20
Language features: everything, except exceptions.
Code style guide: Google C++ Style Guide

Please do not add dependencies without discussing them first.

- [ABSL](https://abseil.io/) - General C++ utilities. Use for logging, flags. Custom types are encouraged to support ABSL stringification.
- [Google Test](https://google.github.io/googletest/primer.html) - Testing library

## General principles

- Small build targets to allow clients only include functionality they need.
- Executable targets are simple bootstrap/flag parsing. All logic should be in libraries.

## Setting up the environment

1. Clone the repository
2. Install Bazelisk: https://github.com/bazelbuild/bazelisk

## Setting up Visual Studio Code

Preferred tool chain is Clang/LLVM. Clangd extension is recommended. I would strongly recommend installing the absolute latest version of Clangd from the LLVM website. The version in the Ubuntu repositories is very old and does not support many features.

1. Make sure Microsoft C++ tools are not installed.
1. Install the extensions in `.vscode/extensions.json` (may need to re)
1. Build `compile_commands.json`: `bazel run @hedron_compile_commands//:refresh_all`
1. Restart the Clangd daemon (Ctrl+Shift+P, Clangd: Restart) - only needed when DB is rebuilt.

## Recipes

### Bazel

- `bazel build //...` - build everything
- `bazel test //...` - run all tests. Use `--test_output=all` to see output from successful runs.
- `bazel run //path/to:target` - run a binary target
- `bazel run //path/to:target -- --flag=value output_file` - pass flags to the binary
- `bazel test //path/to:target --test_output=all` - run a single test with all output
- `--config=clang` after the verb (build/test/run) builds with Clang on Linux. `--config=asan` and `--config=ubsan` enable AddressSanitizer and UBSan, respectively. E.g. `bazel test --config=asan --config=clang //...` will run all tests with AddressSanitizer and Clang.
- `--cxxopt` passes flags to the compiler. E.g. `--cxxopt=-march=native` will build for your CPU, helpful with SIMD. `--linkopt` passes flags to the linker.

### Logging

Rely on ABSL log. `LOG(INFO) << "Hello, world!";`

End-user binaries need to link with `@abseil-cpp//absl/log:flags` so logging can be configured with command line flags. E.g. flag `--stderrthreshold=0` will log everything to stderr. Every binary should call `absl::InitializeLog()`.

Test main fine should usually call `absl::InitializeLog()` and `absl::SetStderrThreshold(INFO)` so everything is logged when tests are ran.

In-depth guide: https://abseil.io/docs/cpp/guides/logging

### Command-line flags

Use ABSL flags. See code for examples, refer to [ABSL flags documentation](https://abseil.io/docs/cpp/guides/flags).

## Some extra build features

### Compile with Clang on Linux

`bazel build --config=clang //...` will enable clang compiler and libc++ on Linux.

### Sanitizers

`bazel build --config=asan //...` will enable AddressSanitizer.
`bazel build --config=ubsan //...` will enable UBSan.

Feel free to add profiles for other sanitizers if necessary.
