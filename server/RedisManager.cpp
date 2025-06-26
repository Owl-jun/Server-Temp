#include "pch.h"
#include "RedisManager.hpp"
#include "utils.hpp"
#include "SessionManager.hpp"

struct RedisReplyDeleter {
    void operator()(redisReply* reply) const {
        if (reply) freeReplyObject(reply);
    }
};

RedisManager& RedisManager::GetInstance() {
    static RedisManager instance;
    return instance;
}

bool RedisManager::Connect(const std::string& host, int port) {
    redisContext* raw = redisConnect(host.c_str(), port);
    if (!raw || raw->err)
    {
        std::cout << "REDIS Connect FAILED !" << std::endl;
        return false;
    }
    std::cout << "REDIS Connected !" << std::endl;

    commandContext = std::shared_ptr<redisContext>(raw, [](redisContext* ctx) {
        if (ctx) redisFree(ctx);
        });
    return true;
}

bool RedisManager::Set(const std::string& key, const std::string& value, int ttl) {
    if (!commandContext) return false;

    std::unique_ptr<redisReply, RedisReplyDeleter> reply(
        ttl > 0
        ? (redisReply*)redisCommand(commandContext.get(), "SETEX %s %d %s", key.c_str(), ttl, value.c_str())
        : (redisReply*)redisCommand(commandContext.get(), "SET %s %s", key.c_str(), value.c_str()));

    return reply && reply->type == REDIS_REPLY_STATUS && std::string(reply->str) == "OK";
}

std::optional<std::string> RedisManager::Get(const std::string& key) {
    if (!commandContext) return std::nullopt;

    std::unique_ptr<redisReply, RedisReplyDeleter> reply(
        (redisReply*)redisCommand(commandContext.get(), "GET %s", key.c_str()));

    if (!reply || reply->type != REDIS_REPLY_STRING)
        return std::nullopt;

    return std::string(reply->str);
}

void RedisManager::StartSubscriber(const std::string& channel)
{
    if (isRunning) return;

    isRunning = true;

    subscriberThread = std::thread([this, channel]() {
        redisContext* raw = redisConnect("127.0.0.1", 6379);  
        if (!raw || raw->err) {
            std::cerr << "REDIS Subscriber Connect FAILED!" << std::endl;
            isRunning = false;
            return;
        }

        subscriberContext = std::shared_ptr<redisContext>(raw, [](redisContext* ctx) {
            if (ctx) redisFree(ctx);
            });

        std::unique_ptr<redisReply, RedisReplyDeleter> reply(
            (redisReply*)redisCommand(subscriberContext.get(), "SUBSCRIBE %s", channel.c_str()));

        if (!reply) {
            std::cerr << "SUBSCRIBE Command Failed!" << std::endl;
            isRunning = false;
            return;
        }

        std::cout << "Subscribed to channel: " << channel << std::endl;

        while (isRunning) {
            redisReply* rawReply = nullptr;
            if (redisGetReply(subscriberContext.get(), (void**)&rawReply) != REDIS_OK || !rawReply) {
                std::cerr << "redisGetReply failed or stopped" << std::endl;
                break;
            }

            std::unique_ptr<redisReply, RedisReplyDeleter> reply(rawReply);
            if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 3) {
                std::string message = reply->element[2]->str;
                //std::cout << "[REDIS SUB] Received message: " << message << std::endl;
                HandleMsg(message);
            }
        }

        std::cout << "Unsubscribing..." << std::endl;
        });
}

void RedisManager::StopSubscriber()
{
    if (!isRunning) return;

    isRunning = false;

    if (subscriberContext) {
        redisCommand(subscriberContext.get(), "UNSUBSCRIBE");
    }

    if (subscriberThread.joinable()) {
        subscriberThread.join();
    }

    subscriberContext.reset();
}

void RedisManager::HandleMsg(const std::string& msg)
{
    json jsonctx = json::parse(msg);
    int opcode = jsonctx.value("oper", -1);
    std::string username = jsonctx["UserState"].value("UserName","NotFound");
    std::cout << "RedisManager opcode : " << opcode << std::endl;
    std::cout << "RedisManager username : " << username << std::endl;
    if (opcode == -1) 
    {
        std::cout << "Error : opcode is not define" << std::endl;
        return; 
    }
    else if (opcode == static_cast<int>(LoginState::LOGIN))
    {
        //SessionManager::GetInstance().BroadCast(
        //    static_cast<int>(Opcode::LOGIN),
        //    std::make_shared<std::string>(username)
        //);
    }
    else if (opcode == static_cast<int>(LoginState::LOGOUT))
    {
        SessionManager::GetInstance().BroadCast(
            static_cast<int>(Opcode::LOGOUT),
            std::make_shared<std::string>(username)
        );
    }
}

