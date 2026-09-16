#include "runnable.h"

#include <stdexcept>

void Runnable::start()
{
    if (m_thread.joinable())
    {
        throw std::logic_error("Runnable already started");
    }

    m_thread = std::thread([this]()
    {
        run();
    });
}

void Runnable::join()
{
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

Runnable::~Runnable()
{
	join();
}