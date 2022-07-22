#pragma once

template <typename T>
struct Vector2
{
    T x;
    T y;

    constexpr Vector2() noexcept : x(T{}), y(T{}) {}
    constexpr Vector2(T x, T y) noexcept : x(x), y(y) {}
    constexpr Vector2(const Vector2<T> &other) noexcept : x(other.x), y(other.y) {}
    constexpr Vector2(const Vector2<T> &&other) noexcept : x(std::move(other.x)), y(std::move(other.y)) {}

    constexpr void operator=(const Vector2<T> &rhs) noexcept
    {
        this->x = rhs.x;
        this->y = rhs.y;
    }

    template <typename T>
    constexpr void operator=(const Vector2<T> &&rhs) noexcept
    {
        this->x = std::move(rhs.x);
        this->y = std::move(rhs.y);
    }
};

template <typename T>
constexpr Vector2<T> operator-(const Vector2<T> &rhs) noexcept
{
    return {-rhs.x, -rhs.y};
}

template <typename T>
constexpr Vector2<T> &operator+=(Vector2<T> &lhs, const Vector2<T> &rhs) noexcept
{
    lhs.x += rhs.x;
    lhs.y += rhs.y;

    return lhs;
}

template <typename T>
constexpr Vector2<T> &operator-=(Vector2<T> &lhs, const Vector2<T> &rhs) noexcept
{
    lhs.x -= rhs.x;
    lhs.y -= rhs.y;

    return lhs;
}

template <typename T>
constexpr Vector2<T> operator+(const Vector2<T> &lhs, const Vector2<T> &rhs) noexcept
{
    return {lhs.x + rhs.x, lhs.y + rhs.y};
}

template <typename T, typename C>
constexpr Vector2<T> operator+(const Vector2<T> &lhs, C rhs) noexcept
{
    return {lhs.x + rhs, lhs.y + rhs};
}

template <typename T>
constexpr Vector2<T> operator-(const Vector2<T> &lhs, const Vector2<T> &rhs) noexcept
{
    return {lhs.x - rhs.x, lhs.y - rhs.y};
}

template <typename T>
constexpr Vector2<T> operator*(const Vector2<T> &lhs, T rhs) noexcept
{
    return {lhs.x * rhs, lhs.y * rhs};
}

template <typename T, typename C>
constexpr Vector2<T> operator*(const Vector2<T> &lhs, C rhs) noexcept
{
    return {lhs.x * rhs, lhs.y * rhs};
}

template <typename T>
constexpr Vector2<T> operator*(T lhs, const Vector2<T> &rhs) noexcept
{
    return {rhs.x * lhs, rhs.y * lhs};
}

template <typename T>
constexpr Vector2<T> &operator*=(Vector2<T> &lhs, T rhs) noexcept
{
    lhs.x *= rhs;
    lhs.y *= rhs;

    return lhs;
}

template <typename T>
constexpr Vector2<T> operator/(const Vector2<T> &lhs, T rhs) noexcept
{
    return {lhs.x / rhs, lhs.y / rhs};
}

template <typename T>
constexpr Vector2<T> &operator/=(Vector2<T> &lhs, T rhs) noexcept
{
    lhs.x /= rhs;
    lhs.y /= rhs;

    return lhs;
}

template <typename T>
constexpr bool operator==(const Vector2<T> &lhs, const Vector2<T> &rhs) noexcept
{
    return (lhs.x == rhs.x) && (lhs.y == rhs.y);
}

template <typename T>
constexpr bool operator!=(const Vector2<T> &lhs, const Vector2<T> &rhs) noexcept
{
    return (lhs.x != rhs.x) || (lhs.y != rhs.y);
}
