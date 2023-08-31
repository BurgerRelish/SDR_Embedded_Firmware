#pragma once

#ifndef TAG_SEARCH_H
#define TAG_SEARCH_H

#include <stdint.h>
#include <string>
#include <iostream>
#include <sstream>
#include <ArduinoJson.h>

#include <ps_stl.h>


class TagSearch {
    private:
    ps::vector<ps::string> class_tags;
    ps::string _nvs_path;

    public:
    TagSearch(const std::vector<std::string>& tag_list) {
        //clearTags();

        ps::string temp;
        for (size_t i = 0; i < tag_list.size(); i++) { // Copy tag list to PSRAM.
            temp <<= tag_list.at(i);
            class_tags.push_back(temp);
            temp.clear();
        }
    }

    void saveTags(JsonArray& tag_array) {
        tag_array.clear();
        for (auto tag : class_tags) {
            auto tag_obj = tag_array.add(tag.c_str());
        }
    }

    ps::vector<ps::string> getTags() {
        return class_tags;
    }

    void clearTags() {
        class_tags.clear();
    }

    void appendTag(const ps::string& tag) {
        class_tags.push_back(tag);
    }

    void appendTag(const ps::vector<ps::string>& tags) {
        for (size_t i = 0; i < tags.size(); i++) {
            class_tags.push_back(tags.at(i));
        }
    }

    void replaceTag(const ps::string& tag) {
        clearTags();
        class_tags.push_back(tag);
    }

    void replaceTag(const ps::vector<ps::string>& tags) {
        clearTags();
        class_tags = tags;
    }

    void replaceTag(ps::string& tag, size_t loc) {
        class_tags.at(loc) = tag;
    }

    /**
     * @brief Check whether a minimum number of tags match.
     * @param tag_list The list of tags to compare.
     * @param n The minimum number of matching tags before a true is returned.
     * @returns True if matching tags >= n, else false.
    */

    const bool tagMinQuantifierSearch(const ps::vector<ps::string>& tag_list, const size_t n) const {
        if ((class_tags.size() < n) || (tag_list.size() < n)) return false; // Not enough tags for a true outcome.

        size_t matches = 0;
        for (size_t src_it = 0; src_it < tag_list.size(); src_it++) {
            ps::string cur_str = tag_list.at(src_it);

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
    const bool tagSubsetComparison(const ps::string& tag) const {
        for (size_t i = 0; i < class_tags.size(); i++) {
            if (tag == class_tags.at(i)) return true;
        }

        return false;
    }

    /**
     * @brief Checks whether any of the tags provided are in the module tag list.
     * @return True if any tag is common between tag_list and the module tag list, else false.
    */
    const bool tagSubsetComparison(const ps::vector<ps::string>& tag_list) const {
        return tagMinQuantifierSearch(tag_list, 1);
    }

    /**
     * @brief Checks whether the provided tag_list matches exactly to the modules tag list. Element order does not matter.
     * @return True if the two vectors match, else false.
    */
    const bool tagEqualityComparison(const ps::vector<ps::string>& tag_list) const {
        return tagMinQuantifierSearch(tag_list, class_tags.size());
    }

};

#endif