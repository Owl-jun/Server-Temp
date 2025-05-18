#include "pch.h"
#include "QueueManager.h"
#include "SessionManager.hpp"
void QueueManager::push(const Task& task)
{
    {
        std::lock_guard<std::mutex> lock(TaskMutex);
        TaskQueue.push(task);
        //std::cout << "[QueueManager::push] task.message -> " << task.message << std::endl;
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
/// ------ 리팩토링 ------
/// TaskHandler 클래스 생성 후 
/// Task 별 작업로직 분리 할 것.
/// </summary>
/// <param name="task"></param>
void QueueManager::process(Task& task)
{
    auto& session = task.session;
    std::string msg = task.message;
    std::istringstream iss(msg);
    
    std::string ID;
    iss >> ID;

    if (ID == "CHAT")
    {
        std::string CHATMSG;
        std::getline(iss, CHATMSG);
        std::shared_ptr<std::string> shared_msg = std::make_shared<std::string>("CHAT " + session->get_chat_id() + " " + CHATMSG + "\n");
        SessionManager::GetInstance().BroadCast(shared_msg);

    }
    else if (ID == "LOGIN")
    {
        std::string CHATID;
        iss >> CHATID;
        
        session->set_chat_id(CHATID);
        session->set_pos(100, 100);
        std::shared_ptr<std::string> msg = std::make_shared<std::string>("LOGIN " + session->get_chat_id() + " " + session->get_position() + "\n");
        session->push_WriteQueue(msg);
        std::vector<std::weak_ptr<Session>>& ses = SessionManager::GetInstance().get_Sessions();
        for (auto it = ses.begin(); it != ses.end(); ++it)
        {
            if (auto ses = it->lock()) {
                std::shared_ptr<std::string> msg = std::make_shared<std::string>("LOGIN " + ses->get_chat_id() + " " + ses->get_position() + "\n");
                SessionManager::GetInstance().BroadCast(msg);
            }
            else
            {
                std::cout << "LOGIN 브로드캐스팅 오류 : Session이 존재하지 않습니다!" << std::endl;
            }
        }
    }

    else if (ID == "MOVE")
    {
        std::string DIR;
        iss >> DIR;
        std::istringstream moveIss(session->get_position());
        std::string curX, curY;
        moveIss >> curX >> curY;
        double dX = std::stod(curX);
        double dY = std::stod(curY);
        if (DIR == "LEFT") { if (dX - 10 >= 0) { session->set_pos(dX - 10, dY); } }
        else if (DIR == "RIGHT") { if (dX + 10 <= 900) { session->set_pos(dX + 10, dY); } }
        else if (DIR == "UP") { if (dY - 10 >= 0) { session->set_pos(dX, dY - 10); } }
        else if (DIR == "DOWN") { if (dY + 10 <= 500) { session->set_pos(dX, dY + 10); } }
        else { std::cout << "이동 불가" << std::endl; return; }
        std::shared_ptr<std::string> shared_msg = std::make_shared<std::string>("MOVE " + session->get_chat_id() + " " + session->get_position() + "\n");
        SessionManager::GetInstance().BroadCast(shared_msg);
    }

    //session->push_WriteQueue(msg);
    std::cout << "[QueueManager::process] Task 작업 완료 -> " << msg << std::endl;
}
