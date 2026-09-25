#include <gtest/gtest.h>
#include <cstdint>

#include "dyn_str_utf8.h"

static const std::string utf8_sample = "Hello World 🦀 - UTF-8 Test String 🦔";

struct FindTestCase {
    const char* description;
    const char* haystack;
    const char* needle;
    intptr_t expected_index;
};

class Utf8FindTest : public ::testing::TestWithParam<FindTestCase> {};

TEST_P(Utf8FindTest, ReturnsCorrectIndex) {
    const auto& param = GetParam();

    dyn_str_utf8_t haystack = DYN_STR_UTF8_ZERO;
    dyn_str_utf8_t needle = DYN_STR_UTF8_ZERO;

    ASSERT_TRUE(dyn_str_utf8_from_cstr(&haystack, STDLIB_ALLOCATOR, param.haystack));
    ASSERT_TRUE(dyn_str_utf8_from_cstr(&needle, STDLIB_ALLOCATOR, param.needle));

    const intptr_t actual_index = dyn_str_utf8_find(&haystack, &needle);

    EXPECT_EQ(actual_index, param.expected_index) << "Failed case: " << param.description;

    dyn_str_utf8_destroy(&haystack);
    dyn_str_utf8_destroy(&needle);
}

INSTANTIATE_TEST_SUITE_P(
    FindCases,
    Utf8FindTest,
    ::testing::Values(
        // Basic & ASCII
        FindTestCase{"Simple ASCII Match", "Hello World", "World", 6},
        FindTestCase{"ASCII Match at Start", "Hello World", "Hello", 0},
        FindTestCase{"ASCII Match at End", "Hello World", "d", 10},
        FindTestCase{"ASCII Not Found", "Hello World", "Planet", -1},

        // Empty Substrings
        FindTestCase{"Empty Needle in Non-Empty Haystack", "Hello", "", 0},
        FindTestCase{"Empty Needle in Empty Haystack", "", "", 0},
        FindTestCase{"Non-Empty Needle in Empty Haystack", "", "A", -1},

        // Multi-byte UTF-8 (Cyrillic, CJK, Emoji)
        FindTestCase{"Cyrillic Match ('Привет')", "Привет, Мир!", "Мир", 14}, // byte offset (14) or codepoint depending on implementation
        FindTestCase{"CJK Match ('日本語')", "私は日本語を話します", "日本語", 6},  // byte index: 3 bytes per char in '私は'
        FindTestCase{"Emoji Match ('😀')", "Hello 😀 World", "😀", 6},
        FindTestCase{"Emoji Not Found", "Hello 😀 World", "😁", -1},

        // Edge Cases
        FindTestCase{"Needle Longer Than Haystack", "Short", "Longer Needle", -1},
        FindTestCase{"Identical Strings", "ExactMatch", "ExactMatch", 0},
        FindTestCase{"Partial Match Repeated", "banana", "ana", 1}
    )
);

struct StartsWithTestCase {
    const char* description;
    const char* str;
    const char* prefix;
    bool expected_result;
};

class Utf8StartsWithTest : public ::testing::TestWithParam<StartsWithTestCase> {};

TEST_P(Utf8StartsWithTest, ReturnsExpectedResult) {
    const auto& param = GetParam();

    dyn_str_utf8_t str = DYN_STR_UTF8_ZERO;
    dyn_str_utf8_t prefix = DYN_STR_UTF8_ZERO;

    ASSERT_TRUE(dyn_str_utf8_from_cstr(&str, STDLIB_ALLOCATOR, param.str));
    ASSERT_TRUE(dyn_str_utf8_from_cstr(&prefix, STDLIB_ALLOCATOR, param.prefix));

    bool result = dyn_str_utf8_starts_with(&str, &prefix);

    EXPECT_EQ(result, param.expected_result) << "Failed case: " << param.description;

    dyn_str_utf8_destroy(&str);
    dyn_str_utf8_destroy(&prefix);
}

INSTANTIATE_TEST_SUITE_P(
    StartsWithCases,
    Utf8StartsWithTest,
    ::testing::Values(
        StartsWithTestCase{"Exact Match", "Hello", "Hello", true},
        StartsWithTestCase{"Valid ASCII Prefix", "Hello World", "Hello", true},
        StartsWithTestCase{"Invalid ASCII Prefix", "Hello World", "World", false},
        StartsWithTestCase{"Empty Prefix", "Hello", "", true},
        StartsWithTestCase{"Empty String and Empty Prefix", "", "", true},
        StartsWithTestCase{"Prefix Longer Than String", "Hi", "Hello", false},
        StartsWithTestCase{"Multi-byte Emoji Prefix", "😀😁😂", "😀", true},
        StartsWithTestCase{"Multi-byte CJK Prefix", "日本語", "日", true},
        StartsWithTestCase{"Mismatched Multi-byte Prefix", "日本語", "本", false}
    )
);

struct EndsWithTestCase {
    const char* description;
    const char* str;
    const char* suffix;
    bool expected_result;
};

class Utf8EndsWithTest : public ::testing::TestWithParam<EndsWithTestCase> {};

TEST_P(Utf8EndsWithTest, ReturnsExpectedResult) {
    const auto& param = GetParam();

    dyn_str_utf8_t str = DYN_STR_UTF8_ZERO;
    dyn_str_utf8_t suffix = DYN_STR_UTF8_ZERO;

    ASSERT_TRUE(dyn_str_utf8_from_cstr(&str, STDLIB_ALLOCATOR, param.str));
    ASSERT_TRUE(dyn_str_utf8_from_cstr(&suffix, STDLIB_ALLOCATOR, param.suffix));

    bool result = dyn_str_utf8_ends_with(&str, &suffix);

    EXPECT_EQ(result, param.expected_result) << "Failed case: " << param.description;

    dyn_str_utf8_destroy(&str);
    dyn_str_utf8_destroy(&suffix);
}

INSTANTIATE_TEST_SUITE_P(
    EndsWithCases,
    Utf8EndsWithTest,
    ::testing::Values(
        EndsWithTestCase{"Exact Match", "World", "World", true},
        EndsWithTestCase{"Valid ASCII Suffix", "Hello World", "World", true},
        EndsWithTestCase{"Invalid ASCII Suffix", "Hello World", "Hello", false},
        EndsWithTestCase{"Empty Suffix", "World", "", true},
        EndsWithTestCase{"Empty String and Empty Suffix", "", "", true},
        EndsWithTestCase{"Suffix Longer Than String", "Hi", "Hello", false},
        EndsWithTestCase{"Multi-byte Emoji Suffix", "😀😁😂", "😂", true},
        EndsWithTestCase{"Multi-byte CJK Suffix", "日本語", "語", true},
        EndsWithTestCase{"Mismatched Multi-byte Suffix", "日本語", "日", false}
    )
);

TEST(Utf8SearchSafetyTest, HandlesNullPointersGracefully) {
    dyn_str_utf8_t valid_str = DYN_STR_UTF8_ZERO;
    ASSERT_TRUE(dyn_str_utf8_from_cstr(&valid_str, STDLIB_ALLOCATOR, "Test"));

    EXPECT_EQ(dyn_str_utf8_find(nullptr, &valid_str), -1);
    EXPECT_EQ(dyn_str_utf8_find(&valid_str, nullptr), -1);
    EXPECT_EQ(dyn_str_utf8_find(nullptr, nullptr), -1);

    EXPECT_FALSE(dyn_str_utf8_starts_with(nullptr, &valid_str));
    EXPECT_FALSE(dyn_str_utf8_starts_with(&valid_str, nullptr));

    EXPECT_FALSE(dyn_str_utf8_ends_with(nullptr, &valid_str));
    EXPECT_FALSE(dyn_str_utf8_ends_with(&valid_str, nullptr));

    dyn_str_utf8_destroy(&valid_str);
}

TEST(U8_nF, Equals)
{
    dyn_str_utf8_t lhs = DYN_STR_UTF8_ZERO;
    dyn_str_utf8_t rhs = DYN_STR_UTF8_ZERO;

    ASSERT_TRUE(dyn_str_utf8_from_cstr(&lhs, STDLIB_ALLOCATOR, utf8_sample.c_str()));
    ASSERT_TRUE(dyn_str_utf8_from_cstr(&rhs, STDLIB_ALLOCATOR, utf8_sample.c_str()));

    ASSERT_TRUE(dyn_str_utf8_equals(&lhs, &rhs));
    dyn_str_utf8_destroy(&lhs);
    dyn_str_utf8_destroy(&rhs);
}

TEST(U8_nF, Equals2)
{
    dyn_str_utf8_t lhs = DYN_STR_UTF8_ZERO;
    dyn_str_utf8_t rhs = DYN_STR_UTF8_ZERO;

    ASSERT_TRUE(dyn_str_utf8_from_cstr(&lhs, STDLIB_ALLOCATOR, utf8_sample.c_str()));
    ASSERT_TRUE(dyn_str_utf8_init(&rhs, 20));

    ASSERT_FALSE(dyn_str_utf8_equals(&lhs, &rhs));
    dyn_str_utf8_destroy(&lhs);
    dyn_str_utf8_destroy(&rhs);
}

struct LengthTestCase {
    const char* description;
    const char* cstr;
    size_t expected_codepoint_count;
};

class Utf8LengthValidTest : public ::testing::TestWithParam<LengthTestCase> {};

TEST_P(Utf8LengthValidTest, ComputesCorrectCodepointCount) {
    const auto& param = GetParam();

    dyn_str_utf8_t str = {.ptr = nullptr, .size = 0, .capacity = 0};
    ASSERT_TRUE(dyn_str_utf8_from_cstr(&str, STDLIB_ALLOCATOR, param.cstr))
        << "Failed to initialize string for case: " << param.description;

    size_t out_len = 99999;
    bool success = dyn_str_utf8_length(&str, &out_len);

    EXPECT_TRUE(success) << "Failed case: " << param.description;
    EXPECT_EQ(out_len, param.expected_codepoint_count) << "Failed case: " << param.description;

    dyn_str_utf8_destroy(&str);
}

INSTANTIATE_TEST_SUITE_P(
    ValidLengthCases,
    Utf8LengthValidTest,
    ::testing::Values(
        LengthTestCase{"Empty String", "", 0},
        LengthTestCase{"Pure ASCII String", "Hello World!", 12},
        LengthTestCase{"2-Byte Cyrillic ('Привет')", "Привет", 6},
        LengthTestCase{"3-Byte CJK ('日本語')", "日本語", 3},
        LengthTestCase{"4-Byte Emojis ('😀😁😂')", "😀😁😂", 3},
        LengthTestCase{"Mixed 1, 2, 3, and 4 byte codepoints", "Hi! Привет 日本語 😀", 16},
        LengthTestCase{"Max Valid Unicode Codepoint (U+10FFFF)", "\xF4\x8F\xBF\xBF", 1}
    )
);

struct InvalidLengthTestCase {
    const char* description;
    const char* cstr;
};

class Utf8LengthInvalidTest : public ::testing::TestWithParam<InvalidLengthTestCase> {};

TEST_P(Utf8LengthInvalidTest, FailsOnInvalidUTF8Sequence) {
    const auto& param = GetParam();

    dyn_str_utf8_t str = DYN_STR_UTF8_ZERO;

    if (dyn_str_utf8_from_cstr(&str, STDLIB_ALLOCATOR, param.cstr)) {
        size_t out_len = 12345;
        bool success = dyn_str_utf8_length(&str, &out_len);

        EXPECT_FALSE(success) << "Should have failed on invalid UTF-8: " << param.description;

        dyn_str_utf8_destroy(&str);
    }
}

INSTANTIATE_TEST_SUITE_P(
    InvalidLengthCases,
    Utf8LengthInvalidTest,
    ::testing::Values(
        InvalidLengthTestCase{"Truncated 2-byte sequence", "\xC2"},
        InvalidLengthTestCase{"Truncated 3-byte sequence", "\xE2\x80"},
        InvalidLengthTestCase{"Truncated 4-byte sequence", "\xF0\x9F\x98"},
        InvalidLengthTestCase{"Standalone continuation byte", "\x80"},
        InvalidLengthTestCase{"Invalid lead byte 0xFF", "\xFF"},
        InvalidLengthTestCase{"Overlong encoding (ASCII NUL in 2 bytes)", "\xC0\x80"},
        InvalidLengthTestCase{"Out of Unicode range (> U+10FFFF)", "\xF7\xBF\xBF\xBF"},
        InvalidLengthTestCase{"UTF-16 Surrogate Halves", "\xED\xA0\x80"}
    )
);

TEST(Utf8LengthSafetyTest, HandlesNullPointersGracefully) {
    size_t out_len = 0;
    dyn_str_utf8_t valid_str = DYN_STR_UTF8_ZERO;
    ASSERT_TRUE(dyn_str_utf8_from_cstr(&valid_str, STDLIB_ALLOCATOR, "Test"));
    EXPECT_FALSE(dyn_str_utf8_length(nullptr, &out_len));
    EXPECT_FALSE(dyn_str_utf8_length(&valid_str, nullptr));
    dyn_str_utf8_destroy(&valid_str);
}

