#pragma once
#include "utilities/OBJLoader.hpp"
#include <vector>

#include <xmmintrin.h>

union sse_float4 {
    float __attribute__ ((vector_size (16))) vector;
    float elements[4];

    //mm_add_sse
    sse_float4 operator+ (sse_float4 other) {
        sse_float4 out;
        out.vector = _mm_add_ss(vector, other.vector);
        return out;
    }

    sse_float4 operator- (sse_float4 other) {
        sse_float4 out;
        out.vector = _mm_sub_ss(vector, other.vector);
        return out;
    }

    sse_float4 operator* (sse_float4 other) {
        sse_float4 out;
        out.vector = _mm_mul_ss(vector, other.vector);
        return out;
    }

    sse_float4 operator/ (sse_float4 other) {
        sse_float4 out;
        out.vector = _mm_div_ss(vector, other.vector);
        return out;
    }

    bool operator!= (float other) {
        return ( elements[0] != other ) || (elements[1] != other ) || (elements[2] != other) || (elements[3] != other );
    }
};

void sse_test(Mesh &);