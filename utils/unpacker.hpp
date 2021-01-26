#pragma once

#ifndef __IFUIJX_UNPACKER__
#define __IFUIJX_UNPACKER__

#include <any>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <universal_to.hpp>

using std::size_t;

template <size_t... _Indices>
struct _IdxTuple {};

/// Generate integer sequence of _IdxTuple
template <typename, size_t, size_t>
struct _Integer_seq_generator {};

template <size_t _Num, size_t... _Indices>
struct _Integer_seq_generator<_IdxTuple<_Indices...>, _Num, _Num> {
    using __type = _IdxTuple<_Indices...>;
};

template <size_t _I, size_t _Num, size_t... _Indices>
struct _Integer_seq_generator<_IdxTuple<_Indices...>, _I, _Num> {
    using __type = typename _Integer_seq_generator<_IdxTuple<_Indices..., _I>, _I + 1, _Num>::__type;
};

/// Generate _IdxTuple<0, 1, ..., _Num - 1>
template <size_t _Num>
struct _Build_index_tuple {
    using __type = typename _Integer_seq_generator<_IdxTuple<>, 0, _Num>::__type;
};

/// Implementation of _Type_extractor
template <size_t _I, typename T, typename... Ts>
struct _Type_extractor_impl {
    using __type = typename _Type_extractor_impl<_I - 1, Ts...>::__type;
};

template <typename T, typename... Ts>
struct _Type_extractor_impl<0, T, Ts...> {
    using __type = T;
};

/// Extract ith (_I) type of Ts...
template <size_t _I, typename... Ts>
struct _Type_extractor {
    using __type = typename _Type_extractor_impl<_I, Ts...>::__type;
};

template <typename AnyCont, typename TElement>
TElement __any_cast_element(AnyCont && cont, size_t i) {
    if constexpr (std::is_rvalue_reference_v<decltype(cont)>) {
        return any_to<TElement>(std::move(cont[i]));
    }
    if constexpr (!std::is_rvalue_reference_v<decltype(cont)>) {
        return any_to<TElement>(cont[i]);
    }
}

/// Cast and collect params from a container at given offset
template <typename AnyCont, typename CurParams, size_t I, typename... Args>
auto __param_pack_helper(AnyCont && cont, CurParams && curparams, size_t offset) {
    if constexpr (I < sizeof...(Args)) {
        using param_t = typename _Type_extractor<I, Args...>::__type;
        auto expand_params = std::tuple_cat(std::forward<CurParams>(curparams), std::tuple<param_t>(
            __any_cast_element<decltype(std::forward<AnyCont>(cont)), std::decay_t<param_t>>(
                std::forward<AnyCont>(cont), I + offset
            )
        ));
        return __param_pack_helper<decltype(std::forward<AnyCont>(cont)), decltype(std::move(expand_params)), I + 1, Args...>(
            std::forward<AnyCont>(cont), std::move(expand_params), offset
        );
    }
    if constexpr (I >= sizeof...(Args)) {
        return std::move(curparams);
    }
}

template <typename AnyCont, typename... Args>
auto __param_pack(AnyCont && cont, size_t offset) {
    return __param_pack_helper<decltype(std::forward<AnyCont>(cont)), std::tuple<> &&, 0, Args...>(
        std::forward<AnyCont>(cont), std::tuple<>(), offset
    );
}

template <typename Callable, typename... Ts, size_t... Indices>
auto __invoke(Callable && callable, std::tuple<Ts...> && params, _IdxTuple<Indices...> *) {
    return std::invoke(std::forward<Callable>(callable), std::get<Indices>(std::move(params))...);
}

template <typename Callable, typename T, typename... Ts, size_t... Indices>
auto __invoke(Callable && callable, T && obj, std::tuple<Ts...> && params, _IdxTuple<Indices...> *) {
    return std::invoke(std::forward<Callable>(callable), std::forward<T>(obj), std::get<Indices>(std::move(params))...);
}

template <bool _return_void, typename...>
struct _Invoke_dispatcher {};

template <typename... Ts>
struct _Invoke_dispatcher<true, Ts...> {
    template <typename AnyCont, typename Callable>
    static void invoke(AnyCont && cont, Callable && callable) {
        for (size_t offset = 0; offset < cont.size(); offset += sizeof...(Ts)) {
            __invoke(
                callable,
                __param_pack<decltype((std::forward<AnyCont>(cont))), Ts...>(std::forward<AnyCont>(cont), offset),
                static_cast<typename _Build_index_tuple<sizeof...(Ts)>::__type*>(nullptr)
            );
        }
    }

    template <typename AnyCont, typename Callable, typename T>
    static void invoke(AnyCont && cont, Callable && callable, T && obj) {
        for (size_t offset = 0; offset < cont.size(); offset += sizeof...(Ts)) {
            __invoke(
                callable,
                obj,
                __param_pack<decltype((std::forward<AnyCont>(cont))), Ts...>(std::forward<AnyCont>(cont), offset),
                static_cast<typename _Build_index_tuple<sizeof...(Ts)>::__type*>(nullptr)
            );
        }
    }
};

template <typename... Ts>
struct _Invoke_dispatcher<false, Ts...> {
    template <typename AnyCont, typename Callable>
    static auto invoke(AnyCont && cont, Callable && callable) {
        std::vector<std::decay_t<std::invoke_result_t<Callable, Ts...>>> ret;
        for (size_t offset = 0; offset < cont.size(); offset += sizeof...(Ts)) {
            ret.push_back(
                __invoke(
                    callable,
                    __param_pack<decltype((std::forward<AnyCont>(cont))), Ts...>(std::forward<AnyCont>(cont), offset),
                    static_cast<typename _Build_index_tuple<sizeof...(Ts)>::__type*>(nullptr)
                )
            );
        }
        return ret;
    }

    template <typename AnyCont, typename Callable, typename T>
    static auto invoke(AnyCont && cont, Callable && callable, T && obj) {
        std::vector<std::decay_t<std::invoke_result_t<Callable, T, Ts...>>> ret;
        for (size_t offset = 0; offset < cont.size(); offset += sizeof...(Ts)) {
            ret.push_back(
                __invoke(
                    callable,
                    obj,
                    __param_pack<decltype((std::forward<AnyCont>(cont))), Ts...>(std::forward<AnyCont>(cont), offset),
                    static_cast<typename _Build_index_tuple<sizeof...(Ts)>::__type*>(nullptr)
                )
            );
        }
        return ret;
    }
};

template <typename... Ts>
struct Unpacker {
    template <typename AnyCont, typename Callable>
    static auto invoke(AnyCont && cont, Callable && callable) {
        return _Invoke_dispatcher<std::is_same_v<void, std::invoke_result_t<Callable, Ts...>>, Ts...>::invoke(
            std::forward<AnyCont>(cont), std::forward<Callable>(callable)
        );
    }

    template <typename AnyCont, typename Callable, typename T>
    static auto invoke(AnyCont && cont, Callable && callable, T && obj) {
        return _Invoke_dispatcher<std::is_same_v<void, std::invoke_result_t<Callable, T, Ts...>>, Ts...>::invoke(
            std::forward<AnyCont>(cont), std::forward<Callable>(callable), std::forward<T>(obj)
        );
    }
};

#endif /* !__IFUIJX_UNPACKER__ */
