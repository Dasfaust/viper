#pragma once
#include <string>

namespace Math
{
    template<typename T>
    struct Vec4
    {
        T x, y, z, w;

        Vec4()
        {
            this->x = 0.0f;
            this->y = 0.0f;
            this->z = 0.0f;
            this->w = 0.0f;
        }

        Vec4(T x, T y, T z, T w)
        {
            this->x = x;
            this->y = y;
            this->z = z;
            this->w = w;
        }

        /*inline Vec4<T>& norm()
        {
            float l = len();
            x /= l;
            y /= l;
            z /= l;
            w /= l;

            return *this;
        }*/

        inline T gx()
        {
            return x;
        }

        inline T gy()
        {
            return y;
        }

        inline T gz()
        {
            return z;
        }

        inline T gw()
        {
            return w;
        }

        inline void sx(T x)
        {
            this->x = x;
        }

        inline void sy(T y)
        {

            this->y = y;
        }

        inline void sz(T z)
        {

            this->z = z;
        }

        inline void sw(T w)
        {

            this->w = w;
        }

        inline Vec4<T>& a(Vec4<T> v)
        {
            x += v.gx();
            y += v.gy();
            z += v.gz();
            w += v.gw();

            return *this;
        }

        inline Vec4<T>& a(T v)
        {
            x += v;
            y += v;
            z += v;
            w += v;

            return *this;
        }

        inline Vec4<T>& s(Vec4<T> v)
        {
            x -= v.gx();
            y -= v.gy();
            z -= v.gz();
            w -= v.gw();

            return *this;
        }

        inline Vec4<T>& s(T v)
        {
            x -= v;
            y -= v;
            z -= z;
            w -= w;

            return *this;
        }

        inline Vec4<T>& m(Vec4<T> v)
        {
            x *= v.gx();
            y *= v.gy();
            z *= v.gz();
            w *= v.gw();

            return *this;
        }

        inline Vec4<T>& m(T v)
        {
            x *= v;
            y *= v;
            z *= v;
            w *= v;

            return *this;
        }

        inline Vec4<T>& d(Vec4<T> v)
        {
            x /= v.gx();
            y /= v.gy();
            z /= v.gz();
            w /= v.gw();

            return *this;
        }

        inline Vec4<T>& d(T v)
        {
            x /= v;
            y /= v;
            z /= v;
            w /= v;

            return *this;
        }
    };
};