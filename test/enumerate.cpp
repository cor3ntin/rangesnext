/*
Copyright (c) 2020 Corentin Jabot

Licenced under modified MIT license. See LICENSE.md for details.
*/

#include <catch2/catch.hpp>
#include <cor3ntin/rangesnext/enumerate.hpp>
#include <list>
#include <sstream>
#include <vector>

namespace r = std::ranges;

template <class RangeT>
void test_enumerate_with(RangeT &&range) {
    auto enumerated_range = rangesnext::enumerate(range);

    std::size_t idx_ref = 0;
    auto it_ref = r::begin(range);

    for (auto &&[i, v] : enumerated_range) {
        CHECK(i == idx_ref++);
        CHECK(v == *it_ref++);
    }
}

TEST_CASE("Input ranges", "[Enumerate]") {
    auto ints = std::istringstream{"1 2 3 4"};
    auto v = r::istream_view<int>(ints);
    auto e = v | rangesnext::enumerate;

    auto expected =
        std::vector<std::tuple<ptrdiff_t, int>>{{0, 1}, {1, 2}, {2, 3}, {3, 4}};

    CHECK(r::equal(e, expected, [](const auto &a, const auto &b) {
        return std::tuple{a.index, a.value} == b;
    }));
}

TEST_CASE("Bidi ranges", "[Enumerate]") {
    auto v = std::vector{4, 3, 2, 1};
    auto expected =
        std::vector<std::tuple<ptrdiff_t, int>>{{3, 1}, {2, 2}, {1, 3}, {0, 4}};
    auto e = v | rangesnext::enumerate;

    static_assert(r::forward_range<decltype(e)>);

    CHECK(r::equal(r::reverse_view(v | rangesnext::enumerate), expected,
                   [](const auto &a, const auto &b) {
                       return std::tuple{a.index, a.value} == b;
                   }));
}

TEST_CASE("Test different range types", "[Enumerate]") {

    SECTION("array") {
        int const es[] = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
        test_enumerate_with(es);
    }
    SECTION("complex types") {
        std::vector<std::list<int>> range{
            {1, 2, 3}, {3, 5, 6, 7}, {10, 5, 6, 1}, {1, 2, 3, 4}};
        const auto rcopy = range;

        test_enumerate_with(range);
        CHECK(rcopy == range);

        // check with empty range
        range.clear();
        test_enumerate_with(range);
    }
    SECTION("initializer_list") {
        test_enumerate_with(
            std::initializer_list<int>{9, 8, 7, 6, 5, 4, 3, 2, 1});
    }
    SECTION("iota") {
        auto range = r::views::iota(0, 0);
        test_enumerate_with(range);

        range = r::views::iota(-10000, 10000);
        test_enumerate_with(range);
    }
    SECTION("max") {
        auto range = r::views::iota((std::uintmax_t)0, (std::uintmax_t)0);
        test_enumerate_with(range);

        auto range2 =
            r::views::iota((std::intmax_t)-10000, (std::intmax_t)10000);
        test_enumerate_with(range2);
    }
}
