#ifndef GZ4D_MATRIX_H
#define GZ4D_MATRIX_H

#include <gz4d/gz4d.h>
#include <gz4d/Vector.h>
#include <limits>

namespace gz4d
{
    /// Column-major matrix with M rows and N columns.
    /// \ingroup base
    template <typename T, std::size_t M, std::size_t N> class Matrix
    {
        Vector<T,M*N> values;
        Matrix(Vector<T,M*N> const &v):values(v){}
        public:
            Matrix(){}
            Matrix(Matrix<T,M,N> const &m):values(m.values){}
            explicit Matrix(T val):values(val){}
            template <typename VI> Matrix(const std::pair<VI,VI> &iterators):values(iterators){}
            template <std::size_t CM, std::size_t CN> Matrix(Matrix<T,CM,CN> const &cm, std::size_t row, std::size_t col);

            static Matrix<T,M,N> Identity();

            T const &operator()(std::size_t row, std::size_t col) const {return values[col*M+row];}
            T &operator()(std::size_t row, std::size_t col) {return values[col*M+row];}

            template <std::size_t P> Matrix<T,M,P> operator*(Matrix<T,N,P> const &rvalue) const;
            Vector<T,M> operator*(Vector<T,N> const &rvalue) const;

            T &front() {return values.front();}
            T const &front() const {return values.front();}

            Matrix<T,M,N> operator-(Matrix const &o) const {return Matrix<T,M,N>(values-o.values);}
            Matrix<T,M,N> operator+(Matrix const &o) const {return Matrix<T,M,N>(values+o.values);}

            Matrix<T,M,N> operator-() const {return Matrix<T,M,N>(-values);}

            /// Element-wise arithmetic
            Matrix<T,M,N> &operator*=(T value) { values*=value; return *this;}
            Matrix<T,M,N> operator*(T value) const { return Matrix<T,M,N>(*this)*=value;}

            Matrix<T,M,N> &operator/=(T value) { values/=value; return *this;}
            Matrix<T,M,N> operator/(T value) const { return Matrix<T,M,N>(*this)/=value;}

            Matrix<T,M,N> &operator+=(T value) { values+=value; return *this;}
            Matrix<T,M,N> operator+(T value) const { return Matrix<T,M,N>(*this)+=value;}
            
            Matrix<T,M,N> &operator-=(T value) { values-=value; return *this;}
            Matrix<T,M,N> operator-(T value) const { return Matrix<T,M,N>(*this)-=value;}

            bool operator!=(Matrix const &o) const {return values != o.values;}
            bool operator==(Matrix const &o) const {return !(*this != o);}
    };

    template <typename T, std::size_t M, std::size_t N> inline Matrix<T,M,N> Matrix<T,M,N>::Identity()
    {
        Matrix<T,M,N> ret;
        for(std::size_t i = 0; i < N && i < M; ++i)
            ret.values[i*M+i] = 1;
        return ret;
    }

    template <typename T, std::size_t M, std::size_t N> template <std::size_t P> inline Matrix<T,M,P> Matrix<T,M,N>::operator*(Matrix<T,N,P> const &rvalue) const 
    {
        Matrix<T,M,P> ret;
        for(std::size_t r = 0; r < M; ++r)
            for(std::size_t c = 0; c < P; ++c)
                for(std::size_t i = 0; i < N; ++i)
                    ret(r,c) += operator()(r,i)*rvalue(i,c);
        return ret;
    }

    template <typename T, std::size_t M, std::size_t N> inline Vector<T,M> Matrix<T,M,N>::operator*(Vector<T,N> const &rvalue) const
    {
        Vector<T,M> ret;
        for(std::size_t r = 0; r < M; ++r)
            for(std::size_t c = 0; c < N; ++c)
                ret[r] += operator()(r,c)*rvalue[c];
        return ret;
    }
    
    template <typename T, std::size_t M, std::size_t N> template <std::size_t CM, std::size_t CN> inline Matrix<T,M,N>::Matrix(Matrix<T,CM,CN> const &cm, std::size_t row, std::size_t col)
    {
        for(std::size_t r = 0; r < M; ++r)
            for(std::size_t c = 0; c < N; ++c)
                values[c*M+r] = cm(row+r,col+c);
    }

    template <typename T> static Matrix<T,4,4> Frustum(T l, T r, T b, T t, T n, T f=std::numeric_limits<T>::infinity())
    {
        Matrix<T,4,4> ret(0);
        T w = r-l;
        T h = t - b;
        ret(0,0) = 2.0*n/w;
        ret(0,2) = (r+l)/w;
        ret(1,1) = 2.0*n/h;
        ret(1,2) = (t+b)/h;
        ret(2,2) = -(f+n)/(f-n);
        //ret(2,2) = -1.0;
        ret(2,3) = -2.0*f*n/(f-n);
        //ret(2,3) = -2.0*n;
        ret(3,2) = -1.0;
        return ret;
    }

//     template <typename T> static Matrix<T,4,4> Frustum(T l, T r, T b, T t, T n, T f)
//     {
//         Matrix<T,4,4> ret = Frustum<T>(l,r,b,t,n);
//         ret(2,2) = -(f+n)/(f-n);
//         ret(2,3) = -2.0*f*n/(f-n);
//         return ret;
//         
// /*        Matrix<T,4,4> ret(0);
//         T w = r-l;
//         T h = t - b;
//         ret(0,0) = 2.0*n/w;
//         ret(0,2) = (r+l)/w;
//         ret(1,1) = 2.0*n/h;
//         ret(1,2) = (t+b)/h;
//         ret(2,2) = -(f+n)/(f-n);
//         ret(2,3) = -2.0*f*n/(f-n);
//         ret(3,2) = -1.0;
//         return ret;*/
//     }
    
    
}

#endif
