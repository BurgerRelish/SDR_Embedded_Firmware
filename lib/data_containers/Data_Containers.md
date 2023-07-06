# Custom C++ Data Containers

This document describes a set of custom C++ data containers that utilize the ESP32's PSRAM and a custom allocator. These containers have been implemented to provide functionality similar to the corresponding containers in the C++ Standard Library (STL). The following containers are included:

- `ps_deque`: Implements `std::deque` using the ESP's PSRAM.
- `ps_queue`: Implements `std::queue` using the ESP's PSRAM.
- `ps_stack`: Implements `std::stack` using the ESP's PSRAM.
- `ps_vector`: Implements `std::vector` using the ESP's PSRAM.
- `ps_string`: Implements `std::string` using the ESP's PSRAM.

## Features

- All containers utilize the ESP32's PSRAM for memory storage.
- A custom allocator has been implemented to manage memory allocation and deallocation on the PSRAM. (`psram_allocator.h`)
- The containers support the following operations:
  - Copying from PSRAM to SRAM or vice versa using the `<<=` operator.
  - Moving from PSRAM to SRAM or vice versa with the `>>=` operator.
  - Some implementations of standard operators for each container and its STL counterpart. (e.g. `==`)

## Usage

To use these containers in your C++ program, include the appropriate header file for the desired container. For example, to use `ps_deque`, include `ps_deque.h`. Ensure that PSRAM is correctly initialised for your framework, i.e for the Arduino Framework, call `psramInit()` in `setup()`.

```cpp
#include <Arduino.h>
#include "ps_deque.h"
#include <deque>

// Example usage of ps_deque
void setup() {
    psramInit(); // Initialise PSRAM.

    ps_deque<int> deque;
    deque.push_back(1);
    deque.push_back(2);
    deque.push_back(3);

    // Access elements
    int front = deque.front();
    int back = deque.back();

    // Copy to SRAM

    std::deque<int> deque_2;
    deque_2 <<= deque;

    int front = deque_2.front();
    int back = deque_2.back();

    // Print elements
    for (auto it = deque.begin(); it != deque.end(); ++it) {
        cout << *it << " ";
    }

    return;
}