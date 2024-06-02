#ifndef __AKK0RD_SDK_DATASETMERGER_TEST_H__
#define __AKK0RD_SDK_DATASETMERGER_TEST_H__

#include <vector>
#include <deque>
#include <array>

#include "gtest/gtest.h"
#include "datasetmerger.h"

TEST(TestSDK, DatasetMerger_VectorVector) {
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

TEST(TestSDK, DatasetMerger_VectorDeque) {
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

TEST(TestSDK, DatasetMerger_VectorSTDArray) {
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


TEST(TestSDK, DatasetMerger_DequeCArray) {
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

TEST(TestSDK, DatasetMerger_CArrayDeque) {
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

#endif // __AKK0RD_SDK_DATASETMERGER_TEST_H__