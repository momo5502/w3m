#include "string.hpp"

#include <algorithm>
#include <cassert>
#include <cstdarg>
#include <sstream>
#include <array>

#include "nt.hpp"
#include "finally.hpp"

#undef max

namespace utils::string
{
	const char* va(const char* format, ...)
	{
		constexpr auto buffer_count = 4;
		thread_local std::array<std::vector<char>, buffer_count> buffers{};
		thread_local size_t current_index{0};

		const auto index = current_index++;
		current_index %= buffers.size();

		auto& buffer = buffers.at(index);

		if (buffer.size() < 10)
		{
			buffer.resize(10);
		}

		while (true)
		{
			va_list ap{};
			va_start(ap, format);

#ifdef _WIN32
			const int res = vsnprintf_s(buffer.data(), buffer.size(), _TRUNCATE, format, ap);
#else
			const int res = vsnprintf(buffer.data(), buffer.size(), format, ap);
#endif

			va_end(ap);

			if (res > 0 && static_cast<size_t>(res) < buffer.size())
			{
				break;
			}
			if (res == 0)
			{
				return nullptr;
			}

			buffer.resize(std::max(buffer.size() * 2, static_cast<size_t>(1)));
		}

		return buffer.data();
	}

	std::vector<std::string> split(const std::string& s, const char delim)
	{
		std::stringstream ss(s);
		std::string item;
		std::vector<std::string> elems;

		while (std::getline(ss, item, delim))
		{
			elems.push_back(item); // elems.push_back(std::move(item)); // if C++11 (based on comment from @mchiasson)
		}

		return elems;
	}

	std::string to_lower(std::string text)
	{
		std::ranges::transform(text, text.begin(), [](const unsigned char input)
		{
			return static_cast<char>(std::tolower(input));
		});

		return text;
	}

	std::string to_upper(std::string text)
	{
		std::ranges::transform(text, text.begin(), [](const unsigned char input)
		{
			return static_cast<char>(std::toupper(input));
		});

		return text;
	}

	std::wstring to_lower(std::wstring text)
	{
		std::ranges::transform(text, text.begin(), [](const wchar_t input)
		{
			if (static_cast<wchar_t>(static_cast<uint8_t>(input)) != input)
			{
				return input;
			}

			return static_cast<wchar_t>(static_cast<uint8_t>(std::tolower(input)));
		});

		return text;
	}

	std::wstring to_upper(std::wstring text)
	{
		std::ranges::transform(text, text.begin(), [](const wchar_t input)
		{
			if (static_cast<wchar_t>(static_cast<uint8_t>(input)) != input)
			{
				return input;
			}

			return static_cast<wchar_t>(static_cast<uint8_t>(std::toupper(input)));
		});

		return text;
	}

	bool starts_with(const std::string& text, const std::string& substring)
	{
		return text.find(substring) == 0;
	}

	bool ends_with(const std::string& text, const std::string& substring)
	{
		if (substring.size() > text.size()) return false;
		return std::equal(substring.rbegin(), substring.rend(), text.rbegin());
	}

	bool is_numeric(const std::string& text)
	{
		auto it = text.begin();
		while (it != text.end() && std::isdigit(static_cast<unsigned char>(*it)))
		{
			++it;
		}

		return !text.empty() && it == text.end();
	}

	std::string dump_hex(const std::string& data, const std::string& separator)
	{
		std::string result;

		for (unsigned int i = 0; i < data.size(); ++i)
		{
			if (i > 0)
			{
				result.append(separator);
			}

			result.append(va("%02X", data[i] & 0xFF));
		}

		return result;
	}

#ifdef _WIN32
	std::string get_clipboard_data()
	{
		bool clipboard_is_open = false;
		const auto _1 = finally([&]
		{
			if (clipboard_is_open)
			{
				CloseClipboard();
			}
		});

		clipboard_is_open = OpenClipboard(nullptr);

		if (!clipboard_is_open)
		{
			return {};
		}

		auto* const clipboard_data = GetClipboardData(CF_TEXT);
		if (!clipboard_data)
		{
			return {};
		}

		const auto* const cliptext = static_cast<char*>(GlobalLock(clipboard_data));
		const auto _2 = finally([&]
		{
			if (cliptext)
			{
				GlobalUnlock(clipboard_data);
			}
		});

		if (!cliptext)
		{
			return {};
		}

		return std::string(cliptext);
	}
#endif

	std::string convert(const std::wstring& wstr)
	{
		std::string result;
		result.reserve(wstr.size());

		for (const auto& chr : wstr)
		{
			result.push_back(static_cast<char>(chr));
		}

		return result;
	}

	std::wstring convert(const std::string& str)
	{
		std::wstring result;
		result.reserve(str.size());

		for (const auto& chr : str)
		{
			result.push_back(static_cast<wchar_t>(chr));
		}

		return result;
	}

	std::string replace(std::string str, const std::string& from, const std::string& to)
	{
		if (from.empty())
		{
			return str;
		}

		size_t start_pos = 0;
		while ((start_pos = str.find(from, start_pos)) != std::string::npos)
		{
			str.replace(start_pos, from.length(), to);
			start_pos += to.length();
		}

		return str;
	}

	std::string& ltrim(std::string& str)
	{
		str.erase(str.begin(), std::ranges::find_if(str, [](const unsigned char input)
		{
			return !std::isspace(input);
		}));

		return str;
	}

	std::string& rtrim(std::string& str)
	{
		str.erase(std::find_if(str.rbegin(), str.rend(), [](const unsigned char input)
		{
			return !std::isspace(input);
		}).base(), str.end());

		return str;
	}

	void trim(std::string& str)
	{
		ltrim(rtrim(str));
	}

	void copy(char* dest, const size_t max_size, const char* src)
	{
		if (!max_size)
		{
			return;
		}

		for (size_t i = 0;; ++i)
		{
			if (i + 1 == max_size)
			{
				dest[i] = 0;
				break;
			}

			dest[i] = src[i];

			if (!src[i])
			{
				break;
			}
		}
	}
}
