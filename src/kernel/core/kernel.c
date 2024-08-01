#define SSFN_CONSOLEBITMAP_TRUECOLOR

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <limine/limine.h>
#include <ssfn/ssfn.h>
#include <printf/printf.h>
#include <libk/string.h>

#include <hal/serial.h>
#include <hal/gdt.h>
#include <hal/idt.h>
#include <hal/hal_logger.h>

#include <kernel/bootloader.h>
#include <kernel/interrupts.h>
#include <kernel/memory.h>
#include <kernel/debug.h>
#include <kernel/panic.h>
#include <kernel/syscalls.h>
#include <kernel/stack.h>
#include <kernel/process.h>

#include <drivers/terminal.h>


// don't look
// #define USER_CODE_ADDRESS 0x400000
// #define USER_STACK_SIZE (16 * 1024)
// #define USER_STACK_ADDRESS 0x7FFFFFFFE000  // Near the top of user space
// static uint8_t user_stack_buffer[USER_STACK_SIZE] __attribute__((aligned(16)));

/**
 * Kernel entry point
 */
void _start(void) {
  serial_initialize();
  debug_logger_initialize();
  hal_logger_initialize();

  log_message(&kernel_debug_logger, LOG_INFO, "Mirus kernel intialization\n");
  log_message(&kernel_debug_logger, LOG_INFO, "Serial driver loaded\n");

  // Ensure the bootloader actually understands our base revision (see spec).
  if (LIMINE_BASE_REVISION_SUPPORTED == false) {
    log_message(&kernel_debug_logger, LOG_FATAL, "Bootloader version not compatible\n");
    hcf();
  }
  log_message(&kernel_debug_logger, LOG_INFO, "Bootloader version compatible\n");

  // Find kernel stack from bootloader
  uintptr_t kernel_stack_top = get_kernel_stack_ptr();
  log_message(&kernel_debug_logger, LOG_INFO, "Successfully found kernel stack at {top=%p}\n", (void*)kernel_stack_top);

  // Ensure we got a framebuffer and fetch the first available one.
  if (framebuffer_request.response == NULL
    || framebuffer_request.response->framebuffer_count < 1) {
    log_message(&kernel_debug_logger, LOG_FATAL, "Couldn't get framebuffer\n");
    hcf();
  }
  struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
  log_message(&kernel_debug_logger, LOG_INFO, "Successfully loaded framebuffer\n");

  // Ensure we got a memory map
  if (memory_map_request.response == NULL
    || memory_map_request.response->entry_count == 0) {
    log_message(&kernel_debug_logger, LOG_FATAL, "Couldn't get memory map\n");
    hcf();
  }
  log_message(&kernel_debug_logger, LOG_INFO, "Successfully loaded memory map\n");

  // load font
  struct limine_file *default_terminal_font = limine_get_file("u_vga16.sfn");
  if (default_terminal_font == NULL) {
    log_message(&kernel_debug_logger, LOG_FATAL, "Couldn't load default font: {file=u_vga16.sfn}\n");
    hcf();
  }
  log_message(&kernel_debug_logger, LOG_INFO, "Successfully loaded default font: {file=u_vga16.sfn}\n");

  terminal_initialize(default_terminal_font, framebuffer);
  log_message(&kernel_debug_logger, LOG_INFO, "Virtual terminal initialized\n");

  log_message(&kernel_debug_logger, LOG_INFO, "Starting GDT initialization\n");
  gdt_initialize(kernel_stack_top);
  log_message(&kernel_debug_logger, LOG_INFO, "Successfully initialized GDT\n");

  log_message(&kernel_debug_logger, LOG_INFO, "Starting IDT initialization\n");
  idt_initialize();
  log_message(&kernel_debug_logger, LOG_INFO, "Successfully initialized IDT\n");

  log_message(&kernel_debug_logger, LOG_INFO, "Starting ISR initialization\n");
  isr_initialize();
  log_message(&kernel_debug_logger, LOG_INFO, "Successfully initialized ISRs\n");

  // Look at memory map
  // do somthing with the entries. we will move this somewhere else
  log_message(&kernel_debug_logger, LOG_INFO, "Starting physical memory manager initialization\n");
  read_memory_map(memory_map_request.response->entry_count, memory_map_request.response->entries, kernel_address_request.response, kernel_file_request.response);
  init_page_allocator();
  log_message(&kernel_debug_logger, LOG_INFO, "Successfully initialized physical memory manager\n");





  log_message(&kernel_debug_logger, LOG_INFO, "Starting system call initialization\n");
  syscalls_initialize();
  log_message(&kernel_debug_logger, LOG_INFO, "Successfully initialized system calls\n");

  log_message(&kernel_debug_logger, LOG_INFO, "Kernel initialization complete\n");
  printf_("Mirus, ahoy!\n\n");

  // debug_test_syscalls();
  // try to load hello program into memory, get its address, and jump to it
  #define USER_STACK_ADDR 0x7FFFFFFFE000  // Example user stack address
  #define USER_PROGRAM_ADDR 0x400000      // Example user program load address
  #define PAGE_SIZE 4096
  #define USER_CS 0x23  // User mode code segment
  #define USER_SS 0x1B  // User mode stack segment
  #define USER_RFLAGS 0x202  // User mode RFLAGS (IF set)




  struct limine_file *hello_world = limine_get_file("hello");
  uintptr_t stack_phys = allocate_page();

  map_page(USER_STACK_ADDR, stack_phys, 0x7);
  // // void* user_stack_top = (char*)user_stack + USER_STACK_SIZE;
  // // void* entry_point = hello_world->address;

  // size_t program_size = hello_world->size;
  //     size_t pages_needed = (program_size + PAGE_SIZE - 1) / PAGE_SIZE;

  //     for (size_t i = 0; i < pages_needed; i++) {
  //         uintptr_t prog_phys = allocate_page();
  //         if (prog_phys == 0) {
  //             // Handle allocation failure
  //             return;
  //         }

  //         // Map the program page with user-mode permissions (read/execute, user-accessible)
  //         map_page(USER_PROGRAM_ADDR + i * PAGE_SIZE, prog_phys, 0x5);  // 0x5 = Present | User | Execute
  //     }

  //     // 4. Copy the program to user memory
  //     memcpy((void*)USER_PROGRAM_ADDR, hello_world->address, program_size);

  //     jump_to_usermode((void*)USER_PROGRAM_ADDR, (void*)USER_STACK_ADDR);


  // We're done, just hang...
  hcf();
}
