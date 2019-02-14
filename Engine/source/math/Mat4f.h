#pragma once
#include <vector>
#include "Math.h"
#include "Vec3.h"

namespace Math
{
    struct Mat4f
    {
        std::vector<std::vector<float>> mat;

        Mat4f(float v = 1.0f)
        {
            mat.resize(4);
            mat[0] = { v, 0.0f, 0.0f, 0.0f };
            mat[1] = { 0.0f, v, 0.0f, 0.0f };
            mat[2] = { 0.0f, 0.0f, v, 0.0f };
            mat[3] = { 0.0f, 0.0f, 0.0f, v };
        };

        inline std::vector<std::vector<float>> gm()
        {
            return mat;
        };

        inline void sm(std::vector<std::vector<float>> m)
        {
            this->mat = m;
        };

        inline float g(int x, int y)
        {
            return mat[x][y];
        }

        inline void s(int x, int y, float v)
        {
            mat[x][y] = v;
        }

        inline Mat4f& m(Mat4f mat)
        {
            for (int i = 0; i < 4; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    this->mat[i][j] = this->mat[i][0] * mat.gm()[0][j]
                                    + this->mat[i][1] * mat.gm()[1][j]
                                    + this->mat[i][2] * mat.gm()[2][j]
                                    + this->mat[i][3] * mat.gm()[3][j];
                }
            }

            return *this;
        }

        inline Mat4f& p(float f, float w, float h, float zn, float zf)
        {
            Mat4f p;

            float fh = tan(rad(f / 2.0f));
            float ar = w / h;
            float zr = zn - zf;

            p.mat[0] = { 1.0f / (fh * ar),                  0.0f, 0.0f,          0.0f };
            p.mat[1] = {             0.0f,             1.0f / fh, 0.0f,          0.0f };
            p.mat[2] = {             0.0f, 0.0f, (-zn - zf) / zr, 2.0f * zf * zn / zr };
            p.mat[3] = {             0.0f, 0.0f,            1.0f,                0.0f };

            return m(p);
        }

        inline Mat4f& tf(Vec3<float> v, Vec3<float> r = Vec3<float>(0.0f, 0.0f, 0.0f), Vec3<float> s = Vec3<float>(1.0f, 1.0f, 1.0f))
        {
            return tf(
                v.gx(), v.gy(), v.gz(),
                r.gx(), r.gy(), r.gz(),
                s.gx(), s.gy(), s.gz()
            );
        };

        inline Mat4f& tf(float x, float y, float z, float xr, float yr, float zr, float xs, float ys, float zs)
        {
            mat[0] = { 1.0f, 0.0f, 0.0f,    x };
            mat[1] = { 0.0f, 1.0f, 0.0f,    y };
            mat[2] = { 0.0f, 0.0f, 1.0f,    z };
            mat[3] = { 0.0f, 0.0f, 0.0f, 1.0f };

            return m(mrt(xr, yr, zr).m(msc(xs, ys, zs)));
        };

        inline static Mat4f mrt(Vec3<float> v)
        {
            return mrt(v.gx(), v.gy(), v.gz());
        };


        inline static Mat4f mrt(float x, float y, float z)
        {
            Mat4f rx;
            Mat4f ry;
            Mat4f rz;

            x = rad(x); y = rad(y); z = rad(z);

            // rotation around yz plane
            rx.mat[0] = { 1.0f,   0.0f,    0.0f, 0.0f };
            rx.mat[1] = { 0.0f, cos(x), -sin(x), 0.0f };
            rx.mat[2] = { 0.0f, sin(x),  cos(x), 0.0f };
            rx.mat[3] = { 0.0f,   0.0f,    0.0f, 1.0f };

            // rotation around xz plane
            ry.mat[0] = { cos(y), 0.0f, -sin(y), 0.0f };
            ry.mat[1] = {   0.0f, 1.0f,    0.0f, 0.0f };
            ry.mat[2] = { sin(y), 0.0f,  cos(y), 0.0f };
            ry.mat[3] = {   0.0f, 0.0f,    0.0f, 1.0f };

            // rotation around xy plane
            rz.mat[0] = { cos(z), -sin(z), 0.0f, 0.0f };
            rz.mat[1] = { sin(z),  cos(z), 0.0f, 0.0f };
            rz.mat[2] = {   0.0f,    0.0f, 1.0f, 0.0f };
            rz.mat[3] = {   0.0f,    0.0f, 0.0f, 1.0f };

            return rz.m(ry.m(rx));
        };

        inline static Mat4f msc(Vec3<float> v)
        {
            return msc(v.gx(), v.gy(), v.gz());
        };

        inline static Mat4f msc(float x, float y, float z)
        {
            Mat4f s;
            s.mat[0] = {   x,  0.0f, 0.0f, 0.0f };
            s.mat[1] = { 0.0f,   y,  0.0f, 0.0f };
            s.mat[2] = { 0.0f, 0.0f,   z,  0.0f };
            s.mat[3] = { 0.0f, 0.0f, 0.0f, 1.0f };

            return s;
        };
    };
}