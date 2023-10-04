#pragma once

#ifndef READING_H
#define READING_H

#include <ArduinoJson.h>
#include <../ModuleInterface/ModuleInterface.h>
#include <math.h>

class Reading {
    public:
        double voltage = 0;
        double frequency = 0;
        double apparent_power = 0;
        double power_factor = 0;
        double kwh_usage = 0;
        double current = 0;
        uint64_t timestamp = 0;

        const double active_power() const {
            return apparent_power * power_factor;
        }

        const double reactive_power() const {
            return sqrt(pow(apparent_power, 2) - pow(active_power(), 2));
        }

        const double phase_angle() const {
            return acos(power_factor);
        }

        Reading(ReadingDataPacket& data, uint64_t timestamp) {
            voltage = data.voltage;
            frequency = data.frequency;
            apparent_power = data.apparent_power;
            power_factor = data.power_factor;
            kwh_usage = data.energy_usage;
            current = data.current;
            Reading::timestamp = timestamp;
            ESP_LOGI("Reading", "New Reading: %fV %fHz %fVA PF:%f TS:%d", voltage, frequency, apparent_power, power_factor, kwh_usage, timestamp);
        }

        Reading(ps::queue<double>& var, uint64_t ts) {
            voltage = var.front();
            var.pop();
            frequency = var.front();
            var.pop();
            apparent_power = var.front();
            var.pop();
            power_factor = var.front();
            var.pop();
            kwh_usage = var.front();
            var.pop();

            timestamp = ts;
        }

        Reading(double v, double fr, double cur, double sp, double pf, double kwh, uint64_t ts) :
        voltage(v),
        frequency(fr),
        apparent_power(sp),
        power_factor(pf),
        kwh_usage(kwh),
        timestamp(ts),
        current(cur)
        {}

};

#endif