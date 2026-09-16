#pragma once

#include "non_copyable.hpp"
#include "logger.h"

#include <algorithm>
#include <ranges>
#include <atomic>
#include <filesystem>
#include <functional>
#include <unordered_set>

struct CaseInsensitiveEqual
{
    bool operator()(std::string const& lhs, std::string const& rhs) const noexcept
    {
        return std::ranges::equal(lhs, rhs,
            [](unsigned char a, unsigned char b)
            {
                return std::tolower(a) == std::tolower(b);
            });
    }
};

class FileScanner : public NonCopyable
{
public:
	using Callback = std::function<void(const std::filesystem::path&)>;
	using MasksType = std::unordered_set<std::string, std::hash<std::string>, CaseInsensitiveEqual>;

public:
	FileScanner(std::filesystem::path const& path, MasksType masks, ILogger& logger);

	void scan(Callback callback);
	void stop() noexcept;

private:
	std::filesystem::path const m_path;
	const MasksType m_masks;
	ILogger& m_logger;
	std::atomic_bool m_stopped = false;
};