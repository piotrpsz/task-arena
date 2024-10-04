//
// Created by Piotr Pszczółkowski on 02/10/2024.
//

#pragma once
#include <iostream>
#include <mutex>

namespace task_arena::printer {
    static std::mutex mtx{};
    template<typename... Args>
    void print(std::string const& text, Args&&... args) {
        std::lock_guard<std::mutex> guard(mtx);
        std::cout << text;
        ((std::cout << std::forward<Args>(args)), ...);
        std::cout << '\n' << std::flush;
    }
}