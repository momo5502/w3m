#include "../std_include.hpp"
#include "loader.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

namespace loader
{
    namespace
    {
        utils::nt::library game_module{};
        utils::nt::library main_module{};

        template <typename T>
        T offset_pointer(void* data, const ptrdiff_t offset)
        {
            return reinterpret_cast<T>(reinterpret_cast<uintptr_t>(data) + offset);
        }

        void load_imports(const utils::nt::library& target, const resolver& import_resolver)
        {
            const auto* const import_directory = &target.get_optional_header()->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];

            const auto* descriptor = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(target.get_ptr() + import_directory->VirtualAddress);

            while (descriptor->Name)
            {
                std::string name = reinterpret_cast<LPSTR>(target.get_ptr() + descriptor->Name);

                const auto* name_table_entry = reinterpret_cast<uintptr_t*>(target.get_ptr() + descriptor->OriginalFirstThunk);
                auto* address_table_entry = reinterpret_cast<uintptr_t*>(target.get_ptr() + descriptor->FirstThunk);

                if (!descriptor->OriginalFirstThunk)
                {
                    name_table_entry = reinterpret_cast<uintptr_t*>(target.get_ptr() + descriptor->FirstThunk);
                }

                while (*name_table_entry)
                {
                    FARPROC function = nullptr;
                    std::string function_name;
                    const char* function_procname;

                    if (IMAGE_SNAP_BY_ORDINAL(*name_table_entry))
                    {
                        function_name = "#" + std::to_string(IMAGE_ORDINAL(*name_table_entry));
                        function_procname = MAKEINTRESOURCEA(IMAGE_ORDINAL(*name_table_entry));
                    }
                    else
                    {
                        const auto* import = reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(target.get_ptr() + *name_table_entry);
                        function_name = import->Name;
                        function_procname = function_name.data();
                    }

                    auto library = utils::nt::library::load(name);
                    if (library)
                    {
                        function = GetProcAddress(library, function_procname);
                    }

                    auto* resolved_function = import_resolver(name, function_name);
                    if (resolved_function)
                    {
                        function = static_cast<FARPROC>(resolved_function);
                    }

                    if (!function)
                    {
                        throw std::runtime_error(
                            utils::string::va("Unable to load import '%s' from library '%s'", function_name.data(), name.data()));
                    }

                    utils::hook::set(address_table_entry, reinterpret_cast<uintptr_t>(function));

                    name_table_entry++;
                    address_table_entry++;
                }

                descriptor++;
            }
        }

        void load_relocations(const utils::nt::library& target)
        {
            if (!utils::nt::is_wine())
            {
                return;
            }

            auto* current_base = target.get_ptr();
            const auto initial_base = target.get_optional_header()->ImageBase;
            const auto delta = reinterpret_cast<ptrdiff_t>(current_base) - initial_base;

            const PIMAGE_DATA_DIRECTORY directory = &target.get_optional_header()->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
            if (directory->Size == 0)
            {
                return;
            }

            auto* relocation = reinterpret_cast<PIMAGE_BASE_RELOCATION>(current_base + directory->VirtualAddress);
            while (relocation->VirtualAddress > 0)
            {
                unsigned char* dest = current_base + relocation->VirtualAddress;

                auto* rel_info = offset_pointer<uint16_t*>(relocation, sizeof(IMAGE_BASE_RELOCATION));
                const auto* rel_info_end =
                    offset_pointer<uint16_t*>(rel_info, static_cast<ptrdiff_t>(relocation->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)));

                for (; rel_info < rel_info_end; ++rel_info)
                {
                    const int type = *rel_info >> 12;
                    const int offset = *rel_info & 0xfff;

                    switch (type)
                    {
                    case IMAGE_REL_BASED_ABSOLUTE:
                        break;

                    case IMAGE_REL_BASED_HIGHLOW: {
                        auto* patch_address = reinterpret_cast<DWORD*>(dest + offset);
                        utils::hook::set(patch_address, *patch_address + static_cast<DWORD>(delta));
                        break;
                    }

                    case IMAGE_REL_BASED_DIR64: {
                        auto* patch_address = reinterpret_cast<ULONGLONG*>(dest + offset);
                        utils::hook::set(patch_address, *patch_address + static_cast<ULONGLONG>(delta));
                        break;
                    }

                    default:
                        throw std::runtime_error("Unknown relocation type: " + std::to_string(type));
                    }
                }

                relocation = offset_pointer<PIMAGE_BASE_RELOCATION>(relocation, relocation->SizeOfBlock);
            }
        }

        void load_tls(const utils::nt::library& source, const utils::nt::library& target)
        {
            if (source.get_optional_header()->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size)
            {
                const auto* const target_tls = reinterpret_cast<PIMAGE_TLS_DIRECTORY>(
                    target.get_ptr() + target.get_optional_header()->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);
                const auto* const source_tls = reinterpret_cast<PIMAGE_TLS_DIRECTORY>(
                    source.get_ptr() + source.get_optional_header()->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);

                auto* target_tls_start = reinterpret_cast<PVOID>(target_tls->StartAddressOfRawData);
                const auto* tls_start = reinterpret_cast<PVOID>(source_tls->StartAddressOfRawData);
                const auto tls_size = source_tls->EndAddressOfRawData - source_tls->StartAddressOfRawData;
                const auto tls_index = *reinterpret_cast<DWORD*>(target_tls->AddressOfIndex);

                utils::hook::set<DWORD>(source_tls->AddressOfIndex, tls_index);

                if (target_tls->AddressOfCallBacks)
                {
                    utils::hook::set<void*>(target_tls->AddressOfCallBacks, nullptr);
                }

                DWORD old_protect;
                VirtualProtect(target_tls_start, tls_size, PAGE_READWRITE, &old_protect);

                auto* const tls_base = *reinterpret_cast<LPVOID*>(__readgsqword(0x58) + 8ull * tls_index);
                std::memmove(tls_base, tls_start, tls_size);
                std::memmove(target_tls_start, tls_start, tls_size);
            }
        }
    }

    utils::nt::library load(const std::filesystem::path& file, const resolver& import_resolver)
    {
        const auto target = utils::nt::library::load(file);
        if (!target)
        {
            throw std::runtime_error{"Failed to map: " + file.string()};
        }

        load_relocations(target);
        load_imports(target, import_resolver);
        load_tls(target, main_module);

        game_module = target;

        return target;
    }

    void apply_main_module(const utils::nt::library& mod)
    {
        auto* const peb = reinterpret_cast<PPEB>(__readgsqword(0x60));
        peb->Reserved3[1] = mod.get_ptr();
        static_assert(offsetof(PEB, Reserved3[1]) == 0x10);
    }

    utils::nt::library get_game_module()
    {
        return game_module;
    }

    utils::nt::library get_main_module()
    {
        return main_module;
    }

    size_t reverse_g(const size_t val)
    {
        static auto base = reinterpret_cast<size_t>(get_game_module().get_ptr());
        assert(base != reinterpret_cast<size_t>(get_main_module().get_ptr()));
        return 0x140000000 + (val - base);
    }

    size_t reverse_g(const void* val)
    {
        return reverse_g(reinterpret_cast<size_t>(val));
    }
}

size_t operator"" _g(const size_t val)
{
    static auto base = reinterpret_cast<size_t>(loader::get_game_module().get_ptr());
    assert(base != reinterpret_cast<size_t>(loader::get_main_module().get_ptr()));
    return base + (val - 0x140000000);
}
