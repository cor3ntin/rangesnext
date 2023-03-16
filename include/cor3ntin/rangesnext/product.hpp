/*
Copyright (c) 2020 - present Corentin Jabot

Licenced under Boost Software License. See LICENSE.md for details.
*/

#pragma once

#include <cor3ntin/rangesnext/__detail.hpp>
#include <ranges>
#include <tuple>

namespace cor3ntin::rangesnext {

namespace r = std::ranges;

namespace detail {

template <typename First, typename... R>
constexpr bool valid_product_pack(r::input_range<First> &&
                                  (r::forward_range<R> && ...));

} // namespace detail

template <r::view... V>
    requires(sizeof...(V) == 0) ||
    detail::valid_product_pack<V...> class product_view
    : public std::ranges::view_interface<product_view<V...>> {

    std::tuple<V...> bases_;

  public:
    constexpr product_view() = default;
    constexpr product_view(V... base) : bases_(std::move(base)...) {
    }

    template <bool IsConst>
    struct sentinel;

    template <bool IsConst>
    struct iterator {
      private:
        using parent =
            std::conditional_t<IsConst, const product_view, product_view>;
        parent *view_ = nullptr;
        std::tuple<r::iterator_t<V>...> its_;
        using result = std::tuple<r::range_reference_t<V>...>;

        template <bool IsConst_>
        friend struct product_view::sentinel;

      public:
        using iterator_category = decltype(detail::iter_cat<V...>());
        using reference = result;
        using value_type = std::tuple<r::range_value_t<V>...>;
        using difference_type = std::common_type_t<r::range_difference_t<V>...>;

        iterator() = default;
        explicit iterator(parent *view, r::iterator_t<V>... its)
            : view_(view), its_(std::move(its)...) {
        }

        auto operator*() const {
            return std::apply(
                [&](const auto &... args) { return result{*(args)...}; }, its_);
        }

        constexpr iterator operator++(int) {
            if constexpr ((r::forward_range<V> && ...)) {
                auto tmp = *this;
                ++*this;
                return tmp;
            }
            ++*this;
        }

        constexpr iterator &operator++() {
            next();
            return *this;
        }

        constexpr iterator &
        operator--() requires(r::bidirectional_range<V> &&...) {
            prev();
            return *this;
        }

        constexpr iterator
        operator--(int) requires(r::bidirectional_range<V> &&...) {
            auto tmp = *this;
            --*this;
            return tmp;
        }

        constexpr iterator &operator+=(difference_type n) requires(
            r::random_access_range<V> &&...) {
            advance(n);
            return *this;
        }

        constexpr iterator &operator-=(difference_type n) requires(
            r::random_access_range<V> &&...) {
            advance(-n);
            return *this;
        }

        friend constexpr iterator
        operator+(iterator i,
                  difference_type n) requires(r::random_access_range<V> &&...) {
            return {i + n};
        }

        friend constexpr iterator
        operator+(difference_type n,
                  iterator i) requires(r::random_access_range<V> &&...) {
            return {i + n};
        }

        friend constexpr iterator
        operator-(iterator i,
                  difference_type n) requires(r::random_access_range<V> &&...) {
            return {i - n};
        }

        friend constexpr difference_type
        operator-(const iterator &x,
                  const iterator &y) requires(r::random_access_range<V> &&...) {
            return y.distance(x);
        }

        constexpr decltype(auto) operator[](difference_type n) const
            requires(r::random_access_range<V> &&...) {
            return *iterator{*this + n};
        }

        constexpr bool operator==(const iterator &other) const {
            if (at_end() && other.at_end())
                return true;
            return eq(*this, other);
        }

        friend constexpr auto operator<=>(
            const iterator &x,
            const iterator &y) requires(r::random_access_range<V> &&...) &&
            (std::three_way_comparable<r::iterator_t<V>> && ...) {
            return compare(x, y);
        }

        friend constexpr bool operator==(const iterator &i,
                                         const sentinel<IsConst> &) {
            return i.at_end();
        }
        friend constexpr bool operator==(const iterator &i,
                                         const sentinel<!IsConst> &) {
            return i.at_end();
        }

      private:
        constexpr bool at_end() const {
            const auto &v = std::get<0>(view_->bases_);
            return std::end(v) == std::get<0>(its_);
        }

        template <auto N = 0>
        constexpr static auto compare(const iterator &a, const iterator &b)
            -> std::strong_ordering {
            auto cmp = std::get<N>(a.its_) <=> std::get<N>(b.its_);
            if constexpr (N + 1 < sizeof...(V)) {
                if (cmp == 0)
                    return compare<N + 1>(a, b);
            }
            return cmp;
        }

        template <auto N = sizeof...(V) - 1>
        constexpr static bool eq(const iterator &a, const iterator &b) {
            if (std::get<N>(a.its_) != std::get<N>(b.its_)) {
                return false;
            }
            if constexpr (N > 0) {
                return eq<N - 1>(a, b);
            }
            return true;
        }

        template <auto N = sizeof...(V) - 1>
        constexpr void next() {
            const auto &v = std::get<N>(view_->bases_);
            auto &it = std::get<N>(its_);
            if (++it ==
                std::end(
                    v)) { // TODO r::end doesn't compile for istream_view Bug ?
                if constexpr (N != 0) {
                    it = r::begin(v);
                    next<N - 1>();
                }
            }
        }

        template <auto N = sizeof...(V) - 1>
        constexpr void prev() {
            const auto &v = std::get<N>(view_->bases_);
            auto &it = std::get<N>(its_);
            if (it == r::begin(v)) {
                r::advance(it, r::end(v));
                if constexpr (N > 0)
                    prev<N - 1>();
            }
            --it;
        }

        template <std::size_t N = sizeof...(V) - 1>
        constexpr difference_type distance(const iterator &other) const {
            if constexpr (N == 0) {
                return std::get<0>(other.its_) - std::get<0>(its_);
            } else {
                const auto d = this->distance<N - 1>(other);
                auto const scale = r::distance(std::get<N>(view_->bases_));
                auto const increment =
                    std::get<N>(other.its_) - std::get<N>(its_);

                return difference_type{(d * scale) + increment};
            }
        }

        template <std::size_t N = sizeof...(V) - 1>
        void advance(difference_type n) {
            if (n == 0)
                return;

            auto &i = std::get<N>(its_);
            auto const size = static_cast<difference_type>(
                r::size(std::get<N>(view_->bases_)));
            auto const first = r::begin(std::get<N>(view_->bases_));

            auto const idx = static_cast<difference_type>(i - first);
            n += idx;

            auto div = size ? n / size : 0;
            auto mod = size ? n % size : 0;

            if constexpr (N != 0) {
                if (mod < 0) {
                    mod += size;
                    div--;
                }
                advance<N - 1>(div);
            } else {
                if (div > 0) {
                    mod = size;
                }
            }
            using D = std::iter_difference_t<decltype(first)>;
            i = first + static_cast<D>(mod);
        }
    };

    template <bool IsConst>
    struct sentinel {
      private:
        friend iterator<false>;
        friend iterator<true>;

        using parent =
            std::conditional_t<IsConst, const product_view, product_view>;
        parent *view_ = nullptr;
        std::tuple<r::sentinel_t<V>...> end_;

      public:
        sentinel() = default;

        constexpr explicit sentinel(parent *view, r::sentinel_t<V>... end)
            : view_(view), end_(std::move(end)...) {
        }

        constexpr sentinel(sentinel<!IsConst> other) requires IsConst &&
            (std::convertible_to<r::sentinel_t<V>, r::sentinel_t<const V>> &&
             ...)
            : view_(other.view_),
        end_(other.end_) {
        }
    };

  public:
    constexpr auto size() const requires((r::sized_range<V>)&&...) {
        return std::apply(
            []<typename... Args>(const Args &... args) {
                using Size = std::common_type_t<r::range_size_t<Args>...>;
                return (Size(r::size(args)) * ...);
            },
            bases_);
    }

    constexpr auto begin() requires(!detail::simple_view<V> || ...) {
        return std::apply(
            [&](auto &... args) {
                using r::begin;
                return iterator<false>{this, begin(args)...};
            },
            bases_);
    }

    constexpr auto begin() const requires(detail::simple_view<V> &&...) {
        return std::apply(
            [&](const auto &... args) {
                using r::begin;
                return iterator<true>{this, begin(args)...};
            },
            bases_);
    }

    constexpr auto end() const requires(r::common_range<V> &&...) {
        return std::apply(
            [&](const auto &first, const auto &... args) {
                using r::end;
                using r::begin;
                return iterator<true>(this, end(first), begin(args)...);
            },
            bases_);
    }

    constexpr auto end() const requires(!r::common_range<V> || ...) {
        return std::apply(
            [&](const auto &... args) {
                return sentinel<true>(this, std::end(args)...);
            },
            bases_);
    }
};

template <typename... Rng>
product_view(Rng &&...) -> product_view<r::views::all_t<Rng>...>;

namespace detail {

struct product_view_fn {
    template <typename... R>
    constexpr auto operator()(R &&... ranges) const {
        return product_view{std::forward<R>(ranges)...};
    }
};
} // namespace detail

inline detail::product_view_fn product;

} // namespace cor3ntin::rangesnext
