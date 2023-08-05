#pragma once

#ifndef JSON_ALLOCATOR_H
#define JSON_ALLOCATOR_H

#include "esp_heap_caps.h"
#include <ArduinoJson.h>

struct JsonPSRAMAllocator {
    void* allocate(size_t n) {
#ifdef BOARD_HAS_PSRAM
        auto p = heap_caps_malloc(n, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

        if (p == NULL) { // Try to allocate on SRAM if PSRAM Allocation fails.
            p = malloc(n);
        }
#else
        auto p = malloc(n);
#endif
        return p;
    }

    void deallocate(void* p) {
    #ifdef BOARD_HAS_PSRAM
        heap_caps_free(p);
    #else
        free(p);
    #endif
    }

    void* reallocate(void* p, size_t n) {
        auto ret = heap_caps_realloc(p, n, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

        if (ret == NULL) {
            ret = realloc(p, n);
        }

        return ret;
    }
};

typedef BasicJsonDocument<JsonPSRAMAllocator> DynamicPSRAMJsonDocument;

#endif