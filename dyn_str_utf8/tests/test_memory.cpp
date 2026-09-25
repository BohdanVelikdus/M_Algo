#include <gtest/gtest.h>

#include "dyn_str_utf8.h"

class U8 : public ::testing::Test {
protected:
    dyn_str_utf8_t str = DYN_STR_UTF8_ZERO;

    void SetUp() override {
        const bool success = dyn_str_utf8_init(&str, 20);
        EXPECT_TRUE(success);
    }

    void TearDown() override {
        dyn_str_utf8_destroy(&str);
    }
};

static const std::string utf8_sample = "Hello World 🦀 - UTF-8 Test String 🦔";

TEST_F(U8, Clear)
{
    dyn_str_utf8_clear(&str);
    ASSERT_EQ(str.size, 0);
    ASSERT_EQ(*str.ptr, '\0');
}

TEST_F(U8, Reserve)
{
    const bool success = dyn_str_utf8_reserve(&str, 1024);
    ASSERT_TRUE(success);
    ASSERT_EQ(str.capacity, 1024);
}

TEST_F(U8, ShrinkToFit)
{
    bool succ = dyn_str_utf8_from_cstr(&str, STDLIB_ALLOCATOR, utf8_sample.c_str());
    ASSERT_TRUE(succ);

    succ = dyn_str_utf8_shrink_to_fit(&str);
    ASSERT_TRUE(succ);
    ASSERT_EQ(str.size, str.capacity);
}