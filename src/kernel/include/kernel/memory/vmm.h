#pragma once

#include <stddef.h>
#include <stdint.h>

void vmm_initialize();
void vmm_map_page(uintptr_t virt, uintptr_t phys, int flags);
void vmm_unmap_page(uintptr_t virt);
// vmm_alloc?
// vmm_free?
// move phys_to_virt here?
// move virt_to_phys here?
void vmm_set_page_flags(uintptr_t virt, int flags);
int vmm_get_page_flags(uintptr_t virt);
// handle_page_fault?
void vmm_create_address_space();
void vmm_switch_address_space();
void vmm_destroy_address_space();
void vmm_clone_address_space();
