#pragma once
#include <string>
#include <optional>
#include <hiredis/hiredis.h>

class RedisManager {
public:
    static RedisManager& GetInstance();

    bool Connect(const std::string& host, int port);
    bool Set(const std::string& key, const std::string& value, int ttlSeconds = 0);
    std::optional<std::string> Get(const std::string& key);

private:
    std::shared_ptr<redisContext> context;

};
