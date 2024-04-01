#pragma once

#include <utility>

template < typename T>
T interpolate( const T& T1, const T& T2, float flProgress )
{
	return T1 + ( ( T2 - T1 ) * flProgress );
}

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

    template<typename T> 
    constexpr float dist_to(const Vector2<T> &rhs) noexcept 
    {
        Vector2<T> delta;
        delta.x = this->x - rhs.x;
        delta.y = this->y - rhs.y;

        return delta.length();
    }

    constexpr float length() noexcept 
    {
        return sqrt(this->x * this->x + this->y * this->y);
    }

    template <typename T>
    constexpr Vector2<T> interpolate_to(const Vector2<T> &transform, float time)
    {
        this->x = interpolate(this->x, transform.x, time);
        this->y = interpolate(this->y, transform.y, time);

        return *this;
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
