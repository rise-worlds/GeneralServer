#define BOOST_TEST_MODULE fc_test_mod
#include <boost/test/included/unit_test.hpp>

#include <fc/uint256.hpp>
using namespace fc;

BOOST_AUTO_TEST_SUITE(fc_test_mod)

BOOST_AUTO_TEST_CASE(test_uint256) try {
    constexpr __uint128_t uint128max = std::numeric_limits<__uint128_t>::max();

    auto test_max_uint256 = uint256(uint128max, uint128max);
    auto test_min_uint256 = uint256();

    BOOST_CHECK_EQUAL(test_min_uint256, uint256::min());
    BOOST_CHECK_EQUAL(test_max_uint256, uint256::max());
} FC_LOG_AND_RETHROW();

BOOST_AUTO_TEST_SUITE_END()