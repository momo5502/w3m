#include <std_include.hpp>
#include "hook.hpp"
#include "string.hpp"

namespace utils::hook
{
	namespace
	{
		class _
		{
		public:
			_()
			{
				if(MH_Initialize() != MH_OK)
				{
					throw std::runtime_error("Failed to initialize MinHook");
				}
			}

			~_()
			{
				MH_Uninitialize();
			}
		} __;
	}

	void signature::process()
	{
		if (this->signatures_.empty()) return;

		const auto start = reinterpret_cast<char*>(this->start_);

		const auto sig_count = this->signatures_.size();
		const auto containers = this->signatures_.data();

		for (size_t i = 0; i < this->length_; ++i)
		{
			const auto address = start + i;

			for (unsigned int k = 0; k < sig_count; ++k)
			{
				const auto container = &containers[k];

				unsigned int j;
				for (j = 0; j < strlen(container->mask); ++j)
				{
					if (container->mask[j] != '?' && container->signature[j] != address[j])
					{
						break;
					}
				}

				if (j == strlen(container->mask))
				{
					container->callback(address);
				}
			}
		}
	}

	void signature::add(const container& container)
	{
		signatures_.push_back(container);
	}

	detour::detour(const size_t place, void* target) : detour(reinterpret_cast<void*>(place), target)
	{

	}

	detour::detour(void* place, void* target) : place_(place)
	{
		if (MH_CreateHook(this->place_, target, &this->original_) != MH_OK)
		{
			throw std::runtime_error(string::va("Unable to create hook at location: %p", this->place_));
		}

		this->enable();
	}

	detour::~detour()
	{
		if (this->place_)
		{
			MH_RemoveHook(this->place_);
		}

		this->place_ = nullptr;
		this->original_ = nullptr;
	}

	void detour::enable() const
	{
		MH_EnableHook(this->place_);
	}

	void detour::disable() const
	{
		MH_DisableHook(this->place_);
	}

	void* detour::get_original() const
	{
		return this->original_;
	}

	void nop(void* place, const size_t length)
	{
		DWORD old_protect;
		VirtualProtect(place, length, PAGE_EXECUTE_READWRITE, &old_protect);

		std::memset(place, 0x90, length);

		VirtualProtect(place, length, old_protect, &old_protect);
		FlushInstructionCache(GetCurrentProcess(), place, length);
	}

	void nop(const size_t place, const size_t length)
	{
		nop(reinterpret_cast<void*>(place), length);
	}

	void copy(void* place, const void* data, const size_t length)
	{
		DWORD old_protect;
		VirtualProtect(place, length, PAGE_EXECUTE_READWRITE, &old_protect);

		std::memmove(place, data, length);

		VirtualProtect(place, length, old_protect, &old_protect);
		FlushInstructionCache(GetCurrentProcess(), place, length);
	}

	void copy(const size_t place, const void* data, const size_t length)
	{
		copy(reinterpret_cast<void*>(place), data, length);
	}

	void jump(const size_t pointer, void* data)
	{
		static const unsigned char jump_data[] = { 0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0xff, 0xe0 };

		auto* patch_pointer = PBYTE(pointer);

		DWORD old_protect;
		VirtualProtect(patch_pointer, sizeof(jump_data), PAGE_EXECUTE_READWRITE, &old_protect);

		std::memmove(patch_pointer, jump_data, sizeof(jump_data));
		std::memmove(patch_pointer + 2, &data, sizeof(data));

		VirtualProtect(patch_pointer, sizeof(jump_data), old_protect, &old_protect);
	}
}
