#include <gtest/gtest.h>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <objbase.h>
#include <oleauto.h>
#endif

#include "wrapper/common/wrapper_error.hpp"
#include "wrapper/common/wrapper_string.hpp"

using namespace sevenzip;
using namespace sevenzip::detail;

// 测试 UTF-8 到 UTF-16 转换
TEST(StringTest, Utf8ToWstringBasic) {
    std::string utf8 = "Hello World";
    std::wstring result = utf8_to_wstring(utf8);

    EXPECT_EQ(result, L"Hello World");
}

TEST(StringTest, Utf8ToWstringEmpty) {
    std::string utf8 = "";
    std::wstring result = utf8_to_wstring(utf8);

    EXPECT_TRUE(result.empty());
}

TEST(StringTest, Utf8ToWstringChinese) {
    std::string utf8 = "你好世界";
    std::wstring result = utf8_to_wstring(utf8);

    EXPECT_EQ(result, L"你好世界");
}

TEST(StringTest, Utf8ToWstringMixed) {
    std::string utf8 = "Hello 世界 123";
    std::wstring result = utf8_to_wstring(utf8);

    EXPECT_EQ(result, L"Hello 世界 123");
}

TEST(StringTest, Utf8ToWstringSpecialChars) {
    std::string utf8 = "Test: äöü ñ € ©";
    std::wstring result = utf8_to_wstring(utf8);

    EXPECT_EQ(result, L"Test: äöü ñ € ©");
}

// 测试 UTF-16 到 UTF-8 转换
TEST(StringTest, WstringToUtf8Basic) {
    std::wstring wstr = L"Hello World";
    std::string result = wstring_to_utf8(wstr);

    EXPECT_EQ(result, "Hello World");
}

TEST(StringTest, WstringToUtf8Empty) {
    std::wstring wstr = L"";
    std::string result = wstring_to_utf8(wstr);

    EXPECT_TRUE(result.empty());
}

TEST(StringTest, WstringToUtf8Chinese) {
    std::wstring wstr = L"你好世界";
    std::string result = wstring_to_utf8(wstr);

    EXPECT_EQ(result, "你好世界");
}

TEST(StringTest, WstringToUtf8Mixed) {
    std::wstring wstr = L"Test 测试 123";
    std::string result = wstring_to_utf8(wstr);

    EXPECT_EQ(result, "Test 测试 123");
}

// 测试往返转换
TEST(StringTest, RoundTripConversion) {
    std::string original = "Round trip test: 往返转换测试";
    std::wstring wide = utf8_to_wstring(original);
    std::string back = wstring_to_utf8(wide);

    EXPECT_EQ(original, back);
}

TEST(StringTest, RoundTripConversionWide) {
    std::wstring original = L"Wide round trip: 宽字符往返";
    std::string utf8 = wstring_to_utf8(original);
    std::wstring back = utf8_to_wstring(utf8);

    EXPECT_EQ(original, back);
}

// 测试路径转换
TEST(StringTest, PathToWstring) {
    std::filesystem::path path = "C:\\test\\path\\file.txt";
    std::wstring result = path_to_wstring(path);

    EXPECT_FALSE(result.empty());
    EXPECT_EQ(result, L"C:\\test\\path\\file.txt");
}

TEST(StringTest, WstringToPath) {
    std::wstring wstr = L"C:\\test\\path\\file.txt";
    std::filesystem::path result = wstring_to_path(wstr);

    EXPECT_EQ(result.wstring(), wstr);
}

TEST(StringTest, PathRoundTrip) {
    std::filesystem::path original = "C:\\test\\中文路径\\file.txt";
    std::wstring wide = path_to_wstring(original);
    std::filesystem::path back = wstring_to_path(wide);

    EXPECT_EQ(original, back);
}

// 测试 BStrGuard
TEST(StringTest, BstrGuardConstruction) {
    BStrGuard guard;
    EXPECT_EQ(guard.get(), nullptr);
}

TEST(StringTest, BstrGuardFromWstring) {
    std::wstring str = L"Test String";
    BStrGuard guard(str.c_str());

#ifdef _WIN32
    EXPECT_NE(guard.get(), nullptr);
    EXPECT_EQ(std::wstring(static_cast<const wchar_t*>(guard.get())), str);
#endif
}

TEST(StringTest, BstrGuardReset) {
    std::wstring str1 = L"First";
    std::wstring str2 = L"Second";

    BStrGuard guard(str1.c_str());
#ifdef _WIN32
    EXPECT_EQ(std::wstring(static_cast<const wchar_t*>(guard.get())), str1);
#endif

    guard.reset(str2.c_str());
#ifdef _WIN32
    EXPECT_EQ(std::wstring(static_cast<const wchar_t*>(guard.get())), str2);
#endif
}

TEST(StringTest, BstrGuardDetach) {
    std::wstring str = L"Test";
    BStrGuard guard(str.c_str());

#ifdef _WIN32
    void* bstr = guard.detach();
    EXPECT_NE(bstr, nullptr);
    EXPECT_EQ(guard.get(), nullptr);

    // 需要手动释放
    ::SysFreeString(static_cast<BSTR>(bstr));
#endif
}

TEST(StringTest, BstrGuardMove) {
    std::wstring str = L"Move test";
    BStrGuard guard1(str.c_str());

#ifdef _WIN32
    void* ptr1 = guard1.get();
    EXPECT_NE(ptr1, nullptr);
#endif

    BStrGuard guard2(std::move(guard1));

#ifdef _WIN32
    EXPECT_EQ(guard1.get(), nullptr);
    EXPECT_EQ(guard2.get(), ptr1);
#endif
}

// 测试特殊情况
TEST(StringTest, NullCharInString) {
    std::string utf8 = "Before\0After";
    // 注意：std::string 的构造函数会在 \0 处停止
    std::wstring result = utf8_to_wstring(utf8);

    EXPECT_EQ(result, L"Before");
}

TEST(StringTest, VeryLongString) {
    std::string utf8(10000, 'A');
    std::wstring result = utf8_to_wstring(utf8);

    EXPECT_EQ(result.length(), 10000);
    EXPECT_TRUE(std::all_of(result.begin(), result.end(), [](wchar_t c) { return c == L'A'; }));
}

TEST(StringTest, MultilineString) {
    std::string utf8 = "Line 1\nLine 2\r\nLine 3";
    std::wstring result = utf8_to_wstring(utf8);

    EXPECT_EQ(result, L"Line 1\nLine 2\r\nLine 3");
}

// 测试错误处理
#ifdef _WIN32
TEST(StringTest, InvalidUtf8) {
    // 无效的 UTF-8 序列
    std::string invalid_utf8 = "\xFF\xFE";

    // 在某些实现中可能会抛出异常或返回空字符串
    // 具体行为依赖于平台
    EXPECT_NO_THROW({ std::wstring result = utf8_to_wstring(invalid_utf8); });
}
#endif
