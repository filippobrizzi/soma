/*
 * Onwer: PERCRO, Scuola Superiore S.Anna
 * License: Internal Code, Not Licensed
 *
 * Authors: initial Paolo Gasparello, then Emanuele Ruffaldi, Paolo Tripicchio
 * Descripotion:
 */
#include <stdexcept>
#include <fstream>
//#include <QTime>
#ifndef USING_PCH
#include "Cloth.hpp"
#include "Triangle.hpp"
#endif
#include "pcg.hpp"
#include "spmv.hpp"
#include "collisionutil.h"

#define SUBFORLOOP 8
//#define SYMMETRIC

#define MAX_TRICONT_PER_BLOCK 8
Scalar closestPtSegmentSegment(Vector3 p1, Vector3 q1, Vector3 p2, Vector3 q2,
                              Scalar &s, Scalar &t, Vector3 &c1, Vector3 &c2);

#undef SUBSTEP
#define SUBSTEP(x) SimLoggerScope sls(logger,(SimLogger::SimState)((int)SimLogger::PhaseBase+(int)StepPhases::x),curStep,h);

class PrePostZ
{
public:
    PrePostZ(const char * a)
    {
     }
};




class PrePostL
{
public:
    static omp_lock_t lock;
    static bool inited;
    QTime t;
    PrePostL(const char * a):x(a)
    {
        init();
        omp_set_lock(&lock);
        std::cerr << a << " enter\n";
         omp_unset_lock(&lock);
         t.start();
     }
    ~PrePostL()
    {
        Scalar e = t.elapsed();
        omp_set_lock(&lock);
        std::cerr << x << " exit(" << e <<")\n";
        omp_unset_lock(&lock);
    }
    void init()
    {
        if(!inited)
        {
            omp_init_lock(&lock);
        }
    }
    const char * x;
};

bool PrePostL::inited = false;
omp_lock_t PrePostL::lock;
typedef PrePostZ PrePost;

static const unsigned short MortonTable256[256] =
{
  0x0000, 0x0001, 0x0004, 0x0005, 0x0010, 0x0011, 0x0014, 0x0015,
  0x0040, 0x0041, 0x0044, 0x0045, 0x0050, 0x0051, 0x0054, 0x0055,
  0x0100, 0x0101, 0x0104, 0x0105, 0x0110, 0x0111, 0x0114, 0x0115,
  0x0140, 0x0141, 0x0144, 0x0145, 0x0150, 0x0151, 0x0154, 0x0155,
  0x0400, 0x0401, 0x0404, 0x0405, 0x0410, 0x0411, 0x0414, 0x0415,
  0x0440, 0x0441, 0x0444, 0x0445, 0x0450, 0x0451, 0x0454, 0x0455,
  0x0500, 0x0501, 0x0504, 0x0505, 0x0510, 0x0511, 0x0514, 0x0515,
  0x0540, 0x0541, 0x0544, 0x0545, 0x0550, 0x0551, 0x0554, 0x0555,
  0x1000, 0x1001, 0x1004, 0x1005, 0x1010, 0x1011, 0x1014, 0x1015,
  0x1040, 0x1041, 0x1044, 0x1045, 0x1050, 0x1051, 0x1054, 0x1055,
  0x1100, 0x1101, 0x1104, 0x1105, 0x1110, 0x1111, 0x1114, 0x1115,
  0x1140, 0x1141, 0x1144, 0x1145, 0x1150, 0x1151, 0x1154, 0x1155,
  0x1400, 0x1401, 0x1404, 0x1405, 0x1410, 0x1411, 0x1414, 0x1415,
  0x1440, 0x1441, 0x1444, 0x1445, 0x1450, 0x1451, 0x1454, 0x1455,
  0x1500, 0x1501, 0x1504, 0x1505, 0x1510, 0x1511, 0x1514, 0x1515,
  0x1540, 0x1541, 0x1544, 0x1545, 0x1550, 0x1551, 0x1554, 0x1555,
  0x4000, 0x4001, 0x4004, 0x4005, 0x4010, 0x4011, 0x4014, 0x4015,
  0x4040, 0x4041, 0x4044, 0x4045, 0x4050, 0x4051, 0x4054, 0x4055,
  0x4100, 0x4101, 0x4104, 0x4105, 0x4110, 0x4111, 0x4114, 0x4115,
  0x4140, 0x4141, 0x4144, 0x4145, 0x4150, 0x4151, 0x4154, 0x4155,
  0x4400, 0x4401, 0x4404, 0x4405, 0x4410, 0x4411, 0x4414, 0x4415,
  0x4440, 0x4441, 0x4444, 0x4445, 0x4450, 0x4451, 0x4454, 0x4455,
  0x4500, 0x4501, 0x4504, 0x4505, 0x4510, 0x4511, 0x4514, 0x4515,
  0x4540, 0x4541, 0x4544, 0x4545, 0x4550, 0x4551, 0x4554, 0x4555,
  0x5000, 0x5001, 0x5004, 0x5005, 0x5010, 0x5011, 0x5014, 0x5015,
  0x5040, 0x5041, 0x5044, 0x5045, 0x5050, 0x5051, 0x5054, 0x5055,
  0x5100, 0x5101, 0x5104, 0x5105, 0x5110, 0x5111, 0x5114, 0x5115,
  0x5140, 0x5141, 0x5144, 0x5145, 0x5150, 0x5151, 0x5154, 0x5155,
  0x5400, 0x5401, 0x5404, 0x5405, 0x5410, 0x5411, 0x5414, 0x5415,
  0x5440, 0x5441, 0x5444, 0x5445, 0x5450, 0x5451, 0x5454, 0x5455,
  0x5500, 0x5501, 0x5504, 0x5505, 0x5510, 0x5511, 0x5514, 0x5515,
  0x5540, 0x5541, 0x5544, 0x5545, 0x5550, 0x5551, 0x5554, 0x5555
};

/*
 Vertex/Triangle Index Functions (used only during the setup phase) can be modified for improving the vertex and triangle
 * allocation in memory for cache coherency during some operations
 */

void inv_rvertexindex(unsigned int idx,unsigned int & x, unsigned int & y, unsigned int w, unsigned int h)
{
    x = idx % w;
    y = idx / w;
}

inline unsigned int rvertexindex(unsigned int x,unsigned int y, unsigned int w, unsigned int h)
{
    return x + y*w;
}

inline unsigned int mvertexindex(unsigned int x,unsigned int y, unsigned int w, unsigned int h)
{
    return MortonTable256[y >> 8]   << 17 |
        MortonTable256[x >> 8]   << 16 |
        MortonTable256[y & 0xFF] <<  1 |
        MortonTable256[x & 0xFF];

}

void inv_mvertexindex(unsigned int idx,unsigned int & x, unsigned int & y, unsigned int w, unsigned int h)
{
    // TODO: make reversed MortonTable256
    x = 0;
    y = 0;
}


inline unsigned int triangleindex(unsigned int x,unsigned int y, unsigned int w, unsigned int h)
{
    return x + y*w;
}


using namespace Eigen;

int Cloth::getVertexIndex(unsigned int x, unsigned int y)
{
    return getVertexIndexer()(x,y,w,h);

}

/**
 * build the triangular patches. In every loop generate two pairs of triangles
 */
void Cloth::initConnectivity()
{
    vertexindex_t vertexindex = getVertexIndexer();

    if(conf.gridLayout)
    {
        for (int y = 0; y < h - 1; ++y)
        for (int x = 0; x < w - 1; ++x) {
                const int idx = 2 * triangleindex(x,y,w-1,h);
                int p[] = {vertexindex(x,y,w,h),vertexindex(x+1,y,w,h),vertexindex(x,y+1,w,h),vertexindex(x+1,y+1,w,h)};
                connectivity.col(idx) = Vector3i(p[0], p[1], p[2]);
                connectivity.col(idx + 1) = Vector3i(p[3], p[2], p[1]);
            }
    }
    else
    {
        for (int y = 0; y < h - 1; ++y)
        for (int x = 0; x < w - 1; ++x) {
                const int idx = 2 * triangleindex(x,y,w-1,h);
                int p[] = {vertexindex(x,y,w,h),vertexindex(x+1,y,w,h),vertexindex(x,y+1,w,h),vertexindex(x+1,y+1,w,h)};
                int even = ((x+y) & 1) == 0;

                if(even)
                {
                    connectivity.col(idx) = Vector3i(p[0], p[1], p[2]);
                    connectivity.col(idx + 1) = Vector3i(p[3], p[2], p[1]);
                }
                else
                {
                    connectivity.col(idx) = Vector3i(p[0], p[1], p[3]);
                    connectivity.col(idx + 1) = Vector3i(p[0], p[3], p[2]);
                }
            }
    }
}




void Cloth::buildAdjacency()
{
    // Build Vertex -> Triangles [Geometry Independent Code]
    vtxtri.build(n_ver,n_tri,connectivity);

    // Build Adjacency [Geometry Independent Code]
    std::vector<std::vector<unsigned> > adj(n_ver);
    for (unsigned i = 0; i < adj.size(); ++i)
        adj[i].reserve(10);
    for (unsigned i = 0; i < adj.size(); ++i)
        adj[i].push_back(i);
    for (int i = 0; i < n_tri; ++i)
        for (int j = 0; j < 3; ++j) {
            const unsigned a = connectivity.col(i)(j);
            const unsigned b = connectivity.col(i)((j + 1) % 3);
            adj[a].push_back(b);
            adj[b].push_back(a);
        }
    for (int i = 0; i < n_ver; ++i) {
        std::sort(adj[i].begin(), adj[i].end());
        std::vector<unsigned>::iterator it = std::unique(adj[i].begin(), adj[i].end());
        adj[i].resize(it - adj[i].begin());
    }

    // Count Matrix Blocks from Adjacency
    // One block for every adjacent of a given vertex (maximum
    n_blocks = 0;
    for (unsigned i = 0; i < adj.size(); ++i)
        n_blocks += adj[i].size();

    // USEELL is ALIGNED => sparse blocks are packed 9*Nver vs 6.75*Nver
    // Otherwise requires extra index for the block as well

#ifdef USEELL
    n_blocks = SparseELL_t::Size*n_ver;
#endif
    sparse_k.resize(n_blocks);

    //std::cout << "Adjacency " << "Total " << n_blocks << " Min " << *std::min_element(sizes.begin(),sizes.end()) << " Max " << *std::max_element(sizes.begin(),sizes.end());

    // In our case: min 3 max 9
    // Every adjacent of a vertex gets a block
    // This means a bout 9 * n_ver
    //
    // block_map(i): adjacent -> block
    std::vector<std::map<unsigned, unsigned> > block_map(n_ver);
    bool moveidentity = true;

    for (int i = 0, k = n_ver; i < adj.size(); ++i)
        for (int j = 0; j < adj[i].size(); ++j)
        {
            int w = adj[i][j];
            if(w == i && moveidentity)
            {
                block_map[i][i] = i; // identity block is mapped to first n_ver
            }
            else
            {
                int iblock;
#ifdef USEELL
            iblock = i*SparseELL_t::Size + block_map[i].size(); // before the assignment!
#else
    #ifdef SYMMETRIC
                // allocate only for upper triangle
                if(i < w)
                    iblock = k++;
                else
                    iblock = block_map[w][i];
    #else
                iblock = k++;
    #endif
#endif
            block_map[i][w] =  iblock;
            }
        }

    // Fill Blocks Pointers
    //std::vector<std::vector<triref> > sparse_kblocks(n_blocks);
    //block_ref.resize(9 * n_tri);
    VectorXi sparse_kblockscount(n_blocks);
    sparse_kblockscount.setZero();
    sparseblocks.resize(n_blocks,8*2);
    sparseblocks.setZero();
    for (int i = 0; i < n_tri; ++i) {
        Vector3i t = connectivity.col(i);
        for (int a = 0; a < 3; ++a)
        {
            //sparse_kblocks[block].push_back(triref(i,3*a+b));
            const std::map<unsigned,unsigned>& mp = block_map[t(a)];
            for (int b = 0; b < 3; ++b)
            {
                std::map<unsigned,unsigned>::const_iterator it = mp.find(t(b));
                int block = block_map.at(t(a))[t(b)];
#ifdef SYMMETRIC
                // store only contributes
                if(t(a) > t(b))
                {
                    //block_ref.at(9 * i + 3 * a + b) = -1;
                    continue;
                }
#endif
                int bcount = sparse_kblockscount(block);
                //block_ref.at(9 * i + 3 * a + b) = block;
                sparseblocks(block,bcount*2) = i+1;
                sparseblocks(block,bcount*2+1) = 4*a+b;
                sparse_kblockscount(block) = bcount+1;
            }
        }
    }

    // Finally fill-in the Sparse Index
    sp_idx.resize(n_blocks, n_ver);

    if(moveidentity)
    {
        sp_idx.outer[0] = n_ver; // skip first n_ver blocks, they are the identity
        for (int i = 0; i < n_ver; ++i) {
            sp_idx.outer[i+1] = sp_idx.outer[i] + adj[i].size()-1; // -1 because we count always the identity in the adj
            std::remove_copy_if(adj[i].begin(), adj[i].end(), sp_idx.inner.begin() + sp_idx.outer[i],  std::bind2nd(std::equal_to<int>(),i));
            diagonal_ref[i] = i;
            std::fill_n(sp_idx.backinner.begin()+sp_idx.outer[i],adj[i].size()-1,i);
        }

    }
    else
    {
        sp_idx.outer[0] = 0; // skip first n_ver blocks, they are the identity
        for (int i = 0; i < n_ver; ++i) {
            sp_idx.outer[i+1] = sp_idx.outer[i] + adj[i].size();
            std::copy(adj[i].begin(), adj[i].end(), sp_idx.inner.begin() + sp_idx.outer[i]);
            std::fill_n(sp_idx.backinner.begin()+sp_idx.outer[i],adj[i].size(),i);
        }
        for (size_t i = 0; i < n_ver; ++i)
            for (size_t j = sp_idx.outer[i]; j < sp_idx.outer[i + 1]; ++j)
                if (sp_idx.inner[j] == i)
                {
                    diagonal_ref[i] = j;
                    break;
                }
    }

    for(int j = 0; j < n_blocks; j++)
    {
        int vi = sp_idx.backinner[j];
        int vj = sp_idx.inner[j];
        if(vi == vj)
            sp_idx.symblock[j] = j;
        else
            sp_idx.symblock[j] = sp_idx.getBlockIndex(vj,vi);
    }
    // diagonal_ref: vtx -> block  IS unique
    //std::vector<std::vector<unsigned> > diagref(n_blocks);

// IN ELL mode the diagonal is effectively not in fixed position, except otherwise implemented

    /*
    */

    //sp_idx9.from(sp_idx);
    //size_t q = 0;
    //for(int i = 0; i < diagref.size(); i++)
        //q = std::max(q,diagref[i].size());
    //std::cout << "diagref " << q << std::endl;
}

/**
 * This is based on the configuration
 */
void Cloth::initConstraints()
{
    vertexindex_t vertexindex = getVertexIndexer();

    // default of S is free, set the constraint of the first top row using Zero
    vtxFixed.clear();
    vtxFixed.reserve(w);

    S_t & S = getSBack();
    if((conf.sideConstraints & 1) != 0)
    {
        for (int i = 0; i < w; ++i)
        {
            int j = vertexindex(i,0,w,h);
            vtxFixed.add(j);
            S.setZero(j);

        }
    }

    if((conf.sideConstraints & 4) != 0)
    {
        for (int i = 0; i < w; ++i)
        {
            int j = vertexindex(i,h-1,w,h);
            vtxFixed.add(j);
            S.setZero(j);
        }
    }

    if((conf.sideConstraints & 2) != 0)
    {
        for (int i = 0; i < h; ++i)
        {
            int j = vertexindex(w-1,i,w,h);
            vtxFixed.add(j);
            S.setZero(j);
        }
    }

    if((conf.sideConstraints & 8) != 0)
    {
        for (int i = 0; i < w; ++i)
        {
            int j = vertexindex(0,i,w,h);
            vtxFixed.add(j);
            S.setZero(j);
        }
    }

    switch(conf.testMode)
    {
        case TestMode::Catenaria:
        {
            Scalar a = conf.catenariaAFactor;
            for (int i = 0; i < w; ++i)
            {
                int j = vertexindex(i,h-1,w,h);
                vtxFixed.add(j);
                S.setZero(j);
                X(VECTORSIZE * j + 2) = a;
            }
            break;
        }
    case TestMode::CenterPick:
        {
            int j = vertexindex(w/2,h/2,w,h);
            vtxFixed.add(j);
            S.setZero(j);
            break;
        }
    }

}

/**
 * Initializes Coordinates and Texture Coordinates.
 *
 * XY or XZ with centering
 */
void Cloth::initCoordinates()
{
    vertexindex_t vertexindex = getVertexIndexer();

    const double dx = conf.dx;
    const double dy = conf.dy;
    double ty = 0;
    const double dty = conf.textureScale/h;
    const double dtx = conf.textureScale/w;
    const double cx = conf.gridCentered ? -conf.width*0.5 : 0.0;
    const double cy = conf.gridCentered ? (conf.gridXZ ? 0.0 : -conf.width*0.5) : 0.0;
    const double cz = conf.gridCentered ? (conf.gridXZ ? conf.height*0.5 : 0.0) : 0.0;
    const double ox = conf.gridOffset(0)+cx;
    const double oy = conf.gridOffset(1)+cy;
    const double oz = conf.gridOffset(2)+cz;

    if(!conf.gridXZ)
    {
        for (int y = 0; y < h; ++y, ty += dty)
        {
            double tx = 0;
            for (int x = 0; x < w; ++x, tx += dtx)
            {
                int vi = vertexindex(x,y,w,h);
                X.segment<VECTORSIZE>(VECTORSIZE * vi) = VectorType(x*dx+ox, -y*dy+oy, oz);
                T(0,vi) = tx;
                T(1,vi) = ty;
            }
        }
    }
    else
    {
        for (int y = 0; y < h; ++y, ty += dty)
        {
            double tx = 0;
            for (int x = 0; x < w; ++x, tx += dtx)
            {
                int vi = vertexindex(x,y,w,h);
                X.segment<VECTORSIZE>(VECTORSIZE * vi) = VectorType(x*dx+ox, oy, -y*dy+oz);
                T(0,vi) = tx;
                T(1,vi) = ty;
            }
        }
    }
}

template <class T>
inline void cleaner(std::vector<T> & x)
{
    for(int i = 0; i < x.size(); i++)
        delete x[i];
    x.clear();
}

// TODO reuse memory
void Cloth::initShells()
{
    Scalar* d = X.data();
    cleaner(shells);
    shells.resize(n_tri+1);
    for (int i = 0; i < n_tri; ++i)
    {
        Vector3i t = connectivity.col(i);
        shells[i] = new Triangle(d + VECTORSIZE * t[0], d + VECTORSIZE * t[1], d + VECTORSIZE * t[2],conf.Material);
        shells[i]->rest_state();
    }
    shells[n_tri] = new Triangle(d,d,d,conf.Material); // single point so it is zero
    shells[n_tri]->setZero();

}

void Cloth::initMass()
{
    const int gravitydir = conf.gravityAxis;

    if(conf.voronoiMass)
    {
        // initializes Mass and Gravity based on shell nodal_mass
        for (int i = 0; i < n_tri; ++i) {
            Vector3i t = connectivity.col(i);
            for (int a = 0; a < 3; ++a) {
                Scalar nm = shells[i]->voronoi_mass(a);
                M.segment<VECTORSIZE>(VECTORSIZE * t(a)) += VectorType::Ones() * nm;
                G(VECTORSIZE * t(a) + gravitydir) += nm*conf.gravity;
            }
        }
    }
    else
    {
        // initializes Mass and Gravity based on shell nodal_mass
        for (int i = 0; i < n_tri; ++i) {
            Vector3i t = connectivity.col(i);
            Scalar nm = shells[i]->nodal_mass();
            for (int a = 0; a < 3; ++a) {
                M.segment<VECTORSIZE>(VECTORSIZE * t(a)) += VectorType::Ones() * nm;
                G(VECTORSIZE * t(a) + gravitydir) += nm*conf.gravity;
            }
        }
    }

    // verification
    Scalar m = 0;
    for (int i = 0; i < n_tri; ++i)
        m += shells[i]->nodal_mass()*3.0;
    if(n_tri > 0)
        std::cout << "M mass " << m << " Area is " << m/shells[0]->mat.mass_density << std::endl;


}

Cloth::Cloth(Configuration & cconf,SimLogger & alogger):
    conf(cconf),
    w(cconf.W), h(cconf.H),
    n_ver(w * h), n_tri(2 * (w - 1) * (h - 1)),
    connectivity(3,n_tri),
    vtxtri(n_ver,n_tri+1),
    Pm(n_ver),
    X(VectorX::Zero(VECTORSIZE* n_ver)),
    X0(VectorX::Zero(VECTORSIZE * n_ver)),
    Fr(VECTORSIZE * n_ver),
    Frtmp(VECTORSIZE * n_ver),
    N(VECTORSIZE, n_ver),
    T(2, n_ver),
    F(VectorX::Zero(VECTORSIZE * n_ver)),
    G(VectorX::Zero(VECTORSIZE * n_ver)),
    V(VectorX::Zero(VECTORSIZE * n_ver)),
    M(VectorX::Zero(VECTORSIZE * n_ver)),
    S(n_ver),
    //S2(n_ver),
    Si(0),
    vtxFixed(S.vtxFixed),
    Y(VectorX::Zero(VECTORSIZE * n_ver)),
    Z(VectorX::Zero(VECTORSIZE * n_ver)),
    stiffedpreb(VectorX::Zero(VECTORSIZE * n_ver)),
    stiffedpostb(VectorX::Zero(VECTORSIZE * n_ver)),
    deltaV(VectorX::Zero(VECTORSIZE * n_ver)),
    F0(VectorX::Zero(VECTORSIZE * n_ver)),
    P(VectorX::Zero(VECTORSIZE * n_ver)),
    b(VectorX::Zero(VECTORSIZE * n_ver)),
    iter(0),
    curStep(0),
    collisions(0),
    frictionLocked(0),
    diagonal_ref(n_ver),
    pcgier(3*n_ver,alogger),
    logger(alogger)
{    

    if(!conf.allocateOnly)
    {
        initCoordinates(); // modifies X
        initConnectivity(); // modifies connectivity
        X0 = X;
        initShells(); // creates shells
        initMass(); // sets M and G
        buildAdjacency(); // build the adjacency
        initConstraints(); // vtxFixed and S.setZero(i)

        std::cerr << "Sparse Matrix: outer " << sp_idx.cols() << " maxouter " << sp_idx.outer[sp_idx.outer.size()-1] << " blocks " << sparse_k.size() << std::endl;
        std::ofstream onf("sparse_k.dat");
        dumpspvm(sp_idx,sparse_k, onf);

        std::ofstream onf2("state.initial.sim",std::ios::binary);
        saveState(onf2);
    }

    std::cerr << "Pm " << (Scalar*)Pm[0].data() << " " << (Scalar*)Pm[1].data() << std::endl;
    std::cerr << "Tolerance is " << conf.pcgTolerance << " => " << getGlobalTolerance() << std::endl;
}

Cloth::~Cloth()
{
}

void Cloth::force(int row, int col, int coordAxis, Scalar f)
{
    if (row > h - 1 || col > w - 1 || coordAxis > 2 || row < 0 || col < 0 || coordAxis < 0)
	throw std::logic_error("Invalid force component");
    F(VECTORSIZE * getVertexIndex(col,row) + coordAxis) = f;
}

void Cloth::apply_force(int row, int col, Vector3 vecf)
{
    if (row > h - 1 || col > w - 1 || row < 0 || col < 0 )
        throw std::logic_error("Invalid component index");
    F.segment<3>(VECTORSIZE * getVertexIndex(col,row)) = vecf;
}


int Cloth::step(const Scalar h)
{
    return step(ParTag(),h);
}


/**
  * OpenMP Task Based
  *
  * In OpenMP3 it is possible to decompose the execution in parallel tasks using better directives than other
  * 1) omp parallel     open the parallel execution
  * 2) omp single       marks part as single tasked
  * 3) omp task         defines the task. Optionally [nowait]
  * 4) omp parallel for inside the task marks
  *
  */
int Cloth::step(ParTaskTag xpar, const Scalar h)
{
    NoParTag par;
    curStep++;
    {
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task
            {
                #pragma omp task
                {
                    // pre contibrution
                    const Scalar damp = conf.damping;
                    #pragma omp parallel for
                    for(int i = 0; i < n_ver*VECTORSIZE/SUBFORLOOP; i++)
                        stiffedpreb.segment<SUBFORLOOP>(i*SUBFORLOOP) = (-h - damp) * V.segment<SUBFORLOOP>(i*SUBFORLOOP) - X.segment<SUBFORLOOP>(i*SUBFORLOOP) - Y.segment<SUBFORLOOP>(i*SUBFORLOOP);
                }
                #pragma omp task
                {
                    // sparse <- qmrot_t <- bend_t <- Pm <- N
                    {
                        #pragma omp parallel for
                        for (int i = 0; i < n_ver; ++i)
                        {
                            const unsigned int count = vertextri_t::maxtri;
                            VectorType n(VectorType::Zero());
                            for (int j = 0; j < count; j++)
                            {
                                const vertextri_t::value_t & w = vtxtri.gettri(i,j);
                                n += shells[w.tri]->get_normal();
                            }
                            n.normalize();
                            N.col(i) = n;
                            Pm[i] = n * n.transpose();
                        }
                    }
                    {
                        #pragma omp parallel for
                        for(int i = 0; i < n_tri; i++)
                        {
                            Vector3i t = connectivity.col(i);
                            const Matrix3 & Pma = Pm[t(0)];
                            const Matrix3 & Pmb = Pm[t(1)];
                            const Matrix3 & Pmc = Pm[t(2)];
                            shells[i]->update_bend(Pma,Pmb,Pmc);
                        }
                    }
                    {
                        #pragma omp parallel for
                        for(int i = 0; i < n_blocks; ++i)
                        {
                            MatrixType mtx;
                            mtx.setZero();
                            for(int j = 0; j < MAX_TRICONT_PER_BLOCK; j++)
                            {
                                int tri = sparseblocks(i,j*2);
                                int ab = sparseblocks(i,j*2+1);
                                if(tri == 0) // zero is ender
                                    break;
                                tri--;
                                int a = ab/4;
                                int b = ab%4;
                                mtx += shells[tri]->QmRot(a,b);
                            }
                            sparse_k[i] = mtx;
                        }
                    }
                }
                // RANDEVOUZ: stiffedpreb and sparse_k
                // compute the post
                #pragma omp taskwait
                {
                    spmv(par,sp_idx, sparse_k, stiffedpreb, stiffedpostb);
                }
            }
            #pragma omp task
            {
                // prepare the B component by F0, using part
                {
                    PrePost top("\tcomputeF0");
                    #pragma omp parallel for
                    for (int i = 0; i < n_ver; ++i)
                    {
                        const unsigned int count = vertextri_t::maxtri;
                        VectorType f0(VectorType::Zero());
                        for (int j = 0; j < count; j++)
                        {
                            const vertextri_t::value_t & w = vtxtri.gettri(i,j);
                            f0 += shells[w.tri]->F0(w.off);
                        }
                        F0.segment<VECTORSIZE>(VECTORSIZE * i) = f0;
                    }
                }
                {
                    PrePost top("\textForces");
                    if(conf.applyExternalForce != 0)
                        conf.applyExternalForce(*this);
                }
                {
                    PrePost top("\tcomputeB1");
                    #pragma omp parallel for
                    // except stiffedpostb
                    for(int i = 0; i < n_ver*VECTORSIZE/SUBFORLOOP; i++)
                        b.segment<SUBFORLOOP>(i*SUBFORLOOP) = h*(F.segment<SUBFORLOOP>(i*SUBFORLOOP) + G.segment<SUBFORLOOP>(i*SUBFORLOOP) + F0.segment<SUBFORLOOP>(i*SUBFORLOOP)) + M.segment<SUBFORLOOP>(i*SUBFORLOOP).cwiseProduct(Z.segment<SUBFORLOOP>(i*SUBFORLOOP));
                }
                assmp(ParTag(),deltaV,Z);
            }
            // RANDEVOUZ: merge b and stiffedpreb
            #pragma omp taskwait

            // integrate
            #pragma omp task
            {
                PrePost top("\tcomputeB2");
                #pragma omp parallel for
                for(int i = 0; i < n_ver*VECTORSIZE/SUBFORLOOP; i++)
                    b.segment<SUBFORLOOP>(i*SUBFORLOOP) += h*stiffedpostb.segment<SUBFORLOOP>(i*SUBFORLOOP);
            }

            // requires only sparse_k
            #pragma omp task
            {
                {
                    PrePost top("\tcomputeSparse");
                    const Scalar damp = conf.damping;
                    // now fill the left part of the equation: M - h df/dv - h^2 df/dx
                    #pragma omp parallel for
                    for (size_t i = 0; i < sparse_k.size(); ++i)
                        sparse_k[i] *= h * (h + damp);
                }
                {
                    PrePost top("\tcomputeP");
                    #pragma omp parallel for
                    for (int i = 0; i < n_ver; ++i)
                    {
                        //MatrixType & mat = sparse_k[diagonal_ref[i]];
                        MatrixType & mat = sparse_k[i];
                        mat.diagonal() += M.segment<VECTORSIZE>(VECTORSIZE * i);
                        P.segment<VECTORSIZE>(VECTORSIZE * i) = mat.diagonal();
                    }
                }
            }
            #pragma omp taskwait

            {
                PrePost top("\tPCG");
            // solve the system
            // deltaV is set to zero but it can be set to non zero values (in paper it is z)
            // TODO: delegated
            if(conf.pcgMaxIterations >= 0)
            {
                iter=pcgier.pcg(ParTaskTag(),P, sp_idx, sparse_k, b, deltaV, getGlobalTolerance(), conf.pcgMinIterations,conf.pcgMaxIterations, S, conf.pcgAscherThreshold && S.size() > 0);
            }
            else
                iter = 0;
            }
            #pragma omp task
            {
                PrePost top("\tFr");
				spmv(par,sp_idx, sparse_k, deltaV, Frtmp);
				submp(par,Fr,Frtmp,b);
				Frtmp = Fr;
				ifilter(par,Fr,S,Frtmp);
           }
            #pragma omp task
            {
                // OPTIONALLY: V => Vtmp => split X and V
                {
                    PrePost top("\tXV");
                    #pragma omp parallel for
                    for(int i = 0; i < n_ver*VECTORSIZE/SUBFORLOOP; i++)
                    {
                        X.segment<SUBFORLOOP>(i*SUBFORLOOP) += h * (deltaV.segment<SUBFORLOOP>(i*SUBFORLOOP)+V.segment<SUBFORLOOP>(i*SUBFORLOOP)) + Y.segment<SUBFORLOOP>(i*SUBFORLOOP);
                        V.segment<SUBFORLOOP>(i*SUBFORLOOP) += deltaV.segment<SUBFORLOOP>(i*SUBFORLOOP);
                    }
                }
                {
                    PrePost topc("\tupdate");
                    #pragma omp parallel for
                    for (int i = 0; i < n_tri; ++i)
                        shells[i]->update();
                }
            }
            #pragma omp taskwait
        }
    } // implicit task wait
    }
    return iter;
}

/**
  * Parallel Step with Manual control of parallelism
  */
int Cloth::step(InParTag par, const Scalar h)
{
    return step(ParTaskTag(),h);
}

/**
 * step with Paralle OpenMP separating each phase
 * parallellevel = 1
 */
int Cloth::step(ParTag par,const Scalar h)
{
    int i;
    // bad SPVM and ScaleSparseK
    for(i = StepPhases::Start; i <= StepPhases::UpdateP; i++)
        //if(i != StepPhases::SPVM) // && i != StepPhases::ScaleSparseK)
            steponepar((StepPhases::Enum)i,h);
        //else
          //  stepone((StepPhases::Enum)i,h);
    for(;i < StepPhases::UpdateXV; i++)
        stepone((StepPhases::Enum)i,h);
    steponepar(StepPhases::UpdateXV,h);
    steponepar(StepPhases::End,h);
    return  iter;


}

int Cloth::steponepar(StepPhases::Enum phase, const Scalar h)
{
    ParTag par;
    SimLoggerScope sls(logger,(SimLogger::SimState)(SimLogger::PhaseBase+phase),curStep,h);
    lasth = h;
    const Scalar damp = conf.damping;
    switch(phase)
    {
    case StepPhases::Start:
        curStep++;
        break;
    case StepPhases::UpdateShells:
        #pragma omp parallel for
        for (int i = 0; i < n_tri; ++i)
            shells[i]->update();
        break;
    case StepPhases::VertexGet:
        #pragma omp parallel for
        for (int i = 0; i < n_ver; ++i)
        {
            const unsigned int count = vertextri_t::maxtri;
            VectorType f0(VectorType::Zero());
            VectorType n(VectorType::Zero());
            for (int j = 0; j < count; j++)
            {
                const vertextri_t::value_t & w = vtxtri.gettri(i,j);
                f0 += shells[w.tri]->F0(w.off);
                n += shells[w.tri]->get_normal();
            }
            F0.segment<VECTORSIZE>(VECTORSIZE * i) = f0;
            n.normalize();
            N.col(i) = n;
            Pm[i] = n * n.transpose();
        }
        break;
    case StepPhases::TriangleRot:
        #pragma omp parallel for
        for(int i = 0; i < n_tri; i++)
        {
            Vector3i t = connectivity.col(i);
            const Matrix3 & Pma = Pm[t(0)];
            const Matrix3 & Pmb = Pm[t(1)];
            const Matrix3 & Pmc = Pm[t(2)];
            shells[i]->update_bend(Pma,Pmb,Pmc);
        }
        break;
    case StepPhases::SparseK:
        {
            const bool symmetricBuildA = conf.symmetricBuildA;
            #pragma omp parallel for
            for(int i = 0; i < n_blocks; ++i)
            {
                MatrixType mtx;
                mtx.setZero();
                // xi = Ab vj
                int vi = sp_idx.backinner[i];
                int vj = sp_idx.inner[i];
                if(!symmetricBuildA || vi <= vj)
                {
                    for(int j = 0; j < MAX_TRICONT_PER_BLOCK; j++)
                    {
                        int tri = sparseblocks(i,j*2);
                        int ab = sparseblocks(i,j*2+1);
                        if(tri == 0) // zero is ender
                            break;
                        tri--;
                        int a = ab/4;
                        int b = ab%4;
                        mtx += shells[tri]->QmRot(a,b);
                    }
                    sparse_k[i] = mtx;
                    if(symmetricBuildA && vi != vj)
                    {
                        int q = sp_idx.symblock[i]; // getBlockIndex(vj,vi);
                        sparse_k[q] = mtx;
                        sparse_k[q].transpose();
                    }
                }
            }
        }
        break;
    case StepPhases::PreSPVM:

        // f = As x - As* xs + Ab x + Bv + fext
        // df/dx = As + Ab + d fext/dx
        // df/dv = B + d fext / dv
        // Dx = h (v0 + Dv) + y
        // Dy = W f (x0+Dx,v0+Dv) + z
        // f(x0+Dx,v0+Dv) = f(x0,v0) + df/dx Dx + df/dv Dy = As x0 -As* xs + Ab x0 + Bv0 +fext(x0,v0) + (As+Ab)(h(v0+Dv)+y) + B Dv
        //  = (As+Ab)(h(v0+Dv)+y+x0) - As* xs + Bv0 + fext(x0,v0) + B Dv
        //
        // Left is: (I - h^2 W (As+Ab) -  h W B ) Dv
        // Right is: h W [(As+Ab)(h(v0+Dv)+y+x0) - As* xs + Bv0 + fext(x0,v0)] + z
        // q = (As+Ab)(h v0+y+x0)
        // b = h W [ q - As xs + Bv0 + fext(x0,v0)] + z
        //
        // Notes:
        // * Bv0 is not used
        // * -As xs is computed by F0
        // * fext is F+G
        //
        // Viscosity:
        //      assuming B  = d (A1+A2)


        //         stiffedpreb = (-conf.damping - h)*V - X - Y;
        #pragma omp parallel for

        for(int i = 0; i < n_ver*VECTORSIZE/SUBFORLOOP; i++)
            // is this SSE2 vectorized?
            stiffedpreb.segment<SUBFORLOOP>(i*SUBFORLOOP) = (-h - damp) * V.segment<SUBFORLOOP>(i*SUBFORLOOP) - X.segment<SUBFORLOOP>(i*SUBFORLOOP) - Y.segment<SUBFORLOOP>(i*SUBFORLOOP);

        // Original: (conf.damping - h)*V + X - Y;
        // Flipped:  (h-conf.damping)*V - X + Y;
        // Correct? (-h-conf.damping)*V - X - Y;
        // Correct with Y flip? (-h-conf.damping)*V - X + Y;
        break;
    case StepPhases::SPVM:


        // At this stage sparse_k contains: As + Ab
        spmv(par,sp_idx, sparse_k, stiffedpreb, stiffedpostb);
        break;
    case StepPhases::ExternalForces:

        if(conf.applyExternalForce != 0)
            conf.applyExternalForce(*this);
    case StepPhases::ComputeB:

        #pragma omp parallel for
        for(int i = 0; i < n_ver*VECTORSIZE/SUBFORLOOP; i++)
            b.segment<SUBFORLOOP>(i*SUBFORLOOP) = h*(F.segment<SUBFORLOOP>(i*SUBFORLOOP) + G.segment<SUBFORLOOP>(i*SUBFORLOOP) + stiffedpostb.segment<SUBFORLOOP>(i*SUBFORLOOP) + F0.segment<SUBFORLOOP>(i*SUBFORLOOP)) + M.segment<SUBFORLOOP>(i*SUBFORLOOP).cwiseProduct(Z.segment<SUBFORLOOP>(i*SUBFORLOOP));

        //std::cerr << b.dot(b) << " " << F.dot(F) << " " << G.dot(G) << stiffedpostb.dot(G) << " " << F0.dot(F0) << " " << M.dot(M) << " " << Z.dot(Z) << std::endl;
    break;
    case StepPhases::ScaleSparseK:
    /*
    {
        Eigen::Map<VectorX> q(sparse_k[0].data(),MATRIXSIZE*MATRIXSIZE*sparse_k.size());
        #pragma omp parallel for
        for(int i = 0; i < q.size()/SUBFORLOOP; i++)
            q.segment<SUBFORLOOP>(i*SUBFORLOOP) *= h*(h+conf.damping);
    }
    */
    {

        // now fill the left part of the equation: M - h df/dv - h^2 df/dx
        // where df/dv due to no viscosity so we have: + h^2 df/dx
        const Scalar damp = conf.damping;
        #pragma omp parallel for
        for (size_t i = 0; i < sparse_k.size(); ++i)
            sparse_k[i] *= h * (h + damp); // SIGNCHECK: sparse_k[i] *= h * (h + damp);

    }
        break;
    case StepPhases::UpdateP:
        // now we jointly build the preconditioning matrix (mass)
        // and add the mass to the Sparse: (M - h df/dv - h^2 df/dx)
        #pragma omp parallel for
        for (int i = 0; i < n_ver; ++i)
        {
            MatrixType & mat = sparse_k[diagonal_ref[i]];
            //MatrixType & mat = sparse_k[i];
            mat.diagonal() += M.segment<VECTORSIZE>(VECTORSIZE * i);
            P.segment<VECTORSIZE>(VECTORSIZE * i) = mat.diagonal();
        }
        break;
    case StepPhases::PCG:
        if(conf.pcgAscherPre)
        {
            /*
             * Modified Preconditioner: P* = P S^-1
             */


            #pragma omp parallel for
            for(int i = 0; i < S.vtxFixed.size(); i++)
                P.segment<VECTORSIZE>(S.vtxFixed[i]*VECTORSIZE).setOnes();

            // C = P
            // S C + (I-S)
            // for S_i constrained and S = I - M where M is getMatrix
            //      (I-M)C + M
            //      C + M (I-C)
            // so
            //      C += M (I-C)
            #pragma omp parallel for
            for(int j = 0; j < S.size(); j++)
            {
                int i = VECTORSIZE*S.getIndex(j);
                P.segment<VECTORSIZE>(i) += (VectorType(1,1,1)-P.segment<VECTORSIZE>(i)).cwiseProduct(S.getMatrix(j).diagonal());
            }
         }


        // solve the system
        // deltaV should be computed from the collision mechanism with the correct z
        if(conf.pcgAscherInit)
        {
            std::cerr << "Ascher Init not implemented\n";
            exit(0);
        }
        else
            assmp(par,deltaV,Z);

        if(conf.pcgMaxIterations >= 0)
            iter=pcgier.pcg(par,P, sp_idx, sparse_k, b, deltaV, getGlobalTolerance(), conf.pcgMinIterations,conf.pcgMaxIterations, S, conf.pcgAscherThreshold && S.size() > 0);
        else
            iter = 0;
        sls.setValue(conf.lastTolerance);
        sls.setExtraInfo(iter);
        break;
       case StepPhases::ComputeForces:
            spmv(par,sp_idx, sparse_k, deltaV, Frtmp);
            submp(par,Fr,Frtmp,b);
            Frtmp = Fr;
            ifilter(par,Fr,S,Frtmp);
            break;
       case StepPhases::UpdateXV:


        // update velocity and position
        //V += deltaV;
        //X += h * V + Y;
        #pragma omp parallel  for
        for(int i = 0; i < n_ver*VECTORSIZE/SUBFORLOOP; i++)
        {
            V.segment<SUBFORLOOP>(i*SUBFORLOOP) += deltaV.segment<SUBFORLOOP>(i*SUBFORLOOP);
            X.segment<SUBFORLOOP>(i*SUBFORLOOP) += h * V.segment<SUBFORLOOP>(i*SUBFORLOOP) + Y.segment<SUBFORLOOP>(i*SUBFORLOOP);
        }
        break;

        case StepPhases::End:
        break;
    }
}



int Cloth::stepone(StepPhases::Enum phase, const Scalar h)
{
    NoParTag par;
    SimLoggerScope sls(logger,(SimLogger::SimState)(SimLogger::PhaseBase+phase),curStep,h);
    lasth = h;
    switch(phase)
    {
    case StepPhases::Start:
        curStep++;
        break;
    case StepPhases::UpdateShells:
        for (int i = 0; i < n_tri; ++i)
            shells[i]->update();
        break;
    case StepPhases::VertexGet:
        for (int i = 0; i < n_ver; ++i)
        {
            const unsigned int count = vertextri_t::maxtri;
            Vector3 f0(VectorType::Zero());
            Vector3 n(VectorType::Zero());
            for (int j = 0; j < count; j++)
            {
                const vertextri_t::value_t & w = vtxtri.gettri(i,j);
                f0 += shells[w.tri]->F0(w.off);
                n += shells[w.tri]->get_normal();
            }
            F0.segment<VECTORSIZE>(VECTORSIZE * i) = f0;
            n.normalize();
            N.col(i) = n;
            Pm[i] = n * n.transpose();
        }
        break;
    case StepPhases::TriangleRot:
        for(int i = 0; i < n_tri; i++)
        {
            Vector3i t = connectivity.col(i);
            const MatrixType & Pma = Pm[t(0)];
            const MatrixType & Pmb = Pm[t(1)];
            const MatrixType & Pmc = Pm[t(2)];
            shells[i]->update_bend(Pma,Pmb,Pmc);
        }
        break;
    case StepPhases::SparseK:
        {
            const bool symmetricBuildA = conf.symmetricBuildA;
            for(int i = 0; i < n_blocks; ++i)
            {
                MatrixType mtx;
                mtx.setZero();
                // xi = Ab vj
                int vi = sp_idx.backinner[i];
                int vj = sp_idx.inner[i];
                if(!symmetricBuildA || vi <= vj)
                {
                    for(int j = 0; j < MAX_TRICONT_PER_BLOCK; j++)
                    {
                        int tri = sparseblocks(i,j*2);
                        int ab = sparseblocks(i,j*2+1);
                        if(tri == 0) // zero is ender
                            break;
                        tri--;
                        int a = ab/4;
                        int b = ab%4;
                        mtx += shells[tri]->QmRot(a,b);
                    }
                    sparse_k[i] = mtx;
                    if(symmetricBuildA && vi != vj)
                    {
                        int q = sp_idx.symblock[i]; // getBlockIndex(vj,vi);
                        sparse_k[q] = mtx;
                        sparse_k[q].transpose();
                    }
                }
            }


            if(false)
            {
                for(int i = 0; i <n_ver; i++)
                    for(int j=i; j <n_ver; j++)
                    {
                        int kij = sp_idx.getBlockIndex(i,j);
                        int kji = sp_idx.getBlockIndex(j,i);
                        if((kij < 0 && kji >= 0) || (kji < 0 && kij >= 0))
                        {
                            printf("A(i) block is not symmetric: one is missing\n");
                            getchar();
                                    exit(0);
                        }
                        else if(kij < 0 && kji < 0)
                            continue;
                        MatrixType mij = sparse_k[kij];
                        MatrixType mji = sparse_k[kji];
                        if(!mij.isApprox(mji.transpose()))
                        {
                            printf("A(i) block is not symmetric: %d,%d => %d,%d\n",i,j,kij,kji);
                            getchar();
                                    exit(0);
                        }
                    }
            }
        }

        break;
    case StepPhases::PreSPVM:
        stiffedpreb = (-conf.damping - h)*V - X - Y;
        break;
    case StepPhases::SPVM:
        spmvV(par,sp_idx, sparse_k, stiffedpreb, stiffedpostb);
        break;
    case StepPhases::ExternalForces:
        if(conf.applyExternalForce != 0)
            conf.applyExternalForce(*this);
        break;
    case StepPhases::ComputeB:
        b = h*(F.array()+G.array()+stiffedpostb.array()+F0.array()) + M.array() * Z.array();
        break;
    case StepPhases::ScaleSparseK:
        {
            Eigen::Map<VectorX> q(sparse_k[0].data(),MATRIXSIZE*MATRIXSIZE*sparse_k.size());
            q *= h*(h+conf.damping);
        }
        break;
    case StepPhases::UpdateP:
        for (int i = 0; i < n_ver; ++i)
        {
            //MatrixType & mat = sparse_k[diagonal_ref[i]];
            MatrixType & mat = sparse_k[i];
            mat.diagonal() += M.segment<VECTORSIZE>(VECTORSIZE*i);
            //if(conf.pcgPrecond == 1)
                P.segment<VECTORSIZE>(VECTORSIZE * i) = mat.diagonal();
        }
        // similar to filter but inlin
        if(conf.pcgAscherPre && conf.pcgPrecond == 1)
        {
            for(int i = 0; i < S.vtxFixed.size(); i++)
                P.segment<VECTORSIZE>(S.vtxFixed[i]*VECTORSIZE).setOnes();

            // C = P
            // S C + (I-S)
            // for S_i constrained and S = I - M where M is getMatrix
            //      (I-M)C + M
            //      C + M (I-C)
            // so
            //      C += M (I-C)
            for(int j = 0; j < S.size(); j++)
            {
                int i = VECTORSIZE*S.getIndex(j);
                VectorType a(1,1,1);
                a -= P.segment<VECTORSIZE>(i);
                P.segment<VECTORSIZE>(i) += a.cwiseProduct(S.getMatrix(j).diagonal());
            }
         }
         break;
    case StepPhases::PCG:        
        if(conf.pcgAscherInit)
        {
            std::cerr << "Ascher Init not implemented\n";
            exit(0);
        }
        else
            assmp(par,deltaV,Z);
        if(conf.pcgMaxIterations >= 0)
        {
            switch(conf.pcgPrecond)
            {
            case 0:
                iter=pcgier.pcg(par,sp_idx, sparse_k, b, deltaV, conf.lastTolerance, conf.pcgDeltaMin,conf.pcgMinIterations,conf.pcgMaxIterations, S, conf.pcgAscherThreshold && S.size() > 0); // 50%
                break;
            case 1:
                iter=pcgier.pcg(par,P,sp_idx, sparse_k, b, deltaV, conf.lastTolerance, conf.pcgDeltaMin,conf.pcgMinIterations,conf.pcgMaxIterations, S, conf.pcgAscherThreshold && S.size() > 0); // 50%
                break;
            case 2:
                iter=pcgier.pcg33(par,P, sp_idx, sparse_k, b, deltaV, conf.lastTolerance, conf.pcgDeltaMin, conf.pcgMinIterations,conf.pcgMaxIterations, S, conf.pcgAscherThreshold && S.size() > 0); // 50%
                break;
            }
        }
            else
            iter = 0;
            sls.setValue(conf.lastTolerance);
            sls.setExtraInfo(iter);
        break;
    case StepPhases::ComputeForces:
                spmvV(par,sp_idx, sparse_k, deltaV, Frtmp);
		submp(par,Fr,Frtmp,b);
		Frtmp = Fr;
		ifilter(par,Fr,S,Frtmp);
        break;
    case StepPhases::UpdateXV:
        V += deltaV;
        X += h * V + Y;
        break;
    default:
        break;
    }
    return 0;
}

int Cloth::step(NoParTag par,const Scalar h)
{
    for(int i = StepPhases::Start; i <= StepPhases::End; i++)
        stepone((StepPhases::Enum)i,h);
    return  iter;
}

void Cloth::addCollideImpact(int ivtx, const Vector3 & d, const Vector3 & dn, const Vector3 & s, Scalar r, const Vector3 & vs, int objects)
{
    // X += dyn + Y
    // d = p-s
    // dn = normalized(d)
    // X_on_S = dn*r + s
    // Y = X_on_S - X = dn*r + s - (d+s) = dn*r - d
    Y.segment<3>(3 * ivtx) = dn*r - d;

    Vector3 dv;
    Matrix3 q;
    bool sticked;
    if(computeImpactVelocity(ivtx, V.segment<3>(3*ivtx),vs,dn,dv,q,sticked))
    {
        unsigned cx,cy;
        inv_vertexindex_t inv = Cloth::getInvVertexIndexer();
        inv(ivtx,cx,cy,w,h);

        Z.segment<VECTORSIZE>(VECTORSIZE * ivtx) = dv;
        S.push_back(S_t::matpt(q,ivtx,cx,cy,s + dn*r,dn,dv,objects,sticked));
    }
    collisions++; //add collision
}

bool Cloth::computeImpactVelocity(int ivtx, const Vector3 & vp, const Vector3 & vs, const Vector3 & dn , Vector3 & dv, Matrix3 & q, bool &sticked)
{
    // vrel > 0
    //  (1+e) vs_n - (1+e) vp_n = (1+eps)(vs_n-vp_n)
    const Scalar vpnl = vp.dot(dn);
    const Scalar vsnl = vs.dot(dn);
    Scalar vreln = vpnl-vsnl;

    // exception is negative relative velocity that means no touch velocity
    /*if(vreln < 0)
    {
        q.setIdentity();
        dv.setZero();
        return false;
    }
    if(vreln > 0)
        vreln = 0;
    */

    // vrel_n(final) = 0 when vrel(pre) <= 0
    // vrel_n(final) = Vfinal - vs = Vnow + dv - vs = 0
    //       dv = vs-Vnow = -vrel
    dv = -dn*((1+conf.bounceFactor)*vreln);
    q = dn*dn.transpose();
    if(conf.friction != 0)
    {
        Vector3 vrel_final = vp-vs; //(vp+dv)-vs;

        // vrel_final_norm = vpnl+(1+conf.bounceFactor)*vrel = vpnl+(1+conf.bounceFactor)*(vsn-vpnl) = (1+e)vsn-e vpnl
        //Scalar vrel_final_norm = (vs-vp).norm(); //(1+conf.bounceFactor) * vsnl - conf.bounceFactor* vpnl;

         // |vrel_final_norm| < epsilon
        if(vrel_final.norm() < conf.frictionMinimumVelocity)
        {

            // Fr1 = Fr + vrel_final*M(ivtx*3)
            Vector3 Fr1 = Fr.segment<3>(ivtx*3) + vrel_final*M(ivtx*3); // residual plus additional relative velocity
            Scalar Fr1n = Fr1.dot(dn);
            Scalar Fr1t = (Fr1 - dn*Fr1n).norm();
           //std::cerr << " " << ivtx << " n:" << Fr1n << " t:" << Fr1t << " vreln:" << vreln << " ratio:" << Fr1t/Fr1n << " Frnorm:" <<
           //         Fr.segment<3>(ivtx*3).norm() << std::endl;

            // Fr1_normal * conf.friction > Fr1_parallel => block
            if(fabs(conf.friction*Fr1n) > Fr1t)
            {
                // block means:
                //  m_final = identity => S is zero => blocked
                //  vrel_blocked = 0 => dv = vs-V = vrel_initial
                frictionLocked++;
                if(!conf.frictionLockPoint)
                {
                    // alternatively remove tangential velocity only not lock
                    Vector3 vpt = vp-dn*vpnl;
                    Vector3 vst = vs-dn*vsnl;
                    Vector3 vrelt = vpt-vst;
                    q.setZero(); // means locking over the surface for the time being
                    dv -= vrelt; // make relative tangential zero plus new normal from bounce
                    return true;
                }
                else
                {
                    q.setZero(); // means locking over the surface for the time being
                    dv = vp-vs; // means vrel is zero in any direction. Just stay there
                    return true;
                }
            }
        }
    }
    return true;
}


void Cloth::updateMaterial()
{
    for (int i = 0; i < n_tri; ++i)
        shells[i]->rest_state_material();

}

void Cloth::collideClear()
{
    Y.setZero(); // cleanup of the displacement
    Z.setZero();
    S.clear(); // remove the collisions
    // block vertices if they have been marked above
    for(VertexConstraints::const_iterator it = vtxFixed.begin(); it != vtxFixed.end(); ++it)
        S.setConstrained(*it);
    collisions = 0; //number of collisions
    frictionLocked = 0;
}


int Cloth::collide(const Vector3 s, Scalar r, const Vector3 vs)
{
    const Scalar r2 = r * r;    
    collideClear();

    for (int i = 0; i < n_ver; ++i) {
        Vector3 p = X.segment<3>(3 * i); // position of the i-th vertex
        Vector3 d = p - s; // vector from sphere center to vertex
        const Scalar dL = d.squaredNorm();
        if (dL <= r2 && !vtxFixed.contains(i))
        {
            Vector3 dn = d.normalized();
            addCollideImpact(i,d,dn,s,r,vs,1);
        }
    }
    if(collisions == 0)
    {
        //Z.segment<VECTORSIZE>(100*VECTORSIZE) = VectorType(0,0,0.00001);
    }
    return collisions;
}

/**
   * Collision Check between textile vertices and TWO spherical objects
   */
int Cloth::collide2(const Vector3 s, Scalar r, const Vector3 vs,const Vector3 s2, Scalar r2, const Vector3 vs2)
{
    const Scalar rsq = r * r;
    const Scalar rsq2 = r2 * r2;
    collideClear();

    // TODO: beware of vertex re-ordering in this loop
    for (int i = 0; i < n_ver; ++i) {
        const Vector3 p = X.segment<3>(3 * i); // position of the i-th vertex
        Vector3 d = p - s; // vector from sphere center to vertex
        const Scalar dL = d.squaredNorm();
        Vector3 dn = d.normalized();

        Vector3 d2 = p - s2; // vector from sphere center to vertex
        const Scalar dL2 = d2.squaredNorm();
        Vector3 dn2 = d2.normalized();
        if (dL < rsq)
        {
            if(vtxFixed.contains(i))
                continue;
            if(dL2 < rsq2)
            {
                // joint contact
                // take the sphere that is deeper in the textile
                if((rsq2-dL2) > (rsq-dL))
                    addCollideImpact(i,d2,dn2,s2,r2,vs2,3);
                else
                    addCollideImpact(i,d,dn,s,r,vs,3);
            }
            else
                addCollideImpact(i,d,dn,s,r,vs,1);
        }
        else if (dL2 < rsq2 && !vtxFixed.contains(i))
            addCollideImpact(i,d2,dn2,s2,r2,vs2,2);
    }
    if(collisions == 0)
    {
        // REMOVED BECAUSE
        //Z.segment<VECTORSIZE>(100*VECTORSIZE) = VectorType(0,0,0.00001);
    }
    return collisions;
}

/*
 * Given a Moving Sphere it computes the next time a vertex is going to collide with one of the vertices
 *
 * When we have no time the result is -1 (should be Inf but it is not portable)
 * When we have a sphere already colliding with a vertex with have 0
 *
 * Algorithm:
 * Vertex P(t) = P0 + t V_p
 * Sphere with center C(t) = C0 + t V_s
 * Same as Sphere is in Origin and Static using: P(t) = D0 + t W
 *   D0 = P0-C0,  W = V_p-V_s
 * Assuming not already colliding: d(t) = (D0 + tW) (D0 + tW)' - r*r = D0*D0 + 2 t D0*W + t*t*W*W - r*r
 */
Scalar Cloth::collide_first_time(const Vector3 s, Scalar r, const Vector3 vs)
{
    const Scalar r2 = r * r;
    bool found = false;
    double time = 0;
    bool inside = false;
    int insideindex = 0;

    collideClear();

    // skip first line of vertices    
    for (int i = 0; i < n_ver; ++i) {
        Vector3 d = X.segment<3>(3 * i) - s; // relative to sphere
        const Scalar dL2 = d.squaredNorm();
        if (dL2 < r2)
        {
            if(vtxFixed.contains(i))
                continue;
            inside = true;
            insideindex = i;
            break;
        }
        else
        {
            Vector3 w = V.segment<3>(3 * i) - vs; // relative to sphere

            //if((w.cross(d).squaredNorm()) > r2*wL2) // Delta is negative continue; Alternative
            const Scalar wL2 = w.squaredNorm(); // a
            if(wL2 == 0)
                continue;

            Scalar dot = d.dot(w);  // k = |d| |s| cos(alpha) of b = 2k
            Scalar diff = dL2 - r2; // c = D*D - r*r
            Scalar delta = dot*dot - wL2*diff; // k^2 - ac
            if(delta < 0) // no collision anyway
                continue;
            if(vtxFixed.contains(i))
                continue;
            Scalar t = (- dot + (Scalar)sqrt(delta))/wL2; // (-k + sqrt(Delta))/a
            if(!found)
            {
                time = t;
                found = true;
            }
            else if (time > t)
                time = t;
        }
    }
    if(inside)
    {
        // already colliding from now on, but before
        for (int j = insideindex; j < n_ver; ++j) {
            Vector3 d = X.segment<3>(3 * j) - s; // relative to sphere
            const Scalar dL = d.squaredNorm();
            Vector3 dn = d.normalized();
            if (dL < r2 && !vtxFixed.contains(j))
                addCollideImpact(j,d,dn,s,r,vs,1);
        }
    }
    else
        return found ? time : -1;
}


/*
 * Given a Moving Sphere it computes the next time a vertex is going to collide with one of the vertices
 *
 * When we have no time the result is -1 (should be Inf but it is not portable)
 * When we have a sphere already colliding with a vertex with have 0
 *
 * Algorithm:
 * Vertex P(t) = P0 + t V_p
 * Sphere with center C(t) = C0 + t V_s
 * Same as Sphere is in Origin and Static using: P(t) = D0 + t W
 *   D0 = P0-C0,  W = V_p-V_s
 * Assuming not already colliding: d(t) = (D0 + tW) (D0 + tW)' - r*r = D0*D0 + 2 t D0*W + t*t*W*W - r*r
 */
Scalar Cloth::collide_first_time2(const Vector3 s, Scalar r, const Vector3 vs,const Vector3 s2, Scalar r2, const Vector3 vs2)
{
    const Scalar rsq = r * r;
    const Scalar rsq2 = r2 * r2;
    bool found = false;
    bool inside = false;
    int insideindex = 0;
    double time = 0;
    collideClear();

    // skip first line of vertices
    for (int i = 0; i < n_ver; ++i) {
        Vector3 d = X.segment<3>(3 * i) - s; // relative to sphere
        Vector3 d2 = X.segment<3>(3 * i) - s2; // relative to sphere
        const Scalar dL = d.squaredNorm();
        const Scalar dL2 = d2.squaredNorm();
        if (dL < rsq || dL2 < rsq2)
        {
            if(vtxFixed.contains(i))
                continue;
            inside = true;
            insideindex = i;
            break;
        }
        else
        {
            Vector3 w = V.segment<3>(3 * i) - vs; // relative to sphere
            Vector3 w2 = V.segment<3>(3 * i) - vs2; // relative to sphere

            //if((w.cross(d).squaredNorm()) > r2*wL2) // Delta is negative continue; Alternative
            const Scalar wL2 = w.squaredNorm(); // a
            const Scalar wL22 = w2.squaredNorm(); // a

            if(wL2 == 0 && wL22 == 0) // both not
                continue;

            Scalar dot = d.dot(w);  // k = |d| |s| cos(alpha) of b = 2k
            Scalar diff = dL - rsq; // c = D*D - r*r
            Scalar delta = dot*dot - wL2*diff; // k^2 - ac

            Scalar dot2 = d2.dot(w2);  // k = |d| |s| cos(alpha) of b = 2k
            Scalar diff2 = dL2 - rsq2; // c = D*D - r*r
            Scalar delta2 = dot2*dot2 - wL22*diff2; // k^2 - ac

            if(delta < 0 && delta2 < 0) // both not
                continue;
            if(vtxFixed.contains(i))
                continue;
            bool first = wL2 != 0 && delta >= 0;
            bool second = wL22 != 0 && delta2 >= 0;
            Scalar t;
            if(first)
            {
                t = (- dot + (Scalar)sqrt(delta))/wL2;
                if(second)
                {
                    Scalar t2 = (- dot2 + (Scalar)sqrt(delta2))/wL22;
                    if(t2 < t)
                        t = t2;
                }
            }
            else
                t = (- dot2 + (Scalar)sqrt(delta2))/wL22;

            if(!found)
            {
                time = t;
                found = true;
            }
            else if (time > t)
                time = t;
        }
    }
    if(inside)
    {
        // already colliding from now on, but before
        for (int j = insideindex; j < n_ver; ++j) {
            Vector3 d = X.segment<3>(3 * j) - s; // relative to sphere
            const Scalar dL = d.squaredNorm();
            Vector3 dn = d.normalized();
            Vector3 d2 = X.segment<3>(3 * j) - s2; // relative to sphere
            const Scalar dL2 = d2.squaredNorm();
            Vector3 dn2 = d2.normalized();
            if (dL < rsq)
            {
                if(vtxFixed.contains(j))
                    continue;
                if(dL < rsq2 && (rsq2-dL) > (rsq-dL))
                    addCollideImpact(j,d2,dn2,s2,r2,vs2,3);
                else
                    addCollideImpact(j,d,dn,s,r,vs,1);
            }
            else if(dL2 < rsq2 && !vtxFixed.contains(j))
                addCollideImpact(j,d2,dn2,s2,r2,vs2,2);
        }
        return 0;
    }
    else
        return found ? time : -1;
}



int Cloth::collide_sphere_tri(const Vector3 s, Scalar r, const Vector3 vs)
{
    return collide2_sphere_tri(s,r,vs,s,0,vs);
}


int Cloth::collide2_sphere_tri(const Vector3 s, Scalar r, const Vector3 vs,const Vector3 s2, Scalar r2, const Vector3 vs2)
{
    inv_vertexindex_t inv = Cloth::getInvVertexIndexer();
    const Scalar rsq = r*r;
    const Scalar rsq2 = r*r;
    collideClear();

    for (int i = 0; i < n_tri; ++i) {
        Scalar ds,ds2;
        Vector3 d,point = closestPtPointTriangle(s,shells[i]->v[0],shells[i]->v[1],shells[i]->v[2],ds);
        Vector3 point2,d2;
        d = (point - s);
        if(r2 != 0)
        {
            point2 = closestPtPointTriangle(s2,shells[i]->v[0],shells[i]->v[1],shells[i]->v[2],ds2);
            d2 = (point2 - s2);
        }

        if(ds2 < rsq2 || ds < rsq)
        {
            const Vector3i t = connectivity.col(i);

            const Vector3 dn = d.normalized();
            const Scalar dL = d.squaredNorm();
            const Matrix3 dnt = dn*dn.transpose();
            const Vector3 target = s + dn*r;
            const Vector3 dt = dn*r-d; // dn*r-point+s

            const Vector3 dn2 = d2.normalized();
            const Scalar dL2 = d2.squaredNorm();
            const Matrix3 dnt2 = dn2*dn2.transpose();
            const Vector3 target2 = s2 + dn2*r2;
            const Vector3 dt2 = dn2*r2-d2; // dn*r-point+s

            // merging of contributions
            for(int q = 0; q < 3; q++)
            {
                const int idx = t(q);
                if(vtxFixed.contains(idx))
                    continue;

                S_t::iterator it = S.getVertex(idx);
                if(it == S.end())
                {
                    // check addCollideImpact for changes this is special actually
                    Y.segment<3>(3 * idx) += dt;
                    // I - dn*dn'
                    Matrix3 q ;
                    unsigned cx,cy;
                    inv(idx,cx,cy,w,h);

                    Vector3 dv;
                    bool sticked;
                    if(computeImpactVelocity(i,V.segment<3>(3*i),vs,dn,dv,q,sticked))
                    {
                        q.setZero(); // SPECIAL HERE
                        Z.segment<3>(3 * i) = dv;
                        S.push_back(S_t::matpt(q,idx,cx,cy,target,dn,dv,1,sticked));
                    }
                }
                else
                {
                    // we have the existing N and the new dn
                    // N and dn have to
                    /*
                    if(it->count++> 1)
                    {
                        //std::cout << "S overlay " << it->count << " " << it->idx << std::endl;
                    }
                    else
                    {
                        Y.segment<3>(3 * idx) += dt;
                        it->mat += dnt;
                    }
                    */
                }
            }
            collisions++; //add collision           
        }
    }
    return collisions;
}



// collision cylinder / triangle
int Cloth::collide_cylinder(const Cylinder & c)
{
    Vector3 k0; // distance origin vertex
    Scalar ds_n; // distance along triangle normal
    Scalar ex_n; // extent along normal
    Scalar s0_n, s1_n;
    Scalar triD;
    Scalar minc,maxc, mint,maxt;
    Vector3 dn; //penetration normal
    Scalar penDepth; //penetration depth
    Scalar minDepth,maxDepth;
    Vector3i vt; //index of triangle vertices
    Vector3 normal; //normalized triangle normal

    collideClear();
     for (int i = 0; i < n_tri; ++i) {

        Triangle & tri = *shells[i];
        normal = tri.normal.normalized();
        k0 = c.orig - tri.v[0];
        ds_n = normal.dot(k0);
        ex_n = normal.dot(c.HAX);
        s0_n = ds_n - ex_n;
        s1_n = ds_n + ex_n;

        if ((s0_n > c.radius && s1_n > c.radius)||(s0_n < 0.0 && s1_n < 0.0)) //first check against capsule
            continue;  //no intersection for this triangle

        // now check in more detail and calculate axis of separation

        // suppose triangle normal

        triD = tri.v[0].dot(normal);

        c.Project(minc,maxc,normal);

        if (maxc < triD || minc > triD)
            continue; // no intersection

        dn = - normal;  //axis
        penDepth = triD - minc; //penetration

        // go on checking - suppose cylinder axis

        if (normal.dot(c.U_HAX))
        {
            minc = c.orig.dot(c.U_HAX);
            maxc = minc + c.extent;
            minc -= c.extent;

            projectTriangle(mint,maxt,tri,c.U_HAX);

            if(maxc<mint || minc >maxt)
                continue; // stop checking collision

            minDepth = maxc -mint;
            maxDepth = maxt -minc;

            if (minDepth >maxDepth)
                if (maxDepth < penDepth) // gone deeper?
                {
                    dn = c.U_HAX;       //change axis of separation
                    penDepth = maxDepth;
                }
            else
                if (minDepth < penDepth) // other side?
                {
                    dn = - c.U_HAX;
                    penDepth = minDepth;
                }


        }

        // now test for radial swept _ still developing

        Scalar rq = c.radius*c.radius;
        Scalar dist, maxdist = 0; // as default don't skipp collision with triangle

        // check distance of triangle plane with cylinder
        Scalar sinalpha = sqrt(fabs(1 - ex_n/c.HAX.norm())); //calculate exact projection
        minDepth = c.radius*sinalpha - s1_n;
        maxDepth = c.radius*sinalpha - s0_n;
        if (s0_n < c.radius)
        {
            if (maxDepth < penDepth) // gone deeper?
            {
                dn = - normal;       //change axis of separation
                penDepth = maxDepth;
            }
            maxdist = s0_n;
        }

        if (s1_n < c.radius)
        {
            if (minDepth < penDepth) // gone deeper?
            {
                dn = - normal;       //change axis of separation
                penDepth = minDepth;
            }
            maxdist = s1_n>maxdist?s1_n:maxdist;
        }

        Vector3 topcyl = c.orig + c.HAX;
        Vector3 bottomcyl = c.orig -c.HAX;
        Scalar s, t;
        Vector3 c1, c2;



        // check distances with triangle edges
        dist = closestPtSegmentSegment(tri.v[0], tri.v[1],topcyl, bottomcyl,s, t, c1, c2);

        if ( dist < rq)
        {
            if ( sqrt(dist) > penDepth)
            {
                dn = c1 - c2;
                penDepth = sqrt(rq-dist);
            }
            maxdist = dist>maxdist?dist:maxdist;
        }
        // second edge
        dist = closestPtSegmentSegment(tri.v[1], tri.v[2],topcyl, bottomcyl,s, t, c1, c2);

        if (dist > maxdist)
            if ( dist < rq)
            {
                if ( sqrt(dist) > penDepth)
                {
                    dn = c1 - c2;
                    penDepth = sqrt(rq-dist);
                }
                maxdist = dist>maxdist?dist:maxdist;
            }
        // third edge
        dist = closestPtSegmentSegment(tri.v[0], tri.v[2],topcyl, bottomcyl,s, t, c1, c2);

        if (dist > maxdist)
            if ( dist < rq)
            {
                if ( sqrt(dist) > penDepth)
                {
                    dn = c1 - c2;
                    penDepth = sqrt(rq-dist);
                }
                maxdist = dist>maxdist?dist:maxdist;
            }

        if (maxdist > rq)
            continue; //no triangle collision


        // finally with surface caps _ not performed

/*
        vt = connectivity.col(i);

        // merging of components
        Y.segment<3>(3 * vt[0]) += dn*penDepth;
        Y.segment<3>(3 * vt[1]) += dn*penDepth;
        Y.segment<3>(3 * vt[2]) += dn*penDepth;

        bool t0flag=false,t1flag=false,t2flag=false;
        // check for merging
        for(unsigned int j = 0; j < S.SM.size(); j++){
            int i = S.SM[j].idx;
            if (i == vt[0]){
                S.SM[j].mat+= dn*dn.transpose();
                t0flag=true;
            }
            if (i == vt[1]){
                S.SM[j].mat+= dn*dn.transpose();
                t1flag=true;
            }
            if (i == vt[2]){
                S.SM[j].mat+= dn*dn.transpose();
                t2flag=true;
            }
            }
        if (!t0flag)
            S.SM.push_back(S_t::matpt(dn*dn.transpose(),vt[0],tri.v[0],dn));
        if (!t1flag)
            S.SM.push_back(S_t::matpt(dn*dn.transpose(),vt[1],tri.v[1],dn));
        if (!t2flag)
            S.SM.push_back(S_t::matpt(dn*dn.transpose(),vt[2],tri.v[2],dn));
        collisions++; //add collision
       // if(collisions == 1)
         //   break;*/
        collisions++; //add collision
    }

    // block vertices if they have been marked above
    for(VertexConstraints::const_iterator it = vtxFixed.begin(); it != vtxFixed.end(); ++it)
        S.setZero(*it);
    return collisions;
}



void VertexTriangles::build(unsigned int n_ver, unsigned int n_tri, const Eigen::MatrixXi & connectivity)
{
    std::vector<std::vector<vtxpos> > adj1(n_ver);
    for (unsigned i = 0; i < adj1.size(); ++i)
        adj1[i].reserve(8); // most of vertices are 8

    for (int i = 0; i < n_tri; ++i)
        for (int j = 0; j < 3; ++j)
            adj1[connectivity.col(i)(j)].push_back(vtxpos(i,j));

    int ivtxstart = 0;
    for(int i = 0; i < n_ver; i++)
    {
        //if(!adj1[i].empty()) // no unconnected vertices
        vtxStart[i] = ivtxstart;
        std::copy(adj1[i].begin(),adj1[i].end(),vtxtriList.begin()+ivtxstart);
        ivtxstart += adj1[i].size();
    }
    vtxStart[n_ver] = ivtxstart;
}



template <int MAXTRI>
void VertexTrianglesMax<MAXTRI>::build(unsigned int n_ver, unsigned int n_tri, const Eigen::MatrixXi & connectivity)
{
    std::vector<std::vector<vtxpos> > adj1(n_ver);
    for (unsigned i = 0; i < adj1.size(); ++i)
        adj1[i].reserve(8); // most of vertices are 8

    for (unsigned int i = 0; i < n_tri; ++i)
        for (unsigned int j = 0; j < 3; ++j)
            adj1[connectivity.col(i)(j)].push_back(vtxpos(i,j));

    for(unsigned int i = 0; i < n_ver; i++)
    {
        //if(!adj1[i].empty()) // no unconnected vertices
        vtxtriList[i].count = adj1[i].size();
        for(int j = adj1[i].size(); j < 8; j++)
            adj1[i].push_back(vtxpos(n_tri,0));
        std::copy(adj1[i].begin(),adj1[i].end(),vtxtriList[i].values);
    }
}

namespace StepPhases
{
    const char *Names[End+1] = {"Start","UpdateShells","VertexGet","TriangleRot","SparseK","PreSPVM","SPVM","ExternalForces","ComputeB","ScaleSparseK","UpdateP","PCG","ComputeForces","UpdateXV","End"};
}

template <int NX, int NY>
struct piecevertex
{
    static unsigned int indexer(unsigned int x,unsigned int y, unsigned int w, unsigned int h)
    {
        unsigned int xpart = w/NX;
        unsigned int ypart = h/NY;
        unsigned int xblock = xpart*ypart; // each block size
        unsigned int yblock = xpart*ypart*NX; // each row of blocks
        int dx = x / xpart;
        int dy = y / ypart;
        int rx = x % xpart;
        int ry = y % ypart;
        return (rx + ry*xpart) + dx * xblock + dy * yblock;
    }

    static void inv_indexer(unsigned int idx,unsigned int & x, unsigned int & y, unsigned int w, unsigned int h)
    {
        unsigned int xpart = w/NX;
        unsigned int ypart = h/NY;
        unsigned int xblock = xpart*ypart; // each block size
        unsigned int yblock = xpart*ypart*NX; // each row of blocks

        unsigned int dy = idx / yblock;
        unsigned int idx2 = idx % yblock;
        unsigned int dx = idx2 / xblock;
        unsigned int idx3 = idx2 % xblock;
        x = dx*xpart + (idx3 % xpart);
        y = dy*ypart + (idx3 / xpart);
    }

};

inline Cloth::vertexindex_t Cloth::getVertexIndexer() const
{
    switch(conf.vertexOrder)
    {
    case -1:
        return mvertexindex;
    case 2:
        return piecevertex<2,2>::indexer;
    case 4:
        return piecevertex<4,4>::indexer;
    case 8:
        return piecevertex<8,8>::indexer;
    case 1:
    default:
        return rvertexindex;
    }
}

inline Cloth::inv_vertexindex_t Cloth::getInvVertexIndexer() const
{
    switch(conf.vertexOrder)
    {
    case -1:
        return inv_mvertexindex;
    case 2:
        return piecevertex<2,2>::inv_indexer;
    case 4:
        return piecevertex<4,4>::inv_indexer;
    case 8:
        return piecevertex<8,8>::inv_indexer;
    case 1:
    default:
        return inv_rvertexindex;
    }
}

// TODO more flexibility in n_tri definition
// this requires same number of triangles and vertices
bool Cloth::loadState(std::istream & ins, Configuration * override)
{
    Configuration c;
    ins >> c;
    if(c.W != conf.W|| c.H != conf.H)
    {
        std::cerr << "Cloth loadState incompatible sizes\n";
        return false;
    }    
    conf = c;
    std::string buffer;
    std::vector<NonScalar> buf;
    VectorX Xsave; // needed because we have to set X=X0 for initShells

    vtxFixed.clear();
    S.clear();

    while(ins)
    {
        getline(ins, buffer);
        if(!ins)
            break;
        std::cerr << " off " << ins.tellg() << std::endl;
        std::istringstream line(buffer);
        std::string element;
        int size = 0;
        int cols = 0;
        char type = 0;
        int itemsize = 0;

        line >> element >> size >> cols >> type >> itemsize;
        if(element == "X")
        {
            if(type != 'f')
            {
                std::cerr << "Invalid field types for: " << buffer << std::endl;
                return false;
            }
            else if(size != X.size() || cols != 3)
            {
                std::cerr << "Invalid size for: " << buffer << std::endl;
                return false;
            }
            else
            {
                Xsave.resize(X.size());
                if(itemsize == sizeof(Scalar))
                    ins.read((char*)Xsave.data(),sizeof(Scalar)*X.size());
                else
                {
                    buf.resize(X.size());
                    ins.read((char*)&buf[0],sizeof(Scalar)*buf.size());
                    std::copy(buf.begin(),buf.end(),(Scalar*)Xsave.data()); // Change Scalar Format
                }
            }
        }
        else if(element == "X0")
        {
            if(type != 'f')
            {
                std::cerr << "Invalid field types for: " << buffer << std::endl;
                return false;
            }
            else if(size != X.size() || cols != 3)
            {
                std::cerr << "Invalid size for: " << buffer << std::endl;
                return false;
            }
            else
            {
                if(itemsize == sizeof(Scalar))
                    ins.read((char*)X0.data(),sizeof(Scalar)*X0.size());
                else
                {
                    buf.resize(X0.size());
                    ins.read((char*)&buf[0],sizeof(Scalar)*buf.size());
                    std::copy(buf.begin(),buf.end(),(Scalar*)X0.data()); // Change Scalar Format
                }
            }
        }
        else if(element == "V")
        {
            if(type != 'f')
            {
                std::cerr << "Invalid field types for: " << buffer << std::endl;
                return false;
            }
            else if(size != V.size() || cols != 3)
            {
                std::cerr << "Invalid size for: " << buffer << std::endl;
                return false;
            }
            else
            {
                if(itemsize == sizeof(Scalar))
                    ins.read((char*)V.data(),sizeof(Scalar)*V.size());
                else
                {
                    buf.resize(V.size());
                    ins.read((char*)&buf[0],sizeof(Scalar)*buf.size());
                    std::copy(buf.begin(),buf.end(),(Scalar*)V.data()); // Change Scalar Format
                }
            }
        }
        else if(element == "T")
        {
            if(type != 'i' || itemsize != sizeof(int))
                std::cerr << "Invalid field types for: " << buffer << std::endl;
            else if(size != n_tri*3 || cols != 3)
                std::cerr << "Invalid size for: " << buffer << std::endl;
            else
            {
                connectivity.resize(3,n_tri);
                ins.read((char*)connectivity.data(),sizeof(int)*connectivity.size());
            }
        }
        else if(element == "c")
        {
            if(type != 'i' || itemsize != sizeof(int))
            {
                std::cerr << "Invalid field types for: " << buffer << std::endl;
                return false;
            }
            else if(cols != 1)
            {
                std::cerr << "Invalid size for: " << buffer << std::endl;
                return false;
            }
            else
            {
                vtxFixed.reserve(size);
                for(int i = 0; i < size; i++)
                {
                    int q;
                    ins.read((char*)&q,sizeof(q));
                    if(ins)
                    {
                        vtxFixed.add(q);
                        S.setZero(q);
                    }
                }
            }
        }
        else
        {
            std::cerr << "Skip unknown: " << buffer << std::endl;
            ins.seekg(size*itemsize,std::ios::cur);
        }

        if(!ins)
        {
            std::cerr << "Failure on Loading element " << buffer << std::endl;
            return false;
        }
    }

    if(override != 0)
    {
        // preserve non configuration parameters
        Configuration old = conf;
        conf = *override;
        conf.W = old.W;
        conf.H = old.H;
        conf.dx = old.dx;
        conf.dy = old.dy;
        conf.width = old.width;
        conf.height = old.height;
        conf.vertexOrder = old.vertexOrder;
        conf.gridLayout = old.gridLayout;
        conf.gridXZ = old.gridXZ;
        conf.gridCentered = old.gridCentered;
        conf.gridOffset = old.gridOffset;
        conf.Material = old.Material;
    }

	
    // These are performed above
    // initCoordinates(); DONE
    // initConnectivity(); DONE
    // initConstraints(); DONE
    std::cerr << "Shells\n";
        X = X0; // get the original configuration for rest_state
        initShells();
        X = Xsave; // load the new state (V is already there)
        // update
        for (int i = 0; i < n_tri; ++i)
            shells[i]->update();
    std::cerr << "Mass\n";
        M.setZero();
        G.setZero();
        initMass();
    std::cerr << "Adjacency\n";
        sp_idx.clear();
        //block_ref.clear();
        buildAdjacency();
    Y.setZero();
    Z.setZero();

    return true;
}

// remember to make it binary
void Cloth::saveState(std::ostream & ons) const
{
    ons << conf;
    ons << "EOF\r\n";


#define DUMPV(name)\
    ons << #name " " << name.size() << " " << VECTORSIZE << " f " << sizeof(Scalar) << "\r\n";\
    ons.write((const char*)name.data(),sizeof(Scalar)*name.size());
#define DUMPVNAMED(xname,name)\
    ons << #xname " " << name.size() << " " << VECTORSIZE << " f " << sizeof(Scalar) << "\r\n";\
    ons.write((const char*)name.data(),sizeof(Scalar)*name.size());

DUMPV(X)
DUMPV(X0)
DUMPV(V)
DUMPVNAMED(dV,deltaV)
DUMPV(Z)
DUMPV(Y)
DUMPV(F)
DUMPV(G)
DUMPV(Fr)
DUMPV(stiffedpostb)
DUMPV(stiffedpreb)
DUMPV(F0)
DUMPV(b)
DUMPV(P)
DUMPV(M)

    std::vector<Scalar> masses(shells.size());
    std::vector<Scalar> K(3*3*2*2*shells.size());
    std::vector<Scalar> Q(3*3*3*3*shells.size());
    std::vector<Scalar> f0plane(3*2*shells.size());
    std::vector<Scalar> tnormals(shells.size()*3);
    std::vector<Scalar> f0tri(3*3*shells.size());

    ons << "N " << N.size() << " " << 3 << " f " << sizeof(Scalar) << "\r\n";
    ons.write((const char*)N.data(),sizeof(Scalar)*N.size());

    ons << "T " << connectivity.size() << " 3 i " << sizeof(int) << "\r\n";
    ons.write((const char*)connectivity.data(),sizeof(int)*connectivity.size());

    ons << "fixed " << vtxFixed.vtxFixed.size() << " 1 i " << sizeof(int) << "\r\n";
    ons.write((const char*)&vtxFixed.vtxFixed[0],sizeof(int)*vtxFixed.size());

    ons << "h " << 1 << " 1 f " << sizeof(Scalar) << "\r\n";
    ons.write((const char*)&lasth,sizeof(Scalar));

    ons << "tol " << 1 << " 1 f " << sizeof(Scalar) << "\r\n";
    ons.write((const char*)&conf.lastTolerance,sizeof(Scalar));

    for(int i = 0; i < shells.size(); i++)
        masses[i] = shells[i]->nodal_mass();
    ons << "m " << shells.size() << " 1 f " << sizeof(Scalar) << "\r\n";
    ons.write((const char*)&masses[0],sizeof(Scalar)*masses.size());

    printf("Ks\n");
    Triangle::flattenKsorKb(shells,&K[0],true);
    ons << "Ks " << K.size() << " 36 f " << sizeof(Scalar) << "\r\n";
    ons.write((const char*)&K[0],sizeof(Scalar)*K.size());

    printf("Kb\n");
    Triangle::flattenKsorKb(shells,&K[0],false);
    ons << "Kb " << K.size() << " 36 f " << sizeof(Scalar) << "\r\n";
    ons.write((const char*)&K[0],sizeof(Scalar)*K.size());

    printf("Qs\n");
    Triangle::flattenQsorQmr(shells,&Q[0],true);
    ons << "Qs " << Q.size() << " 81 f " << sizeof(Scalar) << "\r\n";
    ons.write((const char*)&Q[0],sizeof(Scalar)*Q.size());

    printf("Qmr\n");
    Triangle::flattenQsorQmr(shells,&Q[0],false);
    ons << "Qmr " << Q.size() << " 81 f " << sizeof(Scalar) << "\r\n";
    ons.write((const char*)&Q[0],sizeof(Scalar)*Q.size());

    printf("f0plane\n");
    Triangle::flattenF0porFF(shells,&f0plane[0],true);
    ons << "f0plane " << f0plane.size() << " 6 f " << sizeof(Scalar) << "\r\n";
    ons.write((const char*)&f0plane[0],sizeof(Scalar)*f0plane.size());

    printf("formfactor\n");
    Triangle::flattenF0porFF(shells,&f0plane[0],false);
    ons << "formfactor " << f0plane.size() << " 6 f " << sizeof(Scalar) << "\r\n";
    ons.write((const char*)&f0plane[0],sizeof(Scalar)*f0plane.size());

    printf("f0tri\n");
    Triangle::flattenF0(shells,&f0tri[0]);
    ons << "f0tri " << f0tri.size() << " 9 f " << sizeof(Scalar) << "\r\n";
    ons.write((const char*)&f0tri[0],sizeof(Scalar)*f0tri.size());

    printf("TN\n");
    Triangle::flattenN(shells,&tnormals[0]);
    ons << "TN " << shells.size()*3 << " 3 f " << sizeof(Scalar) << "\r\n";
    ons.write((const char*)tnormals.data(),sizeof(Scalar)*tnormals.size());

	// trivtx or vtxtri
	
    // sparse matrix

    ons << "outer " << sp_idx.outer.size() << " 1 i " << sizeof(int) << "\r\n";
    ons.write((const char*)&sp_idx.outer[0],sizeof(int)*sp_idx.outer.size());

    ons << "inner " << sp_idx.inner.size() << " 1 i " << sizeof(int) << "\r\n";
    ons.write((const char*)&sp_idx.inner[0],sizeof(int)*sp_idx.inner.size());

    ons << "backinner " << sp_idx.backinner.size() << " 1 i " << sizeof(int) << "\r\n";
    ons.write((const char*)&sp_idx.backinner[0],sizeof(int)*sp_idx.backinner.size());

    ons << "symblock " << sp_idx.symblock.size() << " 1 i " << sizeof(int) << "\r\n";
    ons.write((const char*)&sp_idx.symblock[0],sizeof(int)*sp_idx.symblock.size());

    ons << "sparseblocks " << sparseblocks.size() << " 16 i " << sizeof(int) << "\r\n";
    ons.write((const char*)sparseblocks.data(),sizeof(int)*sparseblocks.size());

    ons << "A " << sparse_k.size()*MATRIXSIZE*MATRIXSIZE << " " << MATRIXSIZE*MATRIXSIZE << " f " << sizeof(Scalar) << "\r\n";
    ons.write((const char*)&sparse_k.data[0],sizeof(Scalar)*9*sparse_k.size());

    // constraints
    std::vector<int> SMi(S.size());
    std::vector<Scalar> SMd(S.size()*9);

    int i = 0;
    for(S_t::const_iterator it = S.begin(); it != S.end(); ++it)
    {
        memcpy(&SMd[i*9],it->mat.data(),sizeof(Scalar)*9);
        SMi[i] = it->idx;
        i++;
    }
    ons << "SMi " << S.size() << " " << 1 << " i " << sizeof(int) << "\r\n";
    ons.write((const char*)&SMi[0],SMi.size()*sizeof(int));

    ons << "SM " << SMd.size() << " " << 9 << " f " << sizeof(Scalar) << "\r\n";
    ons.write((const char*)&SMd[0],sizeof(Scalar)*SMd.size());

}

void Cloth::lockS()
{
    Smutex.lock();
}

void Cloth::unlockS()
{
    Smutex.unlock();
}

void Cloth::swapS()
{
    Smutex.lock();
        Sfront = 1-Sfront;
    Smutex.unlock();
}


void Cloth::reset()
{
    Y.setZero(); // cleanup of the displacement
    Z.setZero();
    S.clear(); // remove the collisions
    V.setZero(); // TODO future initial velocities
    deltaV.setZero(); // in case of ascher
    X = X0;
    for (int i = 0; i < n_tri; ++i)
        shells[i]->update();
}

VectorType Cloth::getCollisionResidual()
{
    VectorType force(VectorType::Zero());
    for(S_t::const_iterator it = S.begin(); it != S.end(); ++it)
        force += (Fr.segment<VECTORSIZE>(it->idx*VECTORSIZE)); // G.segment<3>(i*3)+Fr.segment<3>(it->idx*3)
    return force;
}


// project the cylinder along the vector DN
void Cylinder::Project(Scalar & min,Scalar & max, Vector3 DN) const
{
    Scalar UAX_N = DN.dot(U_HAX);
    Scalar dist = DN.dot(orig);
    Scalar e2 = extent* fabs(UAX_N); //abs?
    Scalar e3 = radius* sqrt(fabs(1 - UAX_N*UAX_N));

    min = dist - e2 - e3;
    max = dist + e2 + e3;

}

void Cloth::clear_external_forces()
{
    F.setZero();
}

void Cloth::apply_force(int ivtx,Vector3 f)
{
    F.segment<3>(ivtx*3) = f;
}


/*
    for(int i = 0; i < shells.size(); i++)
    {
        for(int a = 0; a < 3; a++)
            for(int b = 0; b < 3; b++)
                memcpy((p + i*9*9 + (a*3+b)*9),shells[i]->Qmr[a][b].data(),sizeof(Scalar)*9);
    }
    p = &Q[0];
    for(int i = 0; i < shells.size(); i++)
    {
        for(int a = 0; a < 3; a++)
            for(int b = 0; b < 3; b++)
                memcpy((p + i*9*9 + (a*3+b)*9),shells[i]->Qs[a][b].data(),sizeof(Scalar)*9);
    }
    p = &f0plane[0];
    for(int i = 0; i < shells.size(); i++)
    {
        for(int a = 0; a < 3; a++)
            memcpy((p + i*6 + a*2),shells[i]->f0_plane[a].data(),sizeof(Scalar)*2);
    }


*/

/*
Scalar * p = &K[0];
for(int i = 0; i < shells.size(); i++)
{
    for(int a = 0; a < 3; a++)
        for(int b = 0; b < 3; b++)
            memcpy((p + i*9*4 + (a*3+b)*4),shells[i]->Ks[a][b].data(),sizeof(Scalar)*4);
}
for(int i = 0; i < shells.size(); i++)
{
    for(int a = 0; a < 3; a++)
        for(int b = 0; b < 3; b++)
            memcpy((p + i*9*4 + (a*3+b)*4),shells[i]->Kb[a][b].data(),sizeof(Scalar)*4);
}
ons << "Kb " << K.size() << " 36 f " << sizeof(Scalar) << "\r\n";
ons.write((const char*)&K[0],sizeof(Scalar)*K.size());
*/
