/*ckwg +29
 * Copyright 2013-2015 by Kitware, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither name of Kitware, Inc. nor the names of any contributors may be used
 *    to endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file
 * \brief Header for \link kwiver::vector_ vector_<N,T> \endlink class
 *        as well as \link kwiver::vector_2_ vector_2_<T> \endlink,
 *        \link kwiver::vector_3_ vector_3_<T> \endlink, and
 *        \link kwiver::vector_4_ vector_4_<T> \endlink classes
 */

#ifndef KWIVER_VECTOR_H_
#define KWIVER_VECTOR_H_

#include "core-config.h"

#include <cstring>
#include <iostream>
#include <cassert>
#include <boost/static_assert.hpp>

#include "vector_cmath.h"


namespace kwiver
{

/// A representation of a vector
template <unsigned N, typename T>
class vector_
{
public:
  /// typedef for vector cmath
  typedef vector_cmath_<N,T> cmath;

  /// Constructor - does not initialize
  vector_<N,T>() {}

  /// Constructor from an array of data - no bounds checking
  vector_<N,T>(const T* data)
  {
    memcpy( data_, data, sizeof(data_) );
  }

  /// Copy Constructor
  vector_<N,T>(const vector_<N,T>& other)
  {
    memcpy( data_, other.data_, sizeof(data_) );
  }

  /// Copy Constructor from another type
  template <typename U>
  explicit vector_<N,T>(const vector_<N,U>& other)
  {
    const U* in = other.data();
    T* out = this->data_;
    for(unsigned i=0; i<N; ++i, ++in, ++out)
    {
      *out = static_cast<T>(*in);
    }
  }

  /// Assignment operator
  vector_<N,T>& operator=(const vector_<N,T>& other)
  {
    memcpy( data_, other.data_, sizeof(data_) );
    return *this;
  }

  /// Return the i-th element
  T& operator[](unsigned int i) { return data_[i]; }

  /// Return the i-th element (const)
  const T& operator[](unsigned int i) const { return data_[i]; }

  /// Return a pointer to the contiguous block of memory
  T* data() { return data_; }

  /// Return a pointer to the contiguous block of memory
  T const* data() const { return data_; }

  /// Extract a lower dimensional sub-vector
  template <unsigned M>
  void extract(vector_<M,T>& v, unsigned offset=0) const
  {
    BOOST_STATIC_ASSERT(M<=N);
    assert(offset + M <= N);
    v = vector_<M,T>(data_ + offset);
  }

  /// Update values from a lower dimensional sub-vector
  template <unsigned M>
  vector_<N,T>& update(const vector_<M,T>& v, unsigned offset=0)
  {
    BOOST_STATIC_ASSERT(M<=N);
    assert(offset + M <= N);
    memcpy( data_ + offset, v.data(), sizeof(T)*M );
    return *this;
  }


  /// Add a scalar in place
  vector_<N,T>& operator+=( T s ) { cmath::add( data_, s, data_ ); return *this; }

  /// Subract a scalr in place
  vector_<N,T>& operator-=( T s ) { cmath::sub( data_, s, data_ ); return *this; }

  /// Multiply a scalar in place
  vector_<N,T>& operator*=( T s ) { cmath::mul( data_, s, data_ ); return *this; }

  /// Divide by a scalar in place
  vector_<N,T>& operator/=( T s ) { cmath::div( data_, s, data_ ); return *this; }

  /// Add a vector in place
  vector_<N,T>& operator+=( const vector_<N,T>& v ) { cmath::add( data_, v.data_, data_ ); return *this; }

  /// Subract a vector in place
  vector_<N,T>& operator-=( const vector_<N,T>& v ) { cmath::sub( data_, v.data_, data_ ); return *this; }

  /// Negate operator
  vector_<N,T> operator-() const
  {
    vector_<N,T> result;
    cmath::sub( T(0), data_, result.data_ );
    return result;
  }

  /// The squared magnitude (L2 norm) of the vector
  T magnitude_sqr() const { return cmath::l2_norm_squared( data_ ); }

  /// The magnitude (L2 norm) of the vector
  T magnitude() const { return cmath::l2_norm( data_ ); }

protected:
  /// data of the vector
  T data_[N];
};


/// Compute the 2D cross product
/**
 * \relatesalso vector_
 * \param v1 a vector
 * \param v2 another vector
 */
template <typename T>
inline T
cross_product(const vector_<2,T>& v1, const vector_<2,T>& v2)
{
  return v1[0] * v2[1] - v1[1] * v2[0];
}


/// Compute the 3D cross product
/**
 * \relatesalso vector_
 * \param v1 a vector
 * \param v2 another vector
 */
template <typename T>
inline vector_<3,T>
cross_product(const vector_<3,T>& v1, const vector_<3,T>& v2)
{
  vector_<3,T> result;
  result[0] = v1[1] * v2[2] - v1[2] * v2[1];
  result[1] = v1[2] * v2[0] - v1[0] * v2[2];
  result[2] = v1[0] * v2[1] - v1[1] * v2[0];
  return result;
}


/// Compute the inner product
template <unsigned N, typename T>
inline T
inner_product(const vector_<N,T>& v1, const vector_<N,T>& v2)
{
  const T* a = v1.data();
  const T* b = v2.data();
  T sum = T(0);
  for ( unsigned int i=0; i < N; ++i,++a,++b )
  {
    sum += *a * *b;
  }
  return sum;
}


/// Compute a normalized version of this vector
template <unsigned N, typename T>
inline vector_<N,T>
normalized(const vector_<N,T>& v)
{
  return v / v.magnitude();
}


/// A representation of a 2D vector.
/**
 * This derived class exists to add convenience
 * constructors and accessors
 */
template <typename T>
class vector_2_ : public vector_<2,T>
{
public:
  /// Default Constructor
  vector_2_<T> () {}

  /// Copy Constructor
  vector_2_<T> (const vector_2_<T>& other)
  : vector_<2,T>(other) {}

  /// Constructor from base class
  vector_2_<T> (const vector_<2,T>& base)
  : vector_<2,T>(base) {}

  /// Constructor from another data type
  template <typename U>
  explicit vector_2_<T> (const vector_2_<U>& other)
  : vector_<2,T>(other) {}

  /// Constructor for a 2D vector
  vector_2_<T>(const T& x, const T& y)
  {
    this->data_[0] = x;
    this->data_[1] = y;
  }

  /// Accessor for the X coordinate
  T& x() { return this->data_[0]; }
  /// Accessor for the X coordinate (const)
  const T& x() const { return this->data_[0]; }
  /// Accessor for the Y coordinate
  T& y() { return this->data_[1]; }
  /// Accessor for the Y coordinate (const)
  const T& y() const { return this->data_[1]; }
};


/// A representation of a 3D vector.
/**
 * This derived class exists to add convenience
 * constructors and accessors
 */
template <typename T>
class vector_3_ : public vector_<3,T>
{
public:
  /// Default Constructor
  vector_3_<T> () {}

  /// Copy Constructor
  vector_3_<T> (const vector_3_<T>& other)
  : vector_<3,T>(other) {}

  /// Constructor from base class
  vector_3_<T> (const vector_<3,T>& base)
  : vector_<3,T>(base) {}

  /// Constructor from another data type
  template <typename U>
  explicit vector_3_<T> (const vector_3_<U>& other)
  : vector_<3,T>(other) {}

  /// Constructor for a 3D vector
  vector_3_<T>(const T& x, const T& y, const T& z)
  {
    this->data_[0] = x;
    this->data_[1] = y;
    this->data_[2] = z;
  }

  /// Accessor for the X coordinate
  T& x() { return this->data_[0]; }
  /// Accessor for the X coordinate (const)
  const T& x() const { return this->data_[0]; }
  /// Accessor for the Y coordinate
  T& y() { return this->data_[1]; }
  /// Accessor for the Y coordinate (const)
  const T& y() const { return this->data_[1]; }
  /// Accessor for the Z coordinate
  T& z() { return this->data_[2]; }
  /// Accessor for the Z coordinate (const)
  const T& z() const { return this->data_[2]; }
};


/// A representation of a 4D vector.
/**
 * This derived class exists to add convenience
 * constructors and accessors
 */
template <typename T>
class vector_4_ : public vector_<4,T>
{
public:
  /// Default Constructor
  vector_4_<T> () {}

  /// Copy Constructor
  vector_4_<T> (const vector_4_<T>& other)
  : vector_<4,T>(other) {}

  /// Constructor from base class
  vector_4_<T> (const vector_<4,T>& base)
  : vector_<4,T>(base) {}

  /// Constructor from another data type
  template <typename U>
  explicit vector_4_<T> (const vector_4_<U>& other)
  : vector_<4,T>(other) {}

  /// Constructor for a 4D vector
  vector_4_<T>(const T& x, const T& y, const T& z, const T& w)
  {
    this->data_[0] = x;
    this->data_[1] = y;
    this->data_[2] = z;
    this->data_[3] = w;
  }

  /// Accessor for the X coordinate
  T& x() { return this->data_[0]; }
  /// Accessor for the X coordinate (const)
  const T& x() const { return this->data_[0]; }
  /// Accessor for the Y coordinate
  T& y() { return this->data_[1]; }
  /// Accessor for the Y coordinate (const)
  const T& y() const { return this->data_[1]; }
  /// Accessor for the Z coordinate
  T& z() { return this->data_[2]; }
  /// Accessor for the Z coordinate (const)
  const T& z() const { return this->data_[2]; }
  /// Accessor for the W coordinate
  T& w() { return this->data_[3]; }
  /// Accessor for the W coordinate (const)
  const T& w() const { return this->data_[3]; }
};

/// \cond DoxygenSuppress
typedef vector_2_<double> vector_2d;
typedef vector_2_<float> vector_2f;
typedef vector_3_<double> vector_3d;
typedef vector_3_<float> vector_3f;
typedef vector_4_<double> vector_4d;
typedef vector_4_<float> vector_4f;
/// \endcond


// --- Vector-scalar operators ----------------------------------------

/// Vector-scalar addtion operator
/**
 * \relatesalso vector_
 * \param v a vector
 * \param s a scalar to add
 */
template <unsigned N, typename T>
inline vector_<N,T> operator+( const vector_<N,T>& v, const T& s )
{
  vector_<N,T> r;
  vector_cmath_<N,T>::add( v.data(), s, r.data() );
  return r;
}

/// Scalar-vector addition operator
/**
 * \relatesalso vector_
 * \param s a scalar
 * \param v vector to be element-wise multiplied by scalar
 */
template <unsigned N, typename T>
inline vector_<N,T> operator+( const T& s, const vector_<N,T>& v )
{
  vector_<N,T> r;
  vector_cmath_<N,T>::add( v.data(), s, r.data() );
  return r;
}

/// Vector-scalar subraction operator
/**
 * \relatesalso vector_
 * \param s a scalar
 * \param v vector to be element-wise subtracted by scalar
 */
template <unsigned N, typename T>
inline vector_<N,T> operator-( const vector_<N,T>& v, const T& s )
{
  vector_<N,T> r;
  vector_cmath_<N,T>::sub( v.data(), s, r.data() );
  return r;
}

/// Scalar-vector subraction operator
/**
 * \relatesalso vector_
 * \param s a scalar to subtract
 * \param v a vector to subtract from.
 */
template <unsigned N, typename T>
inline vector_<N,T> operator-( const T& s, const vector_<N,T>& v )
{
  vector_<N,T> r;
  vector_cmath_<N,T>::sub( s, v.data(), r.data() );
  return r;
}

/// Scalar post-multiplcation operator
/**
 * \relatesalso vector_
 * \param v a vector to multiply
 * \param s a scalar to multiply with
 */
template <unsigned N, typename T>
inline vector_<N,T> operator*( const vector_<N,T>& v, const T& s )
{
  vector_<N,T> r;
  vector_cmath_<N,T>::mul( v.data(), s, r.data() );
  return r;
}

/// Scalar pre-multiplication operator
/**
 * \relatesalso vector_
 * \param s a scalar to multiply with
 * \param v a vector to multiply
 */
template <unsigned N, typename T>
inline vector_<N,T> operator*( const T& s, const vector_<N,T>& v )
{
  vector_<N,T> r;
  vector_cmath_<N,T>::mul( v.data(), s, r.data() );
  return r;
}

/// Scalar division operator
/**
 * \relatesalso vector_
 * \param v a vector to divide from
 * \param s a scalar to divide out
 */
template <unsigned N, typename T>
inline vector_<N,T> operator/( const vector_<N,T>& v, const T& s )
{
  vector_<N,T> r;
  vector_cmath_<N,T>::div( v.data(), s, r.data() );
  return r;
}


// --- Vector-vector operators ----------------------------------------

/// Addition operator
/**
 * \relatesalso vector_
 * \param a a vector
 * \param b another vector of the same shape
 */
template <unsigned N, typename T>
inline vector_<N,T> operator+( const vector_<N,T>& a, const vector_<N,T>& b )
{
  vector_<N,T> r;
  vector_cmath_<N,T>::add( a.data(), b.data(), r.data() );
  return r;
}

/// Subraction operator
/**
 * \relatesalso vector_
 * \param a a vector
 * \param b another vector of the same shape
 */
template <unsigned N, typename T>
inline vector_<N,T> operator-( const vector_<N,T>& a, const vector_<N,T>& b )
{
  vector_<N,T> r;
  vector_cmath_<N,T>::sub( a.data(), b.data(), r.data() );
  return r;
}

/// Element-wise product
/**
 * \relatesalso vector_
 * \param a a vector
 * \param b another vector of the same shape
 */
template <unsigned N, typename T>
inline vector_<N,T> element_product( const vector_<N,T>& a, const vector_<N,T>& b )
{
  vector_<N,T> r;
  vector_cmath_<N,T>::mul( a.data(), b.data(), r.data() );
  return r;
}

/// Element-wise quotient
/**
 * \relatesalso vector_
 * \param a a vector
 * \param b another vector of the same shape
 */
template <unsigned N, typename T>
inline vector_<N,T> element_quotient( const vector_<N,T>& a, const vector_<N,T>& b )
{
  vector_<N,T> r;
  vector_cmath_<N,T>::div( a.data(), b.data(), r.data() );
  return r;
}

/// Equality operator
/**
 * \relatesalso vector_
 * \param a a vector
 * \param b another vector of the same shape
 */
template <unsigned N, typename T>
inline bool operator==( const vector_<N,T>& a, const vector_<N,T>& b )
{
  return vector_cmath_<N,T>::eq( a.data(), b.data() );
}

/// Inequality operator
/**
 * \relatesalso vector_
 * \param a a vector
 * \param b another vector of the same shape
 */
template <unsigned N, typename T>
inline bool operator!=( const vector_<N,T>& a, const vector_<N,T>& b )
{
  return ! vector_cmath_<N,T>::eq( a.data(), b.data() );
}

/// output stream operator for a vector
/**
 * \param s an output stream
 * \param v vector to stream
 */
template <unsigned N, typename T>
KWIVER_CORE_EXPORT std::ostream&  operator<<(std::ostream& s, const vector_<N,T>& v);

/// input stream operator for a vector
/**
 * \param s an input stream
 * \param v vector to stream into
 */
template <unsigned N, typename T>
KWIVER_CORE_EXPORT std::istream&  operator>>(std::istream& s, vector_<N,T>& v);


} // end namespace kwiver


#endif // KWIVER_VECTOR_H_
