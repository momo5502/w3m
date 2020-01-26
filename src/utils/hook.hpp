#pragma once
#include "signature.hpp"

using namespace asmjit::x86;

namespace utils::hook
{
	class assembler : public Assembler
	{
	public:
		using Assembler::Assembler;
		using Assembler::call;
		using Assembler::jmp;

		void pushad64()
		{
			this->push(rax);
			this->push(rcx);
			this->push(rdx);
			this->push(rbx);
			this->push(rsp);
			this->push(rbp);
			this->push(rsi);
			this->push(rdi);

			this->sub(rsp, 0x40);
		}

		void popad64()
		{
			this->add(rsp, 0x40);

			this->pop(rdi);
			this->pop(rsi);
			this->pop(rbp);
			this->pop(rsp);
			this->pop(rbx);
			this->pop(rdx);
			this->pop(rcx);
			this->pop(rax);
		}

		asmjit::Error call(void* target)
		{
			return Assembler::call(size_t(target));
		}

		asmjit::Error jmp(void* target)
		{
			return Assembler::jmp(size_t(target));
		}
	};

	class detour
	{
	public:
		detour() = default;
		detour(void* place, void* target);
		detour(size_t place, void* target);
		~detour();

		detour(detour&& other) noexcept
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

	void jump(void* pointer, void* data);
	void jump(size_t pointer, void* data);

	void* assemble(const std::function<void(assembler&)>& asm_function);

	template <typename T>
	T extract(void* address)
	{
		const auto data = static_cast<uint8_t*>(address);
		const auto offset = *reinterpret_cast<int32_t*>(data);
		return  reinterpret_cast<T>(data + offset + 4);
	}

	void* follow_branch(void* address);

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