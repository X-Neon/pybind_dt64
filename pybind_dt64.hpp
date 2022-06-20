#pragma once

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <chrono>

namespace pybind_dt64::detail {
    class numpy_c_api
    {
    public:
        static void** get() {
            static numpy_c_api api;
            return api.m_ptr;
        }

    private:
        numpy_c_api() {
            auto m = pybind11::module_::import("numpy.core.multiarray");
            auto c = m.attr("_ARRAY_API");
            m_ptr = (void **)PyCapsule_GetPointer(c.ptr(), nullptr);
        }

        void** m_ptr;
    };

    class datetime_data
    {
    public:
        static auto get() {
            static datetime_data dd;
            return dd.m_func;
        }

    private:
        datetime_data() : m_func(pybind11::module_::import("numpy").attr("datetime_data")) {}

        pybind11::detail::str_attr_accessor m_func;
    };

    template <typename T>
    struct dt_str {};

    template <>
    struct dt_str<std::chrono::years> { static constexpr const char* value = "M8[Y]"; };

    template <>
    struct dt_str<std::chrono::months> { static constexpr const char* value = "M8[M]"; };

    template <>
    struct dt_str<std::chrono::weeks> { static constexpr const char* value = "M8[W]"; };

    template <>
    struct dt_str<std::chrono::days> { static constexpr const char* value = "M8[D]"; };

    template <>
    struct dt_str<std::chrono::hours> { static constexpr const char* value = "M8[h]"; };

    template <>
    struct dt_str<std::chrono::minutes> { static constexpr const char* value = "M8[m]"; };

    template <>
    struct dt_str<std::chrono::seconds> { static constexpr const char* value = "M8[s]"; };

    template <>
    struct dt_str<std::chrono::milliseconds> { static constexpr const char* value = "M8[ms]"; };

    template <>
    struct dt_str<std::chrono::microseconds> { static constexpr const char* value = "M8[us]"; };

    template <>
    struct dt_str<std::chrono::nanoseconds> { static constexpr const char* value = "M8[ns]"; };

    template <>
    struct dt_str<std::chrono::duration<int64_t, std::pico>> { static constexpr const char* value = "M8[ps]"; };

    template <>
    struct dt_str<std::chrono::duration<int64_t, std::femto>> { static constexpr const char* value = "M8[fs]"; };

    template <>
    struct dt_str<std::chrono::duration<int64_t, std::atto>> { static constexpr const char* value = "M8[as]"; };

    template <typename I, typename T>
    struct td_str {};

    template <typename I>
    struct td_str<I, std::chrono::years::period> { static constexpr const char* value = "m8[Y]"; };

    template <typename I>
    struct td_str<I, std::chrono::months::period> { static constexpr const char* value = "m8[M]"; };

    template <typename I>
    struct td_str<I, std::chrono::weeks::period> { static constexpr const char* value = "m8[W]"; };

    template <typename I>
    struct td_str<I, std::chrono::days::period> { static constexpr const char* value = "m8[D]"; };

    template <typename I>
    struct td_str<I, std::chrono::hours::period> { static constexpr const char* value = "m8[h]"; };

    template <typename I>
    struct td_str<I, std::chrono::minutes::period> { static constexpr const char* value = "m8[m]"; };

    template <typename I>
    struct td_str<I, std::chrono::seconds::period> { static constexpr const char* value = "m8[s]"; };

    template <typename I>
    struct td_str<I, std::chrono::milliseconds::period> { static constexpr const char* value = "m8[ms]"; };

    template <typename I>
    struct td_str<I, std::chrono::microseconds::period> { static constexpr const char* value = "m8[us]"; };

    template <typename I>
    struct td_str<I, std::chrono::nanoseconds::period> { static constexpr const char* value = "m8[ns]"; };

    template <typename I>
    struct td_str<I, std::pico> { static constexpr const char* value = "m8[ps]"; };

    template <typename I>
    struct td_str<I, std::femto> { static constexpr const char* value = "m8[fs]"; };

    template <typename I>
    struct td_str<I, std::atto> { static constexpr const char* value = "m8[as]"; };
}

namespace pybind11::detail {
    template <typename Duration>
    struct type_caster<std::chrono::time_point<std::chrono::system_clock, Duration>>
    {
        using Type = std::chrono::time_point<std::chrono::system_clock, Duration>;
        PYBIND11_TYPE_CASTER(Type, const_name("numpy.datetime64"));

        static pybind11::handle cast(Type source, pybind11::return_value_policy, pybind11::handle) {
            pybind11::dtype dtype(pybind_dt64::detail::dt_str<Duration>::value);
            auto dtype_obj = dtype.release().ptr();

            int64_t ticks = source.time_since_epoch().count();

            auto PyArray_Scalar = reinterpret_cast<PyObject*(*)(void*, PyObject*, PyObject*)>(
                pybind_dt64::detail::numpy_c_api::get()[60]
            );
            return PyArray_Scalar(&ticks, dtype_obj, nullptr);
        }

        bool load(pybind11::handle src, bool) {
            auto PyDatetimeArrType_Type = reinterpret_cast<PyTypeObject*>(
                pybind_dt64::detail::numpy_c_api::get()[215]
            );
            if (!PyObject_TypeCheck(src.ptr(), PyDatetimeArrType_Type)) {
                return false;
            }

            int64_t ticks;
            auto PyArray_ScalarAsCtype = reinterpret_cast<void(*)(PyObject*, void*)>(
                pybind_dt64::detail::numpy_c_api::get()[62]
            );
            PyArray_ScalarAsCtype(src.ptr(), &ticks);

            pybind11::tuple result = pybind_dt64::detail::datetime_data::get()(src.attr("dtype"));
            int num = pybind11::int_(result[1]);
            if (num != 1) {
                return false;
            }

            std::string unit = pybind11::str(result[0]);
            using TP = std::chrono::time_point<std::chrono::system_clock>;
            using Period = std::chrono::system_clock::period;
            if (unit == "Y") {
                value = std::chrono::time_point_cast<Duration>(TP(std::chrono::years(ticks)));
            } else if (unit == "M") {
                value = std::chrono::time_point_cast<Duration>(TP(std::chrono::months(ticks)));
            } else if (unit == "W") {
                value = std::chrono::time_point_cast<Duration>(TP(std::chrono::weeks(ticks)));
            } else if (unit == "D") {
                value = std::chrono::time_point_cast<Duration>(TP(std::chrono::days(ticks)));
            } else if (unit == "h") {
                value = std::chrono::time_point_cast<Duration>(TP(std::chrono::hours(ticks)));
            } else if (unit == "m") {
                value = std::chrono::time_point_cast<Duration>(TP(std::chrono::months(ticks)));
            } else if (unit == "s") {
                value = std::chrono::time_point_cast<Duration>(TP(std::chrono::seconds(ticks)));
            } else if (unit == "ms") {
                value = std::chrono::time_point_cast<Duration>(TP(std::chrono::milliseconds(ticks)));
            } else if (unit == "us") {
                value = std::chrono::time_point_cast<Duration>(TP(std::chrono::microseconds(ticks)));
            } else if (unit == "ns") {
                value = std::chrono::time_point_cast<Duration>(TP(std::chrono::nanoseconds(ticks)));
            } else if (unit == "ps") {
                if constexpr (std::ratio_less_v<std::pico, Period>) {
                    return false;
                } else {
                    value = std::chrono::time_point_cast<Duration>(TP(std::chrono::duration<int64_t, std::pico>(ticks)));
                }
            } else if (unit == "fs") {
                if constexpr (std::ratio_less_v<std::femto, Period>) {
                    return false;
                } else {
                    value = std::chrono::time_point_cast<Duration>(TP(std::chrono::duration<int64_t, std::femto>(ticks)));
                }
            } else if (unit == "as") {
                if constexpr (std::ratio_less_v<std::atto, Period>) {
                    return false;
                } else {
                    value = std::chrono::time_point_cast<Duration>(TP(std::chrono::duration<int64_t, std::atto>(ticks)));
                }
            } else {
                return false;
            }

            return true;
        }
    };

    template <typename Rep, typename Period>
    struct type_caster<std::chrono::duration<Rep, Period>>
    {
        using Type = std::chrono::duration<Rep, Period>;
        PYBIND11_TYPE_CASTER(Type, const_name("numpy.timedelta64"));

        static pybind11::handle cast(Type source, pybind11::return_value_policy, pybind11::handle) {
            pybind11::dtype dtype(pybind_dt64::detail::td_str<Rep, Period>::value);
            auto dtype_obj = dtype.release().ptr();

            int64_t ticks = source.count();

            auto PyArray_Scalar = reinterpret_cast<PyObject*(*)(void*, PyObject*, PyObject*)>(
                pybind_dt64::detail::numpy_c_api::get()[60]
            );
            return PyArray_Scalar(&ticks, dtype_obj, nullptr);
        }

        bool load(pybind11::handle src, bool) {
            auto PyTimedeltaArrType_Type = reinterpret_cast<PyTypeObject*>(
                pybind_dt64::detail::numpy_c_api::get()[216]
            );
            if (!PyObject_TypeCheck(src.ptr(), PyTimedeltaArrType_Type)) {
                return false;
            }

            int64_t ticks;
            auto PyArray_ScalarAsCtype = reinterpret_cast<void(*)(PyObject*, void*)>(
                pybind_dt64::detail::numpy_c_api::get()[62]
            );
            PyArray_ScalarAsCtype(src.ptr(), &ticks);

            pybind11::tuple result = pybind_dt64::detail::datetime_data::get()(src.attr("dtype"));
            int num = pybind11::int_(result[1]);
            if (num != 1) {
                return false;
            }

            std::string unit = pybind11::str(result[0]);
            if (unit == "Y") {
                value = std::chrono::duration_cast<Type>(std::chrono::years(ticks));
            } else if (unit == "M") {
                value = std::chrono::duration_cast<Type>(std::chrono::months(ticks));
            } else if (unit == "W") {
                value = std::chrono::duration_cast<Type>(std::chrono::weeks(ticks));
            } else if (unit == "D") {
                value = std::chrono::duration_cast<Type>(std::chrono::days(ticks));
            } else if (unit == "h") {
                value = std::chrono::duration_cast<Type>(std::chrono::hours(ticks));
            } else if (unit == "m") {
                value = std::chrono::duration_cast<Type>(std::chrono::months(ticks));
            } else if (unit == "s") {
                value = std::chrono::duration_cast<Type>(std::chrono::seconds(ticks));
            } else if (unit == "ms") {
                value = std::chrono::duration_cast<Type>(std::chrono::milliseconds(ticks));
            } else if (unit == "us") {
                value = std::chrono::duration_cast<Type>(std::chrono::microseconds(ticks));
            } else if (unit == "ns") {
                value = std::chrono::duration_cast<Type>(std::chrono::nanoseconds(ticks));
            } else if (unit == "ps") {
                value = std::chrono::duration_cast<Type>(std::chrono::duration<int64_t, std::pico>(ticks));
            } else if (unit == "fs") {
                value = std::chrono::duration_cast<Type>(std::chrono::duration<int64_t, std::femto>(ticks));
            } else if (unit == "as") {
                value = std::chrono::duration_cast<Type>(std::chrono::duration<int64_t, std::atto>(ticks));
            } else {
                return false;
            }

            return true;
        }
    };
}

