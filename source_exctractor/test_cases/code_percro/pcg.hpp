/*
 * Onwer: PERCRO, Scuola Superiore S.Anna
 * License: Internal Code, Not Licensed
 *
 * Authors: initial Paolo Gasparello, then Emanuele Ruffaldi, Paolo Tripicchio
 * Descripotion: Preconditioned Conjugate Gradient Method
 */
#pragma once
#include <Eigen/Core>
#include <Eigen/Dense>
//#include <Eigen/Sparse>
#include <set>
#include <map>

#include "types.hpp"


/**
 * Representation of constraints on vertices allowing both to support re-ordering of vertices
 *
 * Requires two operations: iterate and containment
 *
  TODO: optionally use vector<bool> for relatively small sizes
 */
struct VertexConstraints
{
    void reserve(int n)
    {
        vtxFixed.reserve(n);
    }

    void add(int j)
    {
        vtxFixedSet.insert(j);
        vtxFixed.push_back(j);
    }

    bool contains(int i) const
    {
        return vtxFixedSet.find(i) != vtxFixedSet.end();
    }

    void removelast()
    {
        if(!vtxFixed.empty())
        {
            int last = vtxFixed.back();
            vtxFixed.pop_back();
            vtxFixedSet.erase(last);
        }
    }

    int size() const { return vtxFixed.size(); }

    void clear()  { vtxFixed.clear(); vtxFixedSet.clear(); }

    typedef std::vector<int>::const_iterator const_iterator;

    int operator [] (unsigned int i ) const { return vtxFixed[i]; }

    const_iterator begin() const { return vtxFixed.begin(); }
    const_iterator end() const { return vtxFixed.end(); }

    std::set<int> vtxFixedSet;
    std::vector<int> vtxFixed;
};

/**
 * Constaint and Virtual Mass Management
 * Manages 3x3 matrices with a preference for identities
 */
class S_t
{
public:
    S_t(unsigned int an_ver) /*:  S(n_ver*3)*/
        : n_ver(an_ver),Smat(an_ver*9)
    {
        SM.reserve(n_ver);
        clear(); // remove the collisions
    }

    void clear()
    {
        SMmap.clear();
        SM.resize(0); // DO NOTHING

        //S.setOnes();  // all free
        mins.setZero();
        maxs.setZero();
    }


    void updateSMat();

    // setZero do nothing because
    void setZero(int i) {
        /*S.segment<3>(3 * i) = Vector3::Zero();*/
    }

    void setOnes(int i) {
        /*S.segment<3>(3 * i) = Vector3::Ones();*/
    }

    void setFree(int i ) { setOnes(i); }

    void setConstrained(int i ) { setZero(i); }


    struct matpt
    {
        Matrix3 mat; // constraint matrix
        int idx;     // index of vertex
        int x;     // location in grid
        int y;     // location in grid
        Vector3 pt;  // point of contact
        Vector3 N;   // direction of contact (for tangential computation)
        int count;   // number of overlapping
        Vector3 v;   // velocity
        int objects;
        bool sticked;

        matpt(const Matrix3 & m, int aidx, int ax, int ay, const Vector3 & apt, const Vector3 & aN, const Vector3 &aV , int ao, int asticked):
                mat(m), idx(aidx), x(ax), y (ay), pt(apt),N(aN),count(1),v(aV),objects(ao),sticked(asticked) {}
        matpt() {}
    };
    typedef std::vector<matpt>::const_iterator const_iterator;
    typedef std::vector<matpt>::iterator iterator;

    void push_back(const matpt & pp)
    {
        int cx = pp.x;
        int cy = pp.y;
        if(SM.size() == 0)
        {
            maxs(0) = cx;
            maxs(1) = cy;
            maxs(0) = cx;
            maxs(1) = cy;
        }
        else
        {
            maxs(0) = std::max(cx,maxs(0));
            maxs(1) = std::max(cy,maxs(1));
            mins(0) = std::max(cx,mins(0));
            mins(1) = std::max(cy,mins(1));
        }
        SM.push_back(pp);
        SMmap[pp.idx] = SM.begin()+(SM.size()-1);        
    }

    // returns if vertes is mapped
    iterator getVertex(int ivtx)
    {
        std::map<int,iterator>::const_iterator it = SMmap.find(ivtx);
        if(it == SMmap.end())
            return end();
        else
            return it->second;
    }

    const_iterator begin() const { return SM.begin(); }

    const_iterator end() const { return SM.end(); }

    iterator begin() { return SM.begin(); }

    iterator end() { return SM.end(); }

    const Matrix3 & getMatrix(int i) const { return SM[i].mat; }

    int getIndex(int i) const { return SM[i].idx; }

    //const VectorX & getS() const { return S; }

    const matpt & getEntry(int idx) const  { return SM[idx]; }

    int size() const { return SM.size(); }

    const VectorX & getSMat() const { return Smat; }

    const Eigen::Vector2i  & getMinBound() const { return mins; }

    const Eigen::Vector2i  & getMaxBound() const { return maxs; }

    typedef VertexConstraints  fixed_t;
    fixed_t vtxFixed;

private:
    int n_ver;
    std::vector< matpt > SM;
    std::map< int,iterator > SMmap;
    VectorX Smat; // ones or zeros depending on collision
    Eigen::Vector2i mins,maxs;
};

class PCG
{
public:
    PCG(unsigned int size, SimLogger & alogger) :
            Pinv(VectorX::Zero(size)),
            b_filt(VectorX::Zero(size)),
            tmp(VectorX::Zero(size)),
            tmp2(VectorX::Zero(size)),
            r(VectorX::Zero(size)),
            c(VectorX::Zero(size)),
            s(VectorX::Zero(size)),
            q(VectorX::Zero(size)),
            Pinv33(size/VECTORSIZE),
            lastDelta(0),
            logger(alogger)
    {}

    /// Preconditioned Conjugate Gradient Method
    //template<class SparseIdxA>
    int pcg(InParTag,const VectorX& P, const SparseIdx& idx, const SparseData& A, const VectorX& b, VectorX& x, const Scalar tol, const int min_t, const int max_it, const S_t& S, bool ascherbd);

    /// Preconditioned Conjugate Gradient Method
    //template<class SparseIdxA>
    int pcg(ParTag,const VectorX& P, const SparseIdx& idx, const SparseData& A, const VectorX& b, VectorX& x, const Scalar tol, const int min_t, const int max_it, const S_t& S, bool ascherbd);

    /// Preconditioned Conjugate Gradient Method
    //template<class SparseIdxA>
    int pcg(NoParTag,const VectorX& P, const SparseIdx& idx, const SparseData& A, const VectorX& b, VectorX& x, const Scalar tol, const Scalar dtol, const int min_t, const int max_it, const S_t& S, bool ascherbd);

    /// Preconditioned Conjugate Gradient Method
    //template<class SparseIdxA>
    int pcg(NoParTag,const SparseIdx& idx, const SparseData& A, const VectorX& b, VectorX& x, const Scalar tol, const Scalar dtol, const int min_t, const int max_it, const S_t& S, bool ascherbd);

    /// Preconditioned Conjugate Gradient Method
    //template<class SparseIdxA>
    int pcg(ParTaskTag,const VectorX& P, const SparseIdx& idx, const SparseData& A, const VectorX& b, VectorX& x, const Scalar tol, const int min_t, const int max_it, const S_t& S, bool ascherbd);

    /// Preconditioned Conjugate Gradient Method
    //template<class SparseIdxA>
    int pcg33(NoParTag, const VectorX& P, const SparseIdx& idx, const SparseData& A, const VectorX& b, VectorX& x, const Scalar tol, const Scalar dtol,  const int min_t, const int max_it, const S_t& S, bool ascherbd);

    VectorX Pinv;
    typedef Matrix3 Pinv33_t;
    std::vector<Pinv33_t> Pinv33;
    VectorX b_filt;
    VectorX tmp,tmp2;
    VectorX r,c,s,q;
    Scalar lastDelta;
    SimLogger & logger;
};

/**
 * filter can be invoked with r = x because:
 *      fixed points: x gets zeroed and not touched by second loop
 *      other points: x is multiplied by one and looped
 *
 * TODO: embed fixed into S given their small number and zero the vector for them keeping the others
 */
void filter(NoParTag tag,const S_t & S,VectorX & r);
void filter(ParTag tag,const S_t & S,VectorX & r);
void filter(InParTag tag,const S_t & S,VectorX & r);

void filter(NoParTag tag,VectorX & r,const S_t & S, const VectorX & x);
void filter(ParTag tag,VectorX & r,const S_t & S, const VectorX & x);
void filter(InParTag tag,VectorX & r,const S_t & S, const VectorX & x);

void ifilter(NoParTag tag,VectorX & r,const S_t & S, const VectorX & x);
void ifilter(ParTag tag,VectorX & r,const S_t & S, const VectorX & x);
void ifilter(InParTag tag,VectorX & r,const S_t & S, const VectorX & x);
