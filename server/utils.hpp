#pragma once
#include "pch.h"
#include <fstream>
#include <iomanip>
#include <filesystem>
#include <windows.h>
#include <nlohmann/json.hpp>

/////////////////////////////////////////
/////////////////////////////////////////
/////////////////////////////////////////
// log 관련

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
	spdlog::set_level(spdlog::level::debug); // 디버그모드
#else
	spdlog::set_level(spdlog::level::warn);  // 릴리즈모드
#endif
}

inline std::string to_utf8(const std::string& cp949_str) {
    int wide_len = MultiByteToWideChar(949, 0, cp949_str.c_str(), -1, nullptr, 0);
    std::wstring wide_str(wide_len, 0);
    MultiByteToWideChar(949, 0, cp949_str.c_str(), -1, &wide_str[0], wide_len);

    int utf8_len = WideCharToMultiByte(CP_UTF8, 0, wide_str.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string utf8_str(utf8_len, 0);
    WideCharToMultiByte(CP_UTF8, 0, wide_str.c_str(), -1, &utf8_str[0], utf8_len, nullptr, nullptr);

    return utf8_str;
}


/////////////////////////////////////////
/////////////////////////////////////////
/////////////////////////////////////////
// json


//using json = nlohmann::json;

inline bool set_dbInfo(std::string& ip, int& port, std::string& userId, std::string& pwd) 
{
    std::ifstream configFile("secure/config.json");
    if (!configFile.is_open()) {
        std::cerr << "config.json 파일을 열 수 없습니다." << std::endl;
        return false;
    }

    try
    {
        json config;
        configFile >> config;

        ip = config["DBINFO"]["IP"];
        port = config["DBINFO"]["PORT"];
        userId = config["DBINFO"]["DBID"];
        pwd = config["DBINFO"]["PWD"];
    }
    catch (const std::exception& e)
    {
        std::cout << "json 작업 실패 : " << e.what() << std::endl;
        return false;
    }

    return true;
}

inline bool get_json(json& ctx)
{
    std::ifstream configFile("secure/config.json");
    if (!configFile.is_open()) {
        std::cerr << "config.json 파일을 열 수 없습니다." << std::endl;
        return false;
    }
    try
    {
        ctx;
        configFile >> ctx;
    }
    catch (const std::exception& e)
    {
        std::cout << "json 작업 실패 : " << e.what() << std::endl;
        return false;
    }

    return true;
}