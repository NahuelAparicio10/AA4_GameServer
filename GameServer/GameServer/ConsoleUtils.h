#pragma once
#include <mutex>
#include <iostream>

inline std::mutex consoleMutex;

template<typename... Args>
void WriteConsole(Args&&... args)
{
    std::lock_guard<std::mutex> lock(consoleMutex);
    (std::cout << ... << args) << '\n';
}