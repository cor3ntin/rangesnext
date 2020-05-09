/*
Copyright (c) 2020 Corentin Jabot

Licenced under modified MIT license. See LICENSE.md for details.
*/

#include <catch2/catch.hpp>
#include <cor3ntin/rangesnext/product.hpp>

/*
 *
 * int main() {
    auto symbols = std::istringstream{"+ - *"};
    auto sv = std::ranges::istream_view<char>(symbols);

std::vector a{'a', 'b', 'c', 'd'};
std::vector b{1, 2, 3};
std::vector c{0.1, 0.2, 0.3, 0.4, 0.5};

{
  auto v = rangesnext::product(sv, a);

  static_assert(std::ranges::range<decltype(v)>);

  for (auto &&[s, n] : v) {
    printf("%c %d \n", s, n);
  }
}

{
  auto v = rangesnext::product_view(a, b, c);

  static_assert(std::ranges::range<decltype(v)>);

  for (auto &&[l, n, f] : std::ranges::reverse_view(v)) {
    printf("%c %d %.1f\n", l, n, f);
  }
}
}*/
