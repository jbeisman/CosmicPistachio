
#pragma once

// simple roll-my-own Vec3 class
// stores x, y, z + padding
// defines arithmetic (+,-,*,/) operators for
// Vec3 op Vec3, Vec3 op scalar, and scalar op Vec3

template <typename T> class Vec3 {
public:
  using value_type = T;

  T x;
  T y;
  T z;
//  T pad{0.0};

  // constructors
  Vec3();
  Vec3(const T x_in, const T y_in, const T z_in);

  // operator overloads
  Vec3<T> &operator+=(const Vec3<T> &rhs);
  Vec3<T> &operator+=(const T scalar);
  Vec3<T> &operator*=(const Vec3<T> &rhs);
  Vec3<T> &operator*=(const T scalar);
  Vec3<T> &operator-=(const Vec3<T> &rhs);
  Vec3<T> &operator-=(const T scalar);
  Vec3<T> &operator/=(const Vec3<T> &rhs);
  Vec3<T> &operator/=(const T scalar);
};

// non-member Vec3 arithmetic operator overloads
template <typename T, typename U> Vec3<T> operator+(Vec3<T> lhs, const U &rhs);
template <typename T> Vec3<T> operator+(const T &lhs, const Vec3<T> &rhs);
template <typename T, typename U> Vec3<T> operator*(Vec3<T> lhs, const U &rhs);
template <typename T> Vec3<T> operator*(const T &lhs, const Vec3<T> &rhs);
template <typename T, typename U> Vec3<T> operator-(Vec3<T> lhs, const U &rhs);
template <typename T> Vec3<T> operator-(const T &lhs, Vec3<T> rhs);
template <typename T, typename U> Vec3<T> operator/(Vec3<T> lhs, const U &rhs);
template <typename T> Vec3<T> operator/(const T &lhs, Vec3<T> rhs);

#include "vec_impl.hh"
