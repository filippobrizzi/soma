/**
  * Tag Based Vectorial Operations for Eigen and OpenMP (waiting for OpenMP 3.0)
  *
  * Three conditions:
  * NoParTag is No OpenMP
  * ParTag means to invoke parallel for every time
  * InParTag means that we are invoked inside a OpenMP parallel statement
  *
  * TODO dotmp for
  */
#ifndef OPSMP_H
#define OPSMP_H

class InParTag {};
class ParTag {};
class NoParTag {};
class ParTaskTag{};

inline void setZero(NoParTag, VectorX & w)
{
    w.setZero();
}

inline void setZero(ParTag, VectorX & w)
{
#ifdef HAS_OPENMP
    const unsigned int N  = w.size();
    #pragma omp for nowait
    for(unsigned int j = 0; j < N; j++)
        w(j) = 0;
#else
    setZero(NoParTag(),w);
#endif
}

inline void setZero(InParTag, VectorX & w)
{
#ifdef HAS_OPENMP
    const unsigned int N  = w.size();
    #pragma omp parallel for
    for(unsigned int j = 0; j < N; j++)
        w(j) = 0;
#else
    setZero(NoParTag(),w);
#endif
}

inline void assmp(NoParTag,VectorX & x, const VectorX & y)
{
    x = y;
}

inline void assmp(ParTag,VectorX & x, const VectorX & y)
{
#ifdef HAS_OPENMP
    #pragma omp parallel for
    for(int i = 0; i < x.size(); i++)
        x[i] = y[i];
#else
    return assmp(NoParTag(),x,y);
#endif
}

inline void assmp(InParTag,VectorX & x, const VectorX & y)
{
#ifdef HAS_OPENMP
    #pragma omp for
    for(int i = 0; i < x.size(); i++)
        x[i] = y[i];
#else
    assmp(NoParTag(),x,y);
#endif
}

inline void dotmp(NoParTag,Scalar & r, const VectorX & x, const VectorX & y)
{
    r = x.dot(y);
}

inline void dotmp(ParTag,Scalar & w, const VectorX & x, const VectorX & y)
{
#ifdef HAS_OPENMP
    static Scalar r = 0;
    #pragma omp parallel for reduction(+:r)
    for(int i = 0; i < x.size(); i++)
        r += x[i]*y[i];
    w = r;
#else
    dotmp(NoParTag(),w, x,y);
#endif
}

inline void dotmp(InParTag,Scalar & w, const VectorX & x, const VectorX & y)
{
#ifdef HAS_OPENMP
    static Scalar r = 0;
    #pragma omp for reduction(+:r)
    for(int i = 0; i < x.size(); i++)
        r += x[i]*y[i];
    w = r;
#else
    dotmp(NoParTag(),w, x,y);
#endif
}

inline void cwisemp(NoParTag,VectorX & r, const VectorX & x, const VectorX & y)
{
    r = x.array() * y.array();
}

inline void cwisemp(ParTag,VectorX & r, const VectorX & x, const VectorX & y)
{
#ifdef HAS_OPENMP
    #pragma omp parallel for
    for(int i = 0; i < x.size(); i++)
        r(i) = x(i)*y(i);
#else
    cwisemp(NoParTag(),r,x,y);
#endif
}

inline void cwisemp(InParTag,VectorX & r, const VectorX & x, const VectorX & y)
{
#ifdef HAS_OPENMP
    #pragma omp for
    for(int i = 0; i < x.size(); i++)
        r(i) = x(i)*y(i);
#else
    cwisemp(NoParTag(),r,x,y);
#endif
}

inline void cwisedp(NoParTag,VectorX & r, const VectorX & x, const VectorX & y)
{
    r = x.array() / y.array();
}

inline void cwisedp(ParTag,VectorX & r, const VectorX & x, const VectorX & y)
{
#ifdef HAS_OPENMP
    #pragma omp parallel for
    for(int i = 0; i < x.size(); i++)
        r(i) = x(i)/y(i);
#else
    cwisedp(NoParTag(),r,x,y);
#endif
}

inline void cwisedp(InParTag,VectorX & r, const VectorX & x, const VectorX & y)
{
#ifdef HAS_OPENMP
    #pragma omp for
    for(int i = 0; i < x.size(); i++)
        r(i) = x(i)/y(i);
#else
    cwisedp(NoParTag(),r,x,y);
#endif
}


inline void invmp(NoParTag tag, VectorX & r, const VectorX & x)
{
    // EIGEN3 r = x.cwise().inverse();
    for(int i = 0; i < x.size(); i++)
        r(i) = 1.0/x(i);
}

inline void invmp(ParTag,VectorX & r, const VectorX & x)
{
#ifdef HAS_OPENMP
    #pragma omp parallel for
    for(int i = 0; i < x.size(); i++)
        r(i) = 1.0/x(i);
#else
    invmp(NoParTag(),r,x);
#endif
}

inline void invmp(InParTag,VectorX & r, const VectorX & x)
{
#ifdef HAS_OPENMP
    #pragma omp for
    for(int i = 0; i < x.size(); i++)
        r(i) = 1.0/x(i);
#else
    invmp(NoParTag(),r,x);
#endif
}

inline void submp(NoParTag,VectorX & r, const VectorX & x, const VectorX & y)
{
    r = x-y;
}

inline void submp(ParTag,VectorX & r, const VectorX & x, const VectorX & y)
{
#ifdef HAS_OPENMP
    #pragma omp parallel for
    for(int i = 0; i < x.size(); i++)
        r(i) = x(i)-y(i);
#else
    submp(NoParTag(),r,x,y);
#endif
}

inline void submp(InParTag,VectorX & r, const VectorX & x, const VectorX & y)
{
#ifdef HAS_OPENMP
    #pragma omp for
    for(int i = 0; i < x.size(); i++)
        r(i) = x(i)-y(i);
#else
    submp(NoParTag(),r,x,y);
#endif
}

inline void addmp(NoParTag,VectorX & r, const VectorX & x, const VectorX & y)
{
    r = x+y;
}

inline void addmp(ParTag,VectorX & r, const VectorX & x, const VectorX & y)
{
#ifdef HAS_OPENMP
    #pragma omp parallel for
    for(int i = 0; i < x.size(); i++)
        r(i) = x(i)+y(i);
#else
    addmp(NoParTag(),r,x,y);
#endif
}

inline void addmp(InParTag,VectorX & r, const VectorX & x, const VectorX & y)
{
#ifdef HAS_OPENMP
    #pragma omp for
    for(int i = 0; i < x.size(); i++)
        r(i) = x(i)+y(i);
#else
    addmp(NoParTag(),r,x,y);
#endif
}

#ifdef XHAS_OPENMP

typedef struct
{
} omp_lock_t;

inline void omp_init_lock(omp_lock_t * p) {}
inline void omp_set_lock(omp_lock_t * p){}
inline void omp_unset_lock(omp_lock_t * p){}
inline int omp_get_num_procs() { return 1; }
#endif

#endif // OPSMP_H
