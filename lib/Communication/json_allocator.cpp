#include "json_allocator.h"

void convertFromJson(JsonVariantConst src, ps::string& dst) {
        dst = src.as<const char*>();
}