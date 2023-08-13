#pragma once

#ifndef MODULE_INL
#define MODULE_INL

#include "Module.h"

namespace sdr {

/**
 * @brief Gets the maximum value of the attribute
 * 
 * @param attribute 
 * @return double 
 */
template <typename T>
T Module::calc_max(T Reading::* attribute, ps::deque<Reading>& _readings) {
    T max = 0;
    for (auto& reading : _readings) {
        if(reading.*attribute > max) max = reading.*attribute;
    }
    return max;
}

/**
 * @brief Gets the maximum value of the attribute
 * 
 * @param attribute 
 * @return double 
 */
template <typename T>
T Module::calc_min(T Reading::* attribute, ps::deque<Reading>& _readings) {
    T min = _readings.front().*attribute;
    for (auto& reading : _readings) {
        if(reading.*attribute < min) min = reading.*attribute;
    }
    return min;
}

/**
 * @brief Gets the mode of the attribute.
 * 
 * @tparam T 
 * @param _readings 
 * @param attribute 
 * @return T 
 */
template <typename T>
T Module::calc_mode(T Reading::* attribute, ps::deque<Reading>& _readings) {
    std::unordered_map<T, int> frequencyMap;

    for (const auto& reading : _readings) {
        frequencyMap[reading.*attribute]++;
    }

    T mode = _readings.at(0).*attribute;
    int maxFrequency = 0;

    for (const auto& pair : frequencyMap) {
        if (pair.second > maxFrequency) {
            maxFrequency = pair.second;
            mode = pair.first;
        }
    }

    return mode;
}

/**
 * @brief Gets the mean value of the attribute.
 * 
 * @param attribute 
 * @return double 
 */

template <typename T>
T Module::calc_mean(T Reading::* attribute, ps::deque<Reading>& _readings) {
    T ret = 0;

    for (const auto& reading : _readings) {
        ret += reading.*attribute;
    }

    return (ret / _readings.size());
}


/**
 * @brief Calculates the standard deviation of the provided attribute.
 * 
 * @param mean 
 * @param attribute 
 * @return double 
 */
template <typename T>
T Module::calc_stddev(T Reading::* attribute, ps::deque<Reading>& _readings) {
    T mean = calc_mean(_readings, attribute);
    T variance = 0;

    for (const auto& reading : _readings) {
        variance += pow((reading.*attribute - mean), 2);
    }

    variance /= (_readings.size() - 1);

    return sqrt(variance);
}


/**
 * @brief Calculates the IQR of the provided attribute.
 * 
 * @param data 
 * @param attribute 
 * @return double 
 */
template <typename T>
T Module::calc_iqr(T Reading::* attribute, ps::deque<Reading>& _readings) {
    ps::deque<T> attributeValues;
    for (const auto& reading : _readings) {
        attributeValues.push_back(reading.*attribute);
    }

    size_t n = attributeValues.size();
    std::sort(attributeValues.begin(), attributeValues.end());

    size_t q1_index = n / 4;
    size_t q3_index = (3 * n) / 4;

    T q1_value = (attributeValues[q1_index - 1] + attributeValues[q1_index]) / 2.0;
    T q3_value = (attributeValues[q3_index - 1] + attributeValues[q3_index]) / 2.0;

    return q3_value - q1_value;
}


/**
 * @brief Calculates the Kurtosis of the provided attribute.
 * 
 * @param mean 
 * @param attribute 
 * @return double 
 */
template <typename T>
T Module::calc_kurt(T mean, T Reading::* attribute, ps::deque<Reading>& _readings) {
    T variance = calc_stddev<T>(_readings, attribute);

    auto n = _readings.size();

    T C1 = ((n + 1) * n) / ((n - 1) * (n - 2) * (n - 3));
    T C2 = (-3 * pow(n - 1, 2)) / ((n - 2) * (n - 3));

    T C3 = 0;
    for (const auto& reading : _readings) {
        C3 += pow(reading.*attribute - mean, 4);
    }

    return static_cast<T>(C1 * (C3 / variance) - C2);
}


template <typename T>
T Module::max(T Reading::* attribute) {
    return calc_max(attribute, readings);
}

template <typename T>
T Module::min(T Reading::* attribute) {
    return calc_min(attribute, readings); 
}

template <typename T>
T Module::mode(T Reading::* attribute) {
    return calc_mode(attribute, readings);
}

template <typename T>
T Module::mean(T Reading::* attribute) {
    return calc_mean(attribute, readings);
}

template <typename T>
T Module::stddev(T Reading::* attribute) {
    return calc_stddev(attribute, readings);
}

template <typename T>
T Module::iqr(T Reading::* attribute) {
    return calc_iqr(attribute, readings);
}

template <typename T>
T Module::kurt(T Reading::* attribute) {
    T mn = mean(attribute);
    return calc_kurt(mn, attribute, readings);
}

}


#endif