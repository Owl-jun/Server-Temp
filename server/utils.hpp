#pragma once
#include "pch.h"
#include <iomanip>
#include <filesystem>

inline std::string get_current_date_string() {
    auto now = std::chrono::system_clock::now();
    std::time_t time_now = std::chrono::system_clock::to_time_t(now);

    std::tm tm_now;
#ifdef _WIN32
    localtime_s(&tm_now, &time_now);
#else
    localtime_r(&time_now, &tm_now);
#endif

    std::ostringstream oss;
    oss << "./log_" << std::put_time(&tm_now, "%Y-%m-%d") << ".txt";
    return oss.str();
}

inline void set_debug_log() {
	std::filesystem::create_directory("logs");
	std::string log_filename = get_current_date_string();
	auto logger = spdlog::basic_logger_mt("file_logger", "logs/" + log_filename);
	spdlog::set_default_logger(logger);
	spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [TID : %t] %v");

#ifdef _DEBUG
	spdlog::set_level(spdlog::level::debug); // 개발 중엔 다 찍어!
#else
	spdlog::set_level(spdlog::level::warn);  // 릴리즈에선 경고 이상만!
#endif
}

