# Ranges For C++23

This repository implements a set of views
intended to be proposed to a future C++ standard.

The library can only be used with a conforming implementation
of C++20 - Only GCC is known to work.

If you are looking for a battle-tested implementation of
ranges for older C++ version, use [ranges-v3](https://github.com/ericniebler/range-v3).

### `ranges::to`

`ranges::to` is a [C++23 proposal](https://wg21.link/p1206r1)
to convert ranges to a container.

It supports input ranges, nested ranges and container, as well as conversion
between containers of pairs and associative containers.

```cpp
    auto vec = std::views::iota(0, 10) | rangesnext::to<std::vector>();
```

### `views::enumerate`

Enumerates provide a counter in addition to the value of the underlying
view.

```cpp
for(auto && [index, value] : rangesnext::enumerate({"Hello", "World"})) {
    fmt::print("(index : {}, value : {}) ", index, value);
    // (index: 0, value: "Hello") (index: 1, value: "World")
}
```

### `views::product`

Cartesian product of multiple views, equivalent to a nested for-loop

```cpp

std::vector a{'a', 'b', 'c', 'd'};
std::vector b{1, 2, 3};
std::vector c{0.1, 0.2};

for (auto &&[c, d, f] : rangesnext::product(a, b, c)) {
    fmt::print("({} {} {}), ", c, d,n );
    // prints (a, 1, 0.1), (a, 1, 0.2), (a, 2, 0.1), (a, 3, 0.1) ...
}

```

### `generator`

```cpp
rangesnext::generator<double> test() {
    double i = 0;
    while(i++ < 5) {
        co_yield i * 2;
    }
}

void  f() {
    std::ranges::for_each(test(), [](double i){
        printf("%f\t", i);
    });
}
```

**This feature requires the `-fcoroutines` flag under GCC, and might not work properly as the GCC support for coroutines is still experimental.**

## Usage

**This project requires a conformant `<ranges>` implementation.**
Only GCC trunk is known to support this library.

Assuming the project is cloned in a `rangesnext` directory

If your are using cmake:

```
add_subdirectory(rangesnext EXCLUDE_FROM_ALL)
target_link_libraries(your_target rangesnext)
```

Otherwise, just add `rangesnext/include` to
the include path.