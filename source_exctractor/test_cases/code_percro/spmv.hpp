/*
 * Onwer: PERCRO, Scuola Superiore S.Anna
 * License: Internal Code, Not Licensed
 *
 * Authors: initial Paolo Gasparello, then Emanuele Ruffaldi, Paolo Tripicchio
 * Descripotion: Sparse Matrix-Vector Multiplication
 */
#pragma once

#include <vector>
#include <iostream>
#include "types.hpp"

/// Sparse Matrix-Vector Multiplication (SPMV)
void spmvV(ParTag,const SparseIdx& idx, const SparseData& data, const VectorX& b, VectorX& res);

/// Sparse Matrix-Vector Multiplication (SPMV)
void spmvV(InParTag,const SparseIdx& idx, const SparseData& data, const VectorX& b, VectorX& res);

/// Sparse Matrix-Vector Multiplication (SPMV)
void spmvV(NoParTag,const SparseIdx& idx, const SparseData& data, const VectorX& b, VectorX& res);



/// Sparse Matrix-Vector Multiplication (SPMV)
void spmv(ParTag,const SparseIdx& idx, const SparseData& data, const VectorX& b, VectorX& res);

/// Sparse Matrix-Vector Multiplication (SPMV)
void spmv(InParTag,const SparseIdx& idx, const SparseData& data, const VectorX& b, VectorX& res);

/// Sparse Matrix-Vector Multiplication (SPMV)
void spmv(NoParTag,const SparseIdx& idx, const SparseData& data, const VectorX& b, VectorX& res);



/// Sparse Matrix-Vector Multiplication (SPMV)
template <int N>
void spmv(ParTag,const SparseIdxELL<N>& idx, const SparseData& data, const VectorX& b, VectorX& res);

/// Sparse Matrix-Vector Multiplication (SPMV)
template <int N>
void spmv(InParTag,const SparseIdxELL<N>& idx, const SparseData& data, const VectorX& b, VectorX& res);

/// Sparse Matrix-Vector Multiplication (SPMV)
template <int N>
void spmv(NoParTag,const SparseIdxELL<N>& idx, const SparseData& data, const VectorX& b, VectorX& res);


void dumpspvm(const SparseIdx& idx,const SparseData& data, std::ostream & onf);

void dumpspvmdata(const SparseIdx& idx,const SparseData& data, std::ostream & onf);
