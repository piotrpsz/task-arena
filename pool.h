// MIT License
//
// Copyright (c) 2024 Piotr Pszczółkowski
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Created by Piotr Pszczółkowski on 01/10/2024.
//
#pragma once

/*------- include files:
-------------------------------------------------------------------*/
#include <future>
#include "safe_queue.h"
#include "printer.h"
#include "stealing_queue.h"
#include "function_wrapper.h"

namespace task_arena {
    class ThreadPool {
        using TaskType = FunctionWrapper;
        SafeQueue <TaskType> pool_work_queue_;
        std::vector<std::unique_ptr<StealingQueue>> queues_;
        std::vector<std::thread> threads_;
        JoinTreads joiner;
        mutable std::atomic_bool done_;
        mutable std::atomic<bool> canceled_;
        static inline thread_local StealingQueue *local_work_queue_;
        static inline thread_local uint index_;

       void worker_thread(uint const idx) {
            index_ = idx;
            printer::print("Created thread: ", std::this_thread::get_id(), ", idx: ", index_);
            local_work_queue_ = queues_[index_].get();
            while (!done_)
                run_pending_task(idx);
        }
        static bool pop_task_from_local_queue(TaskType& task) {
            return local_work_queue_ and local_work_queue_->try_pop(task);
        }
        bool pop_task_from_pool_queue(TaskType& task) {
            return pool_work_queue_.try_pop(task);
        }
        bool pop_task_from_other_thread_queue(TaskType& task) {
            for (size_t i = 0; i < queues_.size(); ++i) {
                uint const index = (index_ + 1) % queues_.size();
                // TODO check which queue has the most tasks
                if (queues_[index]->try_steal(task))
                    return true;
            }
            return false;
        }

    public:
        // By default, the thread pool uses two fewer POSIX threads than the hardware allows.
        ThreadPool() : ThreadPool(std::thread::hardware_concurrency() - 2) {
        }
        // User specifies number of preallocated threads.
        explicit ThreadPool(uint thread_count) : joiner(threads_), done_{false} {
            canceled_.store(false, std::memory_order_relaxed);
            try {
                // We create the appropriate number of working threads.
                for (uint idx = 0; idx < thread_count; ++idx) {
                    queues_.push_back(std::make_unique<StealingQueue>());
                    threads_.emplace_back(&ThreadPool::worker_thread, this, idx);
                }
            }
            catch (...) {
                // Or we finish work.
                done_ = true;
                throw;
            }
        }
        ~ThreadPool() {
            done_ = true;
        }
        void cancel() {
            canceled_.store(true, std::memory_order_relaxed);
        }
        bool canceled() {
            return canceled_.load(std::memory_order_relaxed);
        }

        /// Add new task to the pool.
        template<typename Fn>
        std::future<std::invoke_result_t<Fn>>
        add_task(Fn f) {
            using result_type = std::invoke_result_t<Fn>;
            std::packaged_task<result_type()> task(f);
            std::future<result_type> res(task.get_future());

            if (local_work_queue_) {
                printer::print("task added to local queue");
                local_work_queue_->push(std::move(task));
            } else {
                printer::print("task added to pool queue");
                pool_work_queue_.push(std::move(task));
            }
            return res;
        }

        void run_pending_task(uint idx) {
            FunctionWrapper task;
            if (pop_task_from_local_queue(task)) {
                printer::print("started task from local queue: ", idx);
                task();
            } else if (pop_task_from_pool_queue(task)) {
                printer::print("started task from pool queue: ", idx);
                task();
            } else if (pop_task_from_other_thread_queue(task)) {
                printer::print("started task stealed from other queue: ", idx);
                task();
            } else {
                std::this_thread::yield();
            }
        }
    };
}
