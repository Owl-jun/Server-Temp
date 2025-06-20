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
        //auto getData = LoginCache::GetInstance().Get(uname);
        
        if (!RedisManager::GetInstance().Get("login:" + uname))
        {
            // 응답 시간 TEST ms
            DBTask task;
            task.func = [uname, pwd](mysqlx::Session& s) {
                try {
                    auto start = std::chrono::high_resolution_clock::now();

                    mysqlx::Schema db = s.getSchema("mydb");
                    mysqlx::Table users = db.getTable("users");

                    auto res = users.select("username", "password")
                        .where("username = :uname AND password = :pwd")
                        .bind("uname", uname)
                        .bind("pwd", pwd)
                        .execute();

                    if (res.count() <= 0) {
                        std::cout << "[LOGIN 실패] 유저 정보 없음\n";
                    }
                    else {
                        for (auto row : res) {
                            std::string name = row[0].get<std::string>();
                            std::string pw = row[1].get<std::string>();
                            std::cout << "[LOGIN 성공] ID: " << name << ", PWD: " << pw << "\n";
                            RedisManager::GetInstance().Set("login:" + name, pw, 60);
                            // LoginCache::GetInstance().Set(uname, pw);
                        }
                    }
                    auto end = std::chrono::steady_clock::now();
                    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                    std::cout << "DBTask Execute Time : " << elapsed << std::endl;
                }
                catch (const mysqlx::Error& e) {
                    std::cerr << "[LOGIN 쿼리 예외] MySQL 오류: " << e.what() << "\n";
                }
                catch (const std::exception& e) {
                    std::cerr << "[LOGIN 예외] std::예외: " << e.what() << "\n";
                }

            };

            DBManager::GetInstance().PushTask(task);    
        }
        else {
            auto start = std::chrono::high_resolution_clock::now();

            if (RedisManager::GetInstance().Get("login:" + uname).has_value() && pwd == RedisManager::GetInstance().Get("login:" + uname).value()) {
                std::cout << "로그인 성공" << std::endl;
            }
            else {
                std::cout << "[LOGIN 실패] 유저 정보 없음\n";
            }
            auto end = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            std::cout << "DBTask Execute Time : " << elapsed << std::endl;
        }



    }
    else if (ID == "CHAT")
    {
    }
    else if (ID == "MOVE")
    {
    }

    //std::cout << "[QueueManager::process] Task 작업 완료 -> " << msg << std::endl;
}
