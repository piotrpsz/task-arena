#include <chrono>
#include <fmt/core.h>
#include "../pool.h"

using namespace std::chrono_literals;
task_arena::ThreadPool tp;

std::optional<int> task() {
    auto f = tp.add_task([]() {
        return 1;
    });
    std::this_thread::sleep_for(1600ms);
    if (tp.canceled()) {
        task_arena::printer::print("-- task 0 canceled");
        return std::nullopt;
    }
    task_arena::printer::print("-- task 0 finished");
    return 4 + 6;
}

int main() {
    task_arena::printer::print("main thread: ", std::this_thread::get_id());

    auto fut1 = tp.add_task(task);

    auto fut2 = tp.add_task([=]() -> std::optional<int> {
        std::this_thread::sleep_for(900ms);
        if (tp.canceled()) {
            task_arena::printer::print("-- task 1 canceled");
            return {};
        }
        task_arena::printer::print("-- task 1 finished");
        return 100 + 200;
    });
//    tp.cancel();

//    std::this_thread::sleep_for(2s);
    if (auto result = fut2.get(); result)
        task_arena::printer::print("*  wynik: ", *result);
    tp.cancel();
    if (auto result = fut1.get(); result)
        task_arena::printer::print("** wynik: ", result.value());

}
