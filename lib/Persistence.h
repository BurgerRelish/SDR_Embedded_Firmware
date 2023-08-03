#pragma once

#ifndef PERSISTENCE_H
#define PERSISTENCE_H
#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
#include <string>
#include "../data_containers/ps_string.h"
#include "../Communication/json_allocator.h"

class Persistence {
    private:
        static int count;
        std::string path;
    public:
        JsonDoc document;
        
        Persistence(const char* path_to_file) : path(path_to_file) {
            document = JsonDoc(1024);
            if (count++ == 0) LITTLEFS.begin();
            auto file = LITTLEFS.open(path_to_file, "r", true);

            if (file.available()) {
                deserializeJson(document, file);
            }

            file.close();

            document.garbageCollect();
            document.shrinkToFit();
        }

        ~Persistence() {
            auto file = LITTLEFS.open(path.c_str(), "w", true);
            serializeJson(document, file);

            file.close();
            if (--count == 0) LITTLEFS.end();
        }
}

int Persistence::count = 0;

#endif