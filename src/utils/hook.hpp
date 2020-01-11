#pragma once

namespace utils::hook
{
	class signature final
	{
	public:
		struct container final
		{
			const char* signature;
			const char* mask;
			std::function<void(char*)> callback;
		};

		signature(void* start, const size_t length) : start_(start), length_(length)
		{
		}

		signature(const size_t start, const size_t length) : signature(reinterpret_cast<void*>(start), length)
		{
		}

		signature() : signature(0x400000, 0x800000)
		{
		}

		void process();
		void add(const container& container);

	private:
		void* start_;
		size_t length_;
		std::vector<container> signatures_;
	};

	void nop(void* place, size_t length);
	void nop(size_t place, size_t length);

	void jump(const size_t pointer, void* data);

	template <typename T>
	static void set(void* place, T value)
	{
		DWORD old_protect;
		VirtualProtect(place, sizeof(T), PAGE_EXECUTE_READWRITE, &old_protect);

		*static_cast<T*>(place) = value;

		VirtualProtect(place, sizeof(T), old_protect, &old_protect);
		FlushInstructionCache(GetCurrentProcess(), place, sizeof(T));
	}

	template <typename T>
	static void set(const size_t place, T value)
	{
		return set<T>(reinterpret_cast<void*>(place), value);
	}
}