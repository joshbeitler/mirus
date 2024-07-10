# bake

`bake` ("Build Automation Kit for Everything") is a simple build tool inspired by Bazel. It is designed to be simple, fast, and easy to configure. `bake` is the build tool for the Mirus operating system project.

## Rationale

Like much of the Mirus project, `bake` was created as a learning experience. The goal of `bake` is to provide a simple, fast, and easy-to-use build tool for small to medium-sized projects, and provide a similar experience to tools like Bazel while theoretically being able to be hosted on Mirus itself.

Right now, `bake` is written in Rust with a variety of third-party dependencies, but the long term goal is to reduce the number of dependencies and make `bake` use the Mirus standard library.

## Features

- Simple, easy-to-read JSON-based configuration files
- Fast, parallel builds
- Dependency tracking

## Building

`bake`, like most of the Mirus userspace, is written in Rust.

Once you have Rust installed, you can build `bake` by running:

```sh
cargo build --release
```

The resulting binary will be located at `target/release/bake`, and can be copied to a location in your `PATH`.

## Usage

`bake` is designed to be simple to use. To start, create a `build.json` file in the root of your project. You can find the schema for this file in `docs/build-file-schema.json`.

Here is an example `build.json` file:

```json
{
  "targets": [
    {
      "name": "dep_a",
      "sources": ["src/dep_a/*.c"],
      "tools": {
        "cc": {
          "cmd": "clang",
          "args": ["{sources}", "-c"]
        }
      },
      "dependencies": []
    }
  ]
}
```

See the `docs` directory for more information on how to configure `bake`, and the `examples` directory for example projects.

## License

This project is licensed under the terms of the MIT license.
