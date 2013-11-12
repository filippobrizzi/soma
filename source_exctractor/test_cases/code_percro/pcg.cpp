/*
 * Onwer: PERCRO, Scuola Superiore S.Anna
 * License: Internal Code, Not Licensed
 *
 * Authors: initial Paolo Gasparello, then Emanuele Ruffaldi, Paolo Tripicchio
 * Descripotion: Preconditioned Conjugate Gradient Method
 */
//#include <Eigen/Array> // removed for Eigen3
#include "pcg.hpp"
#include "spmv.hpp"
//#include <QDebug>
using namespace Eigen;
#define VECTORISZE 3

#define isinf(x) ((x) > 1E10)


/**
 * Disjoint sets:
 *  fixed => identity
 *  free  => zero
 *  constrained 1-2 dof => r := Mx
 *
 * This cannot be easily done inline because zero has to be explicit
 */
void ifilter(NoParTag tag,VectorX & r,const S_t & S, const VectorX & x)
{
    setZero(tag,r);
    for(int i = 0; i < S.vtxFixed.size(); i++)
    {
        int j = S.vtxFixed[i]*3;
        r.segment<3>(j) = x.segment<3>(j);
    }

    for(int j = 0; j < S.size(); j++)
    {
        int i = 3*S.getIndex(j);
        r.segment<3>(i) = S.getMatrix(j)*x.segment<3>(i);
    }
}

void ifilter(ParTag tag,VectorX & r,const S_t & S, const VectorX & x)
{
    setZero(tag,r);

    #pragma omp parallel for
    for(int i = 0; i < S.vtxFixed.size(); i++)
    {
        int j = S.vtxFixed[i]*3;
        r.segment<3>(j) = x.segment<3>(j);
    }

    #pragma omp parallel for
    for(int j = 0; j < S.size(); j++)
    {
        int i = 3*S.getIndex(j);
        r.segment<3>(i) = S.getMatrix(j)*x.segment<3>(i);
    }
}

void ifilter(InParTag tag,VectorX & r,const S_t & S, const VectorX & x)
{
    setZero(tag,r);

    #pragma omp for
    for(int i = 0; i < S.vtxFixed.size(); i++)
    {
        int j = S.vtxFixed[i]*3;
        r.segment<3>(j) = x.segment<3>(j);
    }

    #pragma omp for
    for(int j = 0; j < S.size(); j++)
    {
        int i = 3*S.getIndex(j);
        r.segment<3>(i) = S.getMatrix(j)*x.segment<3>(i);
    }
}

/**
 * x <= S x
 * But we store not S but instead S' = I-S => S = I-S'
 *
 * x <= x - S' x
 *
 *  fixed => S is zero => zero
 *  free  => no changes
 *  constrained => x <= S'x + x
 *
 */
void filter(NoParTag tag,const S_t & S,VectorX & r)
{
    for(S_t::fixed_t::const_iterator it = S.vtxFixed.begin(); it != S.vtxFixed.end(); ++it)
        r.segment<VECTORSIZE>(VECTORSIZE* *it).setZero();

    for(S_t::const_iterator it = S.begin(); it != S.end(); it++)
        r.segment<VECTORSIZE>(VECTORSIZE* it->idx) -= it->mat*r.segment<VECTORSIZE>(VECTORSIZE*it->idx);
}

void filter(ParTag tag,const S_t & S,VectorX & r)
{
    #pragma omp parallel for
    for(int i = 0; i < S.vtxFixed.size(); i++)
        r.segment<3>(S.vtxFixed[i]*3).setZero();

    #pragma omp parallel for
    for(int j = 0; j < S.size(); j++)
    {
        int i = 3*S.getIndex(j);
        r.segment<3>(i) -= S.getMatrix(j)*r.segment<3>(i);
    }
}

void filter(InParTag tag,const S_t & S,VectorX & r)
{
    #pragma omp for
    for(int i = 0; i < S.vtxFixed.size(); i++)
        r.segment<3>(S.vtxFixed[i]*3).setZero();

    #pragma omp for
    for(int j = 0; j < S.size(); j++)
    {
        int i = 3*S.getIndex(j);
        r.segment<3>(i) -= S.getMatrix(j)*r.segment<3>(i);
    }
}


void filter(NoParTag tag,VectorX & r,const S_t & S, const VectorX & x)
{
    //cwisemp(tag,r,S.getS(),x); // S * x as diagonal component
    assmp(tag,r,x);
    for(int i = 0; i < S.vtxFixed.size(); i++)
        r.segment<3>(S.vtxFixed[i]*3).setZero();

    // now subtract p*p' if it has been defined
    for(int j = 0; j < S.size(); j++)
    {
        int i = 3*S.getIndex(j);
        // "I" has been applied above with assmp
        r.segment<3>(i) -= S.getMatrix(j)*x.segment<3>(i);
    }
}

 void filter(ParTag tag,VectorX & r,const S_t & S, const VectorX & x)
{
     assmp(tag,r,x);
     #pragma omp parallel for
     for(int i = 0; i < S.vtxFixed.size(); i++)
         r.segment<3>(S.vtxFixed[i]*3).setZero();

     // now subtract p*p' if it has been defined
     #pragma omp parallel for
     for(int j = 0; j < S.size(); j++)
     {
         int i = 3*S.getIndex(j);
         // "I" has been applied above with S
         r.segment<3>(i) -= S.getMatrix(j)*x.segment<3>(i);
     }
}

 void filter(InParTag tag,VectorX & r,const S_t & S, const VectorX & x)
{
     assmp(tag,r,x);
     #pragma omp parallel for
     for(int i = 0; i < S.vtxFixed.size(); i++)
         r.segment<3>(S.vtxFixed[i]*3).setZero();

     // now subtract p*p' if it has been defined
     #pragma omp parallel for
     for(int j = 0; j < S.size(); j++)
     {
         int i = 3*S.getIndex(j);
         // "I" has been applied above with S
         r.segment<3>(i) -= S.getMatrix(j)*x.segment<3>(i);
     }
}

// pre-conditioned conjugate gradient
// TODO: in the future remove these allocations using a PCG class
//template<class SparseIdxA>
int PCG::pcg(NoParTag par, const VectorX& P, const SparseIdx& idx, const SparseData& A, const VectorX& b, VectorX& x, const Scalar tol, const Scalar dtol,const int min_it, const int max_it, const S_t& S, bool ascherbd)
{
    NoParTag dotpar; // dot product is single processor: DOES NOT WORK IN PARALLEL
    NoParTag filterpar; // filter is single processor: IMPACTS NEGATIVELY TO USE PARALLEL

    qDebug() << "PCG::pcg NoParTag VectorPrecond";

    // Ascher uses b* = S(b-A(I-S)z) ) => filter(b - A ifilter(z))
    // Otherwise b* = S(b)
    // d0 = filter(b)' P filter(b)
    // where P is diagonal of the Sparse_K
    Scalar d0,d0a,d0b,d_new;

    logger.pcg(SimLogger::PcgStart,0,S.size());
    invmp(par,Pinv,P);
    const VectorX & Pthreshold = Pinv;

    if(ascherbd)
    {
        // S(b - A(I-S)z)' P^-1
        // when z is zero due to no collisions => S(b) inv(P) S(b)
        ifilter(filterpar,tmp,S,x); // tmp = (I-S)z
        spmvV(par,idx,A,tmp,tmp2); // tmp2 = A tmp
        submp(par,b_filt,b,tmp2); // b_filt = S(b - tmp2)
        filter(filterpar,S,b_filt);
        cwisemp(par,tmp,Pthreshold,b_filt); // tmp = Pinv * b_filt
        dotmp(dotpar,d0a,b_filt,tmp); // b_filt' * tmp
        d0 = d0a;
    }
    else
    {
        // (Sb)' P (Sb)
        filter(filterpar,b_filt,S,b);    // b_filter = filter(S,b)
        cwisemp(par,tmp,Pthreshold,b_filt); // tmp = P .* b_filt
        dotmp(dotpar,d0b,b_filt,tmp);  // d0 = b_filt' tmp = b_filt' (P .* b_filt) = filter(S,b)' P filter (S,b)
        d0 = d0b;
    }
    const Scalar threshold = fabs(d0 * tol);

    spmvV(par,idx,A,x,tmp);     // tmp = Ax
    submp(par,r,b,tmp);        // r = S(b-Ax)
    filter(filterpar,S,r);
    Scalar d0res;
    dotmp(dotpar,d0res,r,r);
    // c = S (P^-1 r) = P^-1 r because r is filtered and P^-1 is diagonal
    cwisemp(par,c,Pinv,r);     // c = S(P^-1 * r) = S(P^-1 S(b-Ax))
    filter(filterpar,S,c);
    dotmp(dotpar,d_new,r,c);      // d_new = dot(r,c) = filter(S,b-Ax)' * P^-1 * filter(S, b-Ax) = sum (x_i)^2 P^-1



    //std::cerr << "x=" << x.dot(x) << " b=" << b.dot(b) << " bf=" << b_filt.dot(b_filt) << " r=" << r.dot(r) << " c=" << c.dot(c) << " x=" << x.dot(x) << " Pinv" << Pinv.dot(Pinv) << " P " << P.dot(P) << " tmp " << tmp.dot(tmp) << " tmp2 " << tmp2.dot(tmp2) << std::endl;
    //rstd::cerr << "PCGNoPar d_new:" << d_new << "  d0:" << d0 << " tol:" << tol << " th:" << threshold << std::endl;
    logger.pcg(SimLogger::PcgFirstStep,0,d_new);
    logger.pcg(SimLogger::PcgThreshold,0,threshold);
    logger.pcg(SimLogger::PcgThresholdBase,0,d0b*tol);
    logger.pcg(SimLogger::PcgThresholdAsc,0,d0a*tol);

    if(isinf(d_new))
    {
        logger.pcg(SimLogger::PcgEndFlag,-1,5);
        logger.pcg(SimLogger::PcgEnd,-1,0);
        std::cerr << "inf d_new\n";
        return 0;
    }

    if(min_it == 0 && d_new <= threshold)
    {
        lastDelta = d_new-threshold; //not used till now
        logger.pcg(SimLogger::PcgEnd,0,lastDelta);
        return 0;
    }

    int i;
    int badflag = 0;
    for (i = 0; i < min_it || i < max_it; ++i)
    {
        logger.pcg(SimLogger::PcgBeginStep,i,0);        
        logger.pcg(SimLogger::PcgSPVM,i,0);
        spmvV(par,idx,A,c,q); // q = filter(A * c)
        logger.pcg(SimLogger::PcgSPVMEnd,i,0);
        filter(filterpar,S,q);
        Scalar rho;
        dotmp(dotpar,rho,c,q);
        if(rho == 0 || isinf(rho))
        {
            badflag = 1;
            logger.pcg(SimLogger::PcgEndStep,i,d_new);
            break;
        }

        Scalar alpha = d_new / rho; // alpha = d_new / dot(c,q)
        if(alpha == 0 || isinf(alpha))
        {
            badflag = 2;
            logger.pcg(SimLogger::PcgEndStep,i,d_new);
            break;
        }

        x += alpha*c;
        r -= alpha*q;
        s = Pinv.cwiseProduct(r);    // s = r ./ P
        filter(filterpar,S,s);

        Scalar d_old = d_new;
        dotmp(dotpar,d_new,r,s); // d_new = dot(r,s)

        if(d_new <= 0 || isinf(d_new))
        {
            badflag = 3;
            logger.pcg(SimLogger::PcgEndStep,i,d_new);
            break;
        }

        if(d_new <= threshold && i >= min_it)
        {
            logger.pcg(SimLogger::PcgEndStep,i,d_new);
            break;
        }

        if(d_old - d_new < dtol && i >= min_it)
        {
            logger.pcg(SimLogger::PcgEndStep,i,d_new);
            break;
        }
//        std::cerr << "\td_new "<< d_new << std::endl;

        Scalar beta = d_new / d_old;
        if(isinf(beta))
        {
            badflag = 4;
            logger.pcg(SimLogger::PcgEndStep,i,d_new);
            break;
        }
        c = s + beta*c;
        logger.pcg(SimLogger::PcgEndStep,i,d_new);
    }
    lastDelta = d_new-threshold; //not used till now
    logger.pcg(SimLogger::PcgEndFlag,i,badflag);
    logger.pcg(SimLogger::PcgEnd,i,lastDelta);
    if(badflag)
        std::cerr << "pcg badflag " << badflag << std::endl;

   // spmv(par,idx,A,x,tmp);     // tmp = Ax
    //submp(par,r,b,tmp);        // r = S(b-Ax)
  //  filter(filterpar,S,r);
  //  Scalar dfres;
//    dotmp(dotpar,dfres,r,r);
//    std::cerr << "PCG: res:" << dfres << " thr:" << threshold << " tol: " << tol << " fin/start " << (dfres/d0res) << " iter:" << i << " error " << badflag<< std::endl;
    return i;
}


// pre-conditioned conjugate gradient
// TODO: in the future remove these allocations using a PCG class
//template<class SparseIdxA>
int PCG::pcg(NoParTag par, const SparseIdx& idx, const SparseData& A, const VectorX& b, VectorX& x, const Scalar tol, const Scalar dtol,const int min_it, const int max_it, const S_t& S, bool ascherbd)
{
    NoParTag dotpar; // dot product is single processor: DOES NOT WORK IN PARALLEL
    NoParTag filterpar; // filter is single processor: IMPACTS NEGATIVELY TO USE PARALLEL

    qDebug() << "PCG::pcg NoParTag NOPRECOND";

    // Ascher uses b* = S(b-A(I-S)z) ) => filter(b - A ifilter(z))
    // Otherwise b* = S(b)
    // d0 = filter(b)' P filter(b)
    // where P is diagonal of the Sparse_K
    Scalar d0,d0a,d0b,d_new;

    logger.pcg(SimLogger::PcgStart,0,S.size());

    {
        // S(b - A(I-S)z)' P^-1
        // when z is zero due to no collisions => S(b) inv(P) S(b)
        ifilter(filterpar,tmp,S,x); // tmp = (I-S)z
        spmv(par,idx,A,tmp,tmp2); // tmp2 = A tmp
        submp(par,b_filt,b,tmp2); // b_filt = S(b - tmp2)
        filter(filterpar,S,b_filt);
        dotmp(dotpar,d0a,b_filt,b_filt); // b_filt' * tmp
    }
    {
        // (Sb)' P (Sb)
        filter(filterpar,b_filt,S,b);    // b_filter = filter(S,b)
        dotmp(dotpar,d0b,b_filt,b_filt);  // d0 = b_filt' tmp = b_filt' (P .* b_filt) = filter(S,b)' P filter (S,b)
    }
    d0 = ascherbd ? d0a : d0b;
    const Scalar threshold = fabs(d0 * tol);

    spmv(par,idx,A,x,tmp);     // tmp = Ax
    submp(par,r,b,tmp);        // r = S(b-Ax)
    filter(filterpar,S,r);
    Scalar d0res;
    dotmp(dotpar,d0res,r,r);
    // c = S (P^-1 r) = P^-1 r because r is filtered and P^-1 is diagonal
    c = r;
    d_new = d0res;



    //std::cerr << "x=" << x.dot(x) << " b=" << b.dot(b) << " bf=" << b_filt.dot(b_filt) << " r=" << r.dot(r) << " c=" << c.dot(c) << " x=" << x.dot(x) << " Pinv" << Pinv.dot(Pinv) << " P " << P.dot(P) << " tmp " << tmp.dot(tmp) << " tmp2 " << tmp2.dot(tmp2) << std::endl;
    //rstd::cerr << "PCGNoPar d_new:" << d_new << "  d0:" << d0 << " tol:" << tol << " th:" << threshold << std::endl;
    logger.pcg(SimLogger::PcgFirstStep,0,d_new);
    logger.pcg(SimLogger::PcgThreshold,0,threshold);
    logger.pcg(SimLogger::PcgThresholdBase,0,d0b*tol);
    logger.pcg(SimLogger::PcgThresholdAsc,0,d0a*tol);

    if(isinf(d_new))
    {
        logger.pcg(SimLogger::PcgEndFlag,-1,5);
        logger.pcg(SimLogger::PcgEnd,-1,0);
        std::cerr << "inf d_new\n";
        return 0;
    }

    if(min_it == 0 && d_new <= threshold)
    {
        lastDelta = d_new-threshold; //not used till now
        logger.pcg(SimLogger::PcgEnd,0,lastDelta);
        return 0;
    }

    int i;
    int badflag = 0;
    for (i = 0; i < min_it || i < max_it; ++i)
    {
        logger.pcg(SimLogger::PcgBeginStep,i,0);
        logger.pcg(SimLogger::PcgSPVM,i,0);
        spmv(par,idx,A,c,q); // q = filter(A * c)
        logger.pcg(SimLogger::PcgSPVMEnd,i,0);
        filter(filterpar,S,q);
        Scalar rho;
        dotmp(dotpar,rho,c,q);
        if(rho == 0 || isinf(rho))
        {
            badflag = 1;
            logger.pcg(SimLogger::PcgEndStep,i,d_new);
            break;
        }

        Scalar alpha = d_new / rho; // alpha = d_new / dot(c,q)
        if(alpha == 0 || isinf(alpha))
        {
            badflag = 2;
            logger.pcg(SimLogger::PcgEndStep,i,d_new);
            break;
        }

        x += alpha*c;
        r -= alpha*q;
        s = r;    // s = r ./ P
        //s = r.cwise() / P;
        Scalar d_old = d_new;
        dotmp(dotpar,d_new,r,s); // d_new = dot(r,s)

        if(d_new <= 0 || isinf(d_new))
        {
            badflag = 3;
            logger.pcg(SimLogger::PcgEndStep,i,d_new);
            break;
        }

        if(i >= min_it && d_new <= threshold)
        {
            logger.pcg(SimLogger::PcgEndStep,i,d_new);
            break;
        }

        if(d_old - d_new < dtol)
        {
            logger.pcg(SimLogger::PcgEndStep,i,d_new);
            break;
        }
//        std::cerr << "\td_new "<< d_new << std::endl;

        Scalar beta = d_new / d_old;
        if(isinf(beta))
        {
            badflag = 4;
            logger.pcg(SimLogger::PcgEndStep,i,d_new);
            break;
        }
        c = s + beta*c;
        filter(filterpar,S,c);
        logger.pcg(SimLogger::PcgEndStep,i,d_new);
    }
    lastDelta = d_new-threshold; //not used till now
    logger.pcg(SimLogger::PcgEndFlag,i,badflag);
    logger.pcg(SimLogger::PcgEnd,i,lastDelta);
    if(badflag)
        std::cerr << "pcg badflag " << badflag << std::endl;

   // spmv(par,idx,A,x,tmp);     // tmp = Ax
    //submp(par,r,b,tmp);        // r = S(b-Ax)
  //  filter(filterpar,S,r);
  //  Scalar dfres;
//    dotmp(dotpar,dfres,r,r);
//    std::cerr << "PCG: res:" << dfres << " thr:" << threshold << " tol: " << tol << " fin/start " << (dfres/d0res) << " iter:" << i << " error " << badflag<< std::endl;
    return i;
}


#if 0
// pre-conditioned conjugate gradient
// TODO: in the future remove these allocations using a PCG class
//template<class SparseIdxA>
int PCG::pcg33(NoParTag par, const VectorX& Pnotused, const SparseIdx& idx, const SparseData& A, const VectorX& b, VectorX& x, const Scalar tol, const int min_it, const int max_it, const S_t& S, bool ascherbd)
{
    NoParTag dotpar; // dot product is single processor: DOES NOT WORK IN PARALLEL
    NoParTag filterpar; // filter is single processor: IMPACTS NEGATIVELY TO USE PARALLEL
    int size = b.size()/VECTORSIZE;

    /*
     C is original preconditioner

     constrained: inv( S C + I - S)
     free: S = identity => inv(C)
     fixed: S = zero => I

     We store S'=I-S => S = I-S'
     For the constrained case:
      */

#ifdef DOUBLE_PRECISION
#define CASTUP(x) ((x).cast<double>())
#define CASTDN(x) ((x).cast<float>())
#else
#define CASTUP(x) (x)
#define CASTDN(x) (x)
#endif

    for(int i = 0; i < size; i++)
    {
        Pinv33[i] = CASTUP(A[i]);

        //Pinv33[i].setZero();
        //Pinv33[i].diagonal() = CASTUP(A[i].diagonal());

        if(i > 33)
            std::cerr << "C"<<Pinv33[i] << std::endl;
        Matrix3 r;
        Scalar d;
        bool inve = false;
        Pinv33[i].computeInverseAndDetWithCheck(r,d,inve);
        if(inve)
            Pinv33[i] = r;
        else
            Pinv33[i].setIdentity();
        Pinv33[i].setZero();
        Pinv33[i].diagonal() = A[i].diagonal(); //Vector3(1.0/A[i](0,0),1.0/A[i](1,1),1.0/A[2](2,2));
        //std::cerr << " " << Pinv33[i] << std::endl;
        r = Pinv33[i];
        Pinv33[i] = r.inverse();
        if(i > 33)
        {
            std::cerr << "Pis " << i << " " << Pnotused.segment<VECTORSIZE>(i*VECTORSIZE).transpose() << std::endl;
            std::cerr << "Pinv is: "<<Pinv33[i] << "\n" << std::endl;
        }
    }

    for(S_t::fixed_t::const_iterator it = S.vtxFixed.begin(); it != S.vtxFixed.end(); ++it)
    {
        std::cerr << "fix " << *it << std::endl;
        //Pinv33[*it].setIdentity();
    }

    // S' = I - S => S = I - S'
    // inv ( S C + I - S) = inv ( C - S'C + I - I + S') = inv( C - S'C + S')
    for(S_t::const_iterator it = S.begin(); it != S.end(); it++)
    {
        int ivtx = it->idx;
        const Pinv33_t sm = CASTUP(it->mat).transpose();
        const Pinv33_t C = CASTUP(A[ivtx]);
        Pinv33[ivtx] = (C - sm*C + sm).inverse();
    }

    // diagonal is in first blocks of A

    // Ascher uses b* = S(b-A(I-S)z) ) => filter(b - A ifilter(z))
    // Otherwise b* = S(b)
    // d0 = filter(b)' P filter(b)
    // where P is diagonal of the Sparse_K
    Scalar d0,d0a,d0b,d_new;

    logger.pcg(SimLogger::PcgStart,0,S.size());
    //invmp(par,Pinv,P);
    //const VectorX & Pthreshold = Pinv;

    {
        // S(b - A(I-S)z)' P^-1
        // when z is zero due to no collisions => S(b) inv(P) S(b)
        ifilter(filterpar,tmp,S,x); // tmp = (I-S)z
        spmv(par,idx,A,tmp,tmp2); // tmp2 = A tmp
        submp(par,b_filt,b,tmp2); // b_filt = S(b - tmp2)
        filter(filterpar,S,b_filt);

        //cwisemp(par,s,Pinv,r);

#define CASTUP(x) (x)
#define CASTDN(x) (x)
#ifdef DOUBLE_PRECISION
#define PinvProd(to,m,from)\
        for(int i = 0; i < size; i++)\
            to.segment<VECTORISZE>(i*VECTORSIZE) = ( m[i].transpose()*(from.segment<VECTORSIZE>(i*VECTORSIZE)));
#else
#define PinvProd(to,m,from)\
        for(int i = 0; i < size; i++)\
            to.segment<VECTORISZE>(i*VECTORSIZE) = CASTDN( m[i]*CASTUP(from.segment<VECTORSIZE>(i*VECTORSIZE)));
#endif
        PinvProd(tmp,Pinv33,b_filt)
        //cwisemp(par,tmp,Pinv,b_filt); // tmp = Pinv * b_filt
        dotmp(dotpar,d0a,b_filt,tmp); // b_filt' * tmp
    }
    {
        // (Sb)' P (Sb)
        filter(filterpar,b_filt,S,b);    // b_filter = filter(S,b)
        PinvProd(tmp,Pinv33,b_filt)
        //cwisemp(par,tmp,Pinv,b_filt); // tmp = P .* b_filt
        dotmp(dotpar,d0b,b_filt,tmp);  // d0 = b_filt' tmp = b_filt' (P .* b_filt) = filter(S,b)' P filter (S,b)
    }
    d0 = ascherbd ? d0a : d0b;
    const Scalar threshold = fabs(d0 * tol);

    spmv(par,idx,A,x,tmp);     // tmp = Ax
    submp(par,r,b,tmp);        // r = S(b-Ax)
    filter(filterpar,S,r);
    // c = S (P^-1 r) = P^-1 r because r is filtered and P^-1 is diagonal
    PinvProd(c,Pinv33,r)
    //cwisemp(par,c,Pinv,r);     // c = S(P^-1 * r) = S(P^-1 S(b-Ax))
    filter(filterpar,S,c);
    dotmp(dotpar,d_new,r,c);      // d_new = dot(r,c) = filter(S,b-Ax)' * P^-1 * filter(S, b-Ax) = sum (x_i)^2 P^-1


    //std::cerr << "x=" << x.dot(x) << " b=" << b.dot(b) << " bf=" << b_filt.dot(b_filt) << " r=" << r.dot(r) << " c=" << c.dot(c) << " x=" << x.dot(x) << " Pinv" << Pinv.dot(Pinv) << " P " << P.dot(P) << " tmp " << tmp.dot(tmp) << " tmp2 " << tmp2.dot(tmp2) << std::endl;
    //rstd::cerr << "PCGNoPar d_new:" << d_new << "  d0:" << d0 << " tol:" << tol << " th:" << threshold << std::endl;
    logger.pcg(SimLogger::PcgFirstStep,0,d_new);
    logger.pcg(SimLogger::PcgThreshold,0,threshold);
    logger.pcg(SimLogger::PcgThresholdBase,0,d0b*tol);
    logger.pcg(SimLogger::PcgThresholdAsc,0,d0a*tol);

    if(isinf(d_new))
    {
        logger.pcg(SimLogger::PcgEndFlag,-1,5);
        logger.pcg(SimLogger::PcgEnd,-1,0);
        std::cerr << "inf d_new\n";
        return 0;
    }

    if(min_it == 0 && d_new <= threshold)
    {
        lastDelta = d_new-threshold; //not used till now
        logger.pcg(SimLogger::PcgEnd,0,lastDelta);
        return 0;
    }

    int i;
    int badflag = 0;
    for (i = 0; i < min_it || i < max_it; ++i)
    {
        logger.pcg(SimLogger::PcgBeginStep,i,0);
        logger.pcg(SimLogger::PcgSPVM,i,0);

        spmv(par,idx,A,c,q);

        logger.pcg(SimLogger::PcgSPVMEnd,i,0);
        filter(filterpar,S,q);

        Scalar rho;
        dotmp(dotpar,rho,c,q);
        if(rho == 0 || isinf(rho))
        {
            badflag = 1;
            logger.pcg(SimLogger::PcgEndStep,i,d_new);
            break;
        }

        Scalar alpha = d_new / rho; // alpha = d_new / dot(c,q)
        if(alpha == 0 || isinf(alpha))
        {
            badflag = 2;
            logger.pcg(SimLogger::PcgEndStep,i,d_new);
            break;
        }

        x += alpha*c;
        r -= alpha*q;
        PinvProd(s,Pinv33,r)
        Scalar d_old = d_new;
        dotmp(dotpar,d_new,r,s); // d_new = dot(r,s)

        if(d_new <= 0 || isinf(d_new))
        {
            badflag = 3;
            logger.pcg(SimLogger::PcgEndStep,i,d_new);
            break;
        }

        if(i >= min_it && d_new <= threshold)
        {
            logger.pcg(SimLogger::PcgEndStep,i,d_new);
            break;
        }

        Scalar beta = d_new / d_old;
        if(isinf(beta))
        {
            badflag = 4;
            logger.pcg(SimLogger::PcgEndStep,i,d_new);
            break;
        }
        c = s + beta*c;
        filter(filterpar,S,c);
        logger.pcg(SimLogger::PcgEndStep,i,d_new);
    }
    lastDelta = d_new-threshold; //not used till now
    logger.pcg(SimLogger::PcgEndFlag,i,badflag);
    logger.pcg(SimLogger::PcgEnd,i,lastDelta);
    if(badflag)
        std::cerr << "pcg badflag " << badflag << std::endl;
    return i;
}
#else


// pre-conditioned conjugate gradient
// TODO: in the future remove these allocations using a PCG class
//template<class SparseIdxA>
int PCG::pcg33(NoParTag par, const VectorX& Pnotused, const SparseIdx& idx, const SparseData& A, const VectorX& b, VectorX& x, const Scalar tol,const Scalar dtol, const int min_it, const int max_it, const S_t& S, bool ascherbd)
{
    NoParTag dotpar; // dot product is single processor: DOES NOT WORK IN PARALLEL
    NoParTag filterpar; // filter is single processor: IMPACTS NEGATIVELY TO USE PARALLEL

#define CASTUP(x) (x)
#define CASTDN(x) (x)
#define PinvProd(to,m,from)\
        for(int i = 0; i < size; i++)\
            to.segment<VECTORISZE>(i*VECTORSIZE) = CASTDN( m[i]*CASTUP(from.segment<VECTORSIZE>(i*VECTORSIZE)));

#define xPinvProd(to,m,from)\
        cwisemp(par,to,Pthreshold,from); // tmp = Pinv * b_filt

        invmp(par,Pinv,Pnotused);
        const VectorX & Pthreshold = Pinv;

        int size = b.size()/VECTORSIZE;
    for(int i = 0; i < size; i++)
    {
        Matrix3 r;
        Scalar d;
        bool inve = false;

        Pinv33[i] = CASTUP(A[i]);
        Pinv33[i].computeInverseAndDetWithCheck(r,d,inve);
        if(inve) // && d > 1e-5)
            Pinv33[i] = r;
        else
        {
            //Pinv33[i].setIdentity();
            Pinv33[i].setZero();
            Pinv33[i].diagonal() = Pinv.segment<VECTORSIZE>(i*VECTORSIZE);
        }
        if(i > 33)
        {
            std::cerr << d << std::endl;
         //   std::cerr << Pinv33[i] << " vs\n\t " << Pinv.segment<VECTORSIZE>(i*VECTORSIZE).transpose() << std::endl;
        }
    }

    for(S_t::fixed_t::const_iterator it = S.vtxFixed.begin(); it != S.vtxFixed.end(); ++it)
        Pinv33[*it].setIdentity();

    // S' = I - S => S = I - S'
    // inv ( S C + I - S) = inv ( C - S'C + I - I + S') = inv( C - S'C + S')
    for(S_t::const_iterator it = S.begin(); it != S.end(); it++)
    {
        int ivtx = it->idx;
        const Pinv33_t sm = CASTUP(it->mat).transpose();
        const Pinv33_t C = CASTUP(A[ivtx]);
        Pinv33[ivtx] = (C - sm*C + sm).inverse();
    }


    // Ascher uses b* = S(b-A(I-S)z) ) => filter(b - A ifilter(z))
    // Otherwise b* = S(b)
    // d0 = filter(b)' P filter(b)
    // where P is diagonal of the Sparse_K
    Scalar d0,d0a,d0b,d_new;

    logger.pcg(SimLogger::PcgStart,0,S.size());

    {
        // S(b - A(I-S)z)' P^-1
        // when z is zero due to no collisions => S(b) inv(P) S(b)

        ifilter(filterpar,tmp,S,x);
        spmv(par,idx,A,tmp,tmp2);
        submp(par,b_filt,b,tmp2); // b_filt = S(b - A (I-S)z)
        filter(filterpar,S,b_filt);
        PinvProd(tmp,Pinv33,b_filt);
        filter(filterpar,S,tmp);
        dotmp(dotpar,d0a,b_filt,tmp); // b_filt' * tmp = b_filt' * P^-1 * b_filt
    }
    const Scalar threshold = fabs(d0a * tol);

    spmv(par,idx,A,x,tmp);
    submp(par,r,b,tmp);
    filter(filterpar,S,r);  // r = S(b-Ax)

    // c = S (P^-1 r) = P^-1 r because r is filtered and P^-1 is diagonal
    //cwisemp(par,c,Pinv,r);     // c = S(P^-1 * r) = S(P^-1 S(b-Ax))
    PinvProd(c,Pinv33,r);
    filter(filterpar,S,c);
    dotmp(dotpar,d_new,r,c);      // d_new = dot(r,c) = filter(S,b-Ax)' * P^-1 * filter(S, b-Ax) = sum (x_i)^2 P^-1



    //std::cerr << "x=" << x.dot(x) << " b=" << b.dot(b) << " bf=" << b_filt.dot(b_filt) << " r=" << r.dot(r) << " c=" << c.dot(c) << " x=" << x.dot(x) << " Pinv" << Pinv.dot(Pinv) << " P " << P.dot(P) << " tmp " << tmp.dot(tmp) << " tmp2 " << tmp2.dot(tmp2) << std::endl;
    //rstd::cerr << "PCGNoPar d_new:" << d_new << "  d0:" << d0 << " tol:" << tol << " th:" << threshold << std::endl;
    logger.pcg(SimLogger::PcgFirstStep,0,d_new);
    logger.pcg(SimLogger::PcgThreshold,0,threshold);
    logger.pcg(SimLogger::PcgThresholdBase,0,0*tol);
    logger.pcg(SimLogger::PcgThresholdAsc,0,d0a*tol);

    if(isinf(d_new))
    {
        logger.pcg(SimLogger::PcgEndFlag,-1,5);
        logger.pcg(SimLogger::PcgEnd,-1,0);
        std::cerr << "inf d_new\n";
        return 0;
    }

    if(min_it == 0 && d_new <= threshold)
    {
        lastDelta = d_new-threshold; //not used till now
        logger.pcg(SimLogger::PcgEnd,0,lastDelta);
        return 0;
    }

    int i;
    int badflag = 0;
    for (i = 0; i < min_it || i < max_it; ++i)
    {
        logger.pcg(SimLogger::PcgBeginStep,i,0);
        logger.pcg(SimLogger::PcgSPVM,i,0);

        spmv(par,idx,A,c,q); // q = filter(A * c)

        logger.pcg(SimLogger::PcgSPVMEnd,i,0);

        filter(filterpar,S,q);

        Scalar rho;
        dotmp(dotpar,rho,c,q);
        if(rho == 0 || isinf(rho))
        {
            badflag = 1;
            logger.pcg(SimLogger::PcgEndStep,i,d_new);
            break;
        }

        Scalar alpha = d_new / rho; // alpha = d_new / dot(c,q)
        if(alpha == 0 || isinf(alpha))
        {
            badflag = 2;
            logger.pcg(SimLogger::PcgEndStep,i,d_new);
            break;
        }

        x += alpha*c;
        r -= alpha*q;
        PinvProd(s,Pinv33,r);

        Scalar d_old = d_new;
        dotmp(dotpar,d_new,r,s);

        if(d_new <= 0 || isinf(d_new))
        {
            badflag = 3;
            logger.pcg(SimLogger::PcgEndStep,i,d_new);
            break;
        }

        if(d_old-d_new < dtol)
        {
            badflag = 5;
            logger.pcg(SimLogger::PcgEndStep,i,d_new);
            break;
        }

        if(i >= min_it && d_new <= threshold)
        {
            logger.pcg(SimLogger::PcgEndStep,i,d_new);
            break;
        }
//        std::cerr << "\td_new "<< d_new << std::endl;

        Scalar beta = d_new / d_old;
        if(isinf(beta))
        {
            badflag = 4;
            logger.pcg(SimLogger::PcgEndStep,i,d_new);
            break;
        }
        c = s + beta*c;
        filter(filterpar,S,c);
        logger.pcg(SimLogger::PcgEndStep,i,d_new);
    }
    lastDelta = d_new-threshold; //not used till now
    logger.pcg(SimLogger::PcgEndFlag,i,badflag);
    logger.pcg(SimLogger::PcgEnd,i,lastDelta);
    if(badflag)
        std::cerr << "pcg badflag " << badflag << std::endl;

   // spmv(par,idx,A,x,tmp);     // tmp = Ax
    //submp(par,r,b,tmp);        // r = S(b-Ax)
  //  filter(filterpar,S,r);
  //  Scalar dfres;
//    dotmp(dotpar,dfres,r,r);
//    std::cerr << "PCG: res:" << dfres << " thr:" << threshold << " tol: " << tol << " fin/start " << (dfres/d0res) << " iter:" << i << " error " << badflag<< std::endl;
    return i;
}


#endif

// pre-conditioned conjugate gradient
// TODO: in the future remove these allocations using a PCG class
//template<class SparseIdxA>
int PCG::pcg(ParTaskTag xpar, const VectorX& P, const SparseIdx& idx, const SparseData& A, const VectorX& b, VectorX& x, const Scalar tol, const int min_it, const int max_it, const S_t& S, bool ascherbd)
{
    NoParTag par;
    NoParTag dotpar; // dot product is single processor: DOES NOT WORK IN PARALLEL
    NoParTag filterpar; // filter is single processor: IMPACTS NEGATIVELY TO USE PARALLEL

    qDebug() << "PCG::pcg ParTaskTag";

    // Ascher uses b* = S(b-A(I-S)z) ) => filter(b - A ifilter(z))
    // Otherwise b* = S(b)
    // d0 = filter(b)' P filter(b)
    // where P is diagonal of the Sparse_K
    Scalar d0,d0a,d0b,d_new;

    logger.pcg(SimLogger::PcgStart,0,S.size());
    invmp(par,Pinv,P);
    const VectorX & Pthreshold = Pinv;

    {
        // S(b - A(I-S)z)' P^-1
        // when z is zero due to no collisions => S(b) inv(P) S(b)
        ifilter(filterpar,tmp,S,x); // tmp = (I-S)z
        spmv(par,idx,A,tmp,tmp2); // tmp2 = A tmp
        submp(par,b_filt,b,tmp2); // b_filt = S(b - tmp2)
        filter(filterpar,S,b_filt);
        cwisemp(par,tmp,Pthreshold,b_filt); // tmp = Pinv * b_filt
        dotmp(dotpar,d0a,b_filt,tmp); // b_filt' * tmp
    }
    {
        // (Sb)' P (Sb)
        filter(filterpar,b_filt,S,b);    // b_filter = filter(S,b)
        cwisemp(par,tmp,Pthreshold,b_filt); // tmp = P .* b_filt
        dotmp(dotpar,d0b,b_filt,tmp);  // d0 = b_filt' tmp = b_filt' (P .* b_filt) = filter(S,b)' P filter (S,b)
    }
    d0 = ascherbd ? d0a : d0b;
    const Scalar threshold = d0 * tol;

    spmv(par,idx,A,x,tmp);     // tmp = Ax
    submp(par,r,b,tmp);        // r = S(b-Ax)
    filter(filterpar,S,r);
    // c = S (P^-1 r) = P^-1 r because r is filtered and P^-1 is diagonal
    cwisemp(par,c,Pinv,r);     // c = S(P^-1 * r) = S(P^-1 S(b-Ax))
    filter(filterpar,S,c);
    dotmp(dotpar,d_new,r,c);      // d_new = dot(r,c) = filter(S,b-Ax)' * P^-1 * filter(S, b-Ax) = sum (x_i)^2 P^-1


    //std::cerr << "x=" << x.dot(x) << " b=" << b.dot(b) << " bf=" << b_filt.dot(b_filt) << " r=" << r.dot(r) << " c=" << c.dot(c) << " x=" << x.dot(x) << " Pinv" << Pinv.dot(Pinv) << " P " << P.dot(P) << " tmp " << tmp.dot(tmp) << " tmp2 " << tmp2.dot(tmp2) << std::endl;
    //rstd::cerr << "PCGNoPar d_new:" << d_new << "  d0:" << d0 << " tol:" << tol << " th:" << threshold << std::endl;
    logger.pcg(SimLogger::PcgFirstStep,0,d_new);
    logger.pcg(SimLogger::PcgThreshold,0,threshold);
    logger.pcg(SimLogger::PcgThresholdBase,0,d0b*tol);
    logger.pcg(SimLogger::PcgThresholdAsc,0,d0a*tol);

    if(isinf(d_new))
    {
        logger.pcg(SimLogger::PcgEndFlag,-1,5);
        logger.pcg(SimLogger::PcgEnd,-1,0);
        std::cerr << "inf d_new\n";
        return 0;
    }

    if(min_it == 0 && d_new <= threshold)
    {
        lastDelta = d_new-threshold; //not used till now
        logger.pcg(SimLogger::PcgEnd,0,lastDelta);
        return 0;
    }

    int i;
    int badflag = 0;
    for (i = 0; i < min_it || i < max_it; ++i)
    {
        logger.pcg(SimLogger::PcgBeginStep,i,0);
        logger.pcg(SimLogger::PcgSPVM,i,0);
        spmv(par,idx,A,c,q); // q = filter(A * c)
        logger.pcg(SimLogger::PcgSPVMEnd,i,0);
        filter(filterpar,S,q);
        Scalar rho;
        dotmp(dotpar,rho,c,q);
        if(rho == 0 || isinf(rho))
        {
            badflag = 1;
            logger.pcg(SimLogger::PcgEndStep,i,d_new);
            break;
        }

        Scalar alpha = d_new / rho; // alpha = d_new / dot(c,q)
        if(alpha == 0 || isinf(alpha))
        {
            badflag = 2;
            logger.pcg(SimLogger::PcgEndStep,i,d_new);
            break;
        }

        int doexit = 0;
        {
            //#pragma omp task firstprivate
            {
                x += alpha*c;
            }
            //#pragma omp task firstprivate
            {
                r -= alpha*q;
                s = Pinv.cwiseProduct(r);    // s = r ./ P
                //s = r.cwise() / P;
                Scalar d_old = d_new;
                dotmp(dotpar,d_new,r,s); // d_new = dot(r,s)

                if(d_new <= 0 || isinf(d_new))
                {
                    badflag = 3;
                    logger.pcg(SimLogger::PcgEndStep,i,d_new);
                    doexit = 1;
                }
                else
                {
                    if(i >= min_it && d_new <= threshold)
                    {
                        logger.pcg(SimLogger::PcgEndStep,i,d_new);
                        doexit = 1;

                    }
                    else
                    {
                        Scalar beta = d_new / d_old;
                        if(isinf(beta))
                        {
                            badflag = 4;
                            logger.pcg(SimLogger::PcgEndStep,i,d_new);
                            doexit = 1;
                        }
                        c = s + beta*c;
                        filter(filterpar,S,c);
                        logger.pcg(SimLogger::PcgEndStep,i,d_new);
                    }
                }
            }
            #pragma omp taskwait
            if(doexit == 1)
                break;
        }
    }
    lastDelta = d_new-threshold; //not used till now
    logger.pcg(SimLogger::PcgEndFlag,i,badflag);
    logger.pcg(SimLogger::PcgEnd,i,lastDelta);
    if(badflag)
        std::cerr << "pcg badflag " << badflag << std::endl;
    return i;
}
// pre-conditioned conjugate gradient
// TODO: in the future remove these allocations using a PCG class
//template<class SparseIdxA>
int PCG::pcg(ParTag par, const VectorX& P, const SparseIdx& idx, const SparseData& A, const VectorX& b, VectorX& x, const Scalar tol, const int min_it, const int max_it, const S_t& S, bool ascherbd)
{
    const int n_op = P.size();
    NoParTag dotpar; // dot product is single processor: DOES NOT WORK IN PARALLEL
    NoParTag filterpar; // filter is single processor: IMPACTS NEGATIVELY TO USE PARALLEL

    qDebug() << "PCG::pcg ParTag";

    // Ascher uses b* = S(b-A(I-S)z) ) => filter(b - A ifilter(z))
    // Otherwise b* = S(b)
    // d0 = filter(b)' P filter(b)
    // where P is diagonal of the Sparse_K
    Scalar d0,d0a,d0b,d_new;

    logger.pcg(SimLogger::PcgStart,0,S.size());
    invmp(par,Pinv,P);
    const VectorX & Pthreshold = Pinv;

    {
        // S(b - A(I-S)z)' P^-1
        // when z is zero due to no collisions => S(b) inv(P) S(b)
        ifilter(filterpar,tmp,S,x); // tmp = (I-S)z
        spmv(par,idx,A,tmp,tmp2); // tmp2 = A tmp
        submp(par,b_filt,b,tmp2); // b_filt = S(b - tmp2)
        filter(filterpar,S,b_filt);
        cwisemp(par,tmp,Pthreshold,b_filt); // tmp = Pinv * b_filt
        dotmp(dotpar,d0a,b_filt,tmp); // b_filt' * tmp
    }
    {
        // (Sb)' P (Sb)
        filter(filterpar,b_filt,S,b);    // b_filter = filter(S,b)
        cwisemp(par,tmp,Pthreshold,b_filt); // tmp = P .* b_filt
        dotmp(dotpar,d0b,b_filt,tmp);  // d0 = b_filt' tmp = b_filt' (P .* b_filt) = filter(S,b)' P filter (S,b)
    }
    d0 = ascherbd ? d0a : d0b;
    const Scalar threshold = d0 * tol;

    spmv(par,idx,A,x,tmp);     // tmp = Ax
    submp(par,r,b,tmp);        // r = S(b-Ax)
    filter(filterpar,S,r);
    // c = S (P^-1 r) = P^-1 r because r is filtered and P^-1 is diagonal
    cwisemp(par,c,Pinv,r);     // c = S(P^-1 * r) = S(P^-1 S(b-Ax))
    filter(filterpar,S,c);
    dotmp(dotpar,d_new,r,c);      // d_new = dot(r,c) = filter(S,b-Ax)' * P^-1 * filter(S, b-Ax) = sum (x_i)^2 P^-1


    //std::cerr << "x=" << x.dot(x) << " b=" << b.dot(b) << " bf=" << b_filt.dot(b_filt) << " r=" << r.dot(r) << " c=" << c.dot(c) << " x=" << x.dot(x) << " Pinv" << Pinv.dot(Pinv) << " P " << P.dot(P) << " tmp " << tmp.dot(tmp) << " tmp2 " << tmp2.dot(tmp2) << std::endl;
    //rstd::cerr << "PCGNoPar d_new:" << d_new << "  d0:" << d0 << " tol:" << tol << " th:" << threshold << std::endl;
    logger.pcg(SimLogger::PcgFirstStep,0,d_new);
    logger.pcg(SimLogger::PcgThreshold,0,threshold);
    logger.pcg(SimLogger::PcgThresholdBase,0,d0b*tol);
    logger.pcg(SimLogger::PcgThresholdAsc,0,d0a*tol);

    if(isinf(d_new))
    {
        logger.pcg(SimLogger::PcgEndFlag,-1,5);
        logger.pcg(SimLogger::PcgEnd,-1,0);
        return 0;
    }

    if(min_it == 0 && d_new <= threshold)
    {
        lastDelta = d_new-threshold; //not used till now
        logger.pcg(SimLogger::PcgEnd,0,lastDelta);
        return 0;
    }

    int i;
    int badflag = 0;
    for (i = 0; i < min_it || i < max_it; ++i)
    {
        logger.pcg(SimLogger::PcgBeginStep,i,0);
        logger.pcg(SimLogger::PcgSPVM,i,0);
        spmv(par,idx,A,c,q); // tmp = A * c
        logger.pcg(SimLogger::PcgSPVMEnd,i,0);
        filter(filterpar,S,q); // q = filter(A*c)
        Scalar rho;
        dotmp(dotpar,rho,c,q);      // rho = dot(c,q)
        if(rho == 0 || isinf(rho))
        {
            badflag = 1;
            logger.pcg(SimLogger::PcgEndStep,i,d_new);
            break;
        }

        Scalar alpha = d_new / rho;
        if(alpha == 0 || isinf(alpha))
        {
            badflag = 2;
            logger.pcg(SimLogger::PcgEndStep,i,d_new);
            break;
        }

        // from now on we can parallelize
        #pragma omp parallel for
        for(int j = 0; j < n_op; j++)
        {
            x(j) += alpha * c(j);
            r(j) -= alpha * q(j);
            s(j) = r(j) * Pinv(j);
        }
        filter(filterpar,S,s);

        Scalar d_old = d_new;
        dotmp(dotpar,d_new,r,s);
        if(d_new == 0 || isinf(d_new))
        {
            badflag = 3;
            logger.pcg(SimLogger::PcgEndStep,i,d_new);
            break;
        }
        if(d_new <= threshold && i >= min_it )
        {
            lastDelta = d_new-threshold; //not used till now
            logger.pcg(SimLogger::PcgEndStep,i,d_new);
            logger.pcg(SimLogger::PcgEnd,i,lastDelta);
            return i;
        }

        Scalar beta = d_new / d_old;
        if(isinf(beta))
        {
            badflag = 4;
            logger.pcg(SimLogger::PcgEndStep,i,d_new);
            break;
        }

        #pragma omp parallel for
        for(int j = 0; j < n_op; j++)
            c(j) = s(j) + beta * c(j);
        logger.pcg(SimLogger::PcgEndStep,i,d_new);

    }
    lastDelta = d_new-threshold; //not used till now    
    logger.pcg(SimLogger::PcgEndFlag,i,badflag);
    logger.pcg(SimLogger::PcgEnd,i,lastDelta);

    return i;
}



// pre-conditioned conjugate gradient
// TODO: in the future remove these allocations using a PCG class
//template<class SparseIdxA>
int PCG::pcg(InParTag par,const VectorX& P, const SparseIdx& idx, const SparseData& A, const VectorX& b, VectorX& x, const Scalar tol, const int min_it, const int max_it, const S_t& S, bool ascherbd)
{
    const int n_op  = b.size();

    qDebug() << "PCG::pcg InParTag";

    invmp(par,Pinv,P);
    const VectorX & Pthreshold = Pinv;

    filter(par,b_filt,S,b);
    cwisemp(par,tmp,Pthreshold,b_filt);

    // TODO: multiplication can be parallelized
    Scalar d0,d_new;
    dotmp(par,d0, b_filt,tmp); // d0 = filter(b)' P filter(b)
    Scalar threshold = d0 * tol;

    // CHECK initialization of x: IT WAS x = filter(x)
    spmv(par,idx, A, x, tmp);
    submp(par,r,b,tmp);

    filter(par,S,r); // r = filter(b-Ax)
    cwisemp(par,c,Pinv,r);
    filter(par,S,c);
    dotmp(par,d_new,r,c);

    int i;
    for (i = 0; i < min_it || (i < max_it && d_new > threshold); ++i)
    {
        // TODO add filter in SPVM
        spmv(par,idx,A,c,q);
        filter(par,S,q); // q = filter(A*c)
        Scalar rho;
        dotmp(par,rho,c,q);
        Scalar alpha = d_new / rho;

        #pragma omp for
        for(int j = 0; j < n_op; j++)
        {
            x(j) += alpha * c(j);
            r(j) -= alpha * q(j);
            s(j) = r(j) * Pinv(j);
        }
        filter(par,S,s);

        Scalar d_old = d_new;

        dotmp(par,d_new,r,s);

        Scalar beta = d_new/d_old;

        #pragma omp for
        for(int j = 0; j < n_op; j++)
            c(j) = s(j) + beta * c(j);
    }
    lastDelta = d_new-threshold; //not used till now

    return i;
}


//int PCG::pcg<SparseIdx>(NoParTag,const VectorX& P, const SparseIdx& idx, const SparseData& A, const VectorX& b, VectorX& x, const Scalar tol, const int min_t, const int max_it, const S_t& S, bool ascherbd);
//int PCG::pcg<SparseIdx>(ParTag,const VectorX& P, const SparseIdx& idx, const SparseData& A, const VectorX& b, VectorX& x, const Scalar tol, const int min_t, const int max_it, const S_t& S, bool ascherbd);
//int PCG::pcg<SparseIdx>(InParTag,const VectorX& P, const SparseIdx& idx, const SparseData& A, const VectorX& b, VectorX& x, const Scalar tol, const int min_t, const int max_it, const S_t& S, bool ascherbd);


void S_t::updateSMat()
{
    Eigen::Matrix<Scalar,9,1> id;
    id << 1,0,0,0,1,0,0,0,1;
    for(int i = 0; i < n_ver; i++)
        Smat.segment<9>(i*3) = id;

    for(int i = 0; i < vtxFixed.size(); i++)
        Smat.segment<9>(vtxFixed[i]*3).setZero();

    // TODO
    //for(int j = 0; j < size(); j++)
    //    Smat.block<3,3>(3*getIndex(j)) -= getMatrix(j);
}
