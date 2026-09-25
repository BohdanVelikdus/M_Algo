#include <gtest/gtest.h>

#include "dyn_str_utf8.h"

struct CodepointBytesTestCase {
    const char* description;
    uint8_t lead_byte;
    size_t expected_bytes;
};

class Utf8CodepointBytesTest : public ::testing::TestWithParam<CodepointBytesTestCase> {};

TEST_P(Utf8CodepointBytesTest, ComputesCorrectByteLengthFromLeadByte) {
    const auto& param = GetParam();
    size_t actual_bytes = dyn_str_utf8_codepoint_bytes(param.lead_byte);

    EXPECT_EQ(actual_bytes, param.expected_bytes) << "Failed case: " << param.description;
}

INSTANTIATE_TEST_SUITE_P(
    CodepointBytesCases,
    Utf8CodepointBytesTest,
    ::testing::Values(
        // ASCII (1-Byte: 0x00 - 0x7F)
        CodepointBytesTestCase{"ASCII NUL (0x00)", 0x00, 1},
        CodepointBytesTestCase{"ASCII Standard ('A' / 0x41)", 0x41, 1},
        CodepointBytesTestCase{"ASCII Upper Bound (0x7F)", 0x7F, 1},

        // Continuation Bytes (Invalid Lead Bytes -> 0x80 to 0xBF)
        CodepointBytesTestCase{"Continuation Byte (0x80)", 0x80, 0},
        CodepointBytesTestCase{"Continuation Byte (0xBF)", 0xBF, 0},

        // Overlong 2-Byte Lead Bytes (Invalid RFC 3629)
        CodepointBytesTestCase{"Invalid Lead Byte (0xC0)", 0xC0, 0},
        CodepointBytesTestCase{"Invalid Lead Byte (0xC1)", 0xC1, 0},

        // Valid 2-Byte Lead Bytes (0xC2 - 0xDF)
        CodepointBytesTestCase{"2-Byte Min Lead Byte (0xC2)", 0xC2, 2},
        CodepointBytesTestCase{"2-Byte Max Lead Byte (0xDF)", 0xDF, 2},

        // Valid 3-Byte Lead Bytes (0xE0 - 0xEF)
        CodepointBytesTestCase{"3-Byte Min Lead Byte (0xE0)", 0xE0, 3},
        CodepointBytesTestCase{"3-Byte Max Lead Byte (0xEF)", 0xEF, 3},

        // Valid 4-Byte Lead Bytes (0xF0 - 0xF4)
        CodepointBytesTestCase{"4-Byte Min Lead Byte (0xF0)", 0xF0, 4},
        CodepointBytesTestCase{"4-Byte Max Valid Lead Byte (0xF4)", 0xF4, 4},

        // Out of Unicode Range Lead Bytes (0xF5 - 0xFF)
        CodepointBytesTestCase{"Invalid Lead Byte > U+10FFFF (0xF5)", 0xF5, 0},
        CodepointBytesTestCase{"Invalid Lead Byte (0xF7)", 0xF7, 0},
        CodepointBytesTestCase{"Invalid Lead Byte (0xFF)", 0xFF, 0}
    )
);

struct EncodeUtf8TestCase {
    const char* description;
    uint32_t codepoint;
    size_t expected_len;
    uint8_t expected_bytes[4];
    bool should_be_valid;
};

class Utf8EncodeTest : public ::testing::TestWithParam<EncodeUtf8TestCase> {};

TEST_P(Utf8EncodeTest, EncodesCodepointsToBytes) {
    const auto& param = GetParam();

    utf8_delim_bytes_t result = dyn_str_utf8_encode_utf8(param.codepoint);

    if (param.should_be_valid) {
        EXPECT_EQ(result.len, param.expected_len) << "Failed len for: " << param.description;
        for (size_t i = 0; i < result.len; ++i) {
            EXPECT_EQ(static_cast<uint8_t>(result.bytes[i]), param.expected_bytes[i])
                << "Mismatch at byte index " << i << " for: " << param.description;
        }
    } else {
        EXPECT_EQ(result.len, 0) << "Should have failed for invalid codepoint: " << param.description;
    }
}

INSTANTIATE_TEST_SUITE_P(
    EncodeCases,
    Utf8EncodeTest,
    ::testing::Values(
        // 1-Byte ASCII
        EncodeUtf8TestCase{"ASCII 'A' (U+0041)", 0x0041, 1, {0x41}, true},
        EncodeUtf8TestCase{"ASCII NUL (U+0000)", 0x0000, 1, {0x00}, true},

        // 2-Byte Codepoints
        EncodeUtf8TestCase{"Cyrillic 'а' (U+0430)", 0x0430, 2, {0xD0, 0xB0}, true},

        // 3-Byte Codepoints
        EncodeUtf8TestCase{"CJK '語' (U+8A9E)", 0x8A9E, 3, {0xE8, 0xAA, 0x9E}, true},

        // 4-Byte Codepoints
        EncodeUtf8TestCase{"Emoji '😀' (U+1F600)", 0x1F600, 4, {0xF0, 0x9F, 0x98, 0x80}, true},
        EncodeUtf8TestCase{"Max Unicode (U+10FFFF)", 0x10FFFF, 4, {0xF4, 0x8F, 0xBF, 0xBF}, true},

        // Invalid Codepoints
        EncodeUtf8TestCase{"Surrogate Low Half (U+D800)", 0xD800, 0, {}, false},
        EncodeUtf8TestCase{"Surrogate High Half (U+DFFF)", 0xDFFF, 0, {}, false},
        EncodeUtf8TestCase{"Exceeds U+10FFFF (U+110000)", 0x110000, 0, {}, false}
    )
);

TEST(Utf8HelperTest, DecodesBytesToUint32Codepoint) {
    // 1-Byte ASCII
    EXPECT_EQ(utf8_bytes_to_uint32("A"), 0x0041);

    // 2-Byte Cyrillic 'а' (\xD0\xB0)
    EXPECT_EQ(utf8_bytes_to_uint32("\xD0\xB0"), 0x0430);

    // 3-Byte CJK '語' (\xE8\xAA\x9E)
    EXPECT_EQ(utf8_bytes_to_uint32("\xE8\xAA\x9E"), 0x8A9E);

    // 4-Byte Emoji '😀' (\xF0\x9F\x98\x80)
    EXPECT_EQ(utf8_bytes_to_uint32("\xF0\x9F\x98\x80"), 0x1F600);

    // Max Unicode U+10FFFF (\xF4\x8F\xBF\xBF)
    EXPECT_EQ(utf8_bytes_to_uint32("\xF4\x8F\xBF\xBF"), 0x10FFFF);
}

TEST(Utf8HelperTest, HandlesNullPointerForBytesToUint32) {
    EXPECT_EQ(utf8_bytes_to_uint32(nullptr), 0);
}