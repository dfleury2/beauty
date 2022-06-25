# Bazel Build Instructions

## Conan package installation

conan install . -pr default -pr:b default -b missing -r conancenter

## Bazel build

bazel build //server:server --cxxopt="--std=c++20"
