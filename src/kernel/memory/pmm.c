#include <stddef.h>
#include <stdint.h>

#include <jems/jems.h>
#include <limine/limine.h>
#include <logger.h>
#include <printf/printf.h>

#include <hal/serial.h>

#include <kernel/debug.h>
#include <kernel/memory/buddy_allocator.h>
#include <kernel/memory/paging.h>
#include <kernel/memory/pmm.h>
#include <kernel/stack.h>

#define JEMS_MAX_LEVEL 10

/**
 * Intrusive linked list of memory zones.
 */
static MemoryZone *memory_zones;

/**
 * Human-readable names for memory map entry types
 */
static const char *const memmap_type_strings[] = {
	"Usable",
	"Reserved",
	"ACPI Reclaimable",
	"ACPI NVS",
	"Bad Memory",
	"Bootloader Reclaimable",
	"Kernel and Modules",
	"Framebuffer"
};

#define MEMMAP_TYPE_COUNT                                                      \
	(sizeof(memmap_type_strings) / sizeof(memmap_type_strings[0]))

static uint64_t total_memory, usable_memory, kernel_start, kernel_size;

/**
 * Helper function to get a human-readable name for a memory map entry type
 *
 * @param type Memory map entry type
 *
 * @return Human-readable name for the type
 */
static const char *get_memmap_type_string(uint64_t type) {
	if (type < MEMMAP_TYPE_COUNT) {
		return memmap_type_strings[type];
	}

	return "Unknown";
}

/**
 * Utility function that formats a memory size as a human-readable string
 *
 * @param size Size in bytes
 * @param buffer Buffer to write the formatted string to
 * @param buffer_size Size of the buffer
 *
 * @return Pointer to the buffer
 */
static char *format_memory_size(
	uint64_t size, char *buffer, size_t buffer_size
) {
	uint64_t kib = size / 1024;
	uint64_t mib = kib / 1024;
	uint64_t gib = mib / 1024;

	if (gib > 0) {
		uint64_t mib_remainder = mib % 1024;
		snprintf_(
			buffer,
			buffer_size,
			"%llu bytes (%llu GiB + %llu MiB)",
			size,
			gib,
			mib_remainder
		);
	} else if (mib > 0) {
		uint64_t kib_remainder = kib % 1024;
		snprintf_(
			buffer,
			buffer_size,
			"%llu bytes (%llu MiB + %llu KiB)",
			size,
			mib,
			kib_remainder
		);
	} else {
		snprintf_(buffer, buffer_size, "%llu bytes (%llu KiB)", size, kib);
	}

	return buffer;
}

/** testing the json log emitter */
static void jems_writer(char ch, uintptr_t arg) {
	logger_t *logger = (logger_t *)arg;
	char str[2] = {ch, '\0'};
	log_stream_data(logger, str, 1);
}

static void log_memory_map_debug(
	uint64_t entry_count,
	struct limine_memmap_entry **entries,
	uint64_t total_memory,
	uint64_t usable_memory,
	uintptr_t kernel_start,
	uint64_t kernel_size
) {
	static jems_level_t jems_levels[JEMS_MAX_LEVEL];
	static jems_t jems;
	char buffer[64];

	// Start the log stream
	log_stream_start(
		&kernel_debug_logger, LOG_DEBUG, "memory_manager", "Memory Map"
	);

	jems_init(
		&jems,
		jems_levels,
		JEMS_MAX_LEVEL,
		jems_writer,
		(uintptr_t)&kernel_debug_logger
	);
	jems_object_open(&jems);

	format_memory_size(total_memory, buffer, sizeof(buffer));
	jems_key_string(&jems, "total_memory", buffer);

	format_memory_size(usable_memory, buffer, sizeof(buffer));
	jems_key_string(&jems, "usable_memory", buffer);

	snprintf_(buffer, sizeof(buffer), "0x%016llx", kernel_start);
	jems_key_string(&jems, "kernel_start", buffer);

	jems_key_array_open(&jems, "entries");
	for (size_t i = 0; i < entry_count; i++) {
		struct limine_memmap_entry *entry = entries[i];
		jems_object_open(&jems);
		snprintf_(buffer, sizeof(buffer), "0x%016llx", entry->base);
		jems_key_string(&jems, "base", buffer);
		jems_key_integer(&jems, "length", entry->length);
		jems_key_string(&jems, "type", get_memmap_type_string(entry->type));
		jems_object_close(&jems);
	}
	jems_array_close(&jems);
	jems_object_close(&jems);

	// End the log stream
	log_stream_end(&kernel_debug_logger);
}
/** end testing the json log emitter */

void pmm_initialize(
	uint64_t entry_count,
	struct limine_memmap_entry **entries,
	struct limine_kernel_address_response *kernel_address_response,
	struct limine_hhdm_response *hhdm_response
) {
	total_memory = 0;
	usable_memory = 0;
	char size_buffer[64];

	if (DEBUG) {
		log_message(
			&kernel_debug_logger,
			LOG_INFO,
			"memory_manager",
			"Reading memory map\n",
			format_memory_size(total_memory, size_buffer, sizeof(size_buffer))
		);
	}

	// Process each entry
	for (size_t i = 0; i < entry_count; i++) {
		struct limine_memmap_entry *entry = entries[i];

		total_memory += entry->length;

		// Sum up usable memory
		if (entry->type == LIMINE_MEMMAP_USABLE) {
			usable_memory += entry->length;
		}

		// Get kernel zone start and size, we will put the stack and heap after
		if (entry->type == LIMINE_MEMMAP_KERNEL_AND_MODULES &&
			kernel_start == 0) {
			kernel_start = entry->base;
			kernel_size = entry->length;
		}
	}

	if (DEBUG) {
		log_memory_map_debug(
			entry_count,
			entries,
			total_memory,
			usable_memory,
			kernel_start,
			kernel_size
		);
	}

	// Get first usable memory region address
	// uintptr_t usable_memory_start = 0;
	// size_t usable_memory_pool_size = 0;

	memory_zones = NULL;

	for (size_t i = 0; i < entry_count; i++) {
		struct limine_memmap_entry *entry = entries[i];

		if (entry->type == LIMINE_MEMMAP_USABLE) {
			uintptr_t zone_virtual_address =
				(uintptr_t)phys_to_virt(entry->base, hhdm_response->offset);

			// We found a new usable zone, initialize it
			MemoryZone *new_zone = (MemoryZone *)zone_virtual_address;
			new_zone->start_address = zone_virtual_address;
			new_zone->size = entry->length;
			new_zone->allocators = NULL;
			new_zone->next = memory_zones;
			memory_zones = new_zone;

			// Create buddy allocators for this zone
			uintptr_t current_address =
				zone_virtual_address + sizeof(MemoryZone);
			size_t remaining_size = entry->length - sizeof(MemoryZone);

			while (remaining_size >= sizeof(BuddyAllocatorNode) + PAGE_SIZE) {
				size_t allocator_size =
					(remaining_size > (PAGE_SIZE << MAX_ORDER))
						? (PAGE_SIZE << MAX_ORDER)
						: remaining_size;

				BuddyAllocatorNode *new_allocator =
					(BuddyAllocatorNode *)current_address;

				buddy_allocator_initialize(
					&new_allocator->allocator,
					current_address + sizeof(BuddyAllocatorNode),
					allocator_size - sizeof(BuddyAllocatorNode)
				);

				new_allocator->next = new_zone->allocators;
				new_zone->allocators = new_allocator;

				current_address += allocator_size;
				remaining_size -= allocator_size;
			}

			// usable_memory_start = entry->base;
			// usable_memory_pool_size = entry->length;
			if (DEBUG) {
				log_message(
					&kernel_debug_logger,
					LOG_INFO,
					"memory_manager",
					"Created zone: phys=0x%016llx, virt=0x%016llx, size=%zu\n",
					entry->base,
					zone_virtual_address,
					entry->length
				);
			}
		}
	}

	// uintptr_t buddy_allocator_base_virt =
	// 	(uintptr_t)phys_to_virt(usable_memory_start, hhdm_response->offset);

	// log_message(
	// 	&kernel_debug_logger,
	// 	LOG_INFO,
	// 	"memory_manager",
	// 	"buddy allocator phys: 0x%016llx, virt: 0x%016llx, offset: %llu\n",
	// 	usable_memory_start,
	// 	buddy_allocator_base_virt,
	// 	hhdm_response->offset
	// );

	// Init allocator
	// buddy_allocator_init(
	// 	&buddy_allocator, buddy_allocator_base_virt, usable_memory_pool_size
	// );

	if (DEBUG) {
		log_message(
			&kernel_debug_logger,
			LOG_INFO,
			"memory_manager",
			"Total system memory: %s\n",
			format_memory_size(total_memory, size_buffer, sizeof(size_buffer))
		);

		log_message(
			&kernel_debug_logger,
			LOG_INFO,
			"memory_manager",
			"Usable system memory: %s\n",
			format_memory_size(usable_memory, size_buffer, sizeof(size_buffer))
		);

		log_message(
			&kernel_debug_logger,
			LOG_INFO,
			"memory_manager",
			"Kernel address: %p\n",
			kernel_start
		);

		log_message(
			&kernel_debug_logger,
			LOG_INFO,
			"memory_manager",
			"Kernel size: %s\n",
			format_memory_size(kernel_size, size_buffer, sizeof(size_buffer))
		);
	}
}

uintptr_t pmm_alloc(size_t size) {
	for (MemoryZone *zone = memory_zones; zone != NULL; zone = zone->next) {
		for (BuddyAllocatorNode *allocator_node = zone->allocators;
			 allocator_node != NULL;
			 allocator_node = allocator_node->next) {
			uintptr_t result =
				buddy_allocator_allocate(&allocator_node->allocator, size);
			if (result != 0) {
				return result;
			}
		}
	}
	return 0; // No suitable block found
}

void pmm_free(uintptr_t address) {
	for (MemoryZone *zone = memory_zones; zone != NULL; zone = zone->next) {
		if (address >= zone->start_address &&
			address < zone->start_address + zone->size) {
			for (BuddyAllocatorNode *allocator_node = zone->allocators;
				 allocator_node != NULL;
				 allocator_node = allocator_node->next) {
				if (address >= (uintptr_t)allocator_node &&
					address < (uintptr_t)allocator_node +
								  allocator_node->allocator.pool_size) {
					buddy_allocator_free(&allocator_node->allocator, address);
					return;
				}
			}
		}
	}
}

// TODO: we need a more readable way to print the state of the memory manager or
// at least a better way to visualize in our logging tool
void pmm_debug_print_state() {
	static jems_level_t jems_levels[JEMS_MAX_LEVEL];
	static jems_t jems;
	char buffer[32]; // Buffer for string formatting

	// Start the log stream
	log_stream_start(
		&kernel_debug_logger,
		LOG_DEBUG,
		"memory_manager",
		"Physical Memory Manager State"
	);

	jems_init(
		&jems,
		jems_levels,
		JEMS_MAX_LEVEL,
		jems_writer,
		(uintptr_t)&kernel_debug_logger
	);

	jems_object_open(&jems);
	jems_key_array_open(&jems, "memory_zones");

	for (MemoryZone *zone = memory_zones; zone != NULL; zone = zone->next) {
		jems_object_open(&jems);
		snprintf_(buffer, sizeof(buffer), "0x%016llx", zone->start_address);
		jems_key_string(&jems, "start_address", buffer);
		jems_key_integer(&jems, "size", zone->size);

		jems_key_array_open(&jems, "allocators");
		for (BuddyAllocatorNode *allocator_node = zone->allocators;
			 allocator_node != NULL;
			 allocator_node = allocator_node->next) {
			jems_object_open(&jems);
			snprintf_(
				buffer,
				sizeof(buffer),
				"0x%016llx",
				allocator_node->allocator.start_address
			);
			jems_key_string(&jems, "start_address", buffer);
			jems_key_integer(
				&jems, "pool_size", allocator_node->allocator.pool_size
			);

			jems_key_array_open(&jems, "free_lists");
			for (int order = 0; order <= MAX_ORDER; order++) {
				jems_object_open(&jems);
				jems_key_integer(&jems, "order", order);
				jems_key_integer(&jems, "block_size", PAGE_SIZE * (1 << order));

				size_t free_blocks = 0;
				size_t total_free_memory = 0;
				jems_key_array_open(&jems, "free_blocks");
				for (BuddyBlock *block =
						 allocator_node->allocator.free_lists[order];
					 block != NULL;
					 block = block->next) {
					snprintf_(
						buffer, sizeof(buffer), "0x%016llx", (uintptr_t)block
					);
					jems_string(&jems, buffer);
					free_blocks++;
					total_free_memory += PAGE_SIZE * (1 << order);
				}
				jems_array_close(&jems);

				jems_key_integer(&jems, "free_block_count", free_blocks);
				jems_key_integer(&jems, "total_free_memory", total_free_memory);
				jems_object_close(&jems);
			}
			jems_array_close(&jems);

			jems_object_close(&jems);
		}
		jems_array_close(&jems);

		jems_object_close(&jems);
	}

	jems_array_close(&jems);
	jems_object_close(&jems);

	// End the log stream
	log_stream_end(&kernel_debug_logger);
}
