#include "io.hpp"
#include "nt.hpp"
#include <fstream>

namespace utils::io
{
	bool remove_file(const std::filesystem::path& file)
	{
		std::error_code ec{};
		return std::filesystem::remove(file, ec) && !ec;
	}

	bool move_file(const std::filesystem::path& src, const std::filesystem::path& target)
	{
#ifdef _WIN32
		return MoveFileW(src.wstring().data(), target.wstring().data()) == TRUE;
#else
		copy_folder(src, target);
		return remove_file(src);
#endif
	}

	bool file_exists(const std::filesystem::path& file)
	{
		return std::ifstream(file).good();
	}

	bool write_file(const std::filesystem::path& file, const std::string& data, const bool append)
	{
		if (file.has_parent_path())
		{
			io::create_directory(file.parent_path());
		}

		std::ofstream stream(
			file, std::ios::binary | std::ofstream::out | (append ? std::ofstream::app : std::ofstream::out));

		if (stream.is_open())
		{
			stream.write(data.data(), static_cast<std::streamsize>(data.size()));
			stream.close();
			return true;
		}

		return false;
	}

	std::string read_file(const std::filesystem::path& file)
	{
		std::string data;
		read_file(file, &data);
		return data;
	}

	bool read_file(const std::filesystem::path& file, std::string* data)
	{
		if (!data) return false;
		data->clear();

		std::ifstream stream(file, std::ios::binary);
		if (!stream) return false;

		stream.seekg(0, std::ios::end);
		const std::streamsize size = stream.tellg();
		stream.seekg(0, std::ios::beg);

		if (size > -1)
		{
			data->resize(static_cast<std::uint32_t>(size));
			stream.read(data->data(), size);
			stream.close();
			return true;
		}

		return false;
	}

	std::size_t file_size(const std::filesystem::path& file)
	{
		std::ifstream stream(file, std::ios::binary);

		if (stream)
		{
			stream.seekg(0, std::ios::end);
			return static_cast<std::size_t>(stream.tellg());
		}

		return 0;
	}

	bool create_directory(const std::filesystem::path& directory)
	{
		std::error_code ec{};
		return std::filesystem::create_directories(directory, ec) && !ec;
	}

	bool directory_exists(const std::filesystem::path& directory)
	{
		std::error_code ec{};
		return std::filesystem::is_directory(directory, ec) && !ec;
	}

	bool directory_is_empty(const std::filesystem::path& directory)
	{
		std::error_code ec{};
		return std::filesystem::is_empty(directory, ec) && !ec;
	}

	void copy_folder(const std::filesystem::path& src, const std::filesystem::path& target)
	{
		std::error_code ec{};
		std::filesystem::copy(src, target,
		                      std::filesystem::copy_options::overwrite_existing |
		                      std::filesystem::copy_options::recursive, ec);
	}

	std::vector<std::filesystem::path> list_files(const std::filesystem::path& directory, const bool recursive)
	{
		std::error_code code{};
		std::vector<std::filesystem::path> files;

		if (recursive)
		{
			for (auto& file : std::filesystem::recursive_directory_iterator(directory, code))
			{
				files.push_back(file.path());
			}
		}
		else
		{
			for (auto& file : std::filesystem::directory_iterator(directory, code))
			{
				files.push_back(file.path());
			}
		}

		return files;
	}
}
