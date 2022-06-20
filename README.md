# pybind_dt64

pybind_dt64 is a single header include that allows casting datetime types between C++ and Python when using Pybind11. Specifically, it defines the conversions `std::chrono::time_point <---> np.datetime64` and `std::chrono::duration <---> np.timedelta64`.

By default, Pybind11 allows you to cast between `std::chrono::time_point <---> np.datetime64` and `std::chrono::duration <---> np.timedelta64` when including the `<pybind11/chrono.h` header. pybind_dt64 allows you to use the corresponding Numpy types, rather than those in the Pytbon datetime library. This can be useful if you require more precision than the microsecond precision of datetime.

## Example

C++:
```cpp
auto current_time_in_seconds() {
    return std::chrono::time_point_cast<std::chrono::seconds>(
        std::chrono::system_clock::now()
    );
}
```

Python:
```python
>>> current_time_in_seconds()
numpy.datetime64('2022-06-20T21:05:34')
```

## Requirements

pybind_dt64 requires a C++17 compliant compiler.

## Usage

To use pybind_dt64, simply include the single header file:

```cpp
#include "pybind_dt64.hpp"
```