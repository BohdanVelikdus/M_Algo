#include <gtest/gtest.h>

#include "dyn_str_utf8.h"

TEST(U8, Copy)
{
    dyn_str_utf8_t str = DYN_STR_UTF8_ZERO;
    dyn_str_utf8_t str2 = DYN_STR_UTF8_ZERO;

    bool success = dyn_str_utf8_from_cstr(&str, STDLIB_ALLOCATOR, "String Str1");
    ASSERT_TRUE(success);

    success = dyn_str_utf8_init(&str2, 20);
    ASSERT_TRUE(success);

    success = dyn_str_utf8_copy(&str, &str2);
    ASSERT_TRUE(success);

    for (std::size_t i = 0; i < str.size; ++i)
    {
        ASSERT_EQ(str.ptr[i], str2.ptr[i]);
    }

    dyn_str_utf8_destroy(&str);
    dyn_str_utf8_destroy(&str2);
}

TEST(U8, Move)
{
    dyn_str_utf8_t str = DYN_STR_UTF8_ZERO;
    dyn_str_utf8_t str2 = DYN_STR_UTF8_ZERO;

    bool success = dyn_str_utf8_from_cstr(&str, STDLIB_ALLOCATOR, "String Str1");
    ASSERT_TRUE(success);

    success = dyn_str_utf8_from_cstr(&str, STDLIB_ALLOCATOR, "String Str2");
    ASSERT_TRUE(success);
    ASSERT_NE(str.ptr, str2.ptr);
    auto* temp = str.ptr;
    success = dyn_str_utf8_move(&str, &str2);
    ASSERT_TRUE(success);
    ASSERT_EQ(str.ptr, nullptr);
    ASSERT_NE(str2.ptr, nullptr);
    ASSERT_EQ(str2.ptr, temp);

    dyn_str_utf8_destroy(&str);
    dyn_str_utf8_destroy(&str2);
}