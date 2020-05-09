/*

Copyright (c) 2020 - present Corentin Jabot
Copyright (c) 2017 - present Lewis Baker

Thhis code has been adapted from cppcoro
https://github.com/lewissbaker/cppcoro

Licenced under Modified MIT license.
See LICENSE.md for details.
*/

#include <algorithm>
#include <coroutine>
#include <exception>
#include <iterator>
#include <numbers>
#include <optional>
#include <ranges>
#include <type_traits>

namespace rangesnext {
template <typename T>
class generator;

namespace detail {
template <typename T>
struct box : std::optional<T> {
    using std::optional<T>::optional;
};
template <typename T>
struct box<T &> {
    box() = default;
    box(T &u) {
        value = std::addressof(u);
    }
    decltype(auto) operator*() const {
        return *value;
    }

  private:
    T *value = nullptr;
};

template <typename T>
class generator_promise {
  public:
    using value_type = std::remove_reference_t<T>;
    using reference_type = std::conditional_t<std::is_reference_v<T>, T, T &>;
    using pointer_type = value_type *;

    generator_promise() = default;

    generator<T> get_return_object() noexcept;

    constexpr std::suspend_always initial_suspend() const {
        return {};
    }
    constexpr std::suspend_always final_suspend() const {
        return {};
    }

    template <typename U = T>
    requires(!std::is_reference_v<T>) std::suspend_always
        yield_value(U value) noexcept {
        m_value = std::forward<U>(value);
        return {};
    }

    std::suspend_always yield_value(T &value) noexcept
        requires std::is_reference_v<T> {
        m_value = value;
        return {};
    }

    reference_type value() noexcept {
        return *m_value;
    }

    // Don't allow any use of 'co_await' inside the generator coroutine.
    template <typename U>
    std::suspend_never await_transform(U &&value) = delete;

    void return_void() noexcept {
    }

    void unhandled_exception() {
        throw;
    }

  private:
    box<T> m_value;
};

struct generator_sentinel {};

template <typename T>
class generator_iterator {
    using coroutine_handle = std::coroutine_handle<generator_promise<T>>;

  public:
    using iterator_category = std::input_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = generator_promise<T>::value_type;
    using reference = generator_promise<T>::reference_type;
    using pointer = generator_promise<T>::pointer_type;

    generator_iterator() noexcept = default;
    generator_iterator(const generator_iterator &) = delete;
    generator_iterator(generator_iterator &&) = default;

    generator_iterator &operator=(generator_iterator &&) {
        return *this;
    }

    explicit generator_iterator(coroutine_handle coroutine) noexcept
        : m_coroutine(coroutine) {
    }

    friend bool operator==(const generator_iterator &it,
                           generator_sentinel) noexcept {
        return !it.m_coroutine || it.m_coroutine.done();
    }

    friend bool operator==(generator_sentinel s,
                           const generator_iterator &it) noexcept {
        return it == s;
    }

    generator_iterator &operator++() {
        m_coroutine.resume();
        return *this;
    }
    void operator++(int) {
        (void)operator++();
    }

    reference operator*() const noexcept {
        return m_coroutine.promise().value();
    }

    pointer operator->() const noexcept {
        return std::addressof(operator*());
    }

  private:
    coroutine_handle m_coroutine = nullptr;
};
} // namespace detail

template <typename T>
class [[nodiscard]] generator : std::ranges::view_interface<generator<T>> {
  public:
    using promise_type = detail::generator_promise<T>;
    using iterator = detail::generator_iterator<T>;

    generator() = default;

    generator(generator && other) noexcept : m_coroutine(other.m_coroutine) {
        other.m_coroutine = nullptr;
    }

    generator(const generator &other) = delete;

    ~generator() {
        if (m_coroutine) {
            m_coroutine.destroy();
        }
    }

    generator &operator=(generator other) noexcept {
        swap(other);
        return *this;
    }

    iterator begin() {
        if (m_coroutine) {
            m_coroutine.resume();
        }
        return iterator{m_coroutine};
    }

    detail::generator_sentinel end() const noexcept {
        return detail::generator_sentinel{};
    }

    void swap(generator & other) noexcept {
        std::swap(m_coroutine, other.m_coroutine);
    }

  private:
    friend class detail::generator_promise<T>;

    explicit generator(std::coroutine_handle<promise_type> coroutine) noexcept
        : m_coroutine(coroutine) {
    }

    std::coroutine_handle<promise_type> m_coroutine = nullptr;
};

template <typename T>
void swap(generator<T> &a, generator<T> &b) {
    a.swap(b);
}

namespace detail {
template <typename T>
generator<T> generator_promise<T>::get_return_object() noexcept {
    using coroutine_handle = std::coroutine_handle<generator_promise<T>>;
    return generator<T>{coroutine_handle::from_promise(*this)};
}
} // namespace detail
} // namespace rangesnext

static_assert(std::input_iterator<rangesnext::detail::generator_iterator<int>>);
static_assert(
    !std::forward_iterator<rangesnext::detail::generator_iterator<int>>);
static_assert(!std::copyable<rangesnext::detail::generator_iterator<int>>);
static_assert(std::ranges::input_range<rangesnext::generator<int>>);
static_assert(!std::ranges::forward_range<rangesnext::generator<int>>);
