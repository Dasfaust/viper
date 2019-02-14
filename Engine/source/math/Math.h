#pragma once
#define rad(f) Math::radians(f)

#define v2f_t Math::Vec2<float>
#define v2f(x, y) Math::Vec2<float>(x, y)

#define v3f_t Math::Vec3<float>
#define v3f(x, y, z) Math::Vec3<float>(x, y, z)

#define v4f_t Math::Vec4<float>
#define v4f(x, y, z, w) Math::Vec4<float>(x, y, z, w)

#define m4f_t Math::Mat4f
#define m4f() Math::Mat4f()
#define m4fi(v) Math::Mat4(v)

namespace Math
{
    inline static float radians(float f)
    {
        return f * M_PI / 180.0f;
    };
}