#pragma once

#include <string>
#include <optional>
#include <future>

namespace utils::http
{
	using headers = std::unordered_map<std::string, std::string>;

	std::optional<std::string> post_data(const std::string& url, const std::string& post_body,
	                                     const headers& headers = {}, const std::function<void(size_t)>& callback = {},
	                                     uint32_t retries = 2);
	std::future<std::optional<std::string>> post_data_async(const std::string& url, const std::string& post_body, const headers& headers = {});

	std::optional<std::string> get_data(const std::string& url, const headers& headers = {},
	                                    const std::function<void(size_t)>& callback = {}, uint32_t retries = 2);
	std::future<std::optional<std::string>> get_data_async(const std::string& url, const headers& headers = {});
}
