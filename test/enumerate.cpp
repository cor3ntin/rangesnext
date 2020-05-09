/*
Copyright (c) 2020 Corentin Jabot

Licenced under modified MIT license. See LICENSE.md for details.
*/

#include <catch2/catch.hpp>
#include <cor3ntin/rangesnext/enumerate.hpp>
#include <iostream>
#include <sstream>
#include <tuple>

namespace r = std::ranges;

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
                       std::cout << a.index << a.value;
                       return std::tuple{a.index, a.value} == b;
                   }));
}
