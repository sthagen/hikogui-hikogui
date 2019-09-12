
// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/polynomial.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

template<typename T, typename U, int N>
double maxAbsDiff(TTauri::results<T,N> const &lhs, TTauri::results<U,N> const &rhs) {
    if (lhs.count != rhs.count) {
        return std::numeric_limits<double>::infinity();
    }

    double maxDiff = 0.0;
    assert(lhs.count <= lhs.maxCount);
    for (ptrdiff_t i = 0; i < lhs.count; i++) {
        let diff = std::abs(lhs.value[i] - rhs.value[i]);
        maxDiff = std::max(maxDiff, diff);
    }
    return maxDiff;
}

template<typename T, typename U, int N>
testing::AssertionResult ResultsNearPredFormat(const char* expr1,
    const char* expr2,
    const char* abs_error_expr,
    TTauri::results<T, N> val1,
    TTauri::results<U, N> val2,
    double abs_error) {
    let diff = maxAbsDiff(val1, val2);
    if (diff <= abs_error) return testing::AssertionSuccess();

    return testing::AssertionFailure()
        << "The difference between " << expr1 << " and " << expr2
        << " is " << diff << ", which exceeds " << abs_error_expr << ", where\n"
        << expr1 << " evaluates to " << val1 << ",\n"
        << expr2 << " evaluates to " << val2 << ", and\n"
        << abs_error_expr << " evaluates to " << abs_error << ".";
}

#define ASSERT_RESULTS_NEAR(val1, val2, abs_error)\
    ASSERT_PRED_FORMAT3(ResultsNearPredFormat, val1, val2, abs_error)

#define ASSERT_RESULTS(val1, val2) ASSERT_RESULTS_NEAR(val1, val2, 0.000001)

namespace TTauri {
using results1 = TTauri::results<double,1>;
using results2 = TTauri::results<double,2>;
using results3 = TTauri::results<double,3>;
}
