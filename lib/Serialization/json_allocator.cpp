#include "json_allocator.h"

namespace ps {
        
void convertFromJson(JsonVariantConst src, string& dst) {
        dst = src.as<const char*>();
}

}
