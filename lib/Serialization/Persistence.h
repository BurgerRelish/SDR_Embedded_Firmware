#pragma once

#ifndef PERSISTENCE_H
#define PERSISTENCE_H
#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
#include <string>
#include <ArduinoJson.h>

#include <ps_stl.h>

#include "json_allocator.h"

/**
 * @brief This class opens a file on the filesystem and creates a DynamicJsonDocument on the PSRAM during construction. The document can then be modified during the life of the
 * class. Upon destruction, the contents of the Json document are serialized and written to the file on flash memory if write_on_destruction is set to true during construction.
 * 
 * @note Expects the provided file system to be mounted before class construction.
 * 
 */

class Persistence {
    private:
        ps::string path;
        bool write;

    public:
        DynamicPSRAMJsonDocument document;
        
        Persistence(const char* path_to_file, const size_t json_size, bool write_on_destruction = false) : path(path_to_file), document(json_size), write(write_on_destruction) {
            auto file = LittleFS.open(path.c_str(), FILE_READ, true);

            if(!file || file.isDirectory()){
                ESP_LOGE("Persistence","Failed to open file.");
                return;
            }

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
            LittleFS.remove(path.c_str());
        }
        
        /**
         * @brief Set the class to write to the filesystem on destruction.
         * 
         */
        void enable_write() {
            write = true;
        }
        
        /**
         * @brief Set the class to do nothing on destruction.
         * 
         */
        void disable_write() {
            write = false;
        }

        ~Persistence() {
            if (write) {
                ps::ostringstream output;
                if (serializeJson(document, output) == 0) {
                    ESP_LOGE("Persistence", "Serialization Failed.");
                }

                auto file = LittleFS.open(path.c_str(), FILE_READ, true);

                char buf;
                bool write_required = false;
                size_t filePos = 0; // Initialize file position

                while (filePos < output.str().size() && file.readBytes(&buf, 1) > 0) {
                    if (buf != output.str().c_str()[filePos]) {
                        ESP_LOGI("Persistence", "File and output are different.");
                        write_required = true; // Data needs to be written
                        break;
                    }
                    filePos++; // Increment the file position
                }

                file.close();

                if (!write_required) return; // If the file and output are the same, do not write to the file.
            
                file = LittleFS.open(path.c_str(), FILE_WRITE, true);
                file.write((const uint8_t*)output.str().c_str(), output.str().size());
                file.flush();
                file.close();
            }
        }
};

#endif