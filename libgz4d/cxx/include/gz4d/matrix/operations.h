// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_MATRIX_OPERATIONS_H
#define GZ4D_MATRIX_OPERATIONS_H

#include <gz4d/Matrix.h>

namespace gz4d
{
    template <typename T, std::size_t M, std::size_t N> Matrix<T,N,M> transpose(Matrix<T,M,N> const &m)
    {
        Matrix<T,N,M> ret;
        for(std::size_t r = 0; r < M; ++r)
            for(std::size_t c = 0; c < N; ++c)
                ret(c,r) = m(r,c);
        return ret;
    }

    template <typename T> inline Matrix<T,1,1> inverse(Matrix<T,1,1> const &m)
    {
        return Matrix<T,1,1>(1.0/m(0,0));
    }

    template <typename T> inline T determinant(Matrix<T,2,2> const &m)
    {
        return m(0,0)*m(1,1)-m(0,1)*m(1,0);
    }

    template <typename T> inline T determinant(Matrix<T,3,3> const &m)
    {
        return m(0,0)*m(1,1)*m(2,2)+m(0,1)*m(1,2)*m(2,0)+m(0,2)*m(1,0)*m(2,1)-m(0,0)*m(1,2)*m(2,1)-m(0,1)*m(1,0)*m(2,2)-m(0,2)*m(1,1)*m(2,0);
    }
    
    template <typename T, std::size_t N> inline T determinant(Matrix<T,N,N> const &m)
    {
        T ret = 0;
        for(std::size_t i = 0; i < N; ++i)
            ret += m(i,0)*cofactor(m,i,0);
        return ret;
    }
    
    /// Calculates Cij of matrix m.
    /// http://en.wikipedia.org/wiki/Cofactor_(linear_algebra)
    /// Cij=(-1)^(i+j)*Mij
    /// Where Mij is determinant of the submatrix obtained by removing from m its i-th row and j-th column.
    template <typename T, std::size_t N> inline T cofactor(Matrix<T,N,N> const &m, std::size_t i, std::size_t j)
    {
        Matrix<T,N-1,N-1> minor_matrix;
        std::size_t mi = 0;
        for(std::size_t in_row = 0; in_row < N && mi < N-1; ++in_row)
        {
            std::size_t mj = 0;
            for(std::size_t in_col = 0; in_col < N && mj < N-1; ++in_col)
            {
                minor_matrix(mi,mj)=m(in_row,in_col);
                if(in_col != j)
                    ++mj;
            }
            if (in_row != i)
                ++mi;
        }
        T ret = determinant(minor_matrix);
        if((i+j)%2==1)
            return -ret;
        return ret;
    }

    /// Transpose of the cofactor matrix.
    /// http://en.wikipedia.org/wiki/Adjugate_matrix
    template <typename T, std::size_t N> inline Matrix<T,N,N> adjugate(Matrix<T,N,N> const &m)
    {
        Matrix<T,N,N> ret;
        for(std::size_t i = 0; i < N; ++i)
            for(std::size_t j = 0; j < N; ++j)
                ret(i,j)=cofactor(m,j,i); // i and j reversed so we get transpose of cofactor matrix
        return ret;
    }
    

    template <typename T> inline Matrix<T,2,2> inverse(Matrix<T,2,2> const &m)
    {
        Matrix<T,2,2> ret;
        T a = m(0,0);
        T b = m(0,1);
        T c = m(1,0);
        T d = m(1,1);
        ret(0,0) = d/(a*d-b*c);
        ret(0,1) = -b/(a*d-b*c);
        ret(1,0) = -c/(a*d-b*c);
        ret(1,1) = a/(a*d-b*c);
        return ret;
    }

    /// Invert a matrix using Cramer's rule.
    /// http://en.wikipedia.org/wiki/Invertible_matrix
    /// inverse of A is:
    /// (1/det(A))*transpose(C)
    /// where C is matrix of cofactors. 
    template <typename T, std::size_t N> inline Matrix<T,N,N> inverse_cramer(Matrix<T,N,N> const &m)
    {
        return adjugate(m)/determinant(m);
    }
    
    template <typename T> inline Matrix<T,4,4> inverse(Matrix<T,4,4> const &m)
    {
        // Attempt Blockwise inversion first
        Matrix<T,2,2> A(m,0,0);
        Matrix<T,2,2> B(m,0,2);
        Matrix<T,2,2> C(m,2,0);
        Matrix<T,2,2> D(m,2,2);
        
        if(determinant(A) != 0.0)
        {

            Matrix<T,2,2> A_inv = inverse(A);

            Matrix<T,2,2> tmp = D-C*A_inv*B;
            if(determinant(tmp) != 0.0)
            {
                Matrix<T,2,2> D_ret = inverse(tmp);

                Matrix<T,2,2> A_ret = A_inv+A_inv*B*D_ret*C*A_inv;
                Matrix<T,2,2> B_ret = -A_inv*B*D_ret;
                Matrix<T,2,2> C_ret = -D_ret*C*A_inv;

                Matrix<T,4,4> ret;
                for(std::size_t r = 0; r < 2; ++r)
                    for(std::size_t c = 0; c < 2; ++c)
                    {
                        ret(r,c) = A_ret(r,c);
                        ret(r,c+2) = B_ret(r,c);
                        ret(r+2,c) = C_ret(r,c);
                        ret(r+2,c+2) = D_ret(r,c);
                    }
                return ret;
            }
        }
        
        // Blockwise didn't work, try Cramer's rule
        return inverse_cramer(m);
    }
}

#endif
