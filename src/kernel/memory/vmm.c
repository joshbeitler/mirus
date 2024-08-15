#include <stdint.h>
#include <stddef.h>

#include <kernel/memory/vmm.h>
#include <kernel/memory/pmm.h>
#include <kernel/debug.h>

// Note for tomorrow self: we need to be able to access all the memory in our
// buddy allocator first
//
// strategy on that: simply "plug the gaps" with a series of buddy allocators
// to do this:
// 1. use a list of buddy allocators
// 2. assign each buddy allocator a range of memory (based on memory map)
// 2a. if the region is larger than the buddy allocator of a given max order, then that region
//     should have multiple buddy allocators assigned to it, in sequence
// 3. allocate function needs to take this into account
// 4. free function needs to take this into account
// 5. debugging and logging needs to be able to show the entire state of the system

// Note for tomorrow self: as we get into this, a proper unit testing framework
// (unity maybe?) would be nice to ensure things are working as expected, and
// are repeatable
