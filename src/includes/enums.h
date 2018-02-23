#pragma once

enum class Files
{
    Screen = 1,
    Log = 2
};

enum class Buttons
{
    left,
    right,
    middle,
    wheel,
    count
};

template <typename T>
constexpr auto to_underlying(T val) noexcept
{
    return static_cast<std::underlying_type_t<T>>(val);
}
