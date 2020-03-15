# `ranges::to`

This repository implements `ranges::to` a [C++23 proposal](https://wg21.link/p1206r1)
which converts a ranges to a container

## Usage

**This project requires a conformant `<ranges>` implementation.**
Only GCC trunk is known to support this library.

Assuming the project is clone in a `ranges_to` directory

If your are using cmake:

```
add_subdirectory(ranges_to EXCLUDE_FROM_ALL)
target_link_libraries(your_target ranges_to)
```

Otherwise, just add `ranges_to/include` to
the include path.