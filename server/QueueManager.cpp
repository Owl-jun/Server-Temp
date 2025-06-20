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

        //std::cout << "[QueueManager::run] TaskQueue �� �۾��� �ֽ��ϴ�. �۾� ���� -> " << task.message << std::endl;
        process(task);
    }
}

/// <summary>
/// ��ȿ�� ��Ŷ ID �۾� ó�� ����
/// </summary>
/// <param name="task">session , message �� ������ ����ü</param>
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
    else if (ID == "LOGIN") // ��� TEST
    {
        std::string uname, pwd;
        iss >> uname >> pwd;
        //auto getData = LoginCache::GetInstance().Get(uname);
        
        if (!RedisManager::GetInstance().Get("login:" + uname))
        {
            // ���� �ð� TEST ms
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
                        std::cout << "[LOGIN ����] ���� ���� ����\n";
                    }
                    else {
                        for (auto row : res) {
                            std::string name = row[0].get<std::string>();
                            std::string pw = row[1].get<std::string>();
                            std::cout << "[LOGIN ����] ID: " << name << ", PWD: " << pw << "\n";
                            RedisManager::GetInstance().Set("login:" + name, pw, 60);
                            // LoginCache::GetInstance().Set(uname, pw);
                        }
                    }
                    auto end = std::chrono::steady_clock::now();
                    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                    std::cout << "DBTask Execute Time : " << elapsed << std::endl;
                }
                catch (const mysqlx::Error& e) {
                    std::cerr << "[LOGIN ���� ����] MySQL ����: " << e.what() << "\n";
                }
                catch (const std::exception& e) {
                    std::cerr << "[LOGIN ����] std::����: " << e.what() << "\n";
                }

            };

            DBManager::GetInstance().PushTask(task);    
        }
        else {
            auto start = std::chrono::high_resolution_clock::now();

            if (RedisManager::GetInstance().Get("login:" + uname).has_value() && pwd == RedisManager::GetInstance().Get("login:" + uname).value()) {
                std::cout << "�α��� ����" << std::endl;
            }
            else {
                std::cout << "[LOGIN ����] ���� ���� ����\n";
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

    //std::cout << "[QueueManager::process] Task �۾� �Ϸ� -> " << msg << std::endl;
}
