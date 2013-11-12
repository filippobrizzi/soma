/*
 * Onwer: PERCRO, Scuola Superiore S.Anna
 * License: Internal Code, Not Licensed
 *
 * Authors: initial Paolo Gasparello, then Emanuele Ruffaldi, Paolo Tripicchio
 * Descripotion: Sparse Matrix-Vector Multiplication
 * See http://www.cslab.ece.ntua.gr/cgi-bin/twiki/view/CSLab/SPMV
 */
#include "spmv.hpp"


void spmvV(NoParTag,const SparseIdx& idx, const SparseData& data, const VectorX& b, VectorX& res)
{
    // better not collapse because of locality of data
    // complexity is: prod[n_ver x 9 x outer[-1]] + sum[n_ver x 9 x outer[-1]]
    // where n_blocks is sum(adj_i) < 8 * n_ver
    for (size_t i = 0; i < idx.cols(); ++i)
    {
        VectorType r = VectorType::Zero();
        r += data.data[i] * b.segment<VECTORSIZE>(VECTORSIZE*i);
        for (size_t k = idx.outer[i]; k < idx.outer[i + 1]; ++k)
            r += data.data[k] * b.segment<VECTORSIZE>(VECTORSIZE*idx.inner[k]);
        res.segment<VECTORSIZE>(VECTORSIZE * i) = r;
    }

    // should be exactly 0 times, but in case...
    //EIGEN3: res.tail(res.size()-idx.cols()*3).setZero();
    const size_t last = res.size();
    for (size_t i = idx.cols()*VECTORSIZE; i < last; ++i)
        res[i] = 0;
}


void spmvV(ParTag,const SparseIdx& idx, const SparseData& data, const VectorX& b, VectorX& res)
{
    // better not collapse because of locality of data
    // complexity is: prod[n_ver x 9 x outer[-1]] + sum[n_ver x 9 x outer[-1]]
    // where n_blocks is sum(adj_i) < 8 * n_ver
    #pragma omp parallel for
    for (size_t i = 0; i < idx.cols(); ++i)
    {
        VectorType r = VectorType::Zero();
        r += data.data[i] * b.segment<VECTORSIZE>(VECTORSIZE*i);
        for (size_t k = idx.outer[i]; k < idx.outer[i + 1]; ++k)
            r += data.data[k] * b.segment<VECTORSIZE>(VECTORSIZE*idx.inner[k]);
        res.segment<VECTORSIZE>(VECTORSIZE * i) = r;
    }

    // should be exactly 0 times, but in case...
    //EIGEN3: res.tail(res.size()-idx.cols()*3).setZero();
    const size_t last = res.size();
    for (size_t i = idx.cols()*VECTORSIZE; i < last; ++i)
        res[i] = 0;
}

void spmvV(InParTag,const SparseIdx& idx, const SparseData& data, const VectorX& b, VectorX& res)
{
    // better not collapse because of locality of data
    // complexity is: prod[n_ver x 9 x outer[-1]] + sum[n_ver x 9 x outer[-1]]
    // where n_blocks is sum(adj_i) < 8 * n_ver    
    #pragma omp for
    for (size_t i = 0; i < idx.cols(); ++i)
    {
        VectorType r = VectorType::Zero();
        r += data.data[i] * b.segment<VECTORSIZE>(VECTORSIZE*i);
        for (size_t k = idx.outer[i]; k < idx.outer[i + 1]; ++k)
            r += data.data[k] * b.segment<VECTORSIZE>(VECTORSIZE*idx.inner[k]);
        res.segment<VECTORSIZE>(VECTORSIZE * i) = r;
    }

    /*
        // should be exactly 0 times, but in case...
        //EIGEN3: res.tail(res.size()-idx.cols()*3).setZero();
        const size_t last = res.size();
        for (size_t i = idx.cols()*3; i < last; ++i)
            res[i] = 0;
    */
}


template <int N>
void spmvV(NoParTag,const SparseIdxELL<N>& idx, const SparseData& data, const VectorX& b, VectorX& res)
{
    // better not collapse because of locality of data
    // complexity is: prod[n_ver x 9 x outer[-1]] + sum[n_ver x 9 x outer[-1]]
    // where n_blocks is sum(adj_i) < 8 * n_ver
    for (size_t i = 0; i < idx.cols(); ++i)
    {
        VectorType r = VectorType::Zero();
        unsigned j;
        r += data.data[i] * b.segment<VECTORSIZE>(VECTORSIZE*i);
        for (size_t k = 0; k < N && (j = idx.inner[i*N+k]) >= 0; k++)
            r += data.data[i*N+k] * b.segment<VECTORSIZE>(VECTORSIZE*j);
        res.segment<VECTORSIZE>(VECTORSIZE * i) = r;
    }

    // should be exactly 0 times, but in case...
    //EIGEN3: res.tail(res.size()-idx.cols()*3).setZero();
    const size_t last = res.size();
    for (size_t i = idx.cols()*VECTORSIZE; i < last; ++i)
        res[i] = 0;
}

template <int N>

void spmvV(ParTag,const SparseIdx& idx, const SparseData& data, const VectorX& b, VectorX& res)
{
    // better not collapse because of locality of data
    // complexity is: prod[n_ver x 9 x outer[-1]] + sum[n_ver x 9 x outer[-1]]
    // where n_blocks is sum(adj_i) < 8 * n_ver
    #pragma omp parallel for
    for (size_t i = 0; i < idx.cols(); ++i)
    {
        VectorType r = VectorType::Zero();
        r += data.data[i] * b.segment<VECTORSIZE>(VECTORSIZE*i);
        unsigned j;
        for (size_t k = 0; k < N && (j = idx.inner[i*N+k]) >= 0; k++)
            r += data.data[i*N+k] * b.segment<VECTORSIZE>(VECTORSIZE*j);
        res.segment<VECTORSIZE>(VECTORSIZE * i) = r;
    }

    // should be exactly 0 times, but in case...
    //EIGEN3: res.tail(res.size()-idx.cols()*3).setZero();
    const size_t last = res.size();
    for (size_t i = idx.cols()*VECTORSIZE; i < last; ++i)
        res[i] = 0;
}

template <int N>

void spmvV(InParTag,const SparseIdx& idx, const SparseData& data, const VectorX& b, VectorX& res)
{
    // better not collapse because of locality of data
    // complexity is: prod[n_ver x 9 x outer[-1]] + sum[n_ver x 9 x outer[-1]]
    // where n_blocks is sum(adj_i) < 8 * n_ver
    #pragma omp for
    for (size_t i = 0; i < idx.cols(); ++i)
    {
        VectorType r = VectorType::Zero();
        r += data.data[i] * b.segment<VECTORSIZE>(VECTORSIZE*i);
        unsigned j;
        for (size_t k = 0; k < N && (j = idx.inner[i*N+k]) >= 0; k++)
            r += data.data[i*N+k] * b.segment<VECTORSIZE>(VECTORSIZE*j);
        res.segment<VECTORSIZE>(VECTORSIZE * i) = r;
    }

    /*
        // should be exactly 0 times, but in case...
        //EIGEN3: res.tail(res.size()-idx.cols()*3).setZero();
        const size_t last = res.size();
        for (size_t i = idx.cols()*3; i < last; ++i)
            res[i] = 0;
    */
}
void spmv(NoParTag,const SparseIdx& idx, const SparseData& data, const VectorX& b, VectorX& res)
{
    // better not collapse because of locality of data
    // complexity is: prod[n_ver x 9 x outer[-1]] + sum[n_ver x 9 x outer[-1]]
    // where n_blocks is sum(adj_i) < 8 * n_ver
    for (size_t i = 0; i < idx.cols(); ++i)
        for (int j = 0; j < VECTORSIZE; ++j) {
            Scalar r = 0;
            const MatrixType & m = data.data[i];
            for (int l = 0; l < VECTORSIZE; ++l)
                r += m(j, l) * b(VECTORSIZE * i + l);

            for (size_t k = idx.outer[i]; k < idx.outer[i + 1]; ++k)
            {
                const MatrixType & m = data.data[k];
                for (int l = 0; l < VECTORSIZE; ++l)
                    r += m(j, l) * b(VECTORSIZE * idx.inner[k] + l);
            }
            res(VECTORSIZE * i + j) = r;
        }
    // should be exactly 0 times, but in case...
    //EIGEN3: res.tail(res.size()-idx.cols()*3).setZero();
    const size_t last = res.size();
    for (size_t i = idx.cols()*VECTORSIZE; i < last; ++i)
        res[i] = 0;
}


#define PRODUCTLIMITCOORD 3

void spmv(ParTag,const SparseIdx& idx, const SparseData& data, const VectorX& b, VectorX& res)
{
    // better not collapse because of locality of data
    // complexity is: prod[n_ver x 9 x outer[-1]] + sum[n_ver x 9 x outer[-1]]
    // where n_blocks is sum(adj_i) < 8 * n_ver
    #pragma omp parallel for
    for (size_t i = 0; i < idx.cols(); ++i)
        for (int j = 0; j < PRODUCTLIMITCOORD; ++j) { // only first three
            Scalar r = 0;
            const MatrixType & m = data.data[i];
            for (int l = 0; l < VECTORSIZE; ++l)
                r += m(j, l) * b(VECTORSIZE * i + l);

            for (size_t k = idx.outer[i]; k < idx.outer[i + 1]; ++k)
            {
                const MatrixType & m = data.data[k];
                for (int l = 0; l < PRODUCTLIMITCOORD; ++l) // only first three coordinates
                    r += m(j, l) * b(VECTORSIZE * idx.inner[k] + l);
            }
            res(VECTORSIZE * i + j) = r;
        }
    // should be exactly 0 times, but in case...
    //EIGEN3: res.tail(res.size()-idx.cols()*3).setZero();
    const size_t last = res.size();
    for (size_t i = idx.cols()*VECTORSIZE; i < last; ++i)
        res[i] = 0;
}

void spmv(InParTag,const SparseIdx& idx, const SparseData& data, const VectorX& b, VectorX& res)
{
    // better not collapse because of locality of data
    // complexity is: prod[n_ver x 9 x outer[-1]] + sum[n_ver x 9 x outer[-1]]
    // where n_blocks is sum(adj_i) < 8 * n_ver
    #pragma omp for
    for (size_t i = 0; i < idx.cols(); ++i)
        for (int j = 0; j < PRODUCTLIMITCOORD; ++j) { // ONLY FIRST 3 ROWS
            Scalar r = 0;
            const MatrixType & m = data.data[i];
            for (int l = 0; l < VECTORSIZE; ++l)
                r += m(j, l) * b(VECTORSIZE * i + l);

            for (size_t k = idx.outer[i]; k < idx.outer[i + 1]; ++k)
            {
                const MatrixType & m = data.data[k];
                for (int l = 0; l < PRODUCTLIMITCOORD; ++l) // ONLY FIRST 3 ROWS
                    r += m(j, l) * b(VECTORSIZE * idx.inner[k] + l);
            }
            res(3 * i + j) = r;
        }

    /*
        // should be exactly 0 times, but in case...
        //EIGEN3: res.tail(res.size()-idx.cols()*3).setZero();
        const size_t last = res.size();
        for (size_t i = idx.cols()*3; i < last; ++i)
            res[i] = 0;
    */
}




template <int N>

void spmv(NoParTag,const SparseIdxELL<N>& idx, const SparseData& data, const VectorX& b, VectorX& res)
{
    // better not collapse because of locality of data
    // complexity is: prod[n_ver x 9 x outer[-1]] + sum[n_ver x 9 x outer[-1]]
    // where n_blocks is sum(adj_i) < 8 * n_ver
    for (size_t i = 0; i < idx.cols(); ++i)
    {
        const MatrixType & m = data.data[i];
        for (int j = 0; j < VECTORSIZE; ++j) {
            Scalar r = 0;
            size_t c = N;
            // identity
            for (int l = 0; l < VECTORSIZE; ++l)
                r += m(j, l) * b(VECTORSIZE * i + l);

            for (size_t k = 0; k < c; ++k)
            {
                int q = idx.innerstart[i*N+k];
                if(q < 0)
                {
                    c = k;
                    break;
                }
                const MatrixType & m = data.data[i*N+k];
                for (int l = 0; l < VECTORSIZE; ++l)
                    r += m(q, l) * b(VECTORSIZE * q + l);
            }
            res(VECTORSIZE * i + j) = r;
        }
    }
    // should be exactly 0 times, but in case...
    //EIGEN3: res.tail(res.size()-idx.cols()*3).setZero();
    const size_t last = res.size();
    for (size_t i = idx.cols()*VECTORSIZE; i < last; ++i)
        res[i] = 0;
}


#define PRODUCTLIMITCOORD 3

template <int N>
void spmv(ParTag,const SparseIdxELL<N>& idx, const SparseData& data, const VectorX& b, VectorX& res)
{
    // better not collapse because of locality of data
    // complexity is: prod[n_ver x 9 x outer[-1]] + sum[n_ver x 9 x outer[-1]]
    // where n_blocks is sum(adj_i) < 8 * n_ver
    #pragma omp parallel for
    for (size_t i = 0; i < idx.cols(); ++i)
        for (int j = 0; j < PRODUCTLIMITCOORD; ++j) { // only first three
            Scalar r = 0;
            size_t c = N;
            const MatrixType & m = data.data[i];
            for (int l = 0; l < VECTORSIZE; ++l)
                r += m(j, l) * b(VECTORSIZE * i + l);
            for (size_t k = 0; k < c; ++k)
            {
                int q = idx.innerstart[i*N+k];
                if(q < 0)
                {
                    c = k;
                    break;
                }
                const MatrixType & m = data.data[i*N+k];
                for (int l = 0; l < PRODUCTLIMITCOORD; ++l) // only first three coordinates
                    r += m(q, l) * b(VECTORSIZE * q + l);
            }
            res(VECTORSIZE * i + j) = r;
        }
    // should be exactly 0 times, but in case...
    //EIGEN3: res.tail(res.size()-idx.cols()*3).setZero();
    const size_t last = res.size();
    for (size_t i = idx.cols()*VECTORSIZE; i < last; ++i)
        res[i] = 0;
}

template <int N>


void spmv(InParTag,const SparseIdxELL<N>& idx, const SparseData& data, const VectorX& b, VectorX& res)
{
    // better not collapse because of locality of data
    // complexity is: prod[n_ver x 9 x outer[-1]] + sum[n_ver x 9 x outer[-1]]
    // where n_blocks is sum(adj_i) < 8 * n_ver
    #pragma omp for
    for (size_t i = 0; i < idx.cols(); ++i)
        for (int j = 0; j < PRODUCTLIMITCOORD; ++j) { // ONLY FIRST 3 ROWS
            Scalar r = 0;
            size_t c = N;
            const MatrixType & m = data.data[i];
            for (int l = 0; l < VECTORSIZE; ++l)
                r += m(j, l) * b(VECTORSIZE * i + l);
            for (size_t k = 0; k < N; ++k)
            {
                int q = idx.innerstart[i*N+k];
                if(q < 0)
                {
                    c = k;
                    break;
                }
                const MatrixType & m = data.data[i*N+k];
                for (int l = 0; l < PRODUCTLIMITCOORD; ++l) // ONLY FIRST 3 ROWS
                    r += m(j, l) * b(VECTORSIZE * q + l);
            }
            res(3 * i + j) = r;
        }

    /*
        // should be exactly 0 times, but in case...
        //EIGEN3: res.tail(res.size()-idx.cols()*3).setZero();
        const size_t last = res.size();
        for (size_t i = idx.cols()*3; i < last; ++i)
            res[i] = 0;
    */
}

// in MATLAB: S = sparse(m(:,1)+1,m(:,2)+1,m(:,3)+1); or S = spconvert
void dumpspvm(const SparseIdx& idx,const SparseData& data, std::ostream & onf)
{
    for (size_t i = 0; i < idx.cols(); ++i)
        for (size_t k = idx.outer[i]; k < idx.outer[i + 1]; ++k)
            onf << i << " " << idx.inner[k] << " " << k << std::endl;
}

void dumpspvmdata(const SparseIdx& idx,const SparseData& data, std::ostream & onf)
{
    for (size_t i = 0; i < data.size(); ++i)
    {
        onf << data.data[i].data()[0];
        for (int j = 1; j < 9; j++)
            onf << ' ' << data.data[i].data()[j];
        onf << std::endl;
    }
}


/// Sparse Matrix-Vector Multiplication (SPMV)
template
void spmv<9>(ParTag,const SparseIdxELL<9>& idx, const SparseData& data, const VectorX& b, VectorX& res);

/// Sparse Matrix-Vector Multiplication (SPMV)
template
void spmv<9>(InParTag,const SparseIdxELL<9>& idx, const SparseData& data, const VectorX& b, VectorX& res);

/// Sparse Matrix-Vector Multiplication (SPMV)
template
void spmv<9>(NoParTag,const SparseIdxELL<9>& idx, const SparseData& data, const VectorX& b, VectorX& res);


int SparseIdx::getBlockIndex(int i, int j)
{
    if(digonalFirst && i == j)
        return i;

    int k1 = outer[i];
    int k2 = outer[i+1];
    for(int k = k1; k < k2; k++)
    {
        if(inner[k] == j)
            return k;
    }
    return -1;
}
