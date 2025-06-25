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

    void StartSubscriber(const std::string& channel);
    void StopSubscriber();

private:
    std::shared_ptr<redisContext> commandContext;   // GET/SET
    std::shared_ptr<redisContext> subscriberContext; // SUBSCRIBE

    std::atomic<bool> isRunning = false;
    std::thread subscriberThread;

};
