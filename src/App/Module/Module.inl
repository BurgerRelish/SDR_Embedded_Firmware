#pragma once

#ifndef MODULE_INL
#define MODULE_INL

#include "Module.h"

/**
 * @brief Gets the maximum value of the attribute
 * 
 * @param attribute 
 * @return double 
 */
template <typename T>
const T Module::calc_max(const T Reading::* attribute, const ps::deque<Reading>& _readings) const {
    T max = 0;
    for (auto& reading : _readings) {
        if(reading.*attribute > max) max = reading.*attribute;
    }
    return max;
}

/**
 * @brief Gets the minimum value of the attribute
 * 
 * @param attribute 
 * @return double 
 */
template <typename T>
const T Module::calc_min(const T Reading::* attribute, const ps::deque<Reading>& _readings) const {
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
const T Module::calc_mode(const T Reading::* attribute, const ps::deque<Reading>& _readings) const {
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
const T Module::calc_mean(const T Reading::* attribute, const ps::deque<Reading>& _readings) const {
    T ret = 0;

    for (const auto& reading : _readings) {
        ret += reading.*attribute;
    }

    return (ret / _readings.size());
}


/**
 * @brief Calculates the unbiased estimator standard deviation of the provided attribute.
 * 
 * @param mean 
 * @param attribute 
 * @return double 
 */
template <typename T>
const T Module::calc_stddev(const T Reading::* attribute, const ps::deque<Reading>& _readings) const {
    T mean = calc_mean(attribute, _readings);
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
const T Module::calc_iqr(const T Reading::* attribute, const ps::deque<Reading>& _readings) const {
    ps::deque<T> attributeValues;
    for (const auto& reading : _readings) {
        attributeValues.push_back(reading.*attribute);
    }

    size_t n = attributeValues.size();
    std::sort(attributeValues.begin(), attributeValues.end());

    size_t q1_index = n / 4;
    size_t q3_index = (3 * n) / 4;

    if (q1_index < 1) q1_index = 1;
    if (q3_index > n) q3_index = n;

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
const T Module::calc_kurt(const T mean, const T Reading::* attribute, const ps::deque<Reading>& _readings) const {
    T variance = pow(calc_stddev<T>(attribute, _readings), 2);

    auto n = _readings.size();

    if (n < 10) return T(0);

    T C1 = ((n + 1) * n) / ((n - 1) * (n - 2) * (n - 3));
    T C2 = (-3 * pow(n - 1, 2)) / ((n - 2) * (n - 3));
    T C3 = 0;

    for (const auto& reading : _readings) {
        C3 += pow(reading.*attribute - mean, 4);
    }

    return static_cast<T>(C1 * (C3 / pow(variance, 2)) - C2);
}


template <typename T>
const T Module::max(const T Reading::* attribute) {
    return calc_max(attribute, readings);
}

template <typename T>
const T Module::min(const T Reading::* attribute) {
    return calc_min(attribute, readings); 
}

template <typename T>
const T Module::mode(const T Reading::* attribute) {
    return calc_mode(attribute, readings);
}

template <typename T>
const T Module::mean(const T Reading::* attribute) {
    return calc_mean(attribute, readings);
}

template <typename T>
const T Module::stddev(const T Reading::* attribute) {
    return calc_stddev(attribute, readings);
}

template <typename T>
const T Module::iqr(const T Reading::* attribute) {
    return calc_iqr(attribute, readings);
}

template <typename T>
const T Module::kurt(const T Reading::* attribute) {
    T mn = mean(attribute);
    return calc_kurt(mn, attribute, readings);
}



#endif