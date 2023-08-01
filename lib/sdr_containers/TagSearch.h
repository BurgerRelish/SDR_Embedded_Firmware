#pragma once

#ifndef TAG_SEARCH_H
#define TAG_SEARCH_H

#include <Preferences.h>
#include <stdint.h>
#include <string>
#include <iostream>
#include <sstream>

#include "../data_containers/ps_string.h"
#include "../data_containers/ps_vector.h"

class TagSearch {
    private:
    ps_vector<ps_string> class_tags;
    ps_string _nvs_path;

    void loadTags() {
        Preferences nvs;
        nvs.begin(_nvs_path.c_str());
        std::ostringstream path;

        size_t tag_count = nvs.getUInt("/count");

        for (size_t i = 0; i < tag_count; i++) {
            path << "/tag/" << i;
            ps_string tag = nvs.getString(path.str().c_str()).c_str();
            class_tags.push_back(tag);
            path.clear();
        }

        nvs.end();
    }

    void saveTags() {
        Preferences nvs;
        nvs.begin(_nvs_path.c_str());
        std::ostringstream path;

        nvs.putUInt("/count", class_tags.size());

        for (size_t i = 0; i < class_tags.size(); i++) {
            path << "/tag/" << i;
            nvs.putString(path.str().c_str(), class_tags.at(i).c_str());
            path.clear();
        }

        nvs.end();
    }

    public:
    TagSearch(const std::vector<std::string>& tag_list, const ps_string& nvs_path) {
        clearTags();

        ps_string temp;
        for (size_t i = 0; i < tag_list.size(); i++) { // Copy tag list to PSRAM.
            temp <<= tag_list.at(i);
            class_tags.push_back(temp);
            temp.clear();
        }

        saveTags();
    }

    TagSearch(const ps_vector<ps_string>& tag_list, const ps_string& nvs_path) {
        clearTags();
        class_tags = tag_list;
        saveTags();
    }

    TagSearch(const ps_string& nvs_path) {
        loadTags();
    }

    const ps_vector<ps_string>& getTags() {
        return class_tags;
    }

    void clearTags() {
        Preferences nvs;
        nvs.begin(_nvs_path.c_str());

        class_tags.clear();
        nvs.clear();

        nvs.end();
    }

    void appendTag(const ps_string& tag) {
        class_tags.push_back(tag);
        saveTags();
    }

    void appendTag(const ps_vector<ps_string>& tags) {
        for (size_t i = 0; i < tags.size(); i++) {
            class_tags.push_back(tags.at(i));
        }
        saveTags();
    }

    void replaceTag(const ps_string& tag) {
        clearTags();
        class_tags.push_back(tag);
        saveTags();
    }

    void replaceTag(const ps_vector<ps_string>& tags) {
        clearTags();
        class_tags = tags;
        saveTags();
    }

    void replaceTag(ps_string& tag, size_t loc) {
        class_tags.at(loc) = tag;
        saveTags();
    }

    /**
     * @brief Check whether a minimum number of tags match.
     * @param tag_list The list of tags to compare.
     * @param n The minimum number of matching tags before a true is returned.
     * @returns True if matching tags >= n, else false.
    */

    const bool tagMinQuantifierSearch(const ps_vector<ps_string>& tag_list, const size_t n) const {
        if ((class_tags.size() < n) || (tag_list.size() < n)) return false; // Not enough tags for a true outcome.

        size_t matches = 0;
        for (size_t src_it = 0; src_it < tag_list.size(); src_it++) {
            ps_string cur_str = tag_list.at(src_it);

            for (size_t class_it = 0; class_it < class_tags.size(); class_it++) {
                if (cur_str == class_tags.at(class_it)) {
                    matches++;
                    if (matches >= n) return true; // Stop searching after enough matches are found.
                    break;
                }
            }

        }

        return false;
    }

    /**
     * @brief Checks whether the provided tag is part of the modules tag list.
     * @return True if the tag is found in the module tag list, else false.
    */
    const bool tagSubsetComparison(const ps_string& tag) const {
        for (size_t i = 0; i < class_tags.size(); i++) {
            if (tag == class_tags.at(i)) return true;
        }

        return false;
    }

    /**
     * @brief Checks whether any of the tags provided are in the module tag list.
     * @return True if any tag is common between tag_list and the module tag list, else false.
    */
    const bool tagSubsetComparison(const ps_vector<ps_string>& tag_list) const {
        return tagMinQuantifierSearch(tag_list, 1);
    }

    /**
     * @brief Checks whether the provided tag_list matches exactly to the modules tag list. Element order does not matter.
     * @return True if the two vectors match, else false.
    */
    const bool tagEqualityComparison(const ps_vector<ps_string>& tag_list) const {
        return tagMinQuantifierSearch(tag_list, class_tags.size());
    }

};

#endif