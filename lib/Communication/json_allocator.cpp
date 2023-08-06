#include "json_allocator.h"

void convertFromJson(JsonVariantConst src, ps_string& dst) {
        dst = src.as<const char*>();
}