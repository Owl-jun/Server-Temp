#include "pch.h"
#include "QueueManager.hpp"
#include "SessionManager.hpp"
#include "DBManager.hpp"
#include "Session.hpp"
#include <chrono>

void QueueManager::push(const Task& task)
{
    {
        std::lock_guard<std::mutex> lock(TaskMutex);
        TaskQueue.push(task);
        spdlog::info("[QueueManager::push] task.message -> " + task.message);
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
    std::istringstream iss(msg);
    
    // ID Parsing
    std::string ID;
    iss >> ID;


    //////////////////////////////////////////
    //////////////////////////////////////////
    //////////////////////////////////////////
    // TO DO PROCESS
    if (ID == "TEST")
    {
        session->push_WriteQueue(std::make_shared<std::string>("one-to-one test\n"));
        SessionManager::GetInstance().BroadCast(std::make_shared<std::string>("BroadCasting test\n"));
    }
    else if (ID == "LOGIN") // 기능 TEST
    {
        std::string uname, pwd;
        iss >> uname >> pwd;
        DBTask task;
        task.func = [uname, pwd](mysqlx::Session& s) {

            mysqlx::Schema db = s.getSchema("mydb");
            mysqlx::Table users = db.getTable("users");

            // 응답 시간 TEST ms
            auto start = std::chrono::high_resolution_clock::now();
            
            // query execute
            auto res = users.select("username", "password")
                .where("username = :uname AND password = :pwd")
                .bind("uname", uname)
                .bind("pwd", pwd)
                .execute();

            // 응답 시간 TEST ms
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            std::cout << "query execute time : " << duration << std::endl;

            if (res.count() <= 0) {
                std::cout << "로그인실패 ~" << std::endl;
            }
            else {
                for (auto row : res)
                {
                    std::cout << "ID : " << row[0].get<std::string>()
                        << ", PWD : " << row[1].get<std::string>() << std::endl;
                    std::cout << "로그인 성공!" << std::endl;
                }
            }
        };

        DBManager::GetInstance().PushTask(task);
    }
    else if (ID == "CHAT")
    {
    }
    else if (ID == "MOVE")
    {
    }

    std::cout << "[QueueManager::process] Task 작업 완료 -> " << msg << std::endl;
}
