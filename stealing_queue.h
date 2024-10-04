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
// Created by Piotr Pszczółkowski on 02/10/2024.
//
#pragma once

/*------- include files:
-------------------------------------------------------------------*/
#include "join_threads.h"
#include "function_wrapper.h"
#include <deque>
#include <mutex>

namespace task_arena {
    class StealingQueue {
        using DataType = FunctionWrapper;
        std::deque<DataType> queue_;
        mutable std::mutex mutex_;
    public:
        StealingQueue() = default;
        ~StealingQueue() = default;

        // Copy disabled
        StealingQueue(StealingQueue const&) = delete;
        StealingQueue& operator=(StealingQueue const&) = delete;

        void push(DataType data) {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push_front(std::move(data));
        }

        bool empty() const {
            std::lock_guard<std::mutex> lock(mutex_);
            return queue_.empty();
        }

        bool try_pop(DataType& res) {
            std::lock_guard<std::mutex> lock(mutex_);
            if (queue_.empty())
                return false;

            res = std::move(queue_.front());    // front
            queue_.pop_front();                 // front
            return true;
        }

        bool try_steal(DataType& res) {
            std::lock_guard<std::mutex> lock(mutex_);
            if (queue_.empty())
                return false;

            res = std::move(queue_.back());     // back
            queue_.pop_back();                  // back
            return true;
        }
    };
}
