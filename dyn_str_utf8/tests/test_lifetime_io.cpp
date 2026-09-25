#include "gtest/gtest.h"

#include "dyn_str_utf8.h"

#include <fstream>
#include <cstdio>
#include <string>
#include <filesystem>
#include <string_view>
#include <unordered_set>

#if defined(_WIN32)
#include <io.h>
#define dup _dup
#define dup2 _dup2
#define fileno _fileno
#else
#include <unistd.h>
#endif

class U8 : public ::testing::Test {
protected:
    dyn_str_utf8_t str = {.ptr = nullptr, .size = 0, .capacity = 0};

    const char* test_file = "test_input.txt";
    const char* test_file_2 = "test_output.txt";

    int saved_stdin = -1;
    int saved_stdout = -1;

    void SetUp() override {
        bool success = dyn_str_utf8_init(&str, 20);
        EXPECT_TRUE(success);
    }

    void TearDown() override {
        dyn_str_utf8_destroy(&str);

        RestoreIO();

        std::remove(test_file);
        std::remove(test_file_2);
    }

public:
    void SetInput(const std::string& input_text) {
        std::ofstream out(test_file, std::ios::binary);
        out << input_text;
        out.close();

        saved_stdin = dup(fileno(stdin));

        std::freopen(test_file, "r", stdin);
    }

    void RedirectOutput() {
        std::ofstream out(test_file_2);
        out.close();

        saved_stdout = dup(fileno(stdout));

        std::freopen(test_file_2, "w", stdout);
    }

    void RestoreIO() {
        if (saved_stdin != -1) {
            std::fflush(stdin);
            dup2(saved_stdin, fileno(stdin));
            close(saved_stdin);
            saved_stdin = -1;
        }

        if (saved_stdout != -1) {
            std::fflush(stdout);
            dup2(saved_stdout, fileno(stdout));
            close(saved_stdout);
            saved_stdout = -1;
        }
    }
};

static const std::string utf8_sample = "Hello World 🦀 - UTF-8 Test String 🦔";

struct AllocatorTracker {
    size_t malloc_count = 0;
    size_t realloc_count = 0;
    size_t free_count = 0;
    std::unordered_set<void*> active_allocations;

    bool has_leaks() const {
        return !active_allocations.empty();
    }
};

static AllocatorTracker tracker;

void* tracked_malloc(size_t size) {
    void* ptr = std::malloc(size);
    if (ptr) {
        tracker.malloc_count++;
        tracker.active_allocations.insert(ptr);
    }
    return ptr;
}

void* tracked_realloc(void* ptr, const size_t new_size) {

    void* new_ptr = std::realloc(ptr, new_size);

    if (ptr != nullptr) {
        tracker.active_allocations.erase(ptr);
    }

    if (new_ptr != nullptr && new_size > 0) {
        tracker.active_allocations.insert(new_ptr);
        tracker.realloc_count++;
    } else if (new_size == 0) {
        tracker.free_count++;
    }

    return new_ptr;
}

void tracked_free(void* ptr) {
    if (!ptr) return;

    tracker.free_count++;
    tracker.active_allocations.erase(ptr);
    std::free(ptr);
}

TEST(U8_nF, InitWithAllocator)
{
    dyn_allocator_t alloc = {
        .malloc_fn = tracked_malloc,
        .realloc_fn = tracked_realloc,
        .free_fn = tracked_free,
    };

    dyn_str_utf8_t str = DYN_STR_UTF8_ZERO;

    const bool success = dyn_str_utf8_init_with_allocator(&str, alloc, 20);
    ASSERT_TRUE(success);

    dyn_str_utf8_destroy(&str);
    ASSERT_FALSE(tracker.has_leaks());
}

TEST_F(U8, FromCstr)
{
    const std::string msg = "1234567";
    bool success = dyn_str_utf8_from_cstr(&str, STDLIB_ALLOCATOR, msg.c_str());
    EXPECT_TRUE(success);
    RestoreIO();

    size_t out_len = 0;
    success = dyn_str_utf8_length(&str, &out_len);
    ASSERT_TRUE(success);
    EXPECT_EQ(out_len, 7);
    EXPECT_EQ(memcmp(str.ptr, msg.c_str(), str.size), 0);
}

TEST_F(U8, StdinRedirectionTest)
{
    SetInput("Hello UTF-8 🦀\n");
    bool success = dyn_str_utf8_read_line_console_chunked(&str, '\n');
    EXPECT_TRUE(success);
    RestoreIO();

    size_t out_len = 0;
    success = dyn_str_utf8_length(&str, &out_len);
    ASSERT_TRUE(success);
    EXPECT_EQ(out_len, 13);

    EXPECT_EQ(memcmp(str.ptr, "Hello UTF-8 🦀", str.size), 0);
    EXPECT_EQ(str.size, 16);

}

TEST_F(U8, PrintStreamEmpty)
{
    RedirectOutput();
    dyn_str_utf8_print_stream(&str, stdout);
    std::fflush(stdout);
    RestoreIO();

    std::ifstream output(test_file_2);
    EXPECT_TRUE(output.is_open());

    std::string output_str;
    output >> output_str;
    EXPECT_EQ(output_str, "[empty]");
}


TEST_F(U8, PrintStream)
{
    RedirectOutput();
    dyn_str_utf8_from_cstr(&str, STDLIB_ALLOCATOR, "HelloUTF-8🦀");
    dyn_str_utf8_print_stream(&str, stdout);
    std::fflush(stdout);
    RestoreIO();

    std::ifstream output(test_file_2);
    EXPECT_TRUE(output.is_open());

    std::string output_str;
    output >> output_str;
    EXPECT_EQ(output_str, "HelloUTF-8🦀");
}

TEST_F(U8, ReadStreamChunkedCStrAsciiDelim)
{
    const auto tempFile = std::filesystem::temp_directory_path()/ "U8_ReadStreamChunkedCStrAsciiDelim_temp.txt";
    FILE *file = fopen(tempFile.string().c_str(), "w+b");
    ASSERT_NE(file, nullptr);

    std::string data;
    data.reserve(2048);

    int i = 0;
    for (; i < 2048; ++i )
    {
        char ch = static_cast<char>('A' + (i % 58));
        data.push_back(ch);

        std::fwrite(&ch, sizeof(char), 1, file);
    }
    constexpr char newline = '\n';
    std::fwrite(&newline, sizeof(char), 1, file);

    std::fseek(file, 0, SEEK_SET);

    bool succ = dyn_str_utf8_read_line_stream_chunked(&str, '\n', file);
    ASSERT_TRUE(succ);
    i = 0;
    for (; i < data.size(); ++i)
    {
        ASSERT_EQ(data[i], str.ptr[i]);
    }

    std::fclose(file);
    std::filesystem::remove(tempFile.string().c_str());
}

TEST_F(U8, ReadStreamChunkedUtf8)
{
    const auto tempFile = std::filesystem::temp_directory_path() / "ReadStreamChunkedUtf8AsciiDelim_temp.txt";
    FILE *file = std::fopen(tempFile.string().c_str(), "w+b");
    ASSERT_NE(file, nullptr);

    std::string expected_data;

    while (expected_data.size() < 2048)
    {
        expected_data += utf8_sample;
    }

    std::fwrite(expected_data.data(), sizeof(char), expected_data.size(), file);
    std::fwrite("\n", sizeof(char), 1, file);

    std::fseek(file, 0, SEEK_SET);

    bool succ = dyn_str_utf8_read_line_stream_chunked(&str, '\n', file);
    ASSERT_TRUE(succ);

    for (int i = 0; i < expected_data.size(); ++i)
    {
        ASSERT_EQ(expected_data[i], str.ptr[i]);
    }

    std::fclose(file);
    std::filesystem::remove(tempFile);
}

TEST_F(U8, ReadStreamChunkedUtf8Utf8Delim)
{
    const auto tempFile = std::filesystem::temp_directory_path() / "ReadStreamChunkedUtf8Utf8Delim_temp.txt";
    FILE *file = std::fopen(tempFile.string().c_str(), "w+b");
    ASSERT_NE(file, nullptr);

    std::fwrite(utf8_sample.data(), sizeof(char), utf8_sample.size(), file);
    std::fwrite("\n", sizeof(char), 1, file);

    std::fseek(file, 0, SEEK_SET);

    const uint32_t crab = utf8_bytes_to_uint32( "🦀");

    bool succ = dyn_str_utf8_read_line_stream_chunked(&str, crab, file);
    ASSERT_TRUE(succ);

    const std::string stringRes{"Hello World "};
    for (int i = 0; i < stringRes.size(); ++i)
    {
        ASSERT_EQ(str.ptr[i], stringRes[i]);
    }

    std::fclose(file);
    std::filesystem::remove(tempFile);
}
