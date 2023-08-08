#ifndef MESSAGE_SERIALIZER_H
#define MESSAGE_SERIALIZER_H

#include <stdlib.h>
#include <memory>

#include "ps_string.h"
#include "ps_vector.h"

#include <ArduinoJson.h>
#include "json_allocator.h"

#include "../sdr_containers/SDRModule.h"
#include "../sdr_containers/SDRUnit.h"

#include "ps_smart_ptr.h"

class MessageSerializer
{
    private:
        DynamicPSRAMJsonDocument document;
        const std::shared_ptr<SDRUnit>  _unit;
        const ps_vector<std::shared_ptr<Module>> _modules;

        ps_string compressString(const ps_string& message);
    public:
        MessageSerializer(const std::shared_ptr<SDRUnit> unit, const ps_vector<std::shared_ptr<Module>> modules);
        ps_string serializeReadings();
        ps_string serializeUpdateRequest(const ps_vector<Module*> modules);
        ps_string serializeNotification(ps_string notification);

};

#endif