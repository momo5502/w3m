#pragma once

#include "memory.hpp"

#include <array>

template <class Type, size_t n>
constexpr auto ARRAY_COUNT(Type (&)[n]) { return n; }

namespace utils::string
{
	const char* va(const char* format, ...);

	std::vector<std::string> split(const std::string& s, char delim);

	std::string to_lower(std::string text);
	std::string to_upper(std::string text);

	std::wstring to_lower(std::wstring text);
	std::wstring to_upper(std::wstring text);

	bool starts_with(const std::string& text, const std::string& substring);
	bool ends_with(const std::string& text, const std::string& substring);

	bool is_numeric(const std::string& text);

	std::string dump_hex(const std::string& data, const std::string& separator = " ");

#ifdef _WIN32
	std::string get_clipboard_data();
#endif

	std::string convert(const std::wstring& wstr);
	std::wstring convert(const std::string& str);

	std::string replace(std::string str, const std::string& from, const std::string& to);

	void trim(std::string& str);

	void copy(char* dest, size_t max_size, const char* src);

	template <size_t Size>
	void copy(char (&dest)[Size], const char* src)
	{
		copy(dest, Size, src);
	}

	template <size_t Size>
	void copy(std::array<char, Size>& dest, const char* src)
	{
		copy(dest.data(), dest.size(), src);
	}
}
