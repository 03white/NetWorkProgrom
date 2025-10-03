// fake_reactor.cpp
#include <iostream>
#include <queue>
#include <unordered_map>
#include <vector>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <random>
/*
这是一个在用户态模拟“事件驱动+回调"的例子，






*/

// 1. 事件类型
using Event     = std::string;
using Data      = int;               // 简单起见，事件携带一个 int
using Callback  = std::function<void(const Data&)>;

// 2. 全局事件总线
class Reactor {
public:
    // 注册回调
    void on(const Event& e, Callback cb) {
        handlers_[e].push_back(std::move(cb));
    }

    // 发射事件（线程安全）
    void emit(const Event& e, Data d = 0) {
        {
            std::lock_guard<std::mutex> lk(m_);
            q_.push({e, d});
        }
        cv_.notify_one();
    }

    // 事件循环
    void loop() {
        while (true) {
            std::unique_lock<std::mutex> lk(m_);
            cv_.wait(lk, [this]{ return !q_.empty(); });

            auto [ev, data] = q_.front();
            q_.pop();
            lk.unlock();

            // 执行该事件的所有回调
            for (auto& cb : handlers_[ev]) cb(data);

            if (ev == "EXIT") break;
        }
    }

private:
    std::queue<std::pair<Event, Data>> q_;
    std::unordered_map<Event, std::vector<Callback>> handlers_;
    std::mutex m_;
    std::condition_variable cv_;
};

// 3. 业务代码
int main() {
    Reactor r;

    // 回调
    r.on("TICK", [](const Data& ts){
        std::cout << "[TICK] ts = " << ts << '\n';
    });
    r.on("KEY_ENTER", [&r](const Data&){
        std::cout << "[KEY] 用户敲了回车 -> 发送 EXIT\n";
        r.emit("EXIT");
    });
    r.on("EXIT", [](const Data&){
        std::cout << "[EXIT] 事件循环即将结束\n";
    });

    // 4. 事件源：定时器线程
    std::thread ticker([&r]{
        std::mt19937 rng{std::random_device{}()};
        std::uniform_int_distribution<int> dist(500, 1500);
        int ts = 0;
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(dist(rng)));
            r.emit("TICK", ++ts);
        }
    });

    // 5. 事件源：键盘线程
    std::thread kbd([&r]{
        while (true) {
            std::cin.get();            // 阻塞等回车
            r.emit("KEY_ENTER");
        }
    });

    // 6. 启动事件循环（主线程）
    r.loop();

    ticker.detach();
    kbd.detach();
    return 0;
}