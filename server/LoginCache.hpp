#pragma once

#include "ICache.hpp"

#include <iostream>
#include <chrono>
#include <unordered_map>
#include <mutex>

#define DEFAULT_TTL 60

class LoginCache : ICache{
private:
    struct ValueEntry {
        std::string value;
        std::chrono::steady_clock::time_point expire;
    };

    std::unordered_map<std::string, ValueEntry> valueCache;
    
    std::mutex cacheMutex;
    std::mutex cvMutex;
    std::condition_variable cv;

    std::thread cleanupThread;
public:
    static LoginCache& GetInstance();

    // 단일 값 캐시
    void Set(const std::string& key, const std::string& value, int ttl = DEFAULT_TTL);
    std::optional<std::string> Get(const std::string& key);

    void StartCleanupThread();
    void StopCleanupThread();
    bool running = true;
private:
    LoginCache() = default;
    LoginCache(const LoginCache&) = delete;
    LoginCache& operator=(const LoginCache&) = delete;

    void Cleanup();

};

