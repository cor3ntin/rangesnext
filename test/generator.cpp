/*
Copyright (c) 2020 - present Corentin Jabot

Licenced under Boost Software License license. See LICENSE.md for details.
*/

#include <catch2/catch.hpp>
#include <cor3ntin/rangesnext/generator.hpp>
#include <cor3ntin/rangesnext/to.hpp>

#include <sstream>

using namespace cor3ntin::rangesnext;
namespace r = std::ranges;

static_assert(std::ranges::input_range<generator<int>>);
static_assert(!std::ranges::forward_range<generator<int>>);

TEST_CASE("Basic Generator Tests", "[Generator]") {

    auto f = []<typename T>(T &&rng) -> generator<r::range_reference_t<T>> {
        for (auto &&e : rng)
            co_yield e;
    };

    auto assert_same = [&](auto &&rng) {
        auto g = f(rng);
        CHECK_THAT(g | to<std::vector>, Catch::Equals(rng | to<std::vector>));
    };

    SECTION("Move only", "[Generator]") {
        auto ints = std::istringstream{"1 2 5 3 4"};
        auto v = r::istream_view<int>(ints);
        CHECK_THAT(f(v) | to<std::vector>,
                   Catch::Equals(std::vector{1, 2, 5, 3, 4}));
    }

    SECTION("const ints", "[Generator]") {
        std::initializer_list lst{1, 2, 3, 4};
        static_assert(
            std::same_as<r::range_reference_t<decltype(f(lst))>, const int &>);
        assert_same(lst);
    }

    SECTION("ints", "[Generator]") {
        std::vector lst{1, 2, 3, 4};
        static_assert(
            std::same_as<r::range_reference_t<decltype(f(lst))>, int &>);
        assert_same(lst);
    }
}

template <typename Rng1, typename Rng2>
generator<std::tuple<r::range_reference_t<Rng1>, r::range_reference_t<Rng2>>,
          std::tuple<r::range_value_t<Rng1>, r::range_value_t<Rng2>>>
zip(Rng1 r1, Rng2 r2) {
    auto it1 = begin(r1);
    auto it2 = begin(r2);
    auto end1 = end(r1);
    auto end2 = end(r2);
    while (it1 != end1 && it2 != end2) {
        co_yield {*it1++, *it2++};
    }
}

TEST_CASE("Zip Generator", "[Generator]") {
    std::vector d{0, 1, 2, 3};
    std::array<double, 4> f{4.0, 5.0, 6.0, 7};

    auto z = zip(d, f);
    using R = decltype(z);
    static_assert(
        std::same_as<r::range_reference_t<R>, std::tuple<int &, double &> &>);
    static_assert(std::same_as<r::range_value_t<R>, std::tuple<int, double>>);

    std::vector<std::tuple<int, double>> expected = {
        {0, 4}, {1, 5}, {2, 6}, {3, 7}};

    CHECK_THAT(z | to<std::vector>, Catch::Equals(expected));
}
