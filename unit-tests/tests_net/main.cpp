#define BOOST_TEST_MODULE My test module
#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(Tests)

BOOST_AUTO_TEST_CASE(tests_net)
{
    BOOST_TEST(true /* test assertion */);
}

BOOST_AUTO_TEST_SUITE_END()
