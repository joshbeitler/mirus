#include <stdint.h>
#include <stddef.h>

#include <kernel/memory/vmm.h>
#include <kernel/memory/pmm.h>
#include <kernel/debug.h>

void vmm_initialize() {
    // TOOD: what is there to initialize here
    // paging is set up already
    // hhdm is set up already
}

void vmm_map_page(uintptr_t virt, uintptr_t phys, int flags) {

}

void vmm_unmap_page(uintptr_t virt) {

}

void vmm_set_page_flags(uintptr_t virt, int flags) {

}

int vmm_get_page_flags(uintptr_t virt) {
}

void vmm_create_address_space() {

}

void vmm_switch_address_space() {

}

void vmm_destroy_address_space() {

}

void vmm_clone_address_space() {

}
