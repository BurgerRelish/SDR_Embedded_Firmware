#pragma once

#ifndef JSON_ALLOCATOR_H
#define JSON_ALLOCATOR_H

#include "esp_heap_caps.h"
#include <ArduinoJson.h>
#include "../data_containers/ps_string.h"

struct JsonPSRAMAllocator {
    void* allocate(size_t n) {
#ifdef BOARD_HAS_PSRAM
        return heap_caps_malloc(n, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
#else
        return malloc(n);
#endif
    }

    void deallocate(void* p) {
#ifdef BOARD_HAS_PSRAM
        heap_caps_free(p);
#else
        free(p);
#endif
    }

    void* reallocate(void* p, size_t n) {
#ifdef BOARD_HAS_PSRAM
        return heap_caps_realloc(p, n, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
#else
        return realloc(p, n);
#endif
    }
};

typedef BasicJsonDocument<JsonPSRAMAllocator> DynamicPSRAMJsonDocument;

void convertFromJson(JsonVariantConst src, ps_string& dst);


#endif