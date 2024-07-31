# Mirus Runtime - managed userspace architecture

## 1. Overview

This document outlines the architecture for a managed userspace runtime that
runs each userspace program in its own managed runtime instance. The system
uses a managed language as the primary programming language for userspace
applications and system services.

The core kernel is still implemented in native code but as much as we can fit
into the managed runtime will be. This will allow for a more secure and
maintainable system.

WASM is a good candidate for the managed runtime as it is designed to be run in
a sandboxed environment and is a good fit for running untrusted code. Despite
having `web` in the name, it is a general-purpose bytecode format that can be
used for a variety of implementations, and has good community support.

## 1. Runtime architecture

### 1.1 Runtime options

Still to be decided is the runtime implementation. There are a few options:

1. Implement custom runtime/interpretor for WASM
2. Use an existing Zig-native WASM runtime
3. Use an existing WASM runtime written in another language

### 1.2 Runtime components

- WASM
- WASI

## 1. Process lifecycle

1. User or system requests new process
2. Kernel allocates memory for new runtime instance
3. Kernel initializes new runtime with process-specific parameters
4. Runtime loads and verifies bytecode
5. Process executes within its runtime
6. On process termination, kernel cleans up runtime and releases resources

## 2. Porting existing applications

Existing applications can be ported to the managed runtime by compiling them to
WASM bytecode. This can be done with a toolchain that compiles the application
to WASM and links it with the necessary runtime components.

The system will provide a standard library that can be used by applications to
interact with the kernel and other system services via abstractions over system
calls.

## Appendix A: Questions

1. How will the runtime handle system calls?
2. What WASM components are needed?
3. Does the runtime exist in the kernel or as a userspace program?
  - Probably not the most _secure_ option long term, but implementing it
    directly in the kernel will be easier and more performant. Security is a
    theoretical concern at this point anyway, since this is a hobby project.
4. What binary/container format will be used for the WASM bytecode?
5. How can we do AOT compilation of WASM bytecode for performance (e.g. driver)
6. What is our preferred managed language for userspace applications?
  - Obviously, it doesn't really matter as long as the target is WASM, but
    having a preferred language will help with standardization and tooling.
  - Something higher level than C, with nice semantics.
  - [See this list](https://github.com/appcypher/awesome-wasm-langs)
  - Go or Rust. Something with a good standard library and tooling, a strong
    community, and nice memory management.
