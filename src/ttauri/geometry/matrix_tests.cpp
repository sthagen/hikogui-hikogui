// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ttauri/geometry/matrix.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace tt::geo;
using namespace tt;

#define ASSERT_NEAR_VEC(lhs, rhs, abs_err) ASSERT_TRUE(hypot<0b1111>(lhs - rhs) < abs_err)

TEST(matrix, invert)
{
    auto XYZ_to_sRGB =
        matrix<3>{3.24096994f,
            -1.53738318f,
            -0.49861076f,
            0.0f,
            -0.96924364f,
            1.87596750f,
            0.04155506f,
            0.0f,
            0.05563008f,
            -0.20397696f,
            1.05697151f,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            1.0f};

    auto sRGB_to_XYZ = ~XYZ_to_sRGB;

    ASSERT_NEAR_VEC(sRGB_to_XYZ.get<0>(), f32x4(0.41239080f, 0.21263901f, 0.01933082f), 0.001);
    ASSERT_NEAR_VEC(sRGB_to_XYZ.get<1>(), f32x4(0.35758434f, 0.71516868f, 0.11919478f), 0.001);
    ASSERT_NEAR_VEC(sRGB_to_XYZ.get<2>(), f32x4(0.18048079f, 0.07219232f, 0.95053215f), 0.001);
    ASSERT_NEAR_VEC(sRGB_to_XYZ.get<3>(), f32x4(0.0f, 0.0f, 0.0f, 1.0f), 0.001);
}