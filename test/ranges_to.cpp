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

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <algorithm>
#include <array>
#include <cor3ntin/rangesnext/to.hpp>
#include <forward_list>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <tuple>
#include <vector>

namespace r = std::ranges;

constexpr bool eq(const r::range auto &a, const r::range auto &b) {
    return std::equal(r::begin(a), r::end(a), r::begin(b), r::end(b));
}

TEMPLATE_PRODUCT_TEST_CASE("Conversion from container to vector", "",
                           (std::vector, std::list, std::deque,
                            std::forward_list, std::set, std::multiset),
                           (int, double, char)) {
    using type = typename TestType::value_type;

    TestType simple_container{0, 1, 2, 3, 4};
    STATIC_REQUIRE(std::same_as<decltype(rangesnext::to<std::vector<type>>(
                                    simple_container)),
                                std::vector<type>>);
    STATIC_REQUIRE(std::same_as<decltype(simple_container |
                                         rangesnext::to<std::vector<type>>()),
                                std::vector<type>>);
    STATIC_REQUIRE(std::same_as<decltype(simple_container |
                                         rangesnext::to<std::vector<type>>),
                                std::vector<type>>);
    STATIC_REQUIRE(
        std::same_as<decltype(simple_container | rangesnext::to<std::vector>),
                     std::vector<type>>);
    CHECK_THAT((simple_container | rangesnext::to<std::vector<type>>()),
               Catch::Matchers::Equals(std::vector<type>{0, 1, 2, 3, 4}));
    CHECK_THAT((simple_container | rangesnext::to<std::vector<type>>),
               Catch::Matchers::Equals(std::vector<type>{0, 1, 2, 3, 4}));
    CHECK_THAT((simple_container | rangesnext::to<std::vector>),
               Catch::Matchers::Equals(std::vector<type>{0, 1, 2, 3, 4}));
}

TEMPLATE_PRODUCT_TEST_CASE("Conversion to adaptor", "",
                           (std::queue, std::stack, std::priority_queue),
                           (int)) {
    using type = typename TestType::value_type;
    std::vector<type> vec{0, 1, 2, 3, 4};
    STATIC_REQUIRE(
        std::same_as<decltype(rangesnext::to<TestType>(vec)), TestType>);
}

TEMPLATE_TEST_CASE("Conversion from map to vector", "", (std::map<int, int>),
                   (std::multimap<int, int>), (std::unordered_map<int, int>),
                   (std::unordered_multimap<int, int>)) {
    TestType simple_container{
        {1, 1},
        {2, 2},
        {3, 3},
    };
    using ex = std::vector<std::pair<int, int>>;
    using ex_const = std::vector<std::pair<const int, int>>;

    STATIC_REQUIRE(
        std::same_as<decltype(rangesnext::to<ex>(simple_container)), ex>);
    STATIC_REQUIRE(
        std::same_as<decltype(simple_container | rangesnext::to<ex>()), ex>);
    STATIC_REQUIRE(
        std::same_as<decltype(simple_container | rangesnext::to<ex>), ex>);
    STATIC_REQUIRE(
        std::same_as<decltype(simple_container | rangesnext::to<std::vector>),
                     ex_const>);
    CHECK(eq(simple_container, simple_container | rangesnext::to<ex_const>));
}

TEMPLATE_TEST_CASE("Conversion from vector to associative containers", "",
                   (std::map<int, int>), (std::multimap<int, int>),
                   (std::unordered_map<int, int>),
                   (std::unordered_multimap<int, int>)) {
    std::vector<std::pair<int, int>> vec{
        {1, 1},
        {2, 2},
        {3, 3},
    };

    STATIC_REQUIRE(
        std::same_as<decltype(rangesnext::to<TestType>(vec)), TestType>);
    STATIC_REQUIRE(
        std::same_as<decltype(vec | rangesnext::to<TestType>()), TestType>);
    STATIC_REQUIRE(
        std::same_as<decltype(vec | rangesnext::to<TestType>), TestType>);
    auto res = vec | rangesnext::to<TestType> |
               rangesnext::to<std::vector<std::pair<int, int>>>;
    CHECK_THAT(res, Catch::Matchers::UnorderedEquals(vec));
}

TEST_CASE(
    "Conversion from vector to associative containers with unspecified type") {
    std::vector<std::pair<int, int>> vec{
        {1, 1},
        {2, 2},
        {3, 3},
    };
    using ex = std::map<int, int>;

    STATIC_REQUIRE(std::same_as<decltype(rangesnext::to<std::map>(vec)), ex>);
    CHECK(eq(vec, vec | rangesnext::to<std::map> |
                      rangesnext::to<std::vector<std::pair<int, int>>>));
}

TEST_CASE("Conversion from vector to associative containers with unspecified "
          "type and allocator") {
    std::vector<std::pair<int, int>> vec{
        {1, 1},
        {2, 2},
        {3, 3},
    };
    using alloc = std::allocator<std::pair<const int, int>>;
    using ex = std::map<int, int, std::less<int>, alloc>;

    STATIC_REQUIRE(std::same_as<decltype(rangesnext::to<std::map>(
                                    vec, std::less<int>{}, alloc())),
                                ex>);
    CHECK(eq(vec, rangesnext::to<std::map>(vec, std::less<int>{}, alloc()) |
                      rangesnext::to<std::vector<std::pair<int, int>>>));
}

TEST_CASE("Non-sized ranges") {
    auto ints = std::vector{1, 2, 3, 4, 5, 6};
    auto view = ints | std::views::filter([](int x) { return (x % 2) != 0; });
    std::vector<int> vec = view | rangesnext::to<std::vector>;
    CHECK_THAT(vec, Catch::Matchers::Equals(std::vector{1, 3, 5}));
}

TEST_CASE("Move only iterators") {
    auto ints = std::istringstream{"0 1  2   3     4"};
    std::vector<int> vec =
        r::istream_view<int>(ints) | rangesnext::to<std::vector>;
    CHECK_THAT(vec, Catch::Matchers::Equals(std::vector{0, 1, 2, 3, 4}));
}

TEST_CASE("Nested views") {
    std::list<std::list<int>> lst = {{0, 1, 2, 3}, {4, 5, 6, 7}};
    auto vec1 = rangesnext::to<std::vector<std::vector<int>>>(lst);
    auto vec2 = rangesnext::to<std::vector<std::vector<double>>>(lst);
}