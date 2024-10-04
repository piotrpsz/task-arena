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
#include <memory>
#include <mutex>
#include <condition_variable>
#include "join_threads.h"

namespace task_arena {
    template<typename T>
    class SafeQueue {
    private:
        struct node {
            std::shared_ptr<T> data;
            std::unique_ptr<node> next;
        };
        node *tail_{};
        std::unique_ptr<node> head_;
        std::mutex head_mutex_;
        std::mutex tail_mutex_;
        std::condition_variable data_cond_;

        node *get_tail() {
            std::lock_guard<std::mutex> tail_lock(tail_mutex_);
            return tail_;
        }

        std::unique_ptr<node> pop_head() {
            std::unique_ptr<node> old_head = std::move(head_);
            head_ = std::move(old_head->next);
            return old_head;
        }

        std::unique_lock<std::mutex> wait_for_data() {
            std::unique_lock<std::mutex> head_lock(head_mutex_);
            data_cond_.wait(head_lock, [&] {
                return head_ != get_tail();
            });
            return head_lock;
        }

        std::unique_ptr<node> wait_pop_head() {
            std::unique_lock<std::mutex> head_lock(wait_for_data());
            return pop_head();
        }

        std::unique_ptr<node> wait_pop_head(T& value) {
            std::unique_lock<std::mutex> head_lock(wait_for_data());
            value = std::move(*head_->data);
            return pop_head();
        }

        std::unique_ptr<node> try_pop_head() {
            std::lock_guard<std::mutex> head_lock(head_mutex_);
            if (head_.get() == get_tail())
                return std::unique_ptr<node>();
            return pop_head();
        }

        std::unique_ptr<node> try_pop_head(T& value) {
            std::lock_guard<std::mutex> head_lock(head_mutex_);
            if (head_.get() == get_tail())
                return std::unique_ptr<node>();
            value = std::move(*head_->data);
            return pop_head();
        }

    public:
        SafeQueue() : head_(new node) {
            tail_ = head_.get();
        }
        // copy disabled
        SafeQueue(const SafeQueue& other) = delete;
        SafeQueue& operator=(const SafeQueue& other) = delete;

        std::shared_ptr<T> try_pop() {
            std::unique_ptr<node> const old_head = try_pop_head();
            return old_head ? old_head->data : std::shared_ptr<T>();
        }
        bool try_pop(T& value) {
            std::unique_ptr<node> const old_head = try_pop_head(value);
            return old_head.get();
        }
        std::shared_ptr<T> wait_and_pop() {
            std::unique_ptr<node> const old_head = wait_pop_head();
            return old_head->data;
        }
        void wait_and_pop(T& value) {
            std::unique_ptr<node> const old_head = wait_pop_head(value);
        }
        void push(T new_value) {
            std::shared_ptr<T> new_data(std::make_shared<T>(std::move(new_value)));
            std::unique_ptr<node> p(new node);
            {
                std::lock_guard<std::mutex> tail_lock(tail_mutex_);
                tail_->data = new_data;
                node *const new_tail = p.get();
                tail_->next = std::move(p);
                tail_ = new_tail;
            }
            data_cond_.notify_one();
        }
        bool empty() {
            std::lock_guard<std::mutex> head_lock(head_mutex_);
            return (head_ == get_tail());
        }
    };
}
