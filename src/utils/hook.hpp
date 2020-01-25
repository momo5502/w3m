#pragma once
#include "signature.hpp"

using namespace asmjit::x86;

namespace utils::hook
{
	class assembler : public asmjit::x86::Assembler
	{
	public:
		using asmjit::x86::Assembler::Assembler;
		using asmjit::x86::Assembler::call;
		using asmjit::x86::Assembler::jmp;

		void pushad()
		{
			this->push(asmjit::x86::rax);
			this->push(asmjit::x86::rcx);
			this->push(asmjit::x86::rdx);
			this->push(asmjit::x86::rbx);
			this->push(asmjit::x86::rsp);
			this->push(asmjit::x86::rbp);
			this->push(asmjit::x86::rsi);
			this->push(asmjit::x86::rdi);

			this->sub(asmjit::x86::rsp, 0x40);
		}

		void popad()
		{
			this->add(asmjit::x86::rsp, 0x40);

			this->pop(asmjit::x86::rdi);
			this->pop(asmjit::x86::rsi);
			this->pop(asmjit::x86::rbp);
			this->pop(asmjit::x86::rsp);
			this->pop(asmjit::x86::rbx);
			this->pop(asmjit::x86::rdx);
			this->pop(asmjit::x86::rcx);
			this->pop(asmjit::x86::rax);
		}

		asmjit::Error call(void* target)
		{
			return asmjit::x86::Assembler::call(size_t(target));
		}

		asmjit::Error jmp(void* target)
		{
			return asmjit::x86::Assembler::jmp(size_t(target));
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

	void* follow_branch(void* address);
	void* extract(void* address);

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