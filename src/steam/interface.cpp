#include <std_include.hpp>
#include "interface.hpp"
#include "utils/memory.hpp"
#include "utils/nt.hpp"

namespace steam
{
	interface::interface() : interface(nullptr)
	{
	}

	interface::interface(void* interface_ptr) : interface_ptr_(static_cast<void***>(interface_ptr))
	{
	}

	interface::operator bool() const
	{
		return this->interface_ptr_ != nullptr;
	}

	void* interface::find_method(const std::string& name)
	{
		const auto method_entry = this->methods_.find(name);
		if (method_entry != this->methods_.end())
		{
			return method_entry->second;
		}

		return this->search_method(name);
	}

	void* interface::search_method(const std::string& name)
	{
		if (!utils::memory::is_bad_read_ptr(this->interface_ptr_))
		{
			auto vftbl = *this->interface_ptr_;

			while (!utils::memory::is_bad_read_ptr(vftbl) && !utils::memory::is_bad_code_ptr(*vftbl))
			{
				const auto ptr = *vftbl;
				const auto result = this->analyze_method(ptr);
				if (!result.empty())
				{
					this->methods_[result] = ptr;

					if (result == name)
					{
						return ptr;
					}
				}

				++vftbl;
			}
		}

		return {};
	}

	std::string interface::analyze_method(const void* method_ptr)
	{
		if (utils::memory::is_bad_code_ptr(method_ptr)) return {};

		ud_t ud;
		ud_init(&ud);
		ud_set_mode(&ud, 64);
		ud_set_pc(&ud, uint64_t(method_ptr));
		ud_set_input_buffer(&ud, static_cast<const uint8_t*>(method_ptr), INT32_MAX);

		while (true)
		{
			ud_disassemble(&ud);

			if (ud_insn_mnemonic(&ud) == UD_Iret)
			{
				break;
			}

			if (ud_insn_mnemonic(&ud) == UD_Ilea)
			{
				const auto* operand = ud_insn_opr(&ud, 1);
				if (operand && operand->type == UD_OP_MEM && operand->base == UD_R_RIP)
				{
					auto* operand_ptr = reinterpret_cast<char*>(ud_insn_len(&ud) + ud_insn_off(&ud) + operand->lval.sdword);
					if (!utils::memory::is_bad_read_ptr(operand_ptr) && this->is_rdata(operand_ptr))
					{
						return operand_ptr;
					}

				}
			}

			if (*reinterpret_cast<unsigned char*>(ud.pc) == 0xCC) break; // int 3
		}

		return {};
	}

	bool interface::is_rdata(void* pointer)
	{
		const auto pointer_lib = utils::nt::library::get_by_address(pointer);

		for (const auto& section : pointer_lib.get_section_headers())
		{
			const auto size = sizeof(section->Name);
			char name[size + 1];
			name[size] = 0;
			std::memcpy(name, section->Name, size);

			if (name == ".rdata"s)
			{
				const auto target = size_t(pointer);
				const size_t source_start = size_t(pointer_lib.get_ptr()) + section->PointerToRawData;
				const size_t source_end = source_start + section->SizeOfRawData;

				return target >= source_start && target <= source_end;
			}
		}

		return false;
	}
}
