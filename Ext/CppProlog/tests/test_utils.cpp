#include <gtest/gtest.h>
#include "utils/string_utils.h"
#include "utils/memory_pool.h"

using namespace prolog::utils;

class StringUtilsTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(StringUtilsTest, TrimWhitespace) {
    EXPECT_EQ(StringUtils::trim("  hello  "), "hello");
    EXPECT_EQ(StringUtils::trim("hello"), "hello");
    EXPECT_EQ(StringUtils::trim("  hello"), "hello");
    EXPECT_EQ(StringUtils::trim("hello  "), "hello");
    EXPECT_EQ(StringUtils::trim(""), "");
    EXPECT_EQ(StringUtils::trim("   "), "");
}

TEST_F(StringUtilsTest, SplitString) {
    auto result = StringUtils::split("a,b,c", ',');
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "a");
    EXPECT_EQ(result[1], "b");
    EXPECT_EQ(result[2], "c");
    
    auto result2 = StringUtils::split("hello::world::test", "::");
    ASSERT_EQ(result2.size(), 3);
    EXPECT_EQ(result2[0], "hello");
    EXPECT_EQ(result2[1], "world");
    EXPECT_EQ(result2[2], "test");
}

TEST_F(StringUtilsTest, JoinStrings) {
    std::vector<std::string> strings = {"a", "b", "c"};
    EXPECT_EQ(StringUtils::join(strings, ","), "a,b,c");
    EXPECT_EQ(StringUtils::join(strings, " :: "), "a :: b :: c");
    
    std::vector<std::string> empty_strings;
    EXPECT_EQ(StringUtils::join(empty_strings, ","), "");
}

TEST_F(StringUtilsTest, StartsWithEndsWith) {
    EXPECT_TRUE(StringUtils::startsWith("hello world", "hello"));
    EXPECT_FALSE(StringUtils::startsWith("hello world", "world"));
    EXPECT_TRUE(StringUtils::startsWith("hello", "hello"));
    
    EXPECT_TRUE(StringUtils::endsWith("hello world", "world"));
    EXPECT_FALSE(StringUtils::endsWith("hello world", "hello"));
    EXPECT_TRUE(StringUtils::endsWith("world", "world"));
}

TEST_F(StringUtilsTest, CaseConversion) {
    EXPECT_EQ(StringUtils::toLowerCase("Hello World"), "hello world");
    EXPECT_EQ(StringUtils::toUpperCase("Hello World"), "HELLO WORLD");
    
    EXPECT_EQ(StringUtils::toLowerCase(""), "");
    EXPECT_EQ(StringUtils::toUpperCase(""), "");
}

TEST_F(StringUtilsTest, StringReplacement) {
    EXPECT_EQ(StringUtils::replace("hello world", "world", "universe"), "hello universe");
    EXPECT_EQ(StringUtils::replace("hello world", "missing", "replacement"), "hello world");
    
    EXPECT_EQ(StringUtils::replaceAll("hello hello hello", "hello", "hi"), "hi hi hi");
    EXPECT_EQ(StringUtils::replaceAll("test", "missing", "replacement"), "test");
}

TEST_F(StringUtilsTest, StringChecks) {
    EXPECT_TRUE(StringUtils::isWhitespace("   \t\n"));
    EXPECT_FALSE(StringUtils::isWhitespace("hello"));
    EXPECT_TRUE(StringUtils::isWhitespace(""));
    
    EXPECT_TRUE(StringUtils::isAlphanumeric("hello123"));
    EXPECT_FALSE(StringUtils::isAlphanumeric("hello world"));
    EXPECT_FALSE(StringUtils::isAlphanumeric("hello!"));
}

TEST_F(StringUtilsTest, EscapeUnescape) {
    EXPECT_EQ(StringUtils::escape("hello\nworld"), "hello\\nworld");
    EXPECT_EQ(StringUtils::escape("hello\tworld"), "hello\\tworld");
    EXPECT_EQ(StringUtils::escape("hello\"world\""), "hello\\\"world\\\"");
    
    EXPECT_EQ(StringUtils::unescape("hello\\nworld"), "hello\nworld");
    EXPECT_EQ(StringUtils::unescape("hello\\tworld"), "hello\tworld");
    EXPECT_EQ(StringUtils::unescape("hello\\\"world\\\""), "hello\"world\"");
}

class MemoryPoolTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

struct TestObject {
    int value;
    std::string name;
    
    TestObject(int v, const std::string& n) : value(v), name(n) {}
};

TEST_F(MemoryPoolTest, AllocationDeallocation) {
    MemoryPool<TestObject> pool(10);
    
    auto obj1 = pool.allocate(42, "test1");
    auto obj2 = pool.allocate(24, "test2");
    
    EXPECT_NE(obj1, nullptr);
    EXPECT_NE(obj2, nullptr);
    EXPECT_NE(obj1, obj2);
    
    EXPECT_EQ(obj1->value, 42);
    EXPECT_EQ(obj1->name, "test1");
    EXPECT_EQ(obj2->value, 24);
    EXPECT_EQ(obj2->name, "test2");
    
    EXPECT_EQ(pool.usedCount(), 2);
    
    pool.deallocate(obj1);
    EXPECT_EQ(pool.usedCount(), 1);
    
    pool.deallocate(obj2);
    EXPECT_EQ(pool.usedCount(), 0);
}

TEST_F(MemoryPoolTest, MemoryReuse) {
    MemoryPool<int> pool(5);
    
    auto ptr1 = pool.allocate(42);
    auto original_address = ptr1;
    
    pool.deallocate(ptr1);
    
    auto ptr2 = pool.allocate(24);
    
    // Should reuse the same memory location
    EXPECT_EQ(ptr2, original_address);
    EXPECT_EQ(*ptr2, 24);
}

TEST_F(MemoryPoolTest, ChunkExpansion) {
    MemoryPool<int> pool(2); // Small chunk size
    
    std::vector<int*> ptrs;
    
    // Allocate more than one chunk worth
    for (int i = 0; i < 5; ++i) {
        ptrs.push_back(pool.allocate(i));
    }
    
    EXPECT_EQ(pool.usedCount(), 5);
    EXPECT_GE(pool.totalCapacity(), 5);
    
    // Verify all values are correct
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(*ptrs[i], i);
    }
    
    // Clean up
    for (auto ptr : ptrs) {
        pool.deallocate(ptr);
    }
    
    EXPECT_EQ(pool.usedCount(), 0);
}

TEST_F(MemoryPoolTest, PoolClear) {
    MemoryPool<TestObject> pool(5);
    
    for (int i = 0; i < 3; ++i) {
        pool.allocate(i, "test" + std::to_string(i));
    }
    
    EXPECT_EQ(pool.usedCount(), 3);
    
    pool.clear();
    
    EXPECT_EQ(pool.usedCount(), 0);
    EXPECT_EQ(pool.totalCapacity(), 0);
}