#include <gtest/gtest.h>

#include <format>
#include <cstring>

#include "dyn_str_utf8.h"

#define ASSERT_STREQ_SIZE_BASED(ptr1, ptr2, ptr_size, msg) \
    do { \
        ASSERT_GE((ptr_size), 0u) << "The size must be greater equal than 0"; \
        for (size_t i = 0; i < (size_t)(ptr_size); ++i) { \
            ASSERT_EQ(static_cast<uint8_t>((ptr1)[i]), static_cast<uint8_t>((ptr2)[i])) << (msg) << " at index " << i; \
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
        ASSERT_EQ(static_cast<uint8_t>(reversed.c_str()[i]), static_cast<uint8_t>(str.ptr[i]));
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
        ASSERT_EQ(static_cast<uint8_t>(r[i]), static_cast<uint8_t>(slice.ptr[i]));
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
    EXPECT_TRUE(std::string_view(reinterpret_cast<const char*>(str.ptr), str.size).find("🍊") != std::string_view::npos);

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

static const std::string template_string = "Hello 👋 World 🌍";

struct InsertUint32Codepoint {
    const char* description;
    uint32_t input;
    size_t position;
};

class InsertUint32TestParams : public ::testing::TestWithParam<InsertUint32Codepoint> {};

TEST_P(InsertUint32TestParams, InsertingCodepointOnPosition)
{
    const auto& param = GetParam();
    dyn_str_utf8_t str = DYN_STR_UTF8_ZERO;

    ASSERT_TRUE(dyn_str_utf8_from_cstr(&str, STDLIB_ALLOCATOR, template_string.c_str()));

    ASSERT_TRUE(dyn_str_utf8_insert_codepoint_uint32_t(&str, param.position, param.input))
        << "Failed to insert at position " << param.position << ": " << param.description;
    uint32_t actual = 0;
    ASSERT_TRUE(dyn_str_utf8_at_codepoint(&str, param.position, &actual))
        << "Failed to retrieve codepoint at position " << param.position;

    EXPECT_EQ(actual, param.input) << param.description;

    dyn_str_utf8_destroy(&str);
}

INSTANTIATE_TEST_SUITE_P(
    InsertingUint32Codepoint,
    InsertUint32TestParams,
    ::testing::Values(
        InsertUint32Codepoint{"ASCII 1-byte", '!', 2},
        InsertUint32Codepoint{"Greek 2-byte", 0x03B1, 5},
        InsertUint32Codepoint{"Euro 3-byte", 0x20AC, 2},
        InsertUint32Codepoint{"Emoji 4-byte 😀", 0x1F600, 7},
        InsertUint32Codepoint{"Emoji 4-byte 🦀", 0x1F980, 8}
    )
);

struct NegativeInsertParam {
    const char* description;
    uint32_t input;
    size_t position;
};

class InsertNegativeTestParams : public ::testing::TestWithParam<NegativeInsertParam> {};

TEST_P(InsertNegativeTestParams, ShouldFailInsertion)
{
    const auto& param = GetParam();
    dyn_str_utf8_t str = DYN_STR_UTF8_ZERO;
    ASSERT_TRUE(dyn_str_utf8_from_cstr(&str, STDLIB_ALLOCATOR, template_string.c_str()));

    const size_t original_size = str.size;

    EXPECT_FALSE(dyn_str_utf8_insert_codepoint_uint32_t(&str, param.position, param.input))
        << "Expected failure but insertion succeeded: " << param.description;

    EXPECT_EQ(str.size, original_size)
        << "String size changed despite failed insertion: " << param.description;

    dyn_str_utf8_destroy(&str);
}

INSTANTIATE_TEST_SUITE_P(
    InvalidInsertions,
    InsertNegativeTestParams,
    ::testing::Values(
        NegativeInsertParam{"Position slightly out of bounds", '!', 16},
        NegativeInsertParam{"Position far out of bounds", '!', 9999},
        NegativeInsertParam{"Max size_t position", '!', SIZE_MAX},

        NegativeInsertParam{"Surrogate Start (U+D800)", 0xD800, 0},
        NegativeInsertParam{"Surrogate Middle (U+DA00)", 0xDA00, 5},
        NegativeInsertParam{"Surrogate End (U+DFFF)", 0xDFFF, 2},

        NegativeInsertParam{"Just above Unicode max (U+110000)", 0x110000, 0},
        NegativeInsertParam{"Arbitrary high 32-bit integer", 0xDEADBEEF, 3},
        NegativeInsertParam{"Max uint32_t value", 0xFFFFFFFF, 1}
    )
);

TEST(InsertNegativeTest, ReturnsFalseOnNullDestPointer)
{
    EXPECT_FALSE(dyn_str_utf8_insert_codepoint_uint32_t(nullptr, 0, 'A'));
}

TEST(InsertNegativeTest, FailsOnUninitializedOrNullBuffer)
{
    dyn_str_utf8_t str = DYN_STR_UTF8_ZERO; // str.ptr == NULL, size == 0

    EXPECT_FALSE(dyn_str_utf8_insert_codepoint_uint32_t(&str, 1, 'A'));

    EXPECT_FALSE(dyn_str_utf8_insert_codepoint_uint32_t(&str, 0, 0xD800));
}

class InsertCharPtrTest : public U8 {
protected:

    void SetUp() override {
        U8::SetUp();
        ASSERT_TRUE(dyn_str_utf8_from_cstr(&str, STDLIB_ALLOCATOR, template_string.c_str()));
    }

    void TearDown() override {
        U8::TearDown();
    }
};

TEST_F(InsertCharPtrTest, InsertAtBeginning) {
    // Insert "Prefix " at codepoint index 0
    ASSERT_TRUE(dyn_str_utf8_insert_codepoint_char_ptr(&str, 0, "Prefix "));

    // Verify raw bytes match expected output: "Prefix Hello 👋 World 🌍"
    std::string expected = "Prefix " + template_string;
    EXPECT_EQ(str.size, expected.size());
    EXPECT_EQ(memcmp(str.ptr, expected.c_str(), str.size), 0);
}

TEST_F(InsertCharPtrTest, InsertInMiddleBetweenASCIIAndEmoji) {
    // Codepoint index 5 is the space before '👋'
    // String before: "Hello 👋 World 🌍"
    // Insert: "αβγ " (2-byte Greek letters)
    ASSERT_TRUE(dyn_str_utf8_insert_codepoint_char_ptr(&str, 5, "αβγ "));

    std::string expected = "Helloαβγ  👋 World 🌍";
    EXPECT_EQ(str.size, expected.size());
    EXPECT_EQ(memcmp(str.ptr, expected.c_str(), str.size), 0);
}

TEST_F(InsertCharPtrTest, Insert4ByteEmojiInMiddle) {
    // Codepoint index 7 is the space right after '👋'
    // Insert 4-byte crab emoji: "🦀"
    ASSERT_TRUE(dyn_str_utf8_insert_codepoint_char_ptr(&str, 7, "🦀"));

    std::string expected = "Hello 👋🦀 World 🌍";
    EXPECT_EQ(str.size, expected.size());
    EXPECT_EQ(memcmp(str.ptr, expected.c_str(), str.size), 0);
}

TEST_F(InsertCharPtrTest, InsertAtEndBoundary) {
    size_t length = 0;
    ASSERT_TRUE(dyn_str_utf8_length(&str, &length));
    ASSERT_EQ(length, 15u);

    ASSERT_TRUE(dyn_str_utf8_insert_codepoint_char_ptr(&str, length, " [END]"));

    std::string expected = template_string + " [END]";
    EXPECT_EQ(str.size, expected.size());
    EXPECT_EQ(memcmp(str.ptr, expected.c_str(), str.size), 0);
}

TEST_F(InsertCharPtrTest, InsertEmptyStringIsNoOpSuccess) {
    const size_t original_size = str.size;

    EXPECT_TRUE(dyn_str_utf8_insert_codepoint_char_ptr(&str, 3, ""));
    EXPECT_EQ(str.size, original_size);
}

TEST_F(InsertCharPtrTest, FailsOnNullPointers) {
    EXPECT_FALSE(dyn_str_utf8_insert_codepoint_char_ptr(nullptr, 0, "Test"));

    EXPECT_FALSE(dyn_str_utf8_insert_codepoint_char_ptr(&str, 0, nullptr));
}

TEST_F(InsertCharPtrTest, FailsOnOutOfBoundsPosition) {
    const size_t original_size = str.size;

    EXPECT_FALSE(dyn_str_utf8_insert_codepoint_char_ptr(&str, 16, "Fail"));
    EXPECT_FALSE(dyn_str_utf8_insert_codepoint_char_ptr(&str, 999, "Fail"));

    EXPECT_EQ(str.size, original_size);
}

TEST_F(InsertCharPtrTest, FailsOnInvalidUtf8Payloads) {
    const size_t original_size = str.size;

    const char invalid_seq_1[] = { static_cast<char>(0xCE), '\0' };
    EXPECT_FALSE(dyn_str_utf8_insert_codepoint_char_ptr(&str, 0, invalid_seq_1));

    const char invalid_seq_2[] = { static_cast<char>(0xC0), ' ', '\0' };
    EXPECT_FALSE(dyn_str_utf8_insert_codepoint_char_ptr(&str, 2, invalid_seq_2));

    const char surrogate_seq[] = { static_cast<char>(0xED), static_cast<char>(0xA0), static_cast<char>(0x80), '\0' };
    EXPECT_FALSE(dyn_str_utf8_insert_codepoint_char_ptr(&str, 5, surrogate_seq));

    EXPECT_EQ(str.size, original_size);
}