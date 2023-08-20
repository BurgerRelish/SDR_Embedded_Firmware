#pragma once

#ifndef READING_H
#define READING_H

#include <ArduinoJson.h>

namespace sdr {
    
class Reading {
    public:
        double voltage;
        double frequency;
        double active_power;
        double reactive_power;
        double apparent_power;
        double power_factor;
        double kwh_usage;
        uint64_t timestamp;

        Reading(ps::queue<double>& var, uint64_t ts) {
            voltage = var.front();
            var.pop();
            frequency = var.front();
            var.pop();
            active_power = var.front();
            var.pop();
            reactive_power = var.front();
            var.pop();
            apparent_power = var.front();
            var.pop();
            power_factor = var.front();
            var.pop();
            kwh_usage = var.front();
            var.pop();

            timestamp = ts;
        }

        Reading(double v, double fr, double ap, double rp, double sp, double pf, double kwh, uint64_t ts) :
        voltage(v),
        frequency(fr),
        active_power(ap),
        reactive_power(rp),
        apparent_power(sp),
        power_factor(pf),
        kwh_usage(kwh),
        timestamp(ts)
        {}

};

}

#endif