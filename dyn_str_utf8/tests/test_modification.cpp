#include <gtest/gtest.h>

#include <format>
#include <cstring>

#include "dyn_str_utf8.h"

#define ASSERT_STREQ_SIZE_BASED(ptr1, ptr2, ptr_size, msg) \
    do { \
        ASSERT_GE((ptr_size), 0u) << "The size must be greater equal than 0"; \
        for (size_t i = 0; i < (size_t)(ptr_size); ++i) { \
            ASSERT_EQ((ptr1)[i], (ptr2)[i]) << (msg) << " at index " << i; \
        } \
    } while (0)

class U8 : public ::testing::Test {
protected:
    dyn_str_utf8_t str = DYN_STR_UTF8_ZERO;

    void SetUp() override {
        bool success = dyn_str_utf8_init(&str, 20);
        EXPECT_TRUE(success);
    }

    void TearDown() override {
        dyn_str_utf8_destroy(&str);
    }
};

static const std::string utf8_sample = "Hello World 🦀 - UTF-8 Test String 🦔";

TEST_F(U8, ReverseString)
{
    const std::string reversed = "🦔 gnirtS tseT 8-FTU - 🦀 dlroW olleH";
    bool succ = dyn_str_utf8_from_cstr(&str, STDLIB_ALLOCATOR, utf8_sample.c_str());
    ASSERT_TRUE(succ);

    dyn_str_utf8_reverse(&str);
    for (int i = 0; i <  str.size; ++i)
    {
        ASSERT_EQ(reversed.c_str()[i], str.ptr[i]);
    }
}

TEST_F(U8, SliceProp)
{
    bool succ = dyn_str_utf8_from_cstr(&str, STDLIB_ALLOCATOR, utf8_sample.c_str());
    ASSERT_TRUE(succ);

    dyn_str_utf8_t slice;
    succ = dyn_str_utf8_init(&slice, 20);
    ASSERT_TRUE(succ);

    constexpr std::size_t pos = 0;
    constexpr std::size_t count = 2;

    succ = dyn_str_utf8_slice(&str, pos, count, &slice);
    ASSERT_TRUE(succ);

    constexpr auto* r = "He";
    for (int i = 0; i < slice.size; ++i)
    {
        ASSERT_EQ(r[i], slice.ptr[i]);
    }
    dyn_str_utf8_destroy(&slice);
}

TEST_F(U8, SliceU8)
{
    bool succ = dyn_str_utf8_from_cstr(&str, STDLIB_ALLOCATOR, utf8_sample.c_str());
    ASSERT_TRUE(succ);

    dyn_str_utf8_t slice;
    succ = dyn_str_utf8_init(&slice, 20);
    ASSERT_TRUE(succ);

    constexpr std::size_t pos = 10;
    constexpr std::size_t count = 20;

    succ = dyn_str_utf8_slice(&str, pos, count, &slice);
    ASSERT_TRUE(succ);
    const std::string_view r = std::string_view{utf8_sample}.substr(pos, count);
    for (int i = 0; i < slice.size; ++i)
    {
        ASSERT_EQ(r[i], slice.ptr[i]);
    }
    dyn_str_utf8_destroy(&slice);
}

struct AppendCodepointTestCase {
    const char* description;
    const char* initial_str;
    uint32_t codepoint;
    const char* expected_result;
    bool expected_success;
};

class Utf8AppendCodepointTest : public ::testing::TestWithParam<AppendCodepointTestCase> {};

TEST_P(Utf8AppendCodepointTest, AppendsCodepointCorrectly) {
    const auto& param = GetParam();

    dyn_str_utf8_t str = DYN_STR_UTF8_ZERO;
    ASSERT_TRUE(dyn_str_utf8_from_cstr(&str, STDLIB_ALLOCATOR, param.initial_str));

    bool success = dyn_str_utf8_append_codepoint(&str, param.codepoint);

    EXPECT_EQ(success, param.expected_success) << "Failed case: " << param.description;
    if (param.expected_success) {
        ASSERT_STREQ_SIZE_BASED(str.ptr, param.expected_result, std::strlen(param.expected_result), std::format("Failed case: %s", param.description));
    }

    dyn_str_utf8_destroy(&str);
}

INSTANTIATE_TEST_SUITE_P(
    AppendCodepointCases,
    Utf8AppendCodepointTest,
    ::testing::Values(
        // 1-Byte ASCII
        AppendCodepointTestCase{"Append ASCII character", "Hello", 0x0021, "Hello!", true}, // '!'
        // 2-Byte Cyrillic
        AppendCodepointTestCase{"Append 2-Byte Cyrillic", "Привет", 0x0430, "Привета", true}, // 'а'
        // 3-Byte CJK
        AppendCodepointTestCase{"Append 3-Byte CJK", "日本", 0x8A9E, "日本語", true}, // '語'
        // 4-Byte Emoji
        AppendCodepointTestCase{"Append 4-Byte Emoji", "Hi ", 0x1F600, "Hi 😀", true}, // '😀'
        // Boundary Unicode Value (U+10FFFF)
        AppendCodepointTestCase{"Append Max Valid Codepoint", "", 0x10FFFF, "\xF4\x8F\xBF\xBF", true},
        // Invalid Unicode Codepoints
        AppendCodepointTestCase{"Reject Surrogate Halves", "", 0xD800, "", false},
        AppendCodepointTestCase{"Reject Out of Range Codepoint (> U+10FFFF)", "", 0x110000, "", false}
    )
);

struct PopBackTestCase {
    const char* description;
    const char* initial_str;
    const char* expected_result;
    bool expected_success;
};

class Utf8PopBackTest : public ::testing::TestWithParam<PopBackTestCase> {};

TEST_P(Utf8PopBackTest, PopsLastCodepoint) {
    const auto& param = GetParam();

    dyn_str_utf8_t str = DYN_STR_UTF8_ZERO;
    ASSERT_TRUE(dyn_str_utf8_from_cstr(&str, STDLIB_ALLOCATOR, param.initial_str));

    bool success = dyn_str_utf8_pop_back_codepoint(&str);

    EXPECT_EQ(success, param.expected_success) << "Failed case: " << param.description;
    if (param.expected_success) {
        ASSERT_STREQ_SIZE_BASED(str.ptr, param.expected_result, std::strlen(param.expected_result), std::format("Failed case: %s", param.description));
    }

    dyn_str_utf8_destroy(&str);
}

INSTANTIATE_TEST_SUITE_P(
    PopBackCases,
    Utf8PopBackTest,
    ::testing::Values(
        PopBackTestCase{"Pop ASCII char", "Hello!", "Hello", true},
        PopBackTestCase{"Pop 4-byte Emoji", "Hi 😀", "Hi ", true},
        PopBackTestCase{"Pop 3-byte CJK", "日本語", "日本", true},
        PopBackTestCase{"Pop last char leaving empty string", "A", "", true},
        PopBackTestCase{"Fail on empty string", "", "", false}
    )
);

TEST(Utf8MutationTest, InsertAtCodepointIndex) {
    dyn_str_utf8_t dest = DYN_STR_UTF8_ZERO;
    dyn_str_utf8_t src = DYN_STR_UTF8_ZERO;

    ASSERT_TRUE(dyn_str_utf8_from_cstr(&dest, STDLIB_ALLOCATOR, "日本語"));
    ASSERT_TRUE(dyn_str_utf8_from_cstr(&src, STDLIB_ALLOCATOR, "😀"));

    EXPECT_TRUE(dyn_str_utf8_insert(&dest, 1, &src));
    ASSERT_STREQ_SIZE_BASED(dest.ptr, "日😀本語", std::strlen("日😀本語"), std::format("Failed case: %s",  __PRETTY_FUNCTION__));

    EXPECT_TRUE(dyn_str_utf8_insert(&dest, 0, &src));
    ASSERT_STREQ_SIZE_BASED(dest.ptr, "😀日😀本語", std::strlen("😀日😀本語"), std::format("Failed case: %s", __PRETTY_FUNCTION__));

    EXPECT_FALSE(dyn_str_utf8_insert(&dest, 99, &src));

    dyn_str_utf8_destroy(&dest);
    dyn_str_utf8_destroy(&src);
}

TEST(Utf8MutationTest, EraseCodepoints) {
    dyn_str_utf8_t str = DYN_STR_UTF8_ZERO;

    ASSERT_TRUE(dyn_str_utf8_from_cstr(&str, STDLIB_ALLOCATOR, "日😀本語"));

    EXPECT_TRUE(dyn_str_utf8_erase(&str, 1, 1));
    ASSERT_STREQ_SIZE_BASED(str.ptr, "日本語", std::strlen("日本語"), std::format("Failed case: %s", __PRETTY_FUNCTION__));

    EXPECT_TRUE(dyn_str_utf8_erase(&str, 0, 2));
    ASSERT_STREQ_SIZE_BASED(str.ptr, "語", std::strlen("語"), std::format("Failed case: %s", __PRETTY_FUNCTION__));

    EXPECT_FALSE(dyn_str_utf8_erase(&str, 5, 1));
    EXPECT_FALSE(dyn_str_utf8_erase(&str, 0, 10));

    dyn_str_utf8_destroy(&str);
}

TEST(Utf8MutationTest, ConcatStrings) {
    dyn_str_utf8_t dest = DYN_STR_UTF8_ZERO;
    dyn_str_utf8_t src = DYN_STR_UTF8_ZERO;

    ASSERT_TRUE(dyn_str_utf8_from_cstr(&dest, STDLIB_ALLOCATOR, "Hello "));
    ASSERT_TRUE(dyn_str_utf8_from_cstr(&src, STDLIB_ALLOCATOR, "世界! 😀"));

    EXPECT_TRUE(dyn_str_utf8_concat(&dest, &src));
    ASSERT_STREQ_SIZE_BASED(dest.ptr, "Hello 世界! 😀", std::strlen("Hello 世界! 😀"), std::format("Failed case: %s", __PRETTY_FUNCTION__));

    dyn_str_utf8_t empty = DYN_STR_UTF8_ZERO;
    ASSERT_TRUE(dyn_str_utf8_from_cstr(&empty, STDLIB_ALLOCATOR, ""));
    EXPECT_TRUE(dyn_str_utf8_concat(&dest, &empty));
    ASSERT_STREQ_SIZE_BASED(dest.ptr, "Hello 世界! 😀", std::strlen("Hello 世界! 😀"), std::format("Failed case: %s", __PRETTY_FUNCTION__));

    dyn_str_utf8_destroy(&dest);
    dyn_str_utf8_destroy(&src);
    dyn_str_utf8_destroy(&empty);
}

struct TrimTestCase {
    const char* description;
    const char* input;
    const char* expected_output;
};

class Utf8TrimTest : public ::testing::TestWithParam<TrimTestCase> {};

TEST_P(Utf8TrimTest, TrimsWhitespaceCorrectly) {
    const auto& param = GetParam();

    dyn_str_utf8_t str = DYN_STR_UTF8_ZERO;
    ASSERT_TRUE(dyn_str_utf8_from_cstr(&str, STDLIB_ALLOCATOR, param.input));

    EXPECT_TRUE(dyn_str_utf8_trim(&str));
    ASSERT_STREQ_SIZE_BASED(str.ptr, param.expected_output, std::strlen(param.expected_output), std::format("Failed case: %s", param.description));

    dyn_str_utf8_destroy(&str);
}

INSTANTIATE_TEST_SUITE_P(
    TrimCases,
    Utf8TrimTest,
    ::testing::Values(
        TrimTestCase{"Basic ASCII trim", "  Hello World  \t\n", "Hello World"},
        TrimTestCase{"No whitespace", "日本語", "日本語"},
        TrimTestCase{"Whitespace only", "   \t\r\n ", ""},
        TrimTestCase{"UTF-8 Whitespace / String with internal spaces", "  こんにちは 世界  ", "こんにちは 世界"}
    )
);

TEST(Utf8MutationTest, ReplaceSubstrings) {
    dyn_str_utf8_t str = DYN_STR_UTF8_ZERO;
    dyn_str_utf8_t target = DYN_STR_UTF8_ZERO;
    dyn_str_utf8_t replacement = DYN_STR_UTF8_ZERO;

    ASSERT_TRUE(dyn_str_utf8_from_cstr(&str, STDLIB_ALLOCATOR, "I love 🍎 and 🍎!"));
    ASSERT_TRUE(dyn_str_utf8_from_cstr(&target, STDLIB_ALLOCATOR, "🍎"));
    ASSERT_TRUE(dyn_str_utf8_from_cstr(&replacement, STDLIB_ALLOCATOR, "🍊"));

    EXPECT_TRUE(dyn_str_utf8_replace(&str, &target, &replacement));
    EXPECT_TRUE(std::string_view(str.ptr, str.size).find("🍊") != std::string_view::npos);

    dyn_str_utf8_destroy(&str);
    dyn_str_utf8_destroy(&target);
    dyn_str_utf8_destroy(&replacement);
}

TEST(Utf8MutationSafetyTest, HandlesNullPointersGracefully) {
    dyn_str_utf8_t str = DYN_STR_UTF8_ZERO;
    ASSERT_TRUE(dyn_str_utf8_from_cstr(&str, STDLIB_ALLOCATOR, "Test"));

    EXPECT_FALSE(dyn_str_utf8_append_codepoint(nullptr, 0x0041));
    EXPECT_FALSE(dyn_str_utf8_pop_back_codepoint(nullptr));
    EXPECT_FALSE(dyn_str_utf8_insert(nullptr, 0, &str));
    EXPECT_FALSE(dyn_str_utf8_insert(&str, 0, nullptr));
    EXPECT_FALSE(dyn_str_utf8_erase(nullptr, 0, 1));
    EXPECT_FALSE(dyn_str_utf8_concat(nullptr, &str));
    EXPECT_FALSE(dyn_str_utf8_concat(&str, nullptr));
    EXPECT_FALSE(dyn_str_utf8_trim(nullptr));
    EXPECT_FALSE(dyn_str_utf8_replace(nullptr, &str, &str));

    dyn_str_utf8_destroy(&str);
}