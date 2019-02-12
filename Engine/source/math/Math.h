#pragma once
#define rad(f) Math::radians(f)

#define v2f_t Math::Vec2<float>
#define v2f(x, y) Math::Vec2<float>(x, y)

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