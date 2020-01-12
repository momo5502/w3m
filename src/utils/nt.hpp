#pragma once

namespace utils::nt
{
	class module final
	{
	public:
		static module load(const std::string& name);
		static module load(const std::filesystem::path& path);
		static module get_by_address(void* address);

		module();
		explicit module(const std::string& name);
		explicit module(HMODULE handle);

		module(const module& a) : module_(a.module_)
		{
		}

		bool operator!=(const module& obj) const { return !(*this == obj); };
		bool operator==(const module& obj) const;

		operator bool() const;
		operator HMODULE() const;

		void unprotect() const;
		void* get_entry_point() const;
		size_t get_relative_entry_point() const;

		bool is_valid() const;
		std::string get_name() const;
		std::string get_path() const;
		std::string get_folder() const;
		std::uint8_t* get_ptr() const;
		void free();

		HMODULE get_handle() const;

		template <typename T>
		T get_proc(const std::string& process) const
		{
			if (!this->is_valid()) T{};
			return reinterpret_cast<T>(GetProcAddress(this->module_, process.data()));
		}

		template <typename T>
		std::function<T> get(const std::string& process) const
		{
			if (!this->is_valid()) std::function<T>();
			return reinterpret_cast<T*>(this->get_proc<void*>(process));
		}

		template <typename T, typename... Args>
		T invoke(const std::string& process, Args ... args) const
		{
			auto method = this->get<T(__cdecl)(Args ...)>(process);
			if (method) return method(args...);
			return T();
		}

		template <typename T, typename... Args>
		T invoke_pascal(const std::string& process, Args ... args) const
		{
			auto method = this->get<T(__stdcall)(Args ...)>(process);
			if (method) return method(args...);
			return T();
		}

		template <typename T, typename... Args>
		T invoke_this(const std::string& process, void* this_ptr, Args ... args) const
		{
			auto method = this->get<T(__thiscall)(void*, Args ...)>(this_ptr, process);
			if (method) return method(args...);
			return T();
		}

		std::vector<PIMAGE_SECTION_HEADER> get_section_headers() const;

		PIMAGE_NT_HEADERS get_nt_headers() const;
		PIMAGE_DOS_HEADER get_dos_header() const;
		PIMAGE_OPTIONAL_HEADER get_optional_header() const;

		void** get_iat_entry(const std::string& module_name, const std::string& proc_name) const;

	private:
		HMODULE module_;
	};

	void raise_hard_exception();
}
