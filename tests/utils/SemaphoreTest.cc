/**
 * @file SemaphoreTest.cc
 * @brief
 * 信号量测试
 *
 * @author Liu GuangRui
 * @email 675040625@qq.com
 */
#include "../catch.hpp"
#include "ndsl/utils/Semaphore.h"

TEST_CASE("utils/Semaphore")
{
    SECTION("give and take")
    {
        ndsl::utils::Semaphore sem;

        REQUIRE(sem.available() == 0);
        sem.give(2);

        REQUIRE(sem.available() == 2);

        sem.take();
        REQUIRE(sem.available() == 1);

        sem.take();
        REQUIRE(sem.available() == 0);
    }
}