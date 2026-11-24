// Copyright 2026 mcnn authors
// SPDX-License-Identifier: BSD-3-Clause

#include <cstdint>

#include <string>

#include <gtest/gtest.h>
#include <mgraph/log.hpp>

namespace {

struct Capture {
    mgraph::LogLevel level = mgraph::LogLevel::kDebug;
    std::string message;
    void* user_data = nullptr;
    std::int32_t calls = 0;
};

void capture_handler(mgraph::LogLevel level, const char* message, void* user_data) {
    auto* capture = static_cast<Capture*>(user_data);
    capture->level = level;
    capture->message = message;
    capture->user_data = user_data;
    ++capture->calls;
}

}  // namespace

TEST(Log, IsSilentWhenNoHandlerInstalled) {
    mgraph::set_log_handler(nullptr, nullptr);
    EXPECT_EQ(mgraph::log_handler(), nullptr);
    MGRAPH_LOGE("must stay silent");
    EXPECT_EQ(mgraph::log_handler(), nullptr);
}

TEST(Log, ForwardsLevelMessageAndUserData) {
    Capture capture;
    mgraph::set_log_handler(capture_handler, &capture);
    MGRAPH_LOGW("graph failed with code %d", 7);
    EXPECT_EQ(capture.level, mgraph::LogLevel::kWarning);
    EXPECT_EQ(capture.message, "graph failed with code 7");
    EXPECT_EQ(mgraph::log_user_data(), &capture);
    mgraph::set_log_handler(nullptr, nullptr);
}

TEST(Log, CanBeCleared) {
    Capture capture;
    mgraph::set_log_handler(capture_handler, &capture);
    MGRAPH_LOGI("once");
    mgraph::set_log_handler(nullptr, nullptr);
    EXPECT_EQ(mgraph::log_handler(), nullptr);
    EXPECT_EQ(capture.calls, 1);
}
