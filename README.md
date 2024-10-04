# Task-Arena

Task-Arena (hereinafter referred to as Arena) is a set of header files that allow<br>
for managing tasks executed in parallel. Arena works as a pool of threads, pre-allocated when<br>
Arena is created and existing as long as Arena exists. In these posix threads, tasks<br>
added by the user can be executed in parallel. Each of the posix threads of Arena has its<br>
own task queue. Arena supports task stealing, i.e. when the queue of one posix thread is empty,<br>
it takes a task from the queue of another posix thread. Arena implements a cancelation mechanism,<br>
but for it to work, the user must include it in their tasks.<br>

By default, the thread pool (Arena) uses two fewer POSIX threads than the hardware allows.<br>
The user can also specify the number of posix threads pre-allocated by Arena.

When a task is passed to Arena, the user receives `std::future` as feedback through which he<br>
can get the result of the task. Due to the possibility of interruption (cancellation), it <br>
makes sense to declare the result returned by the task as optional (e.g. `std::optional<int>`).<br>
(See examples)


Example of use:
```c++
#include <optional>
#include "task-arena/pool.h"

using namespace std::chrono_literals;

task_arena::ThreadPool pool;

/// Function used as task.
/// As return value we use std::optional<int> instead int.
/// This gives us the ability to return 'nothing' when a cancel occurs.
std::optional<int> task_as_function() {
    auto internal_task_future = pool.add_task([]() {
        std::this_thread::sleep_for(300ms);
        return 5;
    });

    std::this_thread::sleep_for(600ms);
    if (pool.canceled()) {
        task_arena::printer::print("-- Canceled task_as_function");
        return {};
    }
    task_arena::printer::print("-- Finished task_as_function");
    return 6 + internal_task_future.get();
}

/// Lambda used as task.
auto task_as_lambda = []() -> std::optional<int> {
    std::this_thread::sleep_for(900ms);

    if (pool.canceled()) {
        task_arena::printer::print("-- Canceled task_as_lambda");
        return {};
    }
    task_arena::printer::print("-- Finished task_as_lambda");
    return 100 + 200;
};

int main() {
    auto fut1 = pool.add_task(task_as_function);
    auto fut2 = pool.add_task(task_as_lambda);

    if (auto result = fut1.get(); result)
        task_arena::printer::print("** Result of task_as_function: ", result.value());

    pool.cancel();

    if (auto const result = fut2.get(); result)
        task_arena::printer::print("task_as_lambda result: ", *result);

    return 0;
}
```

Result of example execution:
```c++
Created thread: 0x16bb1f000, idx: 0
Created thread: 0x16bcc3000, idx: 3
Created thread: 0x16bd4f000, idx: 4
Created thread: 0x16bc37000, idx: 2
Created thread: 0x16bddb000, idx: 5
Created thread: 0x16bbab000, idx: 1
task added to pool queue
task added to pool queue
started task from pool queue: 2
task added to local queue
started task from pool queue: 0
started task stealed from other queue: 1
-- Finished task_as_function
** Result of task_as_function: 11
-- Canceled task_as_lambda
joiner
```
