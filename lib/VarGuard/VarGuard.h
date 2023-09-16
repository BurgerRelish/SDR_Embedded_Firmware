#pragma once

#ifndef VARGUARD_H
#define VARGUARD_H

/**
 * @brief A simple template class which guards access to a global variable using a semaphore. On construction, the provided mutex is taken. 
 * The data type can then by get/set by reference using the data() method. The semaphore is automatically given back once the class runs out of scope.
 * Cannot be copied.
 * 
 * @tparam DataType The type of data the class should hold.
 */
template <typename DataType>
class VarGuard {
    private:
        SemaphoreHandle_t& _mutex;
        std::shared_ptr<DataType> _data;
    public:
        VarGuard(std::shared_ptr<DataType> data, SemaphoreHandle_t& mutex) :_mutex(mutex), _data(data) {
            xSemaphoreTake(_mutex, portMAX_DELAY);
        }

        VarGuard(SemaphoreHandle_t& mutex) :_mutex(mutex), _data(nullptr) {
            xSemaphoreTake(_mutex, portMAX_DELAY);
        }

        /**
         * @brief Returns a read/write reference to the data in the class.
         * 
         * @return DataType& 
         */
        DataType& data() {
            if (!_data) {
                throw std::runtime_error("Data not initialized");
            }
            return *_data;
        }

        std::shared_ptr<DataType> ptr() {
            return _data;
        }

        /**
         * @brief Operator to set the internal shared pointer of the guard class.
         * 
         * @param data - std::shared_ptr of DataType in guard class.
         * @return VarGuard& 
         */
        VarGuard& operator=(const std::shared_ptr<DataType>& data) {
            _data = data;
            return *this;
        }
        
        // Destructor
        ~VarGuard() {
            xSemaphoreGive(_mutex);
        }
};

#endif