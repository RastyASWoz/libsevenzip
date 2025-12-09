#include <gtest/gtest.h>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <objbase.h>
#endif

#include "wrapper/common/wrapper_error.hpp"

using namespace sevenzip;

// 测试 ErrorCode 枚举
TEST(ErrorTest, ErrorCodeValues) {
    EXPECT_EQ(static_cast<int>(ErrorCode::Success), 0);
    EXPECT_NE(static_cast<int>(ErrorCode::FileNotFound), 0);
    EXPECT_NE(static_cast<int>(ErrorCode::InvalidArgument), 0);
}

// 测试 ErrorCategory
TEST(ErrorTest, ErrorCategoryName) {
    const auto& category = ErrorCategory::instance();
    EXPECT_STREQ(category.name(), "sevenzip");
}

TEST(ErrorTest, ErrorCategoryMessage) {
    const auto& category = ErrorCategory::instance();

    auto msg_success = category.message(static_cast<int>(ErrorCode::Success));
    EXPECT_FALSE(msg_success.empty());

    auto msg_not_found = category.message(static_cast<int>(ErrorCode::FileNotFound));
    EXPECT_FALSE(msg_not_found.empty());
    EXPECT_NE(msg_not_found, msg_success);

    auto msg_invalid_arg = category.message(static_cast<int>(ErrorCode::InvalidArgument));
    EXPECT_FALSE(msg_invalid_arg.empty());
}

// 测试 make_error_code
TEST(ErrorTest, MakeErrorCode) {
    auto ec = make_error_code(ErrorCode::Success);
    EXPECT_FALSE(ec);  // Success 应该为 false（无错误）
    EXPECT_EQ(ec.value(), 0);

    auto ec_error = make_error_code(ErrorCode::FileNotFound);
    EXPECT_TRUE(ec_error);  // 有错误
    EXPECT_NE(ec_error.value(), 0);
}

// 测试 Exception 类
TEST(ErrorTest, ExceptionConstruction) {
    Exception ex(ErrorCode::InvalidArgument, "Test error");
    EXPECT_STREQ(ex.what(), "Test error");
}

TEST(ErrorTest, ExceptionThrow) {
    try {
        throw Exception(ErrorCode::AccessDenied, "Access denied test");
        FAIL() << "Exception should have been thrown";
    } catch (const Exception& e) {
        EXPECT_STREQ(e.what(), "Access denied test");
    } catch (...) {
        FAIL() << "Wrong exception type caught";
    }
}

TEST(ErrorTest, ExceptionInheritance) {
    try {
        throw Exception(ErrorCode::OutOfMemory, "Memory error");
    } catch (const std::exception& e) {
        // Exception 应该继承自 std::exception
        EXPECT_STREQ(e.what(), "Memory error");
    }
}

// 测试 HRESULT 映射
TEST(ErrorTest, HResultToErrorCode) {
    // S_OK -> Success
    EXPECT_EQ(hresult_to_error_code(0x00000000), ErrorCode::Success);

    // E_NOTIMPL -> NotImplemented
    EXPECT_EQ(hresult_to_error_code(0x80004001), ErrorCode::NotImplemented);

    // E_INVALIDARG -> InvalidArgument
    EXPECT_EQ(hresult_to_error_code(0x80070057), ErrorCode::InvalidArgument);

    // E_ACCESSDENIED -> AccessDenied
    EXPECT_EQ(hresult_to_error_code(0x80070005), ErrorCode::AccessDenied);

    // ERROR_FILE_NOT_FOUND -> FileNotFound
    EXPECT_EQ(hresult_to_error_code(0x80070002), ErrorCode::FileNotFound);

    // 未知的 HRESULT -> Unknown
    EXPECT_EQ(hresult_to_error_code(0x99999999), ErrorCode::Unknown);
}

// 测试 SZ_CHECK_HR 宏
TEST(ErrorTest, SzCheckHrMacroSuccess) {
    HRESULT hr = 0x00000000;  // S_OK
    // 不应该抛出异常
    EXPECT_NO_THROW({ SZ_CHECK_HR(hr); });
}

TEST(ErrorTest, SzCheckHrMacroFailure) {
    HRESULT hr = 0x80070002;  // ERROR_FILE_NOT_FOUND

    try {
        SZ_CHECK_HR(hr);
        FAIL() << "SZ_CHECK_HR should have thrown";
    } catch (const Exception& e) {
        // 验证异常被抛出，消息应该是 "File not found"
        EXPECT_STREQ(e.what(), "File not found");
    }
}

TEST(ErrorTest, SzCheckHrMsgMacro) {
    HRESULT hr = 0x80070005;  // ERROR_ACCESS_DENIED

    try {
        SZ_CHECK_HR_MSG(hr, "Custom error message");
        FAIL() << "SZ_CHECK_HR_MSG should have thrown";
    } catch (const Exception& e) {
        EXPECT_STREQ(e.what(), "Custom error message");
    }
}

// 测试错误码比较
TEST(ErrorTest, ErrorCodeComparison) {
    auto ec1 = make_error_code(ErrorCode::FileNotFound);
    auto ec2 = make_error_code(ErrorCode::FileNotFound);
    auto ec3 = make_error_code(ErrorCode::AccessDenied);

    EXPECT_EQ(ec1, ec2);
    EXPECT_NE(ec1, ec3);
}
