/*

Copyright (c) 2020 - present Corentin Jabot
Copyright (c) 2017 - present Lewis Baker

Thhis code has been adapted from cppcoro
https://github.com/lewissbaker/cppcoro

Licenced under Boost Software License license.
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

namespace cor3ntin::rangesnext {

template <typename YieldedType,
          typename ValueType = std::remove_cvref_t<YieldedType>>
class [[nodiscard]] generator
    : std::ranges::view_interface<generator<YieldedType, ValueType>> {
    class promise {
      public:
        using value_type = ValueType;
        using reference_type = std::add_lvalue_reference_t<YieldedType>;
        using pointer_type = std::remove_reference_t<reference_type> *;

        promise() = default;

        auto get_return_object() noexcept {

            return generator{
                std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        constexpr std::suspend_always initial_suspend() const {
            return {};
        }
        constexpr std::suspend_always final_suspend() const {
            return {};
        }

        std::suspend_always
        yield_value(std::remove_reference_t<reference_type> &&value) noexcept {
            m_value = std::addressof(value);
            return {};
        }

        std::suspend_always
        yield_value(std::remove_reference_t<reference_type> &value) noexcept {
            m_value = std::addressof(value);
            return {};
        }

        reference_type value() const noexcept {
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
        pointer_type m_value;
    };

    struct sentinel {};

    class iterator {
        using coroutine_handle = std::coroutine_handle<promise>;

      public:
        using iterator_category = std::input_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = promise::value_type;
        using reference = promise::reference_type;
        using pointer = promise::pointer_type;

        iterator() noexcept = default;
        iterator(const iterator &) = delete;
        iterator(iterator &&) = default;

        iterator &operator=(iterator &&) {
            return *this;
        }

        explicit iterator(coroutine_handle coroutine) noexcept
            : m_coroutine(coroutine) {
        }

        friend bool operator==(const iterator &it, sentinel) noexcept {
            return !it.m_coroutine || it.m_coroutine.done();
        }

        friend bool operator==(sentinel s, const iterator &it) noexcept {
            return it == s;
        }

        iterator &operator++() {
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

  public:
    using promise_type = promise;

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

    auto begin() {
        if (m_coroutine) {
            m_coroutine.resume();
        }
        return iterator{m_coroutine};
    }

    auto end() const noexcept {
        return sentinel{};
    }

    void swap(generator & other) noexcept {
        std::swap(m_coroutine, other.m_coroutine);
    }

  private:
    explicit generator(std::coroutine_handle<promise> coroutine) noexcept
        : m_coroutine(coroutine) {
    }

    std::coroutine_handle<promise> m_coroutine = nullptr;
};

} // namespace cor3ntin::rangesnext
