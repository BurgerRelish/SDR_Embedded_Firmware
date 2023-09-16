#pragma once

#ifndef PERSISTENCE_H
#define PERSISTENCE_H
#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
#include <string>
#include <ArduinoJson.h>

#include <ps_stl.h>

#include "src/json_allocator.h"

/**
 * @brief This class opens a file on the filesystem and creates a DynamicJsonDocument on the PSRAM during construction. The document can then be modified during the life of the
 * class. Upon destruction, the contents of the Json document are serialized and written to the file on flash memory if write_on_destruction is set to true during construction.
 * 
 * @note Expects the provided file system to be mounted before class construction.
 * 
 */
template <class FileSystem>
class Persistence {
    private:
        ps::string path;
        FileSystem& _file_system;
        bool write;
    public:
        DynamicPSRAMJsonDocument document;
        
        Persistence(FileSystem& file_system, const char* path_to_file, const size_t json_size, bool write_on_destruction = false) : _file_system(file_system), path(path_to_file), document(json_size), write(write_on_destruction) {
            if (!file_system.exists(path.c_str())) {
                auto file_ = file_system.open(path.c_str(), FILE_WRITE, true);
                file_.close();
            }

            auto file = file_system.open(path.c_str(), FILE_READ, true);
            ps::string data = file.readString().c_str();

            ESP_LOGI("Persistence" , "File: %s", data.c_str());

            auto result = deserializeJson(document, data.c_str());
            if (result.code() != 0) ESP_LOGE("Persistence", "Json Deserialization Failed: %s", result.c_str());

            file.close();
        }

        /**
         * @brief Clears the Json Document and removes the file from the file system.
         * 
         */
        void clear() {
            document.clear();
            _file_system.remove(path.c_str());
        }

        ~Persistence() {
            if (write) {
                auto file = _file_system.open(path.c_str(), FILE_WRITE, true);

                if(serializeJson(document, file) == 0) ESP_LOGE("Persistence", "Serialization Failed.");
                file.close();
            }
        }
};

#endif