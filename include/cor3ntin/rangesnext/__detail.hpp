/*
Copyright (c) 2020 - present Corentin Jabot

Licenced under Boost Software License license. See LICENSE.md for details.
*/

#pragma once

#include <functional>
#include <ranges>

namespace cor3ntin::rangesnext::detail {

namespace r = std::ranges;

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
