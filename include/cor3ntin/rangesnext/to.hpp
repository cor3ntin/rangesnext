/*
Copyright (c) 2020 - present Corentin Jabot

Licenced under Boost Software License license. See LICENSE.md for details.
*/

#pragma once
#include <algorithm>
#include <iterator>
#include <ranges>
#include <tuple>
#include <utility>

namespace cor3ntin::rangesnext {

struct from_range_t{};
inline constexpr from_range_t from_range;

namespace detail {

namespace r = std::ranges;
template <typename T>
concept container_like = (r::range<T> && !r::view<T>);

template <typename T>
inline constexpr bool always_false_v = false;

template <r::range Rng>
struct range_common_iterator_impl {
    using type =
        std::common_iterator<std::ranges::iterator_t<Rng>, r::sentinel_t<Rng>>;
};

template <typename Rng>
struct dummy_input_iterator {
    using iterator_category = std::input_iterator_tag;
    using value_type = std::ranges::range_value_t<Rng>;
    using difference_type = std::ranges::range_difference_t<Rng>;
    ;
    using pointer = std::ranges::range_value_t<Rng> *;
    using reference = std::ranges::range_reference_t<Rng>;

    int operator*() const;
    bool operator==(const dummy_input_iterator &other) const;
    reference operator++(int);
    dummy_input_iterator &operator++();
};

template <r::common_range Rng>
struct range_common_iterator_impl<Rng> {
    using type = r::iterator_t<Rng>;
};

template <r::range Rng>
requires(!std::copyable<
         std::ranges::iterator_t<Rng>>) struct range_common_iterator_impl<Rng> {
    using type = dummy_input_iterator<Rng>;
};

template <r::range Rng>
using range_common_iterator = typename range_common_iterator_impl<Rng>::type;

template <typename C>
struct container_value;

template <typename C>
requires requires {
    typename r::range_value_t<C>;
}
struct container_value<C> {
    using type = r::range_value_t<C>;
};
template <typename C>
requires(!requires {
    typename r::range_value_t<C>;
}) struct container_value<C> {
    using type = typename C::value_type;
};

template <typename C>
using container_value_t = container_value<C>::type;

template <class C, class R>
concept container_convertible =
    !r::view<C> && r::input_range<R> &&
    std::convertible_to<r::range_reference_t<R>, container_value_t<C>>;

template <typename View>
constexpr bool has_inner_range = r::input_range<container_value_t<View>>;

template <typename View>
using inner_range_t =
    std::conditional_t<has_inner_range<View>, container_value_t<View>, View>;

template <class C, class R>
constexpr bool recursive_container_convertible_impl =
    container_convertible<C, R> ||
    (has_inner_range<C> && has_inner_range<R> &&
     recursive_container_convertible_impl<inner_range_t<C>, inner_range_t<R>>);

template <class C, class R>
concept recursive_container_convertible =
    recursive_container_convertible_impl<C, R>;

template <template <class...> class T>
struct wrap {};

template <r::range Rng>
static auto get_begin(Rng &&rng) {
    using It = r::iterator_t<Rng>;
    if constexpr (!(std::copyable<It>)) {
        return r::begin(rng);
    } else {
        using I = range_common_iterator<Rng>;
        I begin = I(r::begin(rng));
        if constexpr (std::is_rvalue_reference_v<decltype(rng)>) {
            return std::make_move_iterator(std::move(begin));
        } else {
            return begin;
        }
    }
}
template <r::range Rng>
static auto get_end(Rng &&rng) {
    using It = r::iterator_t<Rng>;
    if constexpr (!(std::copyable<It>)) {
        return r::end(rng);
    } else {
        using I = range_common_iterator<Rng>;
        I end = I(r::end(rng));
        if constexpr (std::is_rvalue_reference_v<decltype(rng)>) {
            return std::make_move_iterator(std::move(end));
        } else {
            return end;
        }
    }
}

template <typename Cont, typename Rng, typename... Args>
struct unwrap {
    using type = Cont;
};

template <template <class...> class Cont, typename Rng, typename... Args>
struct unwrap<wrap<Cont>, Rng, Args...> {
    template <typename R>
    static auto from_rng(int) -> decltype(Cont(range_common_iterator<Rng>(),
                                               range_common_iterator<Rng>(),
                                               std::declval<Args>()...));
    template <typename R>
    static auto from_rng(long) -> decltype(Cont(from_range, std::declval<Rng>(), std::declval<Args>()...));

    using type =
        std::remove_cvref_t<std::remove_pointer_t<decltype(from_rng<Rng>(0))>>;
};

template <typename T, typename Rng>
concept reservable_container = requires(T &c, Rng &&rng) {
    c.reserve(decltype(r::size(rng))());
};

template <typename T>
concept insertable_container = requires(T &c, T::value_type &e) {
    c.push_back(e);
};

struct to_container {
  private:
    template <typename I, typename Sentinel, typename Value>
    struct iterator {
      private:
        I it_;

      public:
        using difference_type =
            typename std::iterator_traits<I>::difference_type;
        using value_type = Value;
        using reference = Value;
        using pointer = typename std::iterator_traits<I>::pointer;
        using iterator_category =
            typename std::iterator_traits<I>::iterator_category;

        iterator() = default;

        iterator(auto it) : it_(std::move(it)) {
        }

        friend bool operator==(const iterator &a, const iterator &b) {
            return a.it_ == b.it_;
        }

        friend bool operator==(const iterator &a, const Sentinel &b) {
            return a.it_ == b;
        }

        reference operator*() const {
            return to_container::fn<value_type>()(*it_);
        }

        auto &operator++() {
            ++it_;
            return *this;
        }
        auto operator++(int) requires std::copyable<I> {
            auto tmp = *this;
            ++it_;
            return tmp;
        }

        auto &operator--() requires std::derived_from<
            iterator_category, std::bidirectional_iterator_tag> {
            --it_;
            return *this;
        }
        auto operator--(int) requires std::derived_from<
            iterator_category, std::bidirectional_iterator_tag> {
            auto tmp = *this;
            --it_;
            return tmp;
        }
        auto operator+=(difference_type n) requires std::derived_from<
            iterator_category, std::random_access_iterator_tag> {
            it_ += n;
            return *this;
        }
        auto operator-=(difference_type n) requires std::derived_from<
            iterator_category, std::random_access_iterator_tag> {
            it_ += n;
            return *this;
        }
        friend auto
        operator+(iterator it, difference_type n) requires std::derived_from<
            iterator_category, std::random_access_iterator_tag> {
            return it += n;
        }
        friend auto
        operator-(iterator it, difference_type n) requires std::derived_from<
            iterator_category, std::random_access_iterator_tag> {
            return it -= n;
        }

        friend auto
        operator-(iterator a, iterator b) requires std::derived_from<
            iterator_category, std::random_access_iterator_tag> {
            return a.it_ - b.it_;
        }

        auto operator[](difference_type n) const
            requires std::derived_from<iterator_category,
                                       std::random_access_iterator_tag> {
            return *(*this + n);
        }
    };

    template <typename ToContainer, typename Rng, typename... Args>
    using container_t = typename unwrap<ToContainer, Rng, Args...>::type;

    template <typename C, typename... Args>
    struct fn {
      private:
        template <typename Cont, typename Rng, typename It, typename Sentinel>
        constexpr static auto from_iterators(It begin, Sentinel end, Rng &&rng,
                                             Args &&... args) {
            // copy or move (optimization)
            if constexpr (std::constructible_from<Cont, Rng, Args...>) {
                return Cont(std::forward<Rng>(rng),
                            std::forward<Args>(args)...);
            }
            else if constexpr (std::constructible_from<Cont, from_range_t, Rng, Args...>) {
                return Cont(from_range, std::forward<Rng>(rng),
                            std::forward<Args>(args)...);
            }
            // we can do push back
            else if constexpr (insertable_container<Cont> &&
                               r::sized_range<Rng> &&
                               reservable_container<Cont, Rng> &&
                               std::constructible_from<Cont, Args...>) {
                Cont c(std::forward<Args...>(args)...);
                c.reserve(r::size(rng));
                r::copy(std::move(begin), std::move(end),
                        std::inserter(c, std::end(c)));
                return c;
            }
            // default case
            else if constexpr (std::constructible_from<Cont, It, Sentinel,
                                                       Args...>) {
                return Cont(std::move(begin), std::move(end),
                            std::forward<Args>(args)...);
            }
            // Covers the Move only iterator case
            else if constexpr (std::constructible_from<Cont, Args...>) {
                Cont c(std::forward<Args>(args)...);
                r::copy(std::move(begin), std::move(end),
                        std::inserter(c, std::end(c)));
                return c;
            } else {
                static_assert(always_false_v<Cont>,
                              "Can't construct a container");
            }
        }

        template <container_like Cont, r::range Rng>
        requires container_convertible<Cont, Rng> constexpr static auto
        impl(Rng &&rng, Args &&... args) {
            return from_iterators<Cont>(get_begin(std::forward<Rng>(rng)),
                                        get_end(std::forward<Rng>(rng)),
                                        std::forward<Rng>(rng),
                                        std::forward<Args>(args)...);
        }

        template <container_like Cont, r::range Rng>
            requires recursive_container_convertible<Cont, Rng> &&
                std::constructible_from<Cont, Args...> &&
            (!container_convertible<Cont, Rng>)&&(
                !std::constructible_from<
                    Cont, Rng>)constexpr static auto impl(Rng &&rng,
                                                          Args &&... args) {
            auto begin = get_begin(std::forward<Rng>(rng));
            auto end = get_end(std::forward<Rng>(rng));
            using It =
                iterator<decltype(begin), decltype(end), inner_range_t<Cont>>;
            return from_iterators<Cont, Rng>(It(std::move(begin)), end,
                                             std::forward<Rng>(rng),
                                             std::forward<Args>(args)...);
        }

      public:
        template <typename Rng>
        requires r::input_range<Rng> &&
            recursive_container_convertible<container_t<C, Rng, Args...>,
                                            Rng &&> inline constexpr auto
            operator()(Rng &&rng, Args &&... args) const {
            return impl<container_t<C, Rng, Args...>>(
                std::forward<Rng>(rng), std::forward<Args>(args)...);
        }
        std::tuple<Args...> args;
    };

    template <typename Rng, typename ToContainer, typename... Args>
    requires r::input_range<Rng> && recursive_container_convertible<container_t<ToContainer, Rng, Args...>, Rng>
        constexpr friend auto
        operator|(Rng &&rng, fn<ToContainer, Args...> && f) -> container_t<ToContainer, Rng, Args...> {
      return [&]<size_t...I>(std::index_sequence<I...>)
      {
        return f(std::forward<Rng>(rng), std::forward<Args>(std::get<I>(f.args))...);
      }(std::make_index_sequence<sizeof...(Args)>());
    }
};

template <typename ToContainer, typename... Args>
using to_container_fn = to_container::fn<ToContainer, Args...>;
} // namespace detail

template <template <typename...> class ContT, typename... Args, detail::to_container = {}>
requires (!std::ranges::range<Args>&&...)
constexpr auto to(Args&&... args)
    -> detail::to_container_fn<detail::wrap<ContT>, Args...> {
    detail::to_container_fn<detail::wrap<ContT>, Args...> fn;
    fn.args = std::forward_as_tuple(std::forward<Args>(args)...);
    return fn;
}

template <template <typename...> class ContT, std::ranges::input_range Rng, typename... Args>
requires std::ranges::range<Rng> constexpr auto to(Rng &&rng, Args &&... args) {
    return detail::to_container_fn<detail::wrap<ContT>, Args...>{}(
        std::forward<Rng>(rng), std::forward<Args>(args)...);
}

template <typename Cont, typename... Args, detail::to_container = {}>
requires (!std::ranges::range<Args>&&...)
constexpr auto to(Args&&...args)
    -> detail::to_container_fn<Cont, Args...> {
    detail::to_container_fn<Cont, Args...> fn;
    fn.args = std::forward_as_tuple(std::forward<Args>(args)...);
    return fn;
}

template <typename Cont, std::ranges::input_range Rng, typename... Args>
requires detail::recursive_container_convertible<Cont, Rng> constexpr auto
to(Rng &&rng, Args &&... args) -> Cont {
    return detail::to_container_fn<Cont, Args...>{}(
        std::forward<Rng>(rng), std::forward<Args>(args)...);
}

} // namespace cor3ntin::rangesnext
