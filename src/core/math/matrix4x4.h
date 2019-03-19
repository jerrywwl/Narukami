/*
MIT License

Copyright (c) 2019 ZhuQian

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once
#include "core/narukami.h"
#include "core/math/math.h"
#include "core/sse/sse.h"
#include "core/sse/float4.h"

NARUKAMI_BEGIN
//colmun major
struct SSE_ALIGNAS Matrix4x4{
public:
    union{
        float m[16];
        float mn[4][4];
        __m128 mVec[4];
    };
    typedef float Scalar;
    enum
    {
        N = 16
    };
public:
    Matrix4x4(){
         //col 0
        m[0]=One;
        m[1]=Zero;
        m[2]=Zero;
        m[3]=Zero;
        //col 1
        m[4]=Zero;
        m[5]=One;
        m[6]=Zero;
        m[7]=Zero;
        //col 2
        m[8]=Zero;
        m[9]=Zero;
        m[10]=One;
        m[11]=Zero;
        //col 3
        m[12]=Zero;
        m[13]=Zero;
        m[14]=Zero;
        m[15]=One;
    }
    Matrix4x4(const float m0,const float m1,const float m2,const float m3,const float m4,const float m5,const float m6,const float m7,const float m8,const float m9,const float m10,const float m11,const float m12,const float m13,const float m14,const float m15)
    {
        //col 0
        m[0]=m0;
        m[1]=m1;
        m[2]=m2;
        m[3]=m3;
        //col 1
        m[4]=m4;
        m[5]=m5;
        m[6]=m6;
        m[7]=m7;
        //col 2
        m[8]=m8;
        m[9]=m9;
        m[10]=m10;
        m[11]=m11;
        //col 3
        m[12]=m12;
        m[13]=m13;
        m[14]=m14;
        m[15]=m15;
    }

    Matrix4x4(const float* v){
        //col 0
        m[0]=v[0];
        m[1]=v[1];
        m[2]=v[2];
        m[3]=v[3];
        //col 1
        m[4]=v[4];
        m[5]=v[5];
        m[6]=v[6];
        m[7]=v[7];
        //col 2
        m[8]=v[8];
        m[9]=v[9];
        m[10]=v[10];
        m[11]=v[11];
        //col 3
        m[12]=v[12];
        m[13]=v[13];
        m[14]=v[14];
        m[15]=v[15];
    }
    FINLINE const float& operator[](const int idx) const { assert(idx >= 0 && idx < N); return m[idx];}
    FINLINE float& operator[](const int idx){ assert(idx >= 0 && idx < N); return m[idx];}
};

FINLINE std::ostream &operator<<(std::ostream &out, const Matrix4x4 &v){
    for(int i=0;i<Matrix4x4::N;++i){
        out<<v.m[i];
        out<<(((i+1)==Matrix4x4::N)?')':',');
    }
    return out;
}

FINLINE bool operator==(const Matrix4x4& A,const Matrix4x4& B){
    __m128 mask0=_mm_and_ps(_mm_cmpeq_ps(A.mVec[0], B.mVec[0]),_mm_cmpeq_ps(A.mVec[1], B.mVec[1]));
    __m128 mask1=_mm_and_ps(_mm_cmpeq_ps(A.mVec[2], B.mVec[2]),_mm_cmpeq_ps(A.mVec[3], B.mVec[3]));
    return (_mm_movemask_ps(_mm_and_ps(mask0,mask1))&15)==15;
}

FINLINE bool operator!=(const Matrix4x4& A,const Matrix4x4& B){
    __m128 mask0=_mm_and_ps(_mm_cmpeq_ps(A.mVec[0], B.mVec[0]),_mm_cmpeq_ps(A.mVec[1], B.mVec[1]));
    __m128 mask1=_mm_and_ps(_mm_cmpeq_ps(A.mVec[2], B.mVec[2]),_mm_cmpeq_ps(A.mVec[3], B.mVec[3]));
    return (_mm_movemask_ps(_mm_and_ps(mask0,mask1))&15)!=15;
}

FINLINE Vector3f operator*(const Matrix4x4& M,const Vector3f& v){
    // 8ns   
    // float4 r=float4(M.mVec[0])*float4(v.x);
    // r+=float4(M.mVec[1])*float4(v.y);
    // r+=float4(M.mVec[2])*float4(v.z);
    // return Vector3f(r.x,r.y,r.z);
    
    // 7ns
    float x =  M.m[0]*v.x+M.m[4]*v.y+M.m[8]*v.z;
    float y =  M.m[1]*v.x+M.m[5]*v.y+M.m[9]*v.z;
    float z =  M.m[3]*v.x+M.m[6]*v.y+M.m[10]*v.z;

    return narukami::Vector3f(x,y,z);
}

FINLINE SSEVector3f operator*(const Matrix4x4& M,const SSEVector3f& v){
    float4 vx=swizzle<0,0,0,0>(v.xyzw);
    float4 vy=swizzle<1,1,1,1>(v.xyzw);
    float4 vz=swizzle<2,2,2,2>(v.xyzw);
    
    float4 r=float4(M.mVec[0])*vx;
    r+=float4(M.mVec[1])*vy;
    r+=float4(M.mVec[2])*vz;
    return SSEVector3f(r.xyzw);
}


FINLINE Matrix4x4 operator*(const Matrix4x4& A,const Matrix4x4& B){
    
    Matrix4x4 ret;

    float4 r=float4(A.mVec[0])*float4(B.m[0]);
    r+=float4(A.mVec[1])*float4(B.m[1]);
    r+=float4(A.mVec[2])*float4(B.m[2]);
    r+=float4(A.mVec[3])*float4(B.m[3]);
    ret.mVec[0]=r.xyzw;

    r =float4(A.mVec[0])*float4(B.m[4]);
    r+=float4(A.mVec[1])*float4(B.m[5]);
    r+=float4(A.mVec[2])*float4(B.m[6]);
    r+=float4(A.mVec[3])*float4(B.m[7]);
    ret.mVec[1]=r.xyzw;

    r =float4(A.mVec[0])*float4(B.m[8]);
    r+=float4(A.mVec[1])*float4(B.m[9]);
    r+=float4(A.mVec[2])*float4(B.m[10]);
    r+=float4(A.mVec[3])*float4(B.m[11]);
    ret.mVec[2]=r.xyzw;

    r =float4(A.mVec[0])*float4(B.m[12]);
    r+=float4(A.mVec[1])*float4(B.m[13]);
    r+=float4(A.mVec[2])*float4(B.m[14]);
    r+=float4(A.mVec[3])*float4(B.m[15]);
    ret.mVec[3]=r.xyzw;
    
    return ret;
}

//https://lxjk.github.io/2017/09/03/Fast-4x4-Matrix-Inverse-with-SSE-SIMD-Explained.html#_appendix_2
FINLINE Matrix4x4 transform_inverse_noscale(const Matrix4x4& mat){
    Matrix4x4 r;
    //transpose left-upper 3x3
    __m128 t0 = shuffle<0,1,0,1>(mat.mVec[0],mat.mVec[1]);// 00,10,01,11
    __m128 t1 = shuffle<2,3,2,3>(mat.mVec[0],mat.mVec[1]);// 20,30,21,31
    // mat.mVec[2] : 02,12,22,32
    r.mVec[0] = shuffle<0,2,0,3>(t0,mat.mVec[2]);//00,01,02,32(=0)
    r.mVec[1] = shuffle<1,3,1,3>(t0,mat.mVec[2]);//10,11,12,32(=0)
    r.mVec[2] = shuffle<0,2,2,3>(t1,mat.mVec[2]);//20,21,22,32(=0)

    //translate
    r.mVec[3] =                      _mm_mul_ps(r.mVec[0],swizzle<0>(mat.mVec[3]));
    r.mVec[3] = _mm_add_ps(r.mVec[3],_mm_mul_ps(r.mVec[1],swizzle<1>(mat.mVec[3])));
    r.mVec[3] = _mm_add_ps(r.mVec[3],_mm_mul_ps(r.mVec[2],swizzle<2>(mat.mVec[3])));
    r.mVec[3] =  _mm_sub_ps(_mm_set_ps(One,Zero,Zero,Zero),r.mVec[3]);
    
    return r;
}


FINLINE Matrix4x4 transform_inverse(const Matrix4x4& mat){
    Matrix4x4 r;
    //transpose left-upper 3x3
    __m128 t0 = shuffle<0,1,0,1>(mat.mVec[0],mat.mVec[1]);// 00,10,01,11
    __m128 t1 = shuffle<2,3,2,3>(mat.mVec[0],mat.mVec[1]);// 20,30,21,31
    // mat.mVec[2] : 02,12,22,32
    r.mVec[0] = shuffle<0,2,0,3>(t0,mat.mVec[2]);//00,01,02,32(=0)
    r.mVec[1] = shuffle<1,3,1,3>(t0,mat.mVec[2]);//10,11,12,32(=0)
    r.mVec[2] = shuffle<0,2,2,3>(t1,mat.mVec[2]);//20,21,22,32(=0)

    //calculate sqr size
    __m128 sqrl = _mm_mul_ps(r.mVec[0],r.mVec[0]);
    sqrl=_mm_add_ps(sqrl,_mm_mul_ps(r.mVec[1],r.mVec[1]));
    sqrl=_mm_add_ps(sqrl,_mm_mul_ps(r.mVec[2],r.mVec[2]));

    //here need avoid divide by zero?
    //but I will remove it here,and add (0,0,0,1) to it
     sqrl=_mm_add_ps(sqrl,_mm_set_ps(One,Zero,Zero,Zero));

    __m128 rsqrl= _mm_div_ps(_mm_set1_ps(One),sqrl);

    r.mVec[0] = _mm_mul_ps(r.mVec[0],rsqrl);
    r.mVec[1] = _mm_mul_ps(r.mVec[1],rsqrl);
    r.mVec[2] = _mm_mul_ps(r.mVec[2],rsqrl);

    //translate
    r.mVec[3] =                      _mm_mul_ps(r.mVec[0],swizzle<0>(mat.mVec[3]));
    r.mVec[3] = _mm_add_ps(r.mVec[3],_mm_mul_ps(r.mVec[1],swizzle<1>(mat.mVec[3])));
    r.mVec[3] = _mm_add_ps(r.mVec[3],_mm_mul_ps(r.mVec[2],swizzle<2>(mat.mVec[3])));
    r.mVec[3] =  _mm_sub_ps(_mm_set_ps(One,Zero,Zero,Zero),r.mVec[3]);
    
    return r;
}

NARUKAMI_END