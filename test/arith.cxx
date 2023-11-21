/* Tests
 *
 * Copyright (C) 2022 Markus Wallerberger and others
 * SPDX-License-Identifier: MIT
 */
#include <catch2/catch_test_macros.hpp>
#include "ddouble.h"
#include "mpfloat.h"
#include "catch2-addons.h"

#define CMP_UNARY(fn, x, eps)                                              \
    do {                                                                   \
        DDouble r_d = fn(DDouble(x));                                      \
        MPFloat r_f = fn(MPFloat(x));                                      \
        double eps_d = eps;                                                \
        REQUIRE_THAT(r_d, WithinRel(r_f, eps_d));                          \
    } while(false)

#define CMP_BINARY(fn, x, y, eps)                                          \
    do {                                                                   \
        DDouble r_d = fn(DDouble(x), DDouble(y));                          \
        MPFloat r_f = fn(MPFloat(x), MPFloat(y));                          \
        double eps_d = eps;                                                \
        REQUIRE_THAT(r_d, WithinRel(r_f, eps_d));                          \
    } while(false)

#define CMP_BINARY_1(fn, x, y, eps)                                        \
    do {                                                                   \
        DDouble r_d = fn(DDouble(x), y);                                   \
        MPFloat r_f = fn(MPFloat(x), y);                                   \
        double eps_d = eps;                                                \
        REQUIRE_THAT(r_d, WithinRel(r_f, eps_d));                          \
    } while(false)


TEST_CASE("greater_in_magnitude", "")
{
    REQUIRE(greater_in_magnitude(2.0, 1.0));
    REQUIRE(greater_in_magnitude(-1e100, 1e50));
    REQUIRE(greater_in_magnitude(-0.1, 0.1));
    REQUIRE(!greater_in_magnitude(1.0, 2.0));
    REQUIRE(!greater_in_magnitude(1e200, -INFINITY));
    REQUIRE(greater_in_magnitude(NAN, INFINITY));
    REQUIRE(!greater_in_magnitude(1e200, NAN));
}

TEST_CASE("Relational", "[rel]")
{
    REQUIRE(DDouble(1.0) > 0);
    REQUIRE(DDouble(1.0) - ldexp(1.0, -80) < 1);
    REQUIRE(DDouble(3.0) <= DDouble(3.0));
}

TEST_CASE("Mul", "[arith]")
{
    DDouble x = DDouble(1.25, -ldexp(1.125, -85));
    CMP_BINARY(operator*, x, x, 1e-31);
    CMP_BINARY(operator*, -3.25, x, 1e-31);

    CMP_BINARY_1(operator*, x, -139818.125, 1e-31);
}

TEST_CASE("Divide", "[arith]")
{
    REQUIRE((DDouble(3) / DDouble(2)).as<double>() == 1.5);
    CMP_BINARY(operator/, 3, 4, 1e-31);
    CMP_BINARY(operator/, -149, 53.25, 1e-30);
}

TEST_CASE("Divide pow2", "[arith]")
{
    CMP_BINARY(operator/, 1, 137, 1e-31);
    REQUIRE_THAT(reciprocal(ExDouble(137)),
                 WithinRel(MPFloat(1) / 137, 1e-21));
}

TEST_CASE("sqrt", "[fn]")
{
    CMP_UNARY(sqrt, 2, 1e-31);
    CMP_UNARY(sqrt, 3, 1e-31);
    CMP_UNARY(sqrt, ldexp(67.0, -39), 1e-31);
    CMP_UNARY(sqrt, ldexp(23.0, 105), 1e-31);
}

TEST_CASE("hypot", "[fn]")
{
    CMP_BINARY(hypot, 1.0, 1.0, 1e-31);
    CMP_BINARY(hypot, 3.0, -10000.0, 1e-31);
    CMP_BINARY(hypot, ldexp(3.0, 600), ldexp(1.0, 570), 1e-31);
    CMP_BINARY(hypot, ldexp(-3.0, 600), ldexp(1.0, 640), 1e-31);
    CMP_BINARY(hypot, ldexp(3.0, -600), ldexp(1.0, -570), 1e-31);
    CMP_BINARY(hypot, ldexp(3.14, -600), ldexp(9.4, -640), 1e-31);
}

TEST_CASE("pow", "[fn]")
{
    CMP_BINARY_1(pow, 3.0, 5, 1e-31);
    CMP_BINARY_1(pow, -5.0, 12, 1e-31);
    CMP_BINARY_1(pow, 2.0, 0, 1e-31);
    CMP_BINARY_1(pow, -2.75, 27, 1e-31);

    // Something is very weird with powers ...
    CMP_BINARY_1(pow, 2., -17, 1e-30);
    CMP_BINARY_1(pow, -1.5, -8, 1e-30);
    CMP_BINARY_1(pow, 1.5, 17, 1e-30);
    CMP_BINARY_1(pow, -2.25, -10, 1e-30);

    REQUIRE_THAT(pow(pow(DDouble(-2.25), -10), -10),
                 WithinRel(pow(DDouble(-2.25), 100), 1e-30));
}

TEST_CASE("exp", "[exp]")
{
    CMP_UNARY(exp, 0.0, 1e-31);
    CMP_UNARY(exp, 1.0, 1e-31);
    CMP_UNARY(exp, 1e-4, 1e-31);

    // Small values shall be very accurate
    DDouble x = 0.25;
    while ((x *= 0.9) > 1e-290) {
        CMP_UNARY(exp, x, 5e-32);
        CMP_UNARY(exp, -x, 5e-32);
    }

    // Larger, less so, but let's still strive for 1 ulps
    x = 0.125;
    while ((x *= 1.0041) < 708.0) {
        CMP_UNARY(exp, x, 5e-32);
        if (x < 670)
            CMP_UNARY(exp, -x, 5e-32);
    }

    REQUIRE(exp(DDouble(-1000)) == 0);
}

TEST_CASE("expm1", "[exp]")
{
    CMP_UNARY(expm1, 0.0, 1e-31);
    CMP_UNARY(expm1, 1.0, 1e-31);
    CMP_UNARY(expm1, -1000, 1e-31);

    // Small values shall be very accurate
    DDouble x = 0.25;
    while ((x *= 0.9) > 1e-290) {
        CMP_UNARY(expm1, x, 5e-32);
        CMP_UNARY(expm1, -x, 5e-32);
    }

    // Larger, less so, but let's still strive for 1 ulps
    x = 0.125;
    while ((x *= 1.02) < 708.0) {
        CMP_UNARY(expm1, x, 8e-32);
        if (x < 670)
            CMP_UNARY(expm1, -x, 8e-32);
    }
}

TEST_CASE("log", "[exp]")
{
    CMP_UNARY(log, 1.0, 1e-31);
    CMP_UNARY(log, 3.0, 1e-31);

    DDouble x = 1.;
    while ((x *= 1.13) < 1e300) {
        CMP_UNARY(log, x, 1e-31);
    }

    x = 1.;
    while ((x *= 0.95) > 1e-290) {
        CMP_UNARY(log, x, 1e-31);
    }
}

TEST_CASE("log1p", "[exp]")
{
    CMP_UNARY(log, 1.0, 1e-31);
    CMP_UNARY(log, 3.0, 1e-31);

    DDouble x = 1.;
    while ((x *= 1.13) < 1e300) {
        CMP_UNARY(log1p, x, 1e-31);
    }

    x = 1.;
    while ((x *= 0.92) > 1e-290) {
        CMP_UNARY(log1p, x, 1e-31);
    }
    x = -.9999999;
    while ((x *= 0.92) > 1e-290) {
        CMP_UNARY(log1p, x, 1e-31);
    }
}
