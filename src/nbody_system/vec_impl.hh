
#pragma once

#include "vec.hh"

template <typename T> Vec3<T>::Vec3() : x{0.0}, y{0.0}, z{0.0} {}
template <typename T>
Vec3<T>::Vec3(const T x_in, const T y_in, const T z_in)
    : x{x_in}, y{y_in}, z{z_in} {}

template <typename T> Vec3<T> &Vec3<T>::operator+=(const Vec3<T> &rhs) {
  this->x += rhs.x;
  this->y += rhs.y;
  this->z += rhs.z;
  return *this;
}

template <typename T> Vec3<T> &Vec3<T>::operator+=(const T scalar) {
  this->x += scalar;
  this->y += scalar;
  this->z += scalar;
  return *this;
}

template <typename T> Vec3<T> &Vec3<T>::operator*=(const Vec3<T> &rhs) {
  this->x *= rhs.x;
  this->y *= rhs.y;
  this->z *= rhs.z;
  return *this;
}

template <typename T> Vec3<T> &Vec3<T>::operator*=(const T scalar) {
  this->x *= scalar;
  this->y *= scalar;
  this->z *= scalar;
  return *this;
}

template <typename T> Vec3<T> &Vec3<T>::operator-=(const Vec3<T> &rhs) {
  this->x -= rhs.x;
  this->y -= rhs.y;
  this->z -= rhs.z;
  return *this;
}

template <typename T> Vec3<T> &Vec3<T>::operator-=(const T scalar) {
  this->x -= scalar;
  this->y -= scalar;
  this->z -= scalar;
  return *this;
}

template <typename T> Vec3<T> &Vec3<T>::operator/=(const Vec3<T> &rhs) {
  this->x /= rhs.x;
  this->y /= rhs.y;
  this->z /= rhs.z;
  return *this;
}

template <typename T> Vec3<T> &Vec3<T>::operator/=(const T scalar) {
  this->x /= scalar;
  this->y /= scalar;
  this->z /= scalar;
  return *this;
}

template <typename T, typename U> Vec3<T> operator+(Vec3<T> lhs, const U &rhs) {
  lhs += rhs;
  return lhs;
}

template <typename T> Vec3<T> operator+(const T &lhs, const Vec3<T> &rhs) {
  return rhs + lhs;
}

template <typename T, typename U> Vec3<T> operator*(Vec3<T> lhs, const U &rhs) {
  lhs *= rhs;
  return lhs;
}

template <typename T> Vec3<T> operator*(const T &lhs, const Vec3<T> &rhs) {
  return rhs * lhs;
}

template <typename T, typename U> Vec3<T> operator-(Vec3<T> lhs, const U &rhs) {
  lhs -= rhs;
  return lhs;
}

// modifies and returns copy of rhs
template <typename T> Vec3<T> operator-(const T &lhs, Vec3<T> rhs) {
  rhs.x = lhs - rhs.x;
  rhs.y = lhs - rhs.y;
  rhs.z = lhs - rhs.z;
  return rhs;
}

template <typename T, typename U> Vec3<T> operator/(Vec3<T> lhs, const U &rhs) {
  lhs /= rhs;
  return lhs;
}

// modifies and returns copy of rhs
template <typename T> Vec3<T> operator/(const T &lhs, Vec3<T> rhs) {
  rhs.x = lhs / rhs.x;
  rhs.y = lhs / rhs.y;
  rhs.z = lhs / rhs.z;
  return rhs;
}
