# libs

These libraries are used variously by the kernel and the userland. Some of the libraries are third party dependencies, while others are developed as part of the Mirus project.

The third party libraries are not bundled in the repository, but are fetched and built as part of the build process.

## Third party dependencies

- limine
  - Limine is a bootloader that is used to boot the kernel. We use the `limine.h` file to define the Limine protocol.
- ssfn
  - Simple Small Font Library is used to load and render bitmap fonts to our framebuffer.
- printf (from [freestanding printf implementation](https://github.com/eyalroz/printf))
  - A freestanding implementation of the `printf` family of functions.

## Mirus libraries

- liblogger
  - A simple logging library that provides a `printf`-like interface to log messages along with their severity level.
- libk
  - A collection of utility functions that are used by the kernel.
- jems
  - A form of the really excellent [`jems`](https://github.com/rdpoor/jems) library by rdpoor. Modified to work with the Mirus kernel environment.
