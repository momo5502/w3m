#include "../std_include.hpp"

#include "identity.hpp"

#include <utils/nt.hpp>
#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/byte_buffer.hpp>

#include "smbios.hpp"

#include <dpapi.h>
#pragma comment(lib, "Crypt32.lib")

namespace utils::identity
{
    namespace
    {
        std::string get_hdd_serial()
        {
            DWORD serial{};
            if (!GetVolumeInformationA("C:\\", nullptr, 0, &serial, nullptr, nullptr, nullptr, 0))
            {
                return {};
            }

            return utils::string::va("%08X", serial);
        }

        std::string get_hw_profile_guid()
        {
            HW_PROFILE_INFO info;
            if (!GetCurrentHwProfileA(&info))
            {
                return {};
            }

            return std::string{info.szHwProfileGuid, sizeof(info.szHwProfileGuid)};
        }

        std::string get_protected_data()
        {
            std::string input = "momo5502-w3m-auth";

            DATA_BLOB data_in{}, data_out{};
            data_in.pbData = reinterpret_cast<uint8_t*>(input.data());
            data_in.cbData = static_cast<DWORD>(input.size());
            if (CryptProtectData(&data_in, nullptr, nullptr, nullptr, nullptr, CRYPTPROTECT_LOCAL_MACHINE, &data_out) != TRUE)
            {
                return {};
            }

            const auto size = std::min(data_out.cbData, 52ul);
            std::string result{reinterpret_cast<char*>(data_out.pbData), size};
            LocalFree(data_out.pbData);

            return result;
        }

        std::string get_key_entropy()
        {
            std::string entropy{};
            entropy.append(utils::smbios::get_uuid());
            entropy.append(get_hw_profile_guid());
            entropy.append(get_protected_data());
            entropy.append(get_hdd_serial());

            if (entropy.empty())
            {
                entropy.resize(32);
                utils::cryptography::random::get_data(entropy.data(), entropy.size());
            }

            return entropy;
        }

        bool is_second_instance()
        {
            static const auto is_first = [] {
                static utils::nt::handle<> mutex = CreateMutexA(nullptr, FALSE, "w3m_mutex");
                return mutex && GetLastError() != ERROR_ALREADY_EXISTS;
            }();

            return !is_first;
        }
    }

    utils::cryptography::ecc::key& get_key()
    {
        static auto key = utils::cryptography::ecc::generate_key(512, get_key_entropy());
        return key;
    }

    uint64_t get_guid()
    {
        static const auto guid = []() -> uint64_t {
            if (is_second_instance())
            {
                return 0x110000100000000 | (::utils::cryptography::random::get_integer() & ~0x80000000);
            }

            return get_key().get_hash();
        }();

        return guid;
    }
}
