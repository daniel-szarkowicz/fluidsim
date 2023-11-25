#pragma once

#include <functional>

class Context {
    Context() = delete;
public:
    static void init(int window_width, int window_height, const char* title);
    static void loop(std::function<void()> frame);
    // template<typename... T>
    // static void loop(std::function<void(T...)> frame, T... initial_state);
    static void uninit();
};
