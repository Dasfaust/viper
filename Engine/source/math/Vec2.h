#pragma once
#include "Math.h"
#include <string>

namespace Math
{
    template<typename T>
    struct Vec2
    {
        T x, y;

        Vec2(T x, T y)
        {
            this->x = x;
            this->y = y;
        }

        inline float len()
        {
            return sqrt(x * x + y * y);
        }

        inline float dot(Vec2<T> v)
        {
            return x * v.gx() + y * v.gx();
        }

        inline Vec2<T>& norm()
        {
            float l = len();
            x /= l;
            y /= l;

            return *this;
        }

        inline Vec2<T>& rot(float angle)
        {
            float r = rad(angle);
            float c = cos(r);
            float s = sin(r);

            x = x * c - y * s;
            y = x * s + y * c;

            return *this;
        }

        inline T gx()
        {
            return x;
        }

        inline T gy()
        {
            return y;
        }

        inline void sx(T x)
        {
            this->x = x;
        }

        inline void sy(T y)
        {

            this->y = y;
        }

        inline Vec2<T>& a(Vec2<T> v)
        {
            x += v.gx();
            y += v.gy();

            return *this;
        }

        inline Vec2<T>& a(T v)
        {
            x += v;
            y += v;

            return *this;
        }

        inline Vec2<T>& s(Vec2<T> v)
        {
            x -= v.gx();
            y -= v.gy();

            return *this;
        }

        inline Vec2<T>& s(T v)
        {
            x -= v;
            y -= v;

            return *this;
        }

        inline Vec2<T>& m(Vec2<T> v)
        {
            x *= v.gx();
            y *= v.gy();

            return *this;
        }

        inline Vec2<T>& m(T v)
        {
            x *= v;
            y *= v;

            return *this;
        }

        inline Vec2<T>& d(Vec2<T> v)
        {
            x /= v.gx();
            y /= v.gy();

            return *this;
        }

        inline Vec2<T>& d(T v)
        {
            x /= v;
            y /= v;

            return *this;
        }
    };
};