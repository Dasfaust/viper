#pragma once
#include <string>

namespace Math
{
    template<typename T>
    struct Vec3
    {
        T x, y, z;

        Vec3()
        {
            this->x = 0.0f;
            this->y = 0.0f;
            this->z = 0.0f;
        }

        Vec3(T x, T y, T z)
        {
            this->x = x;
            this->y = y;
            this->z = z;
        }

        inline float len()
        {
            return sqrt(x * x + y * y + z * z);
        }

        inline float dot(Vec3<T> v)
        {
            return x * v.gx() + y * v.gx() + z * v.gz();
        }

        inline Vec3<T>& norm()
        {
            float l = len();
            x /= l;
            y /= l;
            z /= l;

            return *this;
        }

        inline Vec3<T>& rot(float angle)
        {

            return *this;
        }

        inline Vec3<T>& cross(Vec3<T> v)
        {
            x = y * v.gz() - z * v.gy();
            y = z - v.gx() - x * v.gz();
            z = x * v.gy() - y * v.gx();

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

        inline T gz()
        {
            return z;
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

        inline Vec3<T>& a(Vec3<T> v)
        {
            x += v.gx();
            y += v.gy();
            z += v.gz();

            return *this;
        }

        inline Vec3<T>& a(T v)
        {
            x += v;
            y += v;
            z += v;

            return *this;
        }

        inline Vec3<T>& s(Vec3<T> v)
        {
            x -= v.gx();
            y -= v.gy();
            z -= v.gz();

            return *this;
        }

        inline Vec3<T>& s(T v)
        {
            x -= v;
            y -= v;
            z -= z;

            return *this;
        }

        inline Vec3<T>& m(Vec3<T> v)
        {
            x *= v.gx();
            y *= v.gy();
            z *= v.gz();

            return *this;
        }

        inline Vec3<T>& m(T v)
        {
            x *= v;
            y *= v;
            z *= v;

            return *this;
        }

        inline Vec3<T>& d(Vec3<T> v)
        {
            x /= v.gx();
            y /= v.gy();
            z /= v.gz();

            return *this;
        }

        inline Vec3<T>& d(T v)
        {
            x /= v;
            y /= v;
            z /= v;

            return *this;
        }
    };
};