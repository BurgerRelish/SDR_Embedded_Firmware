#pragma once

#ifndef READING_H
#define READING_H

#include <ArduinoJson.h>
#include <../ModuleInterface/ModuleInterface.h>
#include <math.h>

class Reading {
    public:
        double voltage;
        double frequency;
        double apparent_power;
        double power_factor;
        double kwh_usage;
        uint64_t timestamp;

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
            Reading::timestamp = timestamp;
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

        Reading(double v, double fr, double sp, double pf, double kwh, uint64_t ts) :
        voltage(v),
        frequency(fr),
        apparent_power(sp),
        power_factor(pf),
        kwh_usage(kwh),
        timestamp(ts)
        {}

};

#endif