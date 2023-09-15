# Custom C++ Data Containers

This document describes a set of custom C++ data containers that utilize the ESP32's PSRAM and a custom allocator. These containers have been implemented to provide functionality similar to the corresponding containers in the C++ Standard Library (STL).
## Features
- A modified version of the C++ allocator which allocates to PSRAM.

## Usage

To use these containers in your C++ program, include the appropriate header file for the desired container. For example, to use `ps::deque`, include `ps_deque.h`, or to include all, use `ps_stl.h`.

```cpp
#include <Arduino.h>
#include "ps_deque"
#include <deque>

// Example usage of ps::deque
void setup() {
    psramInit(); // Initialise PSRAM.

    ps::deque<int> deque;
    deque.push_back(1);
    deque.push_back(2);
    deque.push_back(3);

    // Access elements
    int front = deque.front();
    int back = deque.back();


    int front = deque.front();
    int back = deque.back();

    // Print elements
    for (auto it = deque.begin(); it != deque.end(); ++it) {
        cout << *it << " ";
    }

    return;
}