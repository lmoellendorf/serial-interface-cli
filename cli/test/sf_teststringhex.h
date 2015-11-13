#ifndef SF_TESTSTRINGHEX_H
#define SF_TESTSTRINGHEX_H

/* Google test and Google mock frameworks. */
#include <gtest/gtest.h>

class TestStringHex : public testing::Test
{
public:
    TestStringHex();
    ~TestStringHex();

    /*
     * Call your initialisation function and whatever you need here.
     * e.g. reset static variables from the tested module, init rand(), ...
     */
    void SetUp();
    void TearDown();

private:

};

#endif // SF_TESTSTRINGHEX_H
