
#pragma once

// Cross product of v1 and v2
template <class vecT> vecT cross_product(const vecT &v1, const vecT &v2);

// Dot product of v1 and v2
template <class vecT> auto dot_product(const vecT &v1, const vecT &v2);

// Calculates the squared magnitude of v
template <class vecT> auto dot_product(const vecT &v);

// Magnitude of v
template <class vecT> auto magnitude(const vecT &v);

// Normalize v
template <class vecT> vecT normalize(const vecT &v);

#include "math_functions_impl.hh"
