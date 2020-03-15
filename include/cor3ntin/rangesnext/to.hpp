/*

Copyright (c) 2019 Corentin Jabot, Christopher Di Bella, Eric Niebler, Casey Carter

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:
The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

--- Optional exception to the license ---
As an exception, if, as a result of your compiling your source code, portions
of this Software are embedded into a machine-executable object form of such
source code, you may redistribute such embedded portions in such object form
without including the above copyright and permission notices.

*/

#pragma once
#include <algorithm>
#include <iterator>
#include <ranges>
#include <type_traits>

namespace rangesnext {

namespace detail {

namespace r = std::ranges;
template <typename T>
concept container_like = (r::range<T> && !r::view<T>);

template <template <class...> class C, typename Rng, typename... Args>
using associative_container_t =
    C<std::tuple_element_t<0, Rng>, std::tuple_element_t<1, Rng>, Args...>;

template <template <typename...> typename T, typename _, typename... Ts>
struct is_template_instantiable : std::false_type {};

template <template <typename...> typename T, typename... Ts>
struct is_template_instantiable<T, std::void_t<decltype(sizeof(T<Ts...>))>,
                                Ts...> : std::true_type {};

template <template <typename...> typename T, typename... Ts>
inline constexpr auto is_template_instantiable_v =
    is_template_instantiable<T, void, Ts...>::value;

template <class T>
concept pair_like = requires {
        typename std::tuple_size<T>::type;
    }
    && std::tuple_size_v<T> == 2;

template <template <typename...> typename C, typename RangeType,
          typename... Args>
concept maybe_associative_cont =
    pair_like<RangeType> &&is_template_instantiable_v<
        C, std::tuple_element_t<0, RangeType>,
        std::tuple_element_t<1, RangeType>, std::allocator<RangeType>>;

template <typename T>
inline constexpr bool always_false_v = false;

template <r::range Rng>
struct range_common_iterator_impl {
    using type =
        std::common_iterator<std::ranges::iterator_t<Rng>, r::sentinel_t<Rng>>;
};

template <r::common_range Rng>
struct range_common_iterator_impl<Rng> {
    using type = r::iterator_t<Rng>;
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
    })
struct container_value<C> {
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
        using I = range_common_iterator<const Rng &>;
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
        using I = range_common_iterator<const Rng &>;
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
    static auto from_rng(int) -> decltype(
        Cont(std::declval<decltype(get_begin(std::declval<R>())())>(),
             std::declval<decltype(get_end(std::declval<R>())())>(),
             std::declval<Args>()...));

    template <typename R>
    static auto from_rng(long) {
        return std::add_pointer_t<Cont<r::range_value_t<R>, Args...>>(nullptr);
    }
    template <typename R>
    requires maybe_associative_cont<Cont, r::range_value_t<R>,
                                    Args...> static auto
    from_rng(long) {
        return std::add_pointer_t<
            associative_container_t<Cont, r::range_value_t<R>, Args...>>(
            nullptr);
    }
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
            // we can do push back
            else if constexpr (insertable_container<Cont> &&
                               r::sized_range<Rng> &&
                               reservable_container<Cont, Rng> &&
                               std::constructible_from<Cont, Args...>) {
                Cont c(std::forward<Args...>(args)...);
                c.reserve(r::size(rng));
                r::copy(std::move(begin), std::move(end),
                        std::back_inserter(c));
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
                        std::back_inserter(c));
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
    };

    template <typename Rng, typename ToContainer>
    requires r::input_range<Rng> &&
        recursive_container_convertible<container_t<ToContainer, Rng>,
                                        Rng> inline constexpr friend auto
        operator|(Rng &&rng, fn<ToContainer> (*)(to_container))
            -> container_t<ToContainer, Rng> {
        return std::move(fn<ToContainer>{}(std::forward<Rng>(rng)));
    }

    template <typename Rng, typename ToContainer>
    requires r::input_range<Rng> &&recursive_container_convertible<
        container_t<ToContainer, Rng>, Rng> constexpr friend auto
    operator|(Rng &&rng, fn<ToContainer> &&) -> container_t<ToContainer, Rng> {
        return std::move(fn<ToContainer>{}(std::forward<Rng>(rng)));
    }
};
template <typename ToContainer, typename... Args>
using to_container_fn = to_container::fn<ToContainer, Args...>;
} // namespace detail

template <template <typename...> class ContT, typename... Args>
constexpr auto to(detail::to_container = {})
    -> detail::to_container_fn<detail::wrap<ContT>, Args...> {
    return {};
}

template <template <typename...> class ContT, typename Rng, typename... Args>
requires std::ranges::range<Rng> constexpr auto to(Rng &&rng, Args &&... args) {
    return detail::to_container_fn<detail::wrap<ContT>, Args...>{}(
        std::forward<Rng>(rng), std::forward<Args>(args)...);
}

template <typename Cont, typename... Args>
constexpr auto to(detail::to_container = {})
    -> detail::to_container_fn<Cont, Args...> {
    return {};
}

template <typename Cont, typename Rng, typename... Args>
requires detail::recursive_container_convertible<Cont, Rng> constexpr auto
to(Rng &&rng, Args &&... args) -> Cont {
    return detail::to_container_fn<Cont, Args...>{}(
        std::forward<Rng>(rng), std::forward<Args>(args)...);
}

} // namespace rangesnext