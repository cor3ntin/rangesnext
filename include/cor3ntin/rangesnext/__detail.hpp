/*
Copyright (c) 2020 Corentin Jabot

Licenced under modified MIT license. See LICENSE.md for details.
*/

#pragma once

#include <functional>
#include <ranges>

namespace cor3ntin::rangesnext::detail {

namespace r = std::ranges;

template <class F, class Tuple, std::size_t... I>
constexpr decltype(auto) apply_impl(F &&f, const Tuple &t,
                                    std::index_sequence<I...>) {
    return f(std::get<std::tuple_size_v<std::remove_reference_t<Tuple>> - 1>(t),
             std::get<I>(t)...);
}

template <class F, class Tuple>
constexpr decltype(auto) apply_last(F &&f, const Tuple &t) {
    return detail::apply_impl(
        std::forward<F>(f), t,
        std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<Tuple>> -
                                 1>{});
}

template <typename T>
concept has_iterator_category = requires {
    typename std::iterator_traits<r::iterator_t<T>>::iterator_category;
};

template <typename T>
concept has_iterator_concept = requires {
    typename std::iterator_traits<r::iterator_t<T>>::iterator_concept;
};

template <typename... V>
consteval auto iter_cat() {
    if constexpr ((r::random_access_range<V> && ...))
        return std::random_access_iterator_tag{};
    else if constexpr ((r::bidirectional_range<V> && ...))
        return std::bidirectional_iterator_tag{};
    else if constexpr ((r::forward_range<V> && ...))
        return std::forward_iterator_tag{};
    else if constexpr ((r::input_range<V> && ...))
        return std::input_iterator_tag{};
    else
        return std::output_iterator_tag{};
}

template <class R>
concept simple_view = r::view<R> &&r::range<const R>
    &&std::same_as<r::iterator_t<R>, r::iterator_t<const R>>
        &&std::same_as<r::sentinel_t<R>, r::sentinel_t<const R>>;

template <typename T>
inline constexpr bool pair_like = false;
template <typename F, typename S>
inline constexpr bool pair_like<std::pair<F, S>> = true;
template <typename F, typename S>
inline constexpr bool pair_like<std::tuple<F, S>> = true;
} // namespace cor3ntin::rangesnext::detail
