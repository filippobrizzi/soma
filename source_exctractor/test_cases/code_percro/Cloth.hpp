/*
 * Onwer: PERCRO, Scuola Superiore S.Anna
 * License: Internal Code, Not Licensed
 *
 * Authors: initial Paolo Gasparello, then Emanuele Ruffaldi, Paolo Tripicchio
 * Descripotion: Cloth Class
 */
#pragma once
#include <Eigen/Core>
//#include <Eigen/Sparse>
#include <Eigen/Geometry>
#include <vector>
#include <set>

#include "types.hpp"
#include "pcg.hpp"
#include "viewer.h"

#ifdef HAS_OPENMP
#include <omp.h>
#endif

extern "C"
{
/*
typedef struct omp_lock_t omp_lock_t;
*/
extern void omp_set_lock(omp_lock_t*);
extern void omp_unset_lock(omp_lock_t*);
extern void omp_init_lock(omp_lock_t*);
}

class QMutex
{
public:
    void lock(){}
    void unlock(){}

};
class QTime
{
public:
    void start() {}
    void stop() {}
    double restart() { return 0; }
    bool isNull() { return true; }
    double elapsed() const { return 0; }
};


class Triangle;
class Cloth;

class Cylinder
{
public:
    Vector3 HAX;    //half axis
    Vector3 U_HAX;  //half axis versor
    Vector3 orig;   //center of the cylinder
    Scalar extent;  //half extent along the axis
    Scalar radius;  //radius of cylinder

    void Project(Scalar & min,Scalar & max, Vector3 DN) const; //projection along vector
};

/*
 VertexGet (triF0,triN): (F0,N,Pm)
 ZeroMem: sparse_k
 SparseK(Pm,triQb,triQs): sparse_k
 PreSPVM((damp - h)*V + X - Y): tmpSPVM2
 SPVM(sparse_k,tmpSPVM2):tmpSPVM
 ExternalForces(): F
 ComputeB(F,tmpSPVM,F0): b
 ScaleSparseK(sparse_k):sparse_k
 UpdateP(sparse_k): sparsek_,P
 PCG (P,sparse_k,b): deltaV
 ComputeForces(sparse_k,deltaV,b): Fr
 UpdateXV(deltaV,X,V,Y): X,V
 UpdateShells(X): triQb,triQs,triN,triF0
 */

namespace StepPhases
{
    enum Enum { Start,UpdateShells,VertexGet,TriangleRot,SparseK,PreSPVM,SPVM,ExternalForces,ComputeB,ScaleSparseK,UpdateP,PCG,ComputeForces,UpdateXV,End};
    extern const char *Names[End+1];
}

class Steppable
{
public:

    virtual ~Steppable() {}
    virtual int step(const Scalar h) = 0;
    virtual int collide(const Vector3 pos, Scalar r, const Vector3 vel) = 0;
    virtual int collide2(const Vector3 pos, Scalar r, const Vector3 vel,const Vector3 pos2, Scalar r2, const Vector3 vel2) = 0;
    virtual int collide_sphere_tri(const Vector3 pos, Scalar r, const Vector3 vel) = 0;
    virtual int collide2_sphere_tri(const Vector3 pos, Scalar r, const Vector3 vel,const Vector3 pos2, Scalar r2, const Vector3 vel2) = 0;
    virtual Scalar collide_first_time(const Vector3 pos, Scalar r, const Vector3 vel) = 0;
    virtual Scalar collide_first_time2(const Vector3 pos, Scalar r, const Vector3 vel,const Vector3 pos2, Scalar r2, const Vector3 vel2) = 0;
    virtual int collide_cylinder(const Cylinder & c)= 0;
    virtual void clear_external_forces() = 0;
    virtual int getStep() const = 0;
    virtual int getCollisions() const = 0;
    virtual int getFrictionLocked() const = 0;
    virtual int getIterations() const = 0;
    virtual Scalar getPCGResidual() const = 0;
    virtual int stepone(StepPhases::Enum phase, const Scalar h) = 0;
    virtual VectorType getCollisionResidual()  = 0;
};

/*
 * Stores, in a packed way, all the triangles of a vertex. In this way we are able to
 * iterate for every vertex over all the triangle contributions (F0 and Normal)
 *
 * Note: given the topology, triangles vary from 2 to 8
 *
 TODO: introduce iterators for begin/end
 */
struct VertexTriangles
{
    struct vtxpos
    {
        bool operator < (const vtxpos & q) const { return tri < q.tri; }
        vtxpos() {}
        vtxpos(unsigned int a_tri,unsigned int a_off): tri(a_tri),off(a_off) {}
        unsigned int tri,off;
    };
    typedef vtxpos value_t;

    VertexTriangles(unsigned int n_ver, unsigned int n_tri): vtxStart(n_ver+1),vtxtriList(n_tri*3)
    {
    }

    const vtxpos & gettri(unsigned int j) const { return vtxtriList[j]; }
    unsigned int begin(unsigned int vtx) const { return vtxStart[vtx]; }
    unsigned int end(unsigned int vtx) const { return vtxStart[vtx+1]; }
    unsigned int count(unsigned int vtx) const { return end(vtx)-begin(vtx); }

    void build(unsigned int n_ver, unsigned int n_tri, const Eigen::MatrixXi & connectivity);

protected:
    std::vector<unsigned int> vtxStart;
    std::vector<vtxpos> vtxtriList; // from 2 to 8 per vertex
};




/*
 * Stores, in a packed way, all the triangles of a vertex. In this way we are able to
 * iterate for every vertex over all the triangle contributions (F0 and Normal)
 *
 * Note: given the topology, triangles vary from 2 to 8
 *
 TODO: introduce iterators for begin/end
 */
template <int MAXTRI>
struct VertexTrianglesMax
{
    struct vtxpos
    {
        bool operator < (const vtxpos & q) const { return tri < q.tri; }
        vtxpos() {}
        vtxpos(unsigned int a_tri,unsigned int a_off): tri(a_tri),off(a_off) {}
        unsigned int tri,off;
    };
    struct vtxinfo
    {
        unsigned int count;
        vtxpos values[MAXTRI];

        vtxinfo() : count(0) {}
    };
    enum Count { maxtri = MAXTRI };

    typedef vtxpos value_t;

    VertexTrianglesMax(unsigned int n_ver, unsigned int n_tri): vtxtriList(n_ver)
    {
    }

    const vtxpos & gettri(unsigned int vtx, unsigned int j) const { return vtxtriList[vtx].values[j]; }
    unsigned int count(unsigned int vtx) const { return vtxtriList[vtx].count; }

    void build(unsigned int n_ver, unsigned int n_tri, const Eigen::MatrixXi & connectivity);

protected:
    std::vector<vtxinfo> vtxtriList; // from 2 to 8 per vertex
};




class Cloth: public Steppable
{    
protected:
    friend class ClothCL;
    friend class GLWidget;

    struct triref
    {
        int tri,ab;
        triref(int t,int a): tri(t),ab(a) {}
    };

    //typedef Eigen::SparseMatrix<Scalar, Eigen::RowMajor> SpMatrix;

    typedef unsigned int (*vertexindex_t)(unsigned int x,unsigned int y, unsigned int w, unsigned int h);
    typedef void (*inv_vertexindex_t)(unsigned int idx,unsigned int & x, unsigned int & y, unsigned int w, unsigned int h);

    Configuration & conf; // first member variable
    const int w, h, n_ver, n_tri;
    int n_blocks;
    std::vector<Triangle*> shells;
    typedef Eigen::Matrix<int,3,Eigen::Dynamic,Eigen::ColMajor> connectivity_t; // same as Dynamic,3,[RowMajor]
    connectivity_t connectivity;
    typedef VertexTrianglesMax<8> vertextri_t;

    vertextri_t vtxtri;

    std::vector<MatrixType,Eigen::aligned_allocator<MatrixType> > Pm;
public:
    const connectivity_t & getConnectivity() const { return connectivity; }
    VectorX X,X0; // position of vertices: 3 by Nvertices
    VectorX Fr; // resulting forces

    typedef Eigen::Matrix<Scalar,VECTORSIZE,Eigen::Dynamic,Eigen::ColMajor> Nmatrix_t;
    Nmatrix_t N;
    MatrixX T;
    VectorX F;
    VectorX G;
    VectorX V;
    VectorX M;    
    Scalar lasth;
    VertexConstraints & vtxFixed;

    S_t S;
    int Si;
    void lockS();
    void unlockS();
    const S_t & getS() const { return S;}
    S_t & getSBack() { return S;}
    void swapS();

    VectorX Y; /// imposed position delta
    VectorX Z; /// imposed velocity delta

    const VectorX & getF0() const { return F0; }
    const VectorX & getDeltaV() const { return deltaV; }

protected:
    int Sfront;
    QMutex Smutex;

    // temps
    VectorX stiffedpreb,stiffedpostb,deltaV,Frtmp;
    VectorX F0,P,b;
    int iter; //num of performed iterations
    int curStep; // current step
public:
    int collisions,frictionLocked; //num of collisions
protected:

    SparseData sparse_k;
    //SparseData sparse_kS; S A
    SparseIdx sp_idx;

    typedef SparseIdxELL<9> SparseELL_t;
    SparseELL_t sp_idx9;

    //std::vector<cl_uint> block_ref;
    std::vector<cl_uint> diagonal_ref;
    typedef Eigen::Matrix<int,Eigen::Dynamic,16,Eigen::RowMajor>  sparseblocks_t; // same as 16,Dynamic,[ColMajor]
    sparseblocks_t sparseblocks; //
    PCG pcgier;    
    SimLogger & logger;

public:
    Cloth(Configuration & conf, SimLogger & alogger);
    virtual ~Cloth();

    /// Applies a force at the celle xy with coordinate component c and force f in Newton
    void force(int x, int y, int coordAxis, Scalar f);

    void apply_force(int x, int y,Vector3 f);

    void apply_force(int ivtx,Vector3 f);

    void clear_external_forces();

    /// Performs a Simulation step with given step h
    int step(const Scalar h);

    /// Performs a Simulation step with given step h
    int step(ParTag par, const Scalar h);

    /// Performs a Simulation step with given step h
    int step(NoParTag par, const Scalar h);

    /// Performs a Simulation step with given step h
    int step(InParTag par, const Scalar h);

    /// Performs a Simulation step with given step h
    int step(ParTaskTag par, const Scalar h);

    /// used for analysis in ClothCL
    int stepone(StepPhases::Enum phase, const Scalar h);

    int steponepar(StepPhases::Enum phase, const Scalar h);

    /// Collision against a Sphere with center xyz and radius r
    int collide(const Vector3 pos, Scalar r, const Vector3 vel);
    
    int collide2(const Vector3 pos, Scalar r, const Vector3 vel,const Vector3 pos2, Scalar r2, const Vector3 vel2);

    Scalar collide_first_time2(const Vector3 pos, Scalar r, const Vector3 vel,const Vector3 pos2, Scalar r2, const Vector3 vel2);

    /// Collision between triangles and sphere
    int collide_sphere_tri(const Vector3 pos, Scalar r, const Vector3 vel);

    int collide2_sphere_tri(const Vector3 pos, Scalar r, const Vector3 vel,const Vector3 pos2, Scalar r2, const Vector3 vel2);


    /// get first time for next collision
    Scalar collide_first_time(const Vector3 pos, Scalar r, const Vector3 vel);

    int collide_cylinder(const Cylinder & c);

    int getStep() const { return curStep; }

    void clearForces() { F.setZero(); }

    void updateMaterial();

    int getVertexIndex(unsigned int x, unsigned int y);

    const Configuration & getConfiguration() const { return conf; }

    // OLD APPROACH virtual void substep(StepPhases::Enum, const char * ) {}

    Cloth::vertexindex_t getVertexIndexer() const;

    Cloth::inv_vertexindex_t getInvVertexIndexer() const;

    void initConnectivity();

    void initConstraints();

    void initCoordinates();

    void initShells();

    void initMass();

    void buildAdjacency();

    void saveState(std::ostream & ons) const;

    bool loadState(std::istream & ins, Configuration * override);

    void reset();

    int getCollisions() const { return collisions; }

    int getIterations() const { return iter; }

    inline Scalar getGlobalTolerance() const { return conf.lastTolerance*conf.lastTolerance; }

    VectorType getCollisionResidual() ;

    virtual int getFrictionLocked() const { return frictionLocked; }

    virtual Scalar getPCGResidual() const { return pcgier.lastDelta; }
#ifdef PROFILE_CLOTH
            #define SUBSTEP(x) substep(StepPhases::x,#x);
#else
    #define SUBSTEP(x)
#endif

protected:
    bool computeImpactVelocity(int ivtx, const Vector3 & vp, const Vector3 & vs, const Vector3 & n, Vector3 & dv, Matrix3 & q,bool &sticked);

    void addCollideImpact(int ivtx, const Vector3 & d, const Vector3 & dn, const Vector3 & s, Scalar r, const Vector3 & vs, int objects);

    void collideClear();

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

};
