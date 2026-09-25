#include <gtest/gtest.h>

#include "dyn_str_utf8.h"

struct ValidTestCase {
    const char* description;
    std::string_view bytes;
};

class Utf8ValidTest : public ::testing::TestWithParam<ValidTestCase> {};

TEST_P(Utf8ValidTest, ReturnsTrueForValidUTF8) {
    const auto& param = GetParam();
    dyn_str_utf8_t str = DYN_STR_UTF8_ZERO;
    ASSERT_TRUE(dyn_str_utf8_from_cstr(&str, STDLIB_ALLOCATOR, param.bytes.data()));

    EXPECT_TRUE(dyn_str_utf8_is_valid(&str)) << "Failed case: " << param.description;
    dyn_str_utf8_destroy(&str);
}

INSTANTIATE_TEST_SUITE_P(
    ValidSequences,
    Utf8ValidTest,
    ::testing::Values(
        ValidTestCase{"Empty String", ""},
        ValidTestCase{"ASCII Standard", "Hello, World!"},
        ValidTestCase{"2-Byte Codepoint (Cyrillic)", "Привет"},
        ValidTestCase{"3-Byte Codepoint (CJK)", "日本語"},
        ValidTestCase{"4-Byte Codepoint (Emoji)", "😀😁😂😃"},
        ValidTestCase{"Mixed Codepoints", "A - \xD0\x96 - \xE5\xAD\x97 - \xF0\x9F\x98\x80"},
        ValidTestCase{"Max Valid Codepoint U+10FFFF", "\xF4\x8F\xBF\xBF"},
        ValidTestCase{"Overlong Threshold Edge (U+0080)", "\xC2\x80"},
        ValidTestCase{"Overlong Threshold Edge (U+0800)", "\xE0\xA0\x80"},
        ValidTestCase{"Overlong Threshold Edge (U+10000)", "\xF0\x90\x80\x80"}
    )
);

struct InvalidTestCase {
    const char* description;
    std::string_view bytes;
};

class Utf8InvalidTest : public ::testing::TestWithParam<InvalidTestCase> {};

TEST_P(Utf8InvalidTest, ReturnsFalseForInvalidUTF8) {
    const auto& param = GetParam();
    dyn_str_utf8_t str = DYN_STR_UTF8_ZERO;
    ASSERT_TRUE(dyn_str_utf8_from_cstr(&str, STDLIB_ALLOCATOR, param.bytes.data()));
    EXPECT_FALSE(dyn_str_utf8_is_valid(&str)) << "Failed case: " << param.description;
    dyn_str_utf8_destroy(&str);
}

INSTANTIATE_TEST_SUITE_P(
    InvalidSequences,
    Utf8InvalidTest,
    ::testing::Values(
        // Continuation Bytes Errors
        InvalidTestCase{"Standalone Continuation Byte", "\x80"},
        InvalidTestCase{"Unexpected Continuation Byte in ASCII", "Hello\xA0World"},
        InvalidTestCase{"Missing Continuation Byte (2-byte prefix)", "\xC2"},
        InvalidTestCase{"Missing Second Continuation Byte (3-byte prefix)", "\xE2\x80"},
        InvalidTestCase{"Missing Third Continuation Byte (4-byte prefix)", "\xF0\x9F\x98"},

        // Invalid Lead Bytes
        InvalidTestCase{"Invalid Lead Byte 0xFE", "\xFE"},
        InvalidTestCase{"Invalid Lead Byte 0xFF", "\xFF"},
        InvalidTestCase{"Obsolete 5-byte Lead Byte 0xF8", "\xF8\x80\x80\x80\x80"},

        // Overlong Encodings (Security Vulnerability Vector)
        InvalidTestCase{"Overlong ASCII NUL (0x00 encoded in 2 bytes)", "\xC0\x80"},
        InvalidTestCase{"Overlong ASCII 'A' (0x41 encoded in 2 bytes)", "\xC1\xA1"},
        InvalidTestCase{"Overlong 3-byte encoding for U+007F", "\xE0\x81\xBF"},
        InvalidTestCase{"Overlong 4-byte encoding for U+07FF", "\xF0\x80\x9F\xBF"},

        // Out-of-Range Codepoints (> U+10FFFF)
        InvalidTestCase{"Codepoint > U+10FFFF (U+110000)", "\xF4\x90\x80\x80"},
        InvalidTestCase{"Codepoint > U+10FFFF (U+13FFFF)", "\xF7\xBF\xBF\xBF"},

        // UTF-16 Surrogates (U+D800 to U+DFFF are illegal in UTF-8)
        InvalidTestCase{"Lead Surrogate U+D800", "\xED\xA0\x80"},
        InvalidTestCase{"Trail Surrogate U+DFFF", "\xED\xBF\xBF"}
    )
);

TEST(Utf8ValidationSafetyTest, HandlesNullPointers) {
    EXPECT_FALSE(dyn_str_utf8_is_valid(nullptr));

    dyn_str_utf8_t null_buf_str = DYN_STR_UTF8_ZERO;
    EXPECT_TRUE(dyn_str_utf8_is_valid(&null_buf_str));
}

