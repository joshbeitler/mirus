#include <stdint.h>
#include <stddef.h>

#include <kernel/memory/vmm.h>
#include <kernel/memory/pmm.h>
#include <kernel/debug.h>

// Note for tomorrow self: as we get into this, a proper unit testing framework
// (unity maybe?) would be nice to ensure things are working as expected, and
// are repeatable

// How necessary is the VMM if we intend to operate a kernel-level VM?
