#pragma once
#include "signature.hpp"

namespace utils::hook
{
	class detour
	{
	public:
		detour() = default;
		detour(void* place, void* target);
		detour(size_t place, void* target);
		~detour();

		detour(detour&& other)
		{
			this->operator=(std::move(other));
		}
		
		detour& operator= (detour&& other) noexcept
		{
			if(this != &other)
			{
				this->~detour();
				
				this->place_ = other.place_;
				this->original_ = other.original_;

				other.place_ = nullptr;
				other.original_ = nullptr;
			}

			return *this;
		}

		detour(const detour&) = delete;
		detour& operator= (const detour&) = delete;

		void enable() const;
		void disable() const;

		template <typename T>
		T* get() const
		{
			return reinterpret_cast<T*>(this->get_original());
		}

		template <typename T, typename... Args>
		T invoke(Args... args)
		{
			return reinterpret_cast<T(*)(Args ...)>(this->get_original())(args...);
		}

		[[nodiscard]] void* get_original() const;

	private:
		void* place_{};
		void* original_{};
	};

	void nop(void* place, size_t length);
	void nop(size_t place, size_t length);

	void copy(void* place, const void* data, size_t length);
	void copy(size_t place, const void* data, size_t length);

	void jump(const size_t pointer, void* data);

	void* assembler(const std::function<void(asmjit::x86::Assembler&)>& asm_function);

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

	template <typename T, typename... Args>
	static T invoke(size_t func, Args... args)
	{
		return reinterpret_cast<T(*)(Args ...)>(func)(args...);
	}

	template <typename T, typename... Args>
	static T invoke(void* func, Args... args)
	{
		return reinterpret_cast<T(*)(Args ...)>(func)(args...);
	}
}