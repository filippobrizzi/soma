/*
 * Onwer: PERCRO, Scuola Superiore S.Anna
 * License: Internal Code, Not Licensed
 *
 * Authors: initial Paolo Gasparello, then Emanuele Ruffaldi, Paolo Tripicchio
 * Descripotion: Triangle Class
 */
#pragma once
#include <vector>
#include <Eigen/Core>
#include "types.hpp"

#ifdef _MSC_VER
struct MapV3: public Eigen::Map<Vector3>
{
    MapV3(Scalar*p = 0): Eigen::Map<Vector3>(p) {}
};
#endif

class Triangle
{
public:
    // static part
    const material_t& mat; /// material
#ifndef _MSC_VER
    typedef Eigen::Map<Vector3> map_t;
#else
    typedef MapV3 map_t;
#endif
    map_t     v[3]; /// vertices expressed as mapped Vector3 pointing to data

    Vector2 p[3]; // form factor, used only for rest_state_material and rest_state
    Scalar voronoiArea[3]; // used only for voronoi_mass

    // material dependent static part
    Vector2 f0_plane[3]; /// vertex position in 2D plane dynamic
    Scalar mass;
    Matrix2 Ks[3][3], Kb[3][3];

    // dynamic part
    Vector3 f0[3];       /// vertex position after rotation dynamic
    Eigen::Matrix<Scalar,3,2> N; /// reference system of the triangle dynamic TEMPORARY
    Eigen::Matrix<Scalar,2,3> Nt; /// transpose of the above dynamic TEMPORARY
    Matrix3 Qs[3][3];
    Matrix3 Qmr[3][3];
    Vector3 normal;

    /// co-rotation matrix
    void get_corotate(Eigen::Matrix<Scalar,3,2>& e, Eigen::Matrix<Scalar,2,3>& et);

public:    
    Triangle(Scalar* a, Scalar* b, Scalar* c, const material_t &amat);
    void rest_state();
    void rest_state_material();
    void setZero();

    /// update the internal state of the triangle receiving (indirectly) the updates
    /// of the vertices that are shared
    void update();

    void update_bend(const Matrix3& Pma, const Matrix3& Pmb, const Matrix3& Pmc);

    const Matrix2& Kms(int a, int b) const { return Ks[a][b]; }
    const Matrix2& Kmb(int a, int b) const { return Kb[a][b]; }
    const Eigen::Matrix<Scalar,3,2>& Rm() const { return N; }
    const Matrix3& Qms(int a, int b) const { return Qs[a][b]; }

    // Memory Layout in C: [ROWS][COLUMNS] => ROW * COLUMNS + COLUMN
    const Matrix3& Qms(int idx) const { return ((Matrix3*)&Qs[0][0])[idx]; }
    //const Matrix3& Qmb(int idx) const { return ((Matrix3*)&Qb[0][0])[idx]; }
    const Matrix3& QmRot(int a, int b) const;
    const Vector3& F0(int i) const { return f0[i]; }
    const Scalar nodal_mass() const { return mass; }
    const Vector3& get_normal() const { return normal; }
    Scalar voronoi_mass(int irelvertex) const;


    // Qs[ntri][3][3][3][3] stored rowsize
    static void flattenQsorQmr(const std::vector<Triangle *>& t, Scalar * target, bool Ks);

    // Ks[ntri][3][3][2][2] stored rowsize
    static void flattenKsorKb(const std::vector<Triangle *>& t, Scalar * target, bool Ks);

    // F0plane[ntri][3][2] stored rowsize
    static void flattenF0porFF(const std::vector<Triangle *>& t, Scalar * target, bool F0);

    // Ks[ntri][3][3][2][2] stored rowsize
    static void unflattenKsorKb(std::vector<Triangle *>& t, const Scalar * target, bool Ks);

    // F0plane[ntri][3][2] stored rowsize
    // p[ntri][3][2]
    static void unflattenF0porFF(std::vector<Triangle *>& t, const Scalar * target, bool F0);

    // F0[ntri][3][3] stored rowsize
    static void flattenF0(const std::vector<Triangle *>& t,  Scalar * target);

    // N[ntri][3][3] stored rowsize
    static void flattenN(const std::vector<Triangle *>& t, Scalar * target);

    static int flattenF0orFFSize() { return 2*3; }

    static int flattenKsorKbSize() { return 3*3*2*2; }


    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};
