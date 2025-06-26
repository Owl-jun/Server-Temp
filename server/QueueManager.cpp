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
    int opcode = data[0];
    // opcode & payload Parsing
    //std::cout << "QueueManager OPCODE : " << opcode << std::endl;

    std::string payload(msg.begin() + 1, msg.end());
    //std::cout << "QueueManager payload : " << payload << std::endl;
    
    //////////////////////////////////////////
    //////////////////////////////////////////
    // TO DO PROCESS
    if (opcode == static_cast<int>(Opcode::LOGIN))
    {
        std::istringstream iss(payload);
        std::string name;
        iss >> name;
        //std::cout << "QueueManager LOGIN name : " << name << std::endl;

        auto flag = RedisManager::GetInstance().Get("user:state:" + name);
        if (flag != std::nullopt)
        {
            //std::cout << "QueueManager Flag != nullopt " << std::endl;

            DBManager::GetInstance().PushTask(
                DBTask{
                    .func = [session, name](mysqlx::Session& ses)
                    {
                        std::cout << "DBTask payload : " << name << std::endl;
                        mysqlx::Schema schema = ses.getSchema("gamedb");
                        mysqlx::Table table = schema.getTable("users");
                        auto result = table
                            .select("*")
                            .where("username = :id")        
                            .bind("id", name)
                            .execute();
                        /// 동기화 체크 필요
                        session->excute_event(result.fetchOne());
                    }
                }
            );

            SessionManager::GetInstance().BroadCast(
                static_cast<int>(Opcode::LOGIN),
                std::make_shared<std::string>(name)
            );

            auto sessions = SessionManager::GetInstance().get_Sessions();
            for (auto [i, ses] : sessions)
            {
                Player& p = ses.lock()->get_player();
                std::shared_ptr<std::string> un = std::make_shared<std::string>(p.get_name());
                if (*un != "")
                {
                    session->push_WriteQueue(
                        static_cast<int>(Opcode::LOGIN), un
                    );
                }
            }
        }
        else { return; }
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
        std::cout << "set Pos -> " << session->get_player().get_name() << std::endl;

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
