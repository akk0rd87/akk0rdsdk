#ifndef __AKK0RD_SDK_DATASETMERGER_TEST_H__
#define __AKK0RD_SDK_DATASETMERGER_TEST_H__

#include <vector>
#include <deque>
#include <array>
#include <set>
#include <unordered_set>
#include <list>
#include <forward_list>

#include "gtest/gtest.h"
#include "datasetmerger.h"

TEST(TestSDK, DatasetMerger_VectorAndVector) {
    const std::vector<int> v{ 0, 5, 10, 20, 30, 10 };
    const std::vector<int> d{ 10, 20, 30, 30, 40 };

    auto result = AkkordSDK::Utils::mergeDatasets(v.begin(), v.end(), d.begin(), d.end());

    // check existence
    for (const auto& value : v) {
        EXPECT_EQ(result.count(value), 1);
    }

    for (const auto& value : d) {
        EXPECT_EQ(result.count(value), 1);
    }

    // remove elements and check that result shoud be empty
    for (const auto& value : v) {
        result.erase(value);
    }

    for (const auto& value : d) {
        result.erase(value);
    }

    EXPECT_TRUE(result.empty());
}

TEST(TestSDK, DatasetMerger_VectorAndDeque) {
    const std::vector<int> v{ 0, 5, 10, 20, 30, 10, -50 };
    const std::deque<int> d{ 10, 20, 30, 30, 40, -60 };

    auto result = AkkordSDK::Utils::mergeDatasets(v.begin(), v.end(), d.begin(), d.end());

    // check existence
    for (const auto& value : v) {
        EXPECT_EQ(result.count(value), 1);
    }

    for (const auto& value : d) {
        EXPECT_EQ(result.count(value), 1);
    }

    // remove elements and check that result shoud be empty
    for (const auto& value : v) {
        result.erase(value);
    }

    for (const auto& value : d) {
        result.erase(value);
    }

    EXPECT_TRUE(result.empty());
}

TEST(TestSDK, DatasetMerger_VectorAndSTDArray) {
    const std::vector<int> v{ 0, 5, 10, 20, 30, 10, -50 };
    const std::array<int, 6> d{ { 10, 20, 30, 30, 40, -60 } };

    auto result = AkkordSDK::Utils::mergeDatasets(v.begin(), v.end(), d.begin(), d.end());

    // check existence
    for (const auto& value : v) {
        EXPECT_EQ(result.count(value), 1);
    }

    for (const auto& value : d) {
        EXPECT_EQ(result.count(value), 1);
    }

    // remove elements and check that result shoud be empty
    for (const auto& value : v) {
        result.erase(value);
    }

    for (const auto& value : d) {
        result.erase(value);
    }

    EXPECT_TRUE(result.empty());
}


TEST(TestSDK, DatasetMerger_DequeAndCArray) {
    const std::deque<int> v{ 0, 5, 10, 20, 30, 10, -50 };
    int d[] = { 10, 20, 30, 30, 40, -60 };

    auto result = AkkordSDK::Utils::mergeDatasets(v.begin(), v.end(), d, d + 6);

    // check existence
    for (const auto& value : v) {
        EXPECT_EQ(result.count(value), 1);
    }

    for (const auto& value : d) {
        EXPECT_EQ(result.count(value), 1);
    }

    // remove elements and check that result shoud be empty
    for (const auto& value : v) {
        result.erase(value);
    }

    for (const auto& value : d) {
        result.erase(value);
    }

    EXPECT_TRUE(result.empty());
}

TEST(TestSDK, DatasetMerger_CArrayAndDeque) {
    const std::deque<int> v{ 0, 5, 10, 20, 30, 10, -50 };
    int d[] = { 10, 20, 30, 30, 40, -60 };

    auto result = AkkordSDK::Utils::mergeDatasets(d, d + 6, v.begin(), v.end());

    // check existence
    for (const auto& value : v) {
        EXPECT_EQ(result.count(value), 1);
    }

    for (const auto& value : d) {
        EXPECT_EQ(result.count(value), 1);
    }

    // remove elements and check that result shoud be empty
    for (const auto& value : v) {
        result.erase(value);
    }

    for (const auto& value : d) {
        result.erase(value);
    }

    EXPECT_TRUE(result.empty());
}

TEST(TestSDK, DatasetMerger_SetAndUSet) {
    std::unordered_set<int> v{ 1, 2, 4, 8, 10, 55 };
    std::set<int> d{ 0, 1, 2, 4, 66 };

    auto result = AkkordSDK::Utils::mergeDatasets(v.begin(), v.end(), d.begin(), d.end());

    // check existence
    for (const auto& value : v) {
        EXPECT_EQ(result.count(value), 1);
    }

    for (const auto& value : d) {
        EXPECT_EQ(result.count(value), 1);
    }

    // remove elements and check that result shoud be empty
    for (const auto& value : v) {
        result.erase(value);
    }

    for (const auto& value : d) {
        result.erase(value);
    }

    EXPECT_TRUE(result.empty());
}

TEST(TestSDK, DatasetMerger_ListAndForwardedList) {
    std::list<int> v{ 1, 2, 4, 8, 10, 55 };
    std::forward_list<int> d{ 0, 1, 2, 4, 66 };

    auto result = AkkordSDK::Utils::mergeDatasets(v.begin(), v.end(), d.begin(), d.end());

    // check existence
    for (const auto& value : v) {
        EXPECT_EQ(result.count(value), 1);
    }

    for (const auto& value : d) {
        EXPECT_EQ(result.count(value), 1);
    }

    // remove elements and check that result shoud be empty
    for (const auto& value : v) {
        result.erase(value);
    }

    for (const auto& value : d) {
        result.erase(value);
    }

    EXPECT_TRUE(result.empty());
}

#endif // __AKK0RD_SDK_DATASETMERGER_TEST_H__