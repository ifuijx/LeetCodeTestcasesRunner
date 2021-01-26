#pragma once

#ifndef __IFUIJX_UNIVERSAL_TO__
#define __IFUIJX_UNIVERSAL_TO__

#include <vector>
#include <string>
#include <any>
#include <type_traits>


template <typename T>
constexpr auto __is_container(T*) -> decltype((typename T::value_type *)(nullptr), true) {
    return true;
}

inline constexpr bool __is_container(...) { return false; }

template <typename T>
struct _Is_container {
    static constexpr bool value = __is_container((T*)nullptr);
};

template <typename T>
constexpr bool __container_of_any = _Is_container<T>::value && !std::is_same_v<T, std::string>;

template <typename T, bool=__container_of_any<T>>
struct _Any_cast_type_deducer {
    using __type = T;

    static T to(std::any const & value) {
        return std::any_cast<T>(value);
    }
};

template <typename T>
struct _Any_cast_type_deducer<T, true> {
    using __type = std::vector<std::any>;

    static T to(std::any const & value) {
        __type anys = std::any_cast<__type>(value);
        std::vector<typename T::value_type> mid_vec;
        for (auto const & v : anys) {
            mid_vec.push_back(_Any_cast_type_deducer<typename T::value_type>::to(v));
        }
        return T(mid_vec.begin(), mid_vec.end());
    }
};

template <typename TDest>
TDest any_to(std::any const & value) {
    return _Any_cast_type_deducer<TDest>::to(value);
}

#endif /* !__IFUIJX_UNIVERSAL_TO__ */
