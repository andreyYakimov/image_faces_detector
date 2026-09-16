#pragma once

#include "non_copyable.hpp"

#include <thread>

class Runnable : public NonCopyable
{
public:
    Runnable() = default;
    virtual ~Runnable();

    void start();
    void join();

protected:
    virtual void run() = 0;

private:
    std::thread m_thread;
};