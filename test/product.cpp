/*
Copyright (c) 2020 - present Corentin Jabot

Licenced under Boost Software License license. See LICENSE.md for details.
*/

#include <catch2/catch.hpp>
#include <cor3ntin/rangesnext/product.hpp>
#include <cor3ntin/rangesnext/to.hpp>

#include <iostream>
#include <sstream>
#include <vector>

using namespace cor3ntin::rangesnext;
namespace r = std::ranges;

TEST_CASE("Basic Product Tests", "product") {
    std::vector<std::tuple<char, int>> expected = {
        {'+', 1}, {'+', 2}, {'+', 3}, {'-', 1}, {'-', 2}, {'-', 3}};

    SECTION("Move only") {
        auto symbols = std::istringstream{"+ -"};
        auto sv = r::istream_view<char>(symbols);

        auto i = std::vector{1, 2, 3};

        auto v = product(sv, i);

        using R = decltype(v);
        static_assert(r::input_range<R>);
        static_assert(!r::forward_range<R>);

        static_assert(
            std::same_as<r::range_reference_t<R>, std::tuple<char &, int &>>);
        static_assert(std::same_as<r::range_value_t<R>, std::tuple<char, int>>);

        CHECK_THAT((v | to<std::vector<std::tuple<char, int>>>()),
                   Catch::Equals(expected));
    }

    SECTION("Simple vector") {
        auto symbols = std::vector{'+', '-'};
        auto i = std::vector{1, 2, 3};

        auto v = product(symbols, i);

        using R = decltype(v);
        static_assert(r::random_access_range<R>);
        static_assert(!r::contiguous_range<R>);

        static_assert(
            std::same_as<r::range_reference_t<R>, std::tuple<char &, int &>>);
        static_assert(std::same_as<r::range_value_t<R>, std::tuple<char, int>>);

        CHECK_THAT((v | to<std::vector<std::tuple<char, int>>>()),
                   Catch::Equals(expected));
    }

    SECTION("Bidi") {
        auto symbols = std::vector{'+', '-'};
        auto i = std::vector{1, 2, 3};
        auto d = std::vector<std::string_view>{"World", "Hello"};

        auto v = product(symbols, i, d) | r::views::reverse;

        using R = decltype(v);
        static_assert(r::random_access_range<R>);
        static_assert(!r::contiguous_range<R>);

        static_assert(
            std::same_as<r::range_reference_t<R>,
                         std::tuple<char &, int &, std::string_view &>>);
        static_assert(std::same_as<r::range_value_t<R>,
                                   std::tuple<char, int, std::string_view>>);

        std::vector<std::tuple<char, int, std::string_view>> expected = {
            {'-', 3, "Hello"}, {'-', 3, "World"}, {'-', 2, "Hello"},
            {'-', 2, "World"}, {'-', 1, "Hello"}, {'-', 1, "World"},

            {'+', 3, "Hello"}, {'+', 3, "World"}, {'+', 2, "Hello"},
            {'+', 2, "World"}, {'+', 1, "Hello"}, {'+', 1, "World"}};


        CHECK(r::distance(v.begin(), v.end()) == 2 * 3 * 2);
        CHECK_THAT((v | to<std::vector<std::tuple<char, int, std::string_view>>>()),
                   Catch::Equals(expected));
    }
}
