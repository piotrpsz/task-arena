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
// Created by Piotr Pszczółkowski on 04/10/2024.
//
#pragma once

/*------- include files:
-------------------------------------------------------------------*/
#include <memory>

/// Moveable wrapper for functions.
class FunctionWrapper {
    struct ImplBase {
        virtual void call() = 0;
        virtual ~ImplBase() = default;
    };
    std::unique_ptr<ImplBase> impl_;

    template<typename Fn>
    struct ImplType: ImplBase {
        Fn f;
        explicit ImplType(Fn&& f_): f(std::move(f_)) {}
        void call() override {
            f();
        }
    };
public:
    FunctionWrapper() = default;

    // Copying is disabled.
    FunctionWrapper(FunctionWrapper const&) = delete;
    FunctionWrapper& operator=(FunctionWrapper const&&) = delete;
    FunctionWrapper(FunctionWrapper&) = delete;

    // Only move.
    template<typename Fn>
    FunctionWrapper(Fn&& f): impl_{new ImplType<Fn>(std::forward<Fn>(f))}
    {}
    FunctionWrapper(FunctionWrapper&& rhs)  noexcept : impl_{std::move(rhs.impl_)}
    {}
    FunctionWrapper& operator=(FunctionWrapper&& rhs) noexcept{
        impl_ = std::move(rhs.impl_);
        return *this;
    }
    void operator()() { impl_->call(); }
};
