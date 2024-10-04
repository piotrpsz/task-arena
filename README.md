# Task-Arena

Task-Arena (hereinafter referred to as Arena) is a set of header files that allow<br>
for managing tasks executed in parallel. Arena works as a pool of threads, allocated when<br>
Arena is created and existing as long as Arena exists. In these posix threads, tasks<br>
added by the user can be executed in parallel. Each of the posix threads of Arena has its<br>
own task queue. Arena supports task stealing, i.e. when the queue of one posix thread is empty,<br>
it takes a task from the queue of another posix thread. Arena implements a cancelation mechanism,<br>
but for it to work, the user must include it in their tasks.<br>

By default, the thread pool (Arena) uses two fewer POSIX threads than the hardware allows.<br>
The user can also specify the number of posix threads preallocated by Arena.

When a task is passed to Arena, the user receives `std::future` as feedback through which he<br>
can get the result of the task. Due to the possibility of interruption (cancellation), it <br>
makes sense to declare the result returned by the task as optional (e.g. `std::optional<int>`).<br>
(See examples)
