#pragma once

#include <stdint.h>
#include <stddef.h>

/**
 * Default page size
 */
#define PAGE_SIZE 4096

/**
 * Page read/write flags
 */
#define PAGE_PRESENT (1 << 0)
#define PAGE_WRITABLE (1 << 1)

/**
 * Number of pages represented by each bitmap word
 */
#define PAGES_PER_WORD 64
