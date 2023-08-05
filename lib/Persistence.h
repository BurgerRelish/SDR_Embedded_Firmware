#pragma once

#ifndef PERSISTENCE_H
#define PERSISTENCE_H
#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
#include <string>
#include <ArduinoJson.h>

#include "../data_containers/ps_string.h"
#include "../Communication/json_allocator.h"

class Persistence {
    private:
        std::string path;
        fs::LittleFSFS& _file_system;
        static uint8_t count;
    public:
        DynamicPSRAMJsonDocument document;
        
        Persistence(fs::LittleFSFS& file_system, const char* path_to_file) : _file_system(file_system), path(path_to_file), document(DynamicPSRAMJsonDocument(1024)) {
            if (count++ == 0) file_system.begin();
            auto file = file_system.open(path.c_str(), "r", true);

            if (file.available()) {
                deserializeJson(document, file);
            }

            file.close();

            document.garbageCollect();
            document.shrinkToFit();
        }

        ~Persistence() {
            auto file = _file_system.open(path.c_str(), "w", true);
            serializeJson(document, file);

            file.close();
            if (--count == 0) _file_system.end();
        }
};

uint8_t Persistence::count = 0;

#endif