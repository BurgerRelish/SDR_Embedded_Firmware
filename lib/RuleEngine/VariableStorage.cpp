#include "VariableStorage.h"

namespace re {

/**
 * @brief Gets the underlying variable type of the identifier.
 * 
 * @param identifier 
 * @return VariableType 
 */
VariableType VariableStorage::type(ps::string identifier) {
    auto result = storage.find(identifier);
    if (result == storage.end()) return VAR_UNKNOWN;
    return result -> second.first;
}

}