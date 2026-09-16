#pragma once

#include "non_copyable.hpp"

#include <algorithm>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <span>
#include <stdexcept>
#include <utility>
#include <mutex>


template <typename T>
struct PodTypePolicy
{
    std::size_t operator()(T const& value) const noexcept
    {
        return sizeof(value);
    }
};


template <typename T, typename PolicyType>
class ByteLimitedMPMCQueue : public NonCopyable
{
public:
    explicit ByteLimitedMPMCQueue(std::size_t max_bytes):
        m_max_bytes(max_bytes),
        m_size_plocy()
    {
        if (max_bytes == 0)
        {
            throw std::invalid_argument("max_bytes must not be 0");
        }
    }

    bool push(std::span<T> values)
    {
        if (values.empty())
        {
            return true;
        }

        const auto batch_bytes = calculate_bytes(values);

        if (batch_bytes > m_max_bytes)
        {
            throw std::invalid_argument("batch size exceeds queue capacity");
        }

        {
            std::unique_lock lock(m_mutex);

            m_not_full.wait(lock, [this, batch_bytes]
                {
                    return m_stopped || m_max_bytes - m_size_bytes >= batch_bytes;
                });

            if (m_stopped)
            {
                return false;
            }

            for (auto& value : values)
            {
                m_queue.push_back(std::move(value));
            }

            m_size_bytes += batch_bytes;
        }

        m_not_empty.notify_all();

        return true;
    }

    std::size_t pop(std::span<T> values)
    {
        if (values.empty())
        {
            return 0;
        }

        std::size_t count = 0;

        {
            std::unique_lock lock(m_mutex);

            m_not_empty.wait(lock, [this]
                {
                    return m_stopped || !m_queue.empty();
                });

            if (m_stopped && m_queue.empty())
            {
                return 0;
            }

            count = std::min(values.size(), m_queue.size());

            for (auto& value : values.first(count))
            {
                const auto bytes = m_size_plocy(m_queue.front());

                value = std::move(m_queue.front());

                m_size_bytes -= bytes;

                m_queue.pop_front();
            }
        }

        m_not_full.notify_all();

        return count;
    }

    void stop()
    {
        {
            std::lock_guard lock(m_mutex);
            m_stopped = true;
        }

        m_not_full.notify_all();
        m_not_empty.notify_all();
    }

    std::size_t size() const
    {
        std::lock_guard lock(m_mutex);

        return m_queue.size();
    }

    std::size_t size_bytes() const
    {
        std::lock_guard lock(m_mutex);

        return m_size_bytes;
    }

    std::size_t capacity_bytes() const noexcept
    {
        return m_max_bytes;
    }

private:
    std::size_t calculate_bytes(std::span<T const> values) const
    {
        std::size_t result = 0;

        for (auto const& value : values)
        {
            result += m_size_plocy(value);
        }

        return result;
    }

private:
    std::deque<T> m_queue;
    PolicyType m_size_plocy;

    // Maximum total payload size.
    const std::size_t m_max_bytes;

    // Current total payload size.
    std::size_t m_size_bytes = 0;

    bool m_stopped = false;

    std::condition_variable m_not_empty;
    std::condition_variable m_not_full;

    mutable std::mutex m_mutex;
};