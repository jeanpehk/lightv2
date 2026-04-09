#pragma once

#include <assert.h>
#include <math.h>
#include <stdio.h>

struct Vec3 {
    float x;
    float y;
    float z;

    Vec3() { zero(); }
    Vec3(float f) : x(f), y(f), z(f) {}
    Vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

    void dump() {
        printf("Vec3: %f %f %f\n", x, y, z);
    }

    float len() const {
        return sqrtf(len_sq());
    }

    float len_sq() const {
        return x*x + y*y + z*z;
    }

    Vec3 normalize() {
        *this /= len();

        return *this;
    }

    static Vec3 normalize(const Vec3 v) {
        return v / v.len();
    }

    static Vec3 cross(const Vec3 a, const Vec3 b) {
        return Vec3(
            a.y*b.z - a.z*b.y,
            a.z*b.x - a.x*b.z,
            a.x*b.y - a.y*b.x
        );
    }

    static float dist(const Vec3 a, const Vec3 b) {
        return (b-a).len();
    }

    static float dot(const Vec3 a, const Vec3 b) {
        return a.x*b.x + a.y*b.y + a.z*b.z;
    }

    static Vec3 negate(const Vec3 v) {
        return -1 * v;
    }

    static const Vec3 zero() {
        return Vec3(0.0f, 0.0f, 0.0f);
    }

    // Operator overloads

    friend Vec3 operator+(const Vec3 lhs, const Vec3 rhs) {
        return Vec3(lhs.x+rhs.x, lhs.y+rhs.y, lhs.z+rhs.z);
    }

   friend Vec3 operator-(const Vec3 lhs, const Vec3 rhs) {
        return Vec3(lhs.x-rhs.x, lhs.y-rhs.y, lhs.z-rhs.z);
    }

    friend Vec3 operator*(const Vec3 lhs, const Vec3 rhs) {
        return Vec3(lhs.x*rhs.x, lhs.y*rhs.y, lhs.z*rhs.z);
    }

    friend Vec3 operator*(const Vec3 v, float scalar) {
        return Vec3(v.x*scalar, v.y*scalar, v.z*scalar);
    }

    friend Vec3 operator*(float scalar, const Vec3 v) {
        return Vec3(v.x*scalar, v.y*scalar, v.z*scalar);
    }

    friend Vec3 operator/(const Vec3 v, float scalar) {
        assert(scalar != 0);
        return Vec3(v.x/scalar, v.y/scalar, v.z/scalar);
    }

    Vec3 operator+=(const Vec3 v) {
        x += v.x;
        y += v.y;
        z += v.z;

        return *this;
    }

    Vec3 operator-=(const Vec3 v) {
        x -= v.x;
        y -= v.y;
        z -= v.z;

        return *this;
    }

    Vec3 operator-=(float scalar) {
        x -= scalar;
        y -= scalar;
        z -= scalar;

        return *this;
    }

    Vec3 operator*=(const Vec3 v) {
        x *= v.x;
        y *= v.y;
        z *= v.z;

        return *this;
    }

    Vec3 operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;

        return *this;
    }

    Vec3 operator/=(float scalar) {
        x /= scalar;
        y /= scalar;
        z /= scalar;

        return *this;
    }
};