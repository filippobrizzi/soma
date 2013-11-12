/*bb
 * Onwer: PERCRO, Scuola Superiore S.Anna
 * License: Internal Code, Not Licensed
 *
 * Authors: initial Paolo Gasparello, then Emanuele Ruffaldi, Paolo Tripicchio
 * Descripotion: Cloth implementation
 * See: Fast Finite Element Solution for Cloth Modelling
 */
#include <Eigen/LU>
#include <Eigen/Geometry>
#include "viewer.h"
#include "Triangle.hpp"


// VERIFIED
#define SYMMETRIC

const bool checkSymmetry = false;

Triangle::Triangle(Scalar* a, Scalar* b, Scalar* c, const material_t & amat):
    mat(amat)
#ifndef _MSC_VER
  , v((Eigen::Map<Vector3>[3]){a,b,c})
#endif
{
#ifdef _MSC_VER
    v[0] = MapV3(a);
    v[1] = MapV3(b);
    v[2] = MapV3(c);
#endif
}

void Triangle::get_corotate(Eigen::Matrix<Scalar,3,2> & e, Eigen::Matrix<Scalar,2,3>& et)
{
    e.col(0) = (v[0] - v[2]).normalized();
    Vector3 e2 = (v[1] - v[2]).normalized();
    e.col(1) = (e2 - e.col(0).dot(e2) * e.col(0)).normalized();
    et = e.transpose();
}

void Triangle::rest_state()
{
    get_corotate(N,Nt);
    for (int i = 0; i < 3; ++i)
        p[i] = Nt * v[i];


    // 1/8 othertwovectorsnormals by cotangent of relative angle
    Vector2 P(p[0]);
    Vector2 Q(p[1]);
    Vector2 R(p[2]);
    Vector2 PQ = P-Q;
    Vector2 PR = P-R;
    Vector2 RQ = R-Q;
    Scalar nPQ = PQ.norm();
    Scalar nPR = PR.norm();
    Scalar nRQ = RQ.norm();
    Scalar dPRPQ = (-PR.dot(-PQ))/(nPR*nPQ);
    Scalar dPQRQ = (PQ.dot(RQ))/(nPQ*nRQ);
    Scalar dPRRQ = (PR.dot(-RQ))/(nPR*nRQ);
    Scalar cP = 1/tan(acos(dPRPQ));
    Scalar cQ = 1/tan(acos(dPQRQ ));
    Scalar cR = 1/tan(acos(dPRRQ));
    Scalar nRQcP = nRQ*nRQ*cP;
    Scalar nPRcQ = nPR*nPR*cQ;
    Scalar nPQcR = nPQ*nPQ*cR;

    voronoiArea[0] = ( nPQcR + nPRcQ )/8.0;
    voronoiArea[1] = ( nPQcR + nRQcP )/8.0;
    voronoiArea[2] = ( nPRcQ + nRQcP )/8.0;

    // (2*nPQcR+2nPRcQ+2*nRQcP)/8 = (nRQ/tan(aP)+nPR/tan(aQ)+nPQ/tan(aR))/4


    Matrix3 m;
    m <<
        1, p[0].transpose(),
        1, p[1].transpose(),
        1, p[2].transpose();
    Scalar det = m.determinant();
    Scalar area = det / 2.0;
    rest_state_material();
}

Scalar Triangle::voronoi_mass(int irelvertex) const
{
    return voronoiArea[irelvertex]*mat.mass_density;
}

void Triangle::rest_state_material()
{
    Matrix3 m;
    m <<
        1, p[0].transpose(),
        1, p[1].transpose(),
        1, p[2].transpose();
    Scalar det = m.determinant();
    Scalar area = det / 2.0;
    //Scalar area = 0.5*((v[1]-v[0]).cross(v[0]-v[2])).norm();

    mass = area * mat.mass_density / 3.0;


    Eigen::Matrix<Scalar,2,3> B; // form factor

     // form factor dN_j/du_i  j=a,b,c i=1,2
    B <<
        p[1].y() - p[2].y(), p[2].y() - p[0].y(), p[0].y() - p[1].y(),
        p[2].x() - p[1].x(), p[0].x() - p[2].x(), p[1].x() - p[0].x();
    B /= det;

    Scalar C[16];
    std::fill(C, C + 16, 0.0);
    C[0] = mat.weft_stretch;
    C[15] = mat.warp_stretch;
    C[5] = C[10] = C[9] = C[6] = mat.shear_modulus;
    C[3] = C[12] = mat.transverse_contraction;

    // B is now 1/m
    // C is symmetric and N/m
        for (int a = 0; a < 3; ++a)
            for (int b = 0; b < 3; ++b)
        {
            Matrix2& K = Ks[a][b];
            K.setZero();
            for (int i = 0; i < 2; ++i)
                for (int j = 0; j < 2; ++j)
                {
                    Scalar z = 0;
                    for (int k = 0; k < 2; ++k)
                        for (int l = 0; l < 2; ++l) {

                           {
                            int idx = l + 2 * k + 4 * j + 8 * i;
                              z += B(k, a) * C[idx] * B(l, b); // 1/m^2 * N/m = N/m^3
                          }
                        }
                    K(i,j) = z;
                }
            K *= area; // N/m
            // no B/= det => K *= 0.5
        }
        for (int a = 0; a < 3; ++a)
            for (int b = 0; b < 3; ++b)
        {
            Matrix2& K = Kb[a][b];
            K <<
                B(0, a) * mat.weft_bend * B(0, b), 0,
                0, B(1, a) * mat.warp_bend * B(1, b);
            K *= area; // N/m
        }

    // Kab xasig
    // xasig is in planar coordinates
    for (int i = 0; i < 3; ++i)
    {
        f0_plane[i].setZero();
        for (int j = 0; j < 3; ++j)
            f0_plane[i] += Ks[i][j] * p[j]; // N
    }


    if(checkSymmetry)
    {
    for (int a = 0; a < 3; ++a)
        for (int b = 0; b < 3; ++b)
        {
        if(!Ks[a][b].isApprox(Ks[a][b].transpose()))
        {
            printf("Ks(a,b) not symmetric: %d,%d\n",a,b);
            std::cout << Ks[a][b] << std::endl;
            getchar();
                    exit(0);
        }

            if(!Ks[a][b].isApprox(Ks[b][a].transpose()))
            {
                printf("Ks(a,b) != Ks(b,a)' %d,%d\n",a,b);
                std::cout << Ks[a][b] << std::endl;
                std::cout << Ks[b][a] << std::endl;
                getchar();
                        exit(0);
            }

        }
    }

    update();
}

void Triangle::update()
{
    get_corotate(N,Nt);

    Eigen::Matrix<Scalar,3,2> Rm = N;
    Eigen::Matrix<Scalar,2,3> Rmt = Nt;

    // current vertices => planar representation => is this the correct R^m?
    //

    // Qs = R^m Ks (R^m)'
#ifdef SYMMETRIC
        for (int a = 0; a < 3; ++a)
            for (int b = a; b < 3; ++b)
            {
                Qs[a][b] = Rm * Ks[a][b] * Rmt;
                if(a != b)
                    Qs[b][a] = Qs[a][b];
            }
#else
        for (int a = 0; a < 3; ++a)
            for (int b = 0; b < 3; ++b)
            {
                Qs[a][b] = Rm * Ks[a][b] * Rmt;
                //Qb[a][b] = N * Kb[a][b] * Nt;
                }
#endif
    for (int i = 0; i < 3; ++i)
                f0[i] = Rm * f0_plane[i];
    normal = (v[1] - v[2]).cross(v[0] - v[2]);
    normal.normalize();

    if(checkSymmetry)
    {
        for (int a = 0; a < 3; ++a)
            for (int b = 0; b < 3; ++b)
            {
                if(!Qs[a][b].isApprox(Qs[b][a].transpose()))
                {
                    printf("Qs(a,b) != Qs(b,a)'\n");
                    getchar();
                            exit(0);
                }

                // single block symmetry
                if(!Qs[a][b].isApprox(Qs[a][b].transpose()))
                {
                    printf("Qs is not a symmetric matrix\n");
                    getchar();
                            exit(0);
                }
            }
    }
}

// TODO: Qb computed here is different from the one from update, because it is actually rotated => GPU code and self update
void Triangle::update_bend(const Matrix3& Pma, const Matrix3& Pmb, const Matrix3& Pmc)
{
    Eigen::Matrix<Scalar,3,2> ma,mb,mc;
    Eigen::Matrix<Scalar,2,3> mat,mbt,mct;

    Eigen::Matrix<Scalar,3,2> Rm = N;
    // Pma = n*n' => Pma' = n'*n
    // ma = Pma * N = n*n'*N
    // ma' = N' * n*n' = N'*Pma
    // exactly what we need
    ma = Pma * Rm;
    mat = ma.transpose();
    mb = Pmb * Rm;
    mbt = mb.transpose();
    mc = Pmc * Rm;
    mct = mc.transpose();
#ifdef SYMMETRIC
        for (int a = 0; a < 3; ++a)
            for (int b = a; b < 3; ++b)
            {
                Qmr[a][b] = Qs[a][b] + ((ma * Kb[a][b] * mat)+(mb * Kb[a][b] * mbt)+(mc * Kb[a][b] * mct)) / 3.0;
                if(a != b)
                    Qmr[b][a] = Qmr[a][b];
            }
#else
        // Qbbase = N Kb[a][b] N'
        // Original: Qbnew = (Pma Qbase Pma) + ...
        //
        // Qbnew = (Pma N Kb[a][b] N' Pma)
        // fisso ma := Pma N
        // ma' = N' Pma
        // Qbnew = (ma Kb[a][b] ma')
        for (int a = 0; a < 3; ++a)
            for (int b = 0; b < 3; ++b)
                Qmr[a][b] = Qs[a][b] + ((ma * Kb[a][b] * mat)+(mb * Kb[a][b] * mbt)+(mc * Kb[a][b] * mct)) / 3.0;
#endif
}


void Triangle::setZero()
{
    for(int i = 0; i < 3; i++)
        f0[i].setZero();
    normal.setZero();
    mass = 0;
    N.setZero();
    for (int a = 0; a < 3; ++a)
        for (int b = 0; b < 3; ++b)
        {
            Ks[a][b].setZero();
            Kb[a][b].setZero();
            //Qb[a][b].setZero();
            Qs[a][b].setZero();
            Qmr[a][b].setZero();
        }
}

const Matrix3& Triangle::QmRot(int a, int b) const
{
#ifdef SYMMETRIC
    return a <= b ? Qmr[a][b] : Qmr[b][a];
#else
    return Qmr[a][b];
#endif
}

void Triangle::flattenKsorKb(const std::vector<Triangle *>& t, Scalar * target, bool Ks)
{
    int ntri = t.size();
    for(int i = 0; i < ntri; i++)
    {
        const Triangle * tris = t[i];
        for(int a = 0; a < 3; a++)
        {
            for(int b = 0; b < 3; b++)
            {
                // Eigen default is column, major, we do row major
                const Matrix2 & m = Ks ? tris->Ks[a][b] : tris->Kb[a][b];
                *target++ = m(0,0);
                *target++ = m(0,1);
                *target++ = m(1,0);
                *target++ = m(1,1);
            }
        }
        tris++;
    }
}


void Triangle::flattenQsorQmr(const std::vector<Triangle *>& t, Scalar * target, bool fQs)
{
    int ntri = t.size();
    for(int i = 0; i < ntri; i++)
    {
        const Triangle * tris = t[i];
        for(int a = 0; a < 3; a++)
        {
            for(int b = 0; b < 3; b++)
            {
                // Eigen default is column, major, we do row major
                const Matrix3 & m = fQs ? tris->Qs[a][b] : tris->Qmr[a][b];
                *target++ = m(0,0);
                *target++ = m(0,1);
                *target++ = m(0,2);
                *target++ = m(1,0);
                *target++ = m(1,1);
                *target++ = m(1,2);
                *target++ = m(2,0);
                *target++ = m(2,1);
                *target++ = m(2,2);
            }
        }
        tris++;
    }
}

void Triangle::flattenN(const std::vector<Triangle *>& t, Scalar * target)
{
    int ntri = t.size();
    for(int i = 0; i < ntri; i++)
    {
        const Triangle * tris = t[i];
        (Eigen::Map<Vector3>(target)) = tris->normal;
    }
}

void Triangle::flattenF0(const std::vector<Triangle *>& t, Scalar * target)
{
    int ntri = t.size();
    for(int i = 0; i < ntri; i++)
    {
        const Triangle * tris = t[i];
        for(int a = 0; a < 3; a++, target += 3)
            (Eigen::Map<Vector3>(target)) = tris->f0[a];
    }
}

void Triangle::flattenF0porFF(const std::vector<Triangle *>& t, Scalar * target,bool F0)
{
    int ntri = t.size();
    for(int i = 0; i < ntri; i++)
    {
        const Triangle * tris = t[i];
        for(int a = 0; a < 3; a++, target += 2)
            (Eigen::Map<Vector2>(target)) = F0 ? tris->f0_plane[a] : tris->p[a];
    }
}

void Triangle::unflattenKsorKb( std::vector<Triangle *>& t,  const Scalar * target, bool Ks)
{
    int ntri = t.size();
    for(int i = 0; i < ntri; i++)
    {
        Triangle * tris = t[i];
        for(int a = 0; a < 3; a++)
        {
            for(int b = 0; b < 3; b++)
            {
                // Eigen default is column, major, we do row major
                Matrix2 & m = Ks ? tris->Ks[a][b] : tris->Kb[a][b];
                m(0,0) = *target++;
                m(0,1) = *target++;
                m(1,0) = *target++;
                m(1,1) = *target++;
            }
        }
        tris++;
    }
}

void Triangle::unflattenF0porFF( std::vector<Triangle *>& t, const Scalar * target, bool F0)
{
    int ntri = t.size();
    if(F0)
        for(int i = 0; i < ntri; i++)
        {
        Triangle * tris = t[i];
            for(int a = 0; a < 3; a++, target += 2)
                tris->f0_plane[a] = (Eigen::Map<Vector2>((Scalar*)target));
        }
    else
        for(int i = 0; i < ntri; i++)
        {
        Triangle * tris = t[i];
            for(int a = 0; a < 3; a++, target += 2)
                tris->p[a] = (Eigen::Map<Vector2>((Scalar*)target));
        }

}



