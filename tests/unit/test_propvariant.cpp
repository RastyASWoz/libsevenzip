#include <gtest/gtest.h>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <objbase.h>
#include <oleauto.h>
#include <propidl.h>
#endif

#include "wrapper/common/wrapper_propvariant.hpp"

using namespace sevenzip;
using namespace sevenzip::detail;

// 测试 PropVariantGuard 构造和析构
TEST(PropVariantTest, GuardConstruction) {
    PropVariantGuard guard;
    EXPECT_NE(guard.get(), nullptr);

    // 应该初始化为 VT_EMPTY
    EXPECT_EQ(guard.get()->vt, VT_EMPTY);
}

TEST(PropVariantTest, GuardDestruction) {
    // 测试 RAII - 应该自动清理
    {
        PropVariantGuard guard;
        wstring_to_prop(L"Test", guard.get());
        EXPECT_EQ(guard.get()->vt, VT_BSTR);
    }
    // 此时 guard 应该已经被析构并清理
}

// 测试 wstring 转换
TEST(PropVariantTest, PropToWstringBasic) {
    PropVariantGuard guard;
    wstring_to_prop(L"Hello World", guard.get());

    auto result = prop_to_wstring(*guard.get());
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, L"Hello World");
}

TEST(PropVariantTest, PropToWstringEmpty) {
    PropVariantGuard guard;

    auto result = prop_to_wstring(*guard.get());
    EXPECT_FALSE(result.has_value());
}

TEST(PropVariantTest, PropToWstringChinese) {
    PropVariantGuard guard;
    wstring_to_prop(L"你好世界", guard.get());

    auto result = prop_to_wstring(*guard.get());
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, L"你好世界");
}

TEST(PropVariantTest, WstringToProp) {
    PropVariantGuard guard;
    std::wstring test = L"Test String";

    wstring_to_prop(test, guard.get());

    EXPECT_EQ(guard.get()->vt, VT_BSTR);
    EXPECT_NE(guard.get()->bstrVal, nullptr);
}

// 测试 uint64 转换
TEST(PropVariantTest, PropToUint64) {
    PropVariantGuard guard;
    uint64_to_prop(12345678901234ULL, guard.get());

    auto result = prop_to_uint64(*guard.get());
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 12345678901234ULL);
}

TEST(PropVariantTest, PropToUint64Zero) {
    PropVariantGuard guard;
    uint64_to_prop(0, guard.get());

    auto result = prop_to_uint64(*guard.get());
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 0ULL);
}

TEST(PropVariantTest, PropToUint64Max) {
    PropVariantGuard guard;
    uint64_to_prop(UINT64_MAX, guard.get());

    auto result = prop_to_uint64(*guard.get());
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, UINT64_MAX);
}

TEST(PropVariantTest, Uint64ToProp) {
    PropVariantGuard guard;
    uint64_to_prop(9876543210ULL, guard.get());

    EXPECT_EQ(guard.get()->vt, VT_UI8);
}

// 测试 uint32 转换
TEST(PropVariantTest, PropToUint32) {
    PropVariantGuard guard;
    uint32_to_prop(123456, guard.get());

    auto result = prop_to_uint32(*guard.get());
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 123456U);
}

TEST(PropVariantTest, Uint32ToProp) {
    PropVariantGuard guard;
    uint32_to_prop(987654, guard.get());

    EXPECT_EQ(guard.get()->vt, VT_UI4);
}

// 测试 bool 转换
TEST(PropVariantTest, PropToBoolTrue) {
    PropVariantGuard guard;
    bool_to_prop(true, guard.get());

    auto result = prop_to_bool(*guard.get());
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(*result);
}

TEST(PropVariantTest, PropToBoolFalse) {
    PropVariantGuard guard;
    bool_to_prop(false, guard.get());

    auto result = prop_to_bool(*guard.get());
    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(*result);
}

TEST(PropVariantTest, BoolToProp) {
    PropVariantGuard guard;
    bool_to_prop(true, guard.get());

    EXPECT_EQ(guard.get()->vt, VT_BOOL);
    EXPECT_EQ(guard.get()->boolVal, VARIANT_TRUE);

    bool_to_prop(false, guard.get());
    EXPECT_EQ(guard.get()->boolVal, VARIANT_FALSE);
}

// 测试 FILETIME 转换
TEST(PropVariantTest, FiletimeToProp) {
    FILETIME ft;
    ft.dwLowDateTime = 0x12345678;
    ft.dwHighDateTime = 0x9ABCDEF0;

    PropVariantGuard guard;
    filetime_to_prop(ft, guard.get());

    EXPECT_EQ(guard.get()->vt, VT_FILETIME);
    EXPECT_EQ(guard.get()->filetime.dwLowDateTime, ft.dwLowDateTime);
    EXPECT_EQ(guard.get()->filetime.dwHighDateTime, ft.dwHighDateTime);
}

TEST(PropVariantTest, PropToFiletime) {
    FILETIME ft;
    ft.dwLowDateTime = 0x11111111;
    ft.dwHighDateTime = 0x22222222;

    PropVariantGuard guard;
    filetime_to_prop(ft, guard.get());

    auto result = prop_to_filetime(*guard.get());
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->dwLowDateTime, ft.dwLowDateTime);
    EXPECT_EQ(result->dwHighDateTime, ft.dwHighDateTime);
}

// 测试 chrono 转换
TEST(PropVariantTest, FiletimeToTimepoint) {
    // 测试一个已知的时间点
    FILETIME ft;
    ft.dwLowDateTime = 0xD53E8000;
    ft.dwHighDateTime = 0x01D6A0B4;

    auto tp = filetime_to_timepoint(ft);

    // 转换回来应该得到相同的值（允许小的精度误差）
    auto ft_back = timepoint_to_filetime(tp);

    EXPECT_EQ(ft_back.dwHighDateTime, ft.dwHighDateTime);
    // dwLowDateTime 可能有小的精度损失（< 100 纳秒 = 1 tick）
    EXPECT_NEAR(ft_back.dwLowDateTime, ft.dwLowDateTime, 100);
}

TEST(PropVariantTest, TimepointToFiletime) {
    auto now = std::chrono::system_clock::now();

    auto ft = timepoint_to_filetime(now);
    auto tp_back = filetime_to_timepoint(ft);

    // 由于精度损失，我们只检查是否在合理范围内（1秒内）
    auto diff = std::chrono::duration_cast<std::chrono::seconds>(now - tp_back);

    EXPECT_LE(std::abs(diff.count()), 1);
}

// 测试 clear 方法
TEST(PropVariantTest, ClearMethod) {
    PropVariantGuard guard;
    wstring_to_prop(L"Test", guard.get());

    EXPECT_EQ(guard.get()->vt, VT_BSTR);

    guard.clear();

    EXPECT_EQ(guard.get()->vt, VT_EMPTY);
}

// 测试多次赋值
TEST(PropVariantTest, MultipleAssignments) {
    PropVariantGuard guard;

    wstring_to_prop(L"First", guard.get());
    auto str1 = prop_to_wstring(*guard.get());
    ASSERT_TRUE(str1.has_value());
    EXPECT_EQ(*str1, L"First");

    uint64_to_prop(123, guard.get());
    auto num = prop_to_uint64(*guard.get());
    ASSERT_TRUE(num.has_value());
    EXPECT_EQ(*num, 123ULL);

    bool_to_prop(true, guard.get());
    auto b = prop_to_bool(*guard.get());
    ASSERT_TRUE(b.has_value());
    EXPECT_TRUE(*b);
}

// 测试类型不匹配
TEST(PropVariantTest, TypeMismatch) {
    PropVariantGuard guard;
    wstring_to_prop(L"Not a number", guard.get());

    // 尝试读取为数字应该返回 nullopt
    auto num = prop_to_uint64(*guard.get());
    EXPECT_FALSE(num.has_value());
}

// 测试空值处理
TEST(PropVariantTest, EmptyVariant) {
    PropVariantGuard guard;

    EXPECT_FALSE(prop_to_wstring(*guard.get()).has_value());
    EXPECT_FALSE(prop_to_uint64(*guard.get()).has_value());
    EXPECT_FALSE(prop_to_bool(*guard.get()).has_value());
    EXPECT_FALSE(prop_to_filetime(*guard.get()).has_value());
}

// 测试 int64 转换
TEST(PropVariantTest, PropToInt64Positive) {
    PropVariantGuard guard;
    guard.get()->vt = VT_I8;
#ifdef _WIN32
    guard.get()->hVal.QuadPart = 1234567890LL;
#else
    guard.get()->hVal = 1234567890LL;
#endif

    auto result = prop_to_int64(*guard.get());
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 1234567890LL);
}

TEST(PropVariantTest, PropToInt64Negative) {
    PropVariantGuard guard;
    guard.get()->vt = VT_I8;
#ifdef _WIN32
    guard.get()->hVal.QuadPart = -1234567890LL;
#else
    guard.get()->hVal = -1234567890LL;
#endif

    auto result = prop_to_int64(*guard.get());
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, -1234567890LL);
}
