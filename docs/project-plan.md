# Mirus OS project plan

## NEXT

We are ready to jump to user mode, but we now need to implement memory
management and a few other things before we can do that. Once that is done, we
can start on implementing the managed runtime, which is where we will jump to.

- [ ] memory management
  - [ ] physical memory manager
  - [ ] kmalloc / kfree
  - [ ] virtual memory manager/paging
- [ ] mirus runtime
  - [ ] wasm VM
  - [-] syscalls
  - [ ] userspace runtime library
  - [x] TSS
  - [ ] processes
- [ ] example userspace program compile as wasm and bundle with kernel for testing

## TODO

- [x] defines for debug and use them
  - [ ] make dynamic
- [ ] docs
  - [ ] design goals
  - [ ] building
  - [ ] debugging
  - [ ] managed userspace white paper
  - [ ] system call interface
- [ ] code formatter?
- [ ] .editorconfig?
- [ ] switch to zig
  - [ ] kernel
  - [ ] libs
    - [ ] printf (do we need this with zig?)
    - [ ] libk
    - [ ] ssfn
    - [ ] limine
  - [ ] hal
  - [ ] build system
- [ ] servers
  - [ ] display compositor/window manager
    - [ ] framebuffer in userspace
  - [ ] filesystem
  - [ ] disk drivers
  - [ ] network drivers
  - [ ] simple USB implementation
  - [ ] keyboard driver
  - [ ] mouse driver
- [ ] misc
  - [ ] AIPC
  - [ ] irq
  - [ ] object manager?
  - [ ] clock and timers
- [ ] pre-emptive multitasking
- [ ] port libc
- [ ] threads
- [ ] scheduler
