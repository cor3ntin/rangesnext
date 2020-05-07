#include <ranges>
#include <vector>

namespace rangesnext {

namespace r = std::ranges;

template <class R>
concept simple_view = // exposition only
    r::view<R> &&r::range<const R>
        &&std::same_as<r::iterator_t<R>, r::iterator_t<const R>>
            &&std::same_as<r::sentinel_t<R>, r::sentinel_t<const R>>;

template <r::input_range V>
requires r::view<V> class enumerate_view
    : public r::view_interface<enumerate_view<V>> {

    V base_ = {};

    template <bool Const>
    struct iterator {
      private:
        using Base = std::conditional_t<Const, const V, V>;

        struct result {
            const r::range_difference_t<V> index;
            r::range_reference_t<Base> value;
        };

        r::iterator_t<Base> current_ = r::iterator_t<Base>();
        r::range_difference_t<Base> pos_ = 0;

        template <bool>
        friend class iterator;

      public:
        using iterator_category = typename std::iterator_traits<
            r::iterator_t<Base>>::iterator_category;
        using value_type = result;
        using difference_type = r::range_difference_t<Base>;

        iterator() = default;

        constexpr explicit iterator(r::iterator_t<Base> current,
                                    r::range_difference_t<Base> pos)
            : current_(std::move(current)), pos_(pos) {
        }
        constexpr iterator(iterator<!Const> i) requires Const
            &&std::convertible_to<r::iterator_t<V>, r::iterator_t<Base>>
            : current_(std::move(i.current_)), pos_(i.pos_) {
        }

        constexpr const r::iterator_t<V> &
        base() const &requires std::copyable<r::iterator_t<Base>> {
            return current_;
        }

        constexpr r::iterator_t<V> base() && {
            return std::move(current_);
        }

        constexpr decltype(auto) operator*() const {
            return result{pos_, *current_};
        }

        constexpr iterator &operator++() {
            ++pos_;
            ++current_;
            return *this;
        }

        constexpr auto operator++(int) {
            ++pos_;
            if constexpr (r::forward_range<V>) {
                auto tmp = *this;
                ++*this;
                return tmp;
            } else {
                ++current_;
            }
        }

        constexpr auto operator--() requires r::bidirectional_range<V> {
            --pos_;
            --current_;
            return *this;
        }

        constexpr auto operator--(int) requires r::bidirectional_range<V> {
            auto tmp = *this;
            --*this;
            return tmp;
        }

        constexpr auto
        operator+=(difference_type n) requires r::random_access_range<V> {
            current_ += n;
            pos_ += n;
            return *this;
        }

        constexpr auto
        operator-=(difference_type n) requires r::random_access_range<V> {
            current_ -= n;
            pos_ -= n;
            return *this;
        }

        friend constexpr auto
        operator+(iterator i,
                  difference_type n) requires r::random_access_range<V> {
            return iterator{i.current_ + n,
                            static_cast<difference_type>(i.pos_ + n)};
        }

        friend constexpr auto
        operator-(iterator i,
                  difference_type n) requires r::random_access_range<V> {
            return iterator{i.current_ - n,
                            static_cast<difference_type>(i.pos_ - n)};
        }

        constexpr decltype(auto) operator[](difference_type n) const
            requires r::random_access_range<Base> {
            return result{static_cast<difference_type>(pos_ + n),
                          *(current_ + n)};
        }

        friend constexpr bool operator==(
            const iterator &x,
            const iterator
                &y) requires std::equality_comparable<r::iterator_t<Base>> {
            return x.current_ == y.current_;
        }

        friend constexpr bool
        operator<(const iterator &x,
                  const iterator &y) requires r::random_access_range<Base> {
            return x.current_ < y.current_;
        }

        friend constexpr bool
        operator>(const iterator &x,
                  const iterator &y) requires r::random_access_range<Base> {
            return x.current_ > y.current_;
        }

        friend constexpr bool
        operator<=(const iterator &x,
                   const iterator &y) requires r::random_access_range<Base> {
            return x.current_ <= y.current_;
        }
        friend constexpr bool
        operator>=(const iterator &x,
                   const iterator &y) requires r::random_access_range<Base> {
            return x.current_ >= y.current_;
        }
        friend constexpr auto
        operator<=>(const iterator &x,
                    const iterator &y) requires r::random_access_range<Base>
            &&std::three_way_comparable<r::iterator_t<Base>> {
            return x.current_ <=> y.current_;
        }

        friend constexpr iterator
        operator+(const iterator &x,
                  difference_type y) requires r::random_access_range<Base> {
            return iterator{x} += y;
        }
        friend constexpr iterator
        operator+(difference_type x,
                  const iterator &y) requires r::random_access_range<Base> {
            return {y + x};
        }
        friend constexpr iterator
        operator-(const iterator &x,
                  difference_type y) requires r::random_access_range<Base> {
            return iterator{x} -= y;
        }
        friend constexpr difference_type
        operator-(const iterator &x,
                  const iterator &y) requires r::random_access_range<Base> {
            return {y - x};
        }
    };

    template <bool Const>
    struct sentinel {
      private:
        friend iterator<false>;
        friend iterator<true>;

        using Base = std::conditional_t<Const, const V, V>;

        r::sentinel_t<V> end_;

      public:
        sentinel() = default;

        constexpr explicit sentinel(r::sentinel_t<V> end)
            : end_(std::move(end)) {
        }

        constexpr auto base() {
            return end_;
        }

        friend constexpr bool operator==(const iterator<Const> &i,
                                         const sentinel &s) {
            return i.base() == s.s;
        }

        friend constexpr r::range_difference_t<Base>
        operator-(const iterator<Const> &x, const sentinel &y) requires std::
            sized_sentinel_for<r::sentinel_t<Base>, r::iterator_t<Base>> {
            return x.current_ - y.end_;
        }

        friend constexpr r::range_difference_t<Base>
        operator-(const sentinel &x, const iterator<Const> &y) requires std::
            sized_sentinel_for<r::sentinel_t<Base>, r::iterator_t<Base>> {
            return x.end_ - y.current_;
        }
    };

  public:
    constexpr enumerate_view() = default;
    constexpr enumerate_view(V base) : base_(std::move(base)) {
    }

    constexpr auto begin() requires(!simple_view<V>) {
        return iterator<false>(std::ranges::begin(base_), 0);
    }

    constexpr auto begin() const requires simple_view<V> {
        return iterator<true>(std::ranges::begin(base_), 0);
    }

    constexpr auto end() {
        return sentinel<false>{r::end(base_)};
    }

    constexpr auto end() requires r::common_range<V> {
        return iterator<false>{std::ranges::end(base_),
                               static_cast<r::range_difference_t<V>>(size())};
    }

    constexpr auto end() const requires r::range<const V> {
        return sentinel<true>{std::ranges::end(base_)};
    }

    constexpr auto end() const requires r::common_range<const V> {
        return iterator<true>{std::ranges::end(base_),
                              static_cast<r::range_difference_t<V>>(size())};
    }

    constexpr auto size() requires r::sized_range<V> {
        return std::ranges::size(base_);
    }

    constexpr auto size() const requires r::sized_range<const V> {
        return std::ranges::size(base_);
    }

    constexpr V base() const &requires std::copyable<V> {
        return base_;
    }

    constexpr V base() && {
        return std::move(base_);
    }
};

template <typename R>
requires r::input_range<R> enumerate_view(R &&r)
    -> enumerate_view<r::views::all_t<R>>;

namespace detail {

struct enumerate_view_fn {
    template <typename R>
    constexpr auto operator()(R &&r) const {
        return enumerate_view{std::forward<R>(r)};
    }

    template <r::input_range R>
    constexpr friend auto operator|(R &&rng, const enumerate_view_fn &) {
        return enumerate_view{std::forward<R>(rng)};
    }
};
} // namespace detail

inline detail::enumerate_view_fn enumerate;

} // namespace rangesnext

int main() {
    std::array vec{1, 2, 3, 4};

#ifndef UGLY
    for (auto &&[i, e] : rangesnext::enumerate(vec)) {
        printf("%lu %d\n", i, e);
        e *= 10;
    }
#else
    for (std::size_t i = 0; i < 4; ++i) {
        printf("%lu %d\n", i, vec[i]);
        vec[i] *= 10;
    }
#endif
}