/*
 * Onwer: PERCRO, Scuola Superiore S.Anna
 * License: Internal Code, Not Licensed
 *
 * Authors: initial Paolo Gasparello, then Emanuele Ruffaldi, Paolo Tripicchio
 * Descripotion: common types specifiers for the whole project. Which float type?
 */
#pragma once
#include "config.h"
#include <new>
#include <stdlib.h>
#include <vector>
#include <iostream>

inline std::ostream & qDebug() {return std::cout;}


#ifdef HAS_OPENCL
//include basic scalar and vector types of ViennaCL
#include "viennacl/scalar.hpp"
#include "viennacl/vector.hpp"
#include "viennacl/matrix.hpp"

#include "viennacl/linalg/direct_solve.hpp"
#include "viennacl/linalg/prod.hpp"       //generic matrix-vector product
#include "viennacl/linalg/inner_prod.hpp"
#include "viennacl/linalg/norm_2.hpp"
#include "viennacl/linalg/norm_1.hpp"
#include "viennacl/linalg/cg.hpp"
#endif

namespace Eigen {

template<typename T> class aligned_allocator;

// meta programming to determine if a class has a given member
struct ei_does_not_have_aligned_operator_new_marker_sizeof {int a[1];};
struct ei_has_aligned_operator_new_marker_sizeof {int a[2];};

template<typename ClassType>
struct ei_has_aligned_operator_new {
    template<typename T>
    static ei_has_aligned_operator_new_marker_sizeof
    test(T const *, typename T::ei_operator_new_marker_type const * = 0);
    static ei_does_not_have_aligned_operator_new_marker_sizeof
    test(...);

    // note that the following indirection is needed for gcc-3.3
    enum {ret =  sizeof(test(static_cast<ClassType*>(0)))
              == sizeof(ei_has_aligned_operator_new_marker_sizeof) };
};

#ifdef _MSC_VER

  // sometimes, MSVC detects, at compile time, that the argument x
  // in std::vector::resize(size_t s,T x) won't be aligned and generate an error
  // even if this function is never called. Whence this little wrapper.
  #define _EIGEN_WORKAROUND_MSVC_STD_VECTOR(T) Eigen::ei_workaround_msvc_std_vector<T>
  template<typename T> struct ei_workaround_msvc_std_vector : public T
  {
    inline ei_workaround_msvc_std_vector() : T() {}
    inline ei_workaround_msvc_std_vector(const T& other) : T(other) {}
    inline operator T& () { return *static_cast<T*>(this); }
    inline operator const T& () const { return *static_cast<const T*>(this); }
    template<typename OtherT>
    inline T& operator=(const OtherT& other)
    { T::operator=(other); return *this; }
    inline ei_workaround_msvc_std_vector& operator=(const ei_workaround_msvc_std_vector& other)
    { T::operator=(other); return *this; }
  };

#else

  #define _EIGEN_WORKAROUND_MSVC_STD_VECTOR(T) T

#endif

}



namespace Eigen
{
extern int ei_aligned_malloc_size;
}

#if 0
inline void *operator new(size_t size) throw (std::bad_alloc)
{
    // if size == 0, we must return a valid object! (ARM 5.3.3)
    if (size <= 0)
        size = 1;
    void *ret = (malloc)(size);
    if(!ret)
        throw std::bad_alloc();
    Eigen::ei_aligned_malloc_size += size;
    return ret;
}

inline void operator delete(void *ptr)
  {
      // delete of NULL is okay
      if (ptr == NULL)
          return;
      (free)(ptr);
}
#endif


#include <Eigen/Core>
#include <stdio.h>

#ifdef DOUBLE_PRECISION
typedef double Scalar;
typedef float NonScalar;
typedef Eigen::VectorXd VectorX;
typedef Eigen::Vector2d Vector2;
typedef Eigen::Vector3d Vector3;
typedef Eigen::Vector4d Vector4;
typedef Eigen::MatrixXd MatrixX;
typedef Eigen::Matrix2d Matrix2;
typedef Eigen::Matrix3d Matrix3;
typedef Eigen::Matrix4d Matrix4;

#else
typedef float Scalar;
typedef double NonScalar;
typedef Eigen::VectorXf VectorX;
typedef Eigen::Vector2f Vector2;
typedef Eigen::Vector3f Vector3;
typedef Eigen::Vector4f Vector4;
typedef Eigen::MatrixXf MatrixX;
typedef Eigen::Matrix2f Matrix2;
typedef Eigen::Matrix3f Matrix3;
typedef Eigen::Matrix4f Matrix4;
#endif


#ifdef HAS_OPENCL
typedef viennacl::scalar<Scalar> ScalarCL;
typedef viennacl::vector<Scalar> VectorXCL;
typedef viennacl::matrix<Scalar> MatrixXCL;
typedef viennacl::scalar<cl_int> ScalariCL;
typedef viennacl::vector<cl_int> VectoriXCL;
typedef viennacl::matrix<cl_int> MatrixiXCL;
#else
typedef unsigned int cl_uint;
#endif

#define VECTORSIZE 3
#define MATRIXSIZE 3
typedef Matrix3 MatrixType;
typedef Vector3 VectorType;

#include "opsmp.h"

/*
 * Data is stored depending on the SparseIndex
 */
class SparseData
{
public:
    typedef MatrixType BaseMatrix;
    std::vector<BaseMatrix> data;

    void resize(int size)
    {
        data.resize(size);
    }

    void setZero(InParTag tag)
    {
#ifdef HAS_OPENMP
        const unsigned int N = data.size();
        BaseMatrix x;
        x.setZero();

        #pragma omp parallel for
        for (unsigned int i = 0; i < N; ++i)
            data[i] = x;
#else
        setZero(NoParTag());
#endif
    }

    void setZero(NoParTag tag)
    {
        for (unsigned int i = 0; i < data.size(); ++i)
            data[i].setZero();
    }

    void setZero(ParTag tag)
    {
#ifdef HAS_OPENMP
        const unsigned int N = data.size();
        BaseMatrix x;
        x.setZero();

        #pragma omp for
        for (unsigned int i = 0; i < N; ++i)
            data[i] = x;
#else
        setZero(NoParTag());
#endif
    }

    size_t size() const { return  data.size(); }

    BaseMatrix & operator [] (unsigned int i) { return data[i]; }
    const BaseMatrix & operator [] (unsigned int i) const { return data[i]; }
};



struct SparseIdx
{
    /// indices of inner and outer elements for the computation
    std::vector<unsigned> inner, outer, backinner, symblock;
	
    void clear()
    {
            inner.clear();
            outer.clear();
            backinner.clear();
            symblock.clear();
    }

    /// resizes the indices
    void resize(size_t nz, size_t cols) {
        inner.resize(nz);
        outer.resize(cols + 1);
        backinner.resize(nz);
        symblock.resize(nz);
    }

    /// returns number of columns
    size_t cols() const { return outer.size() - 1; }

    int getBlockIndex(int i, int j);

    SparseIdx(): digonalFirst(true) {}

    bool digonalFirst;
};

struct SparseIdxSym: public SparseIdx
{
};


template <int N = 9>
struct SparseIdxELL
{
    enum { Size = N };

    /// indices of inner and outer elements for the computation
    std::vector<unsigned> innerstart;
    std::vector<unsigned> innercount;

    void clear()
    {
        std::fill(innerstart.begin(),innerstart.end(),-1);
        std::fill(innercount.begin(),innercount.end(),0);
    }

    /// resizes the indices
    void resize(size_t cols)
    {
        innerstart.resize(cols);
        innercount.resize(cols);
    }

    // build from the CSR structure into a ELL with N elements
    void from(const SparseIdx & sp)
    {
        resize(sp.cols());
        for(unsigned i = 0; i < sp.cols(); i++)
        {
            unsigned  b = sp.outer[i];
            unsigned  e = sp.outer[i+1];
            unsigned m = e-b;
            if (m > N)
                m = N;
            std::fill(std::copy(sp.inner.begin() + b,sp.inner.begin() + (b+m),innerstart.begin() + i*N),innerstart.begin() + (i+1)*N,-1);
            innercount[i] = m;
        }
    }

    // build from the CSR structure into a ELL with N elements, skipping identities (that is i == inner[j] for some j
    void fromNoIdentity(const SparseIdx & sp)
    {
        resize(sp.cols());
        for(unsigned i = 0; i < sp.cols(); i++)
        {
            unsigned b = sp.outer[i];
            unsigned e = sp.outer[i+1];
            unsigned m = e-b;
            if (m > N+1)
                m = N;
            std::vector<unsigned>::iterator itb = innerstart.begin() + i*N;
            std::vector<unsigned>::iterator ite = std::remove_copy_if(
                    sp.inner.begin() + b,sp.inner.begin() + (b+m),itb,
                        std::bind2nd(std::not_equal_to<unsigned>(), i));
            std::fill(ite,innerstart.begin() + (i+1)*N,-1);
            innercount[i] = ite-itb;
        }
    }

    /// returns number of columns
    size_t cols() const { return innercount.size(); }
};

template <class X>
class MATrait
{
public:
        static const int rows = 1;
        static const int cols = 1;
        static const int size = 1;
        typedef X value_t ;
        typedef X full_t ;
};

template <typename X, int A>
class MATrait<X [A]>
{
public:
        static const int rows = A;
        static const int cols = 1;
        static const int size = A;
        typedef X value_t ;
        typedef X full_t[A];
};

template <typename X, int A, int B>
class MATrait<X [A][B]>
{
public:
        static const int rows = A;
        static const int cols = B;
        static const int size = A*B;
        typedef X value_t ;
        typedef X full_t [A][B];
};

template <class T>
inline int matraitCols(const T & x)
{
        return MATrait<T>::cols;
}

template <class T>
inline int matraitSize(const T & x)
{
        return MATrait<T>::size;
}

template <class T>
inline int matraitRows(const T & x)
{
        return MATrait<T>::rows;
}

template <class X>
class MAWrapEig // use X basic Eigen type and size
{
public:
        typedef typename X::value_t xvalue_t;
        typedef typename X::value_t::Scalar value_t;
        static const int itemsize = xvalue_t::SizeAtCompileTime;
        static const int blocksize = xvalue_t::SizeAtCompileTime*X::size;
        MAWrapEig(value_t * p) : p_(p) {}

        void set(int j, const typename X::full_t & w)
        {
                const xvalue_t * q = X::ptr(w);	 // array w as sequence of xvalue_t
                value_t * b = begin(j);
                for(int i = 0; i < X::size; i++, b += blocksize, q++)
                        std::copy(q->data(),q->data()+itemsize, b);
        }

        void get(int j, typename X::full_t & w)  const
        {
                xvalue_t * q = X::ptr(w);	 // array w as sequence of xvalue_t
                const value_t * b = begin(j);
                for(int i = 0; i < X::size; i++, b += blocksize, q++)
                        std::copy(b,b+itemsize,q->data());
        }

        value_t * begin(int i) { return p_+i*blocksize; }
        value_t * end(int i) { return p_+(i+1)*blocksize; }
        const value_t * begin(int i) const { return p_+i*blocksize; }
        const value_t * end(int i) const { return p_+(i+1)*blocksize; }


        value_t * p_; // base pointer
};

class SimLogger
{
public:
    inline virtual ~SimLogger() {}
    // PcgFirstStep PcgEndStep: d_new
    // PcgEnd has lastDelta
    // PcgStart marks start
    enum PcgPhase { PcgStart, PcgFirstStep, PcgThreshold,PcgBeginStep, PcgEndStep, PcgEnd, PcgThresholdBase, PcgThresholdAsc,PcgEndFlag, PcgSPVM,PcgSPVMEnd};
    enum SimState { SimLoop,StartStep,Collision, CollisionResult, PcgResult, PaintGL,GraphicClothUpdate,PhaseBase=20};

    virtual void pcg(PcgPhase phase, int step, Scalar value) = 0;

    virtual void event(SimState state, int step, Scalar value = 0.0) = 0;
    virtual int begin(SimState state, Scalar value = 0.0, int step = 0, int extrainfo = 0) = 0;
    virtual void end(SimState state, int id,Scalar value = 0.0, int step = 0, int extrainfo = 0) = 0;

    virtual void save() = 0;
    virtual void start() = 0;
};

class SimLoggerScope
{
public:
    SimLoggerScope(SimLogger & l, SimLogger::SimState state, int xstep = 0, Scalar value = 0.0): ll(l),xvalue(value),extrainfo(0), step(xstep) {
            id = ll.begin(state,value,step,extrainfo);


    }
    void setExtraInfo(int xi) { extrainfo = xi; }
    void setValue(Scalar v) { xvalue = v; }
    ~SimLoggerScope()

    {
        ll.end(state,id,xvalue,step,extrainfo);
    }
    SimLogger & ll;
    int id;
    SimLogger::SimState state;
    int extrainfo;
    int step;
    Scalar xvalue;
};
