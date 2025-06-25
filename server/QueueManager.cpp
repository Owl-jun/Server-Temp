#include "pch.h"
#include "QueueManager.hpp"
#include "SessionManager.hpp"
#include "RedisManager.hpp"
#include "LoginCache.hpp"
#include "DBManager.hpp"
#include "Session.hpp"

void QueueManager::push(const Task& task)
{
    {
        std::lock_guard<std::mutex> lock(TaskMutex);
        TaskQueue.push(task);
        //std::cout << "Recv : " << task.message << std::endl;
        // spdlog::info("[QueueManager::push] task.message -> " + task.message);
    }
    TaskCV.notify_one();
}

void QueueManager::run()
{
    while (true) {
        std::unique_lock<std::mutex> lock(TaskMutex);
        TaskCV.wait(lock, [this]() { return !TaskQueue.empty(); });

        auto task = TaskQueue.front();
        TaskQueue.pop();
        lock.unlock();

        //std::cout << "[QueueManager::run] TaskQueue 에 작업이 있습니다. 작업 진행 -> " << task.message << std::endl;
        process(task);
    }
}

/// <summary>
/// 유효한 패킷 ID 작업 처리 로직
/// </summary>
/// <param name="task">session , message 로 구성된 구조체</param>
void QueueManager::process(Task& task)
{
    auto& session = task.session;
    std::string msg = task.message;
    const uint8_t* data = reinterpret_cast<const uint8_t*>(msg.data());
    
    // opcode & payload Parsing
    uint8_t opcode = data[0];
    std::string payload(msg.begin() + 1, msg.end());
    
    //////////////////////////////////////////
    //////////////////////////////////////////
    // TO DO PROCESS
    if (opcode == static_cast<int>(Opcode::LOGIN))
    {
        std::istringstream iss(payload);
        std::string uid;
        iss >> uid;
        SessionManager::GetInstance().BroadCast(
            static_cast<int>(Opcode::LOGIN),
            std::make_shared<std::string>(uid)
        );
    }
    else if (static_cast<int>(Opcode::MOVE))
    {
        std::cout << "opcode : " << std::to_string(opcode) << " payload : " << payload << std::endl;

        std::istringstream iss(payload);
        std::string uid, sx, sy, sz;
        iss >> uid;
        std::getline(iss, sx, ',');
        std::getline(iss, sy, ',');  // dummy
        std::getline(iss, sz, ',');

        float x = std::stod(sx);
        float y = std::stod(sz);
        session->set_player_position(x,y);
        std::cout << "set Pos -> " << session->get_player().get_player_data().pos.GetString() << std::endl;

        SessionManager::GetInstance().BroadCast(
            static_cast<int>(Opcode::MOVE),
            std::make_shared<std::string>(payload)
        );
    }
    else if (static_cast<int>(Opcode::ATTACK))
    {
    }
    else 
    {
        std::cout << "Unknown opcode: " << opcode << std::endl;
        return;
    }
}
