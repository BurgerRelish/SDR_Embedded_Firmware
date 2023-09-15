#pragma once

#ifndef PS_UNORDERED_MAP_H
#define PS_UNORDERED_MAP_H

#include <cstddef>
#include <cstdlib>
#include <unordered_map>
#include <string>
#include <utility>

#include "ps_allocator.h"

#include <bits/functional_hash.h>

namespace ps {

template <typename T>
struct hash {
    size_t operator()(const T& s) const {
        return std::hash<T>(s);
    }
};

template <>
struct hash<string> {
    size_t operator()(const string& s) const {
        std::hash<std::string>();
        return std::_Hash_impl::hash(s.data(), s.length());
    }
};

template <typename Key, typename T, typename Hash = hash<Key>, typename KeyEqual = std::equal_to<Key>>
using unordered_map = std::unordered_map<Key, T, Hash, KeyEqual, allocator<std::pair<Key, T>>>;

}  // namespace ps

#endif