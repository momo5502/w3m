#include "std_include.hpp"
#include "console.hpp"

#include <utils/finally.hpp>

#ifdef _WIN32
#define COLOR_LOG_INFO  11 // 15
#define COLOR_LOG_WARN  14
#define COLOR_LOG_ERROR 12
#define COLOR_LOG_DEBUG 15 // 7
#else
#define COLOR_LOG_INFO  "\033[0;36m"
#define COLOR_LOG_WARN  "\033[0;33m"
#define COLOR_LOG_ERROR "\033[0;31m"
#define COLOR_LOG_DEBUG "\033[0m"
#endif

namespace console
{
    namespace
    {
        constinit std::atomic_bool signal_installed{false};

        std::function<void()>& get_signal_callback()
        {
            static std::function<void()> signal_callback{};
            return signal_callback;
        }

#ifdef _WIN32
#define COLOR(win, posix) win
        using color_type = WORD;
#else
#define COLOR(win, posix) posix
        using color_type = const char*;
#endif

        const color_type color_array[] = {
            COLOR(0x8, "\033[0;90m"), // 0 - black
            COLOR(0xC, "\033[0;91m"), // 1 - red
            COLOR(0xA, "\033[0;92m"), // 2 - green
            COLOR(0xE, "\033[0;93m"), // 3 - yellow
            COLOR(0x9, "\033[0;94m"), // 4 - blue
            COLOR(0xB, "\033[0;96m"), // 5 - cyan
            COLOR(0xD, "\033[0;95m"), // 6 - pink
            COLOR(0xF, "\033[0;97m"), // 7 - white
        };

#ifdef _WIN32
        BOOL WINAPI handler(const DWORD signal)
        {
            const auto& signal_callback = get_signal_callback();

            if (signal == CTRL_C_EVENT && signal_callback)
            {
                signal_callback();
            }

            return TRUE;
        }

#else
        void handler(int signal)
        {
            const auto& signal_callback = get_signal_callback();

            if (signal == SIGINT && signal_callback)
            {
                signal_callback();
            }
        }
#endif

        std::string format(va_list* ap, const char* message)
        {
            thread_local char buffer[0x1000];

#ifdef _WIN32
            const int count = _vsnprintf_s(buffer, sizeof(buffer), sizeof(buffer), message, *ap);
#else
            const int count = vsnprintf(buffer, sizeof(buffer), message, *ap);
#endif

            if (count < 0)
                return {};
            return {buffer, static_cast<size_t>(count)};
        }

#ifdef _WIN32
        HANDLE get_console_handle()
        {
            return GetStdHandle(STD_OUTPUT_HANDLE);
        }
#endif

        void set_color(const color_type color)
        {
#ifdef _WIN32
            SetConsoleTextAttribute(get_console_handle(), color);
#else
            printf("%s", color);
#endif
        }

        bool apply_color(const std::string& data, const size_t index, const color_type base_color)
        {
            if (data[index] != '^' || (index + 1) >= data.size())
            {
                return false;
            }

            auto code = data[index + 1] - '0';
            if (code < 0 || code > 11)
            {
                return false;
            }

            code = std::min(code, 7); // Everything above white is white
            if (code == 7)
            {
                set_color(base_color);
            }
            else
            {
                set_color(color_array[code]);
            }

            return true;
        }

        void print_colored(const std::string& line, const color_type base_color)
        {
            lock _{};
            set_color(base_color);

            for (size_t i = 0; i < line.size(); ++i)
            {
                if (apply_color(line, i, base_color))
                {
                    ++i;
                    continue;
                }

                putchar(line[i]);
            }

            reset_color();
        }
    }

    lock::lock()
    {
#ifdef _WIN32
        _lock_file(stdout);
#else
        flockfile(stdout);
#endif
    }

    lock::~lock()
    {
#ifdef _WIN32
        _unlock_file(stdout);
#else
        funlockfile(stdout);
#endif
    }

    void reset_color()
    {
        lock _{};

#ifdef _WIN32
        SetConsoleTextAttribute(get_console_handle(), 7);
#else
        printf("\033[0m");
#endif

        (void)fflush(stdout);
    }

    void info(const char* message, ...)
    {
        va_list ap;
        va_start(ap, message);
        const auto _ = utils::finally([&ap] { va_end(ap); });

        const auto data = format(&ap, message);
        print_colored("[+] " + data + "\n", COLOR_LOG_INFO);
    }

    void warn(const char* message, ...)
    {
        va_list ap;
        va_start(ap, message);
        const auto _ = utils::finally([&ap] { va_end(ap); });

        const auto data = format(&ap, message);
        print_colored("[!] " + data + "\n", COLOR_LOG_WARN);
    }

    void error(const char* message, ...)
    {
        va_list ap;
        va_start(ap, message);
        const auto _ = utils::finally([&ap] { va_end(ap); });

        const auto data = format(&ap, message);
        print_colored("[-] " + data + "\n", COLOR_LOG_ERROR);
    }

    void log(const char* message, ...)
    {
        va_list ap;
        va_start(ap, message);
        const auto _ = utils::finally([&ap] { va_end(ap); });

        const auto data = format(&ap, message);
        print_colored("[*] " + data + "\n", COLOR_LOG_DEBUG);
    }

    void set_title(const std::string& title)
    {
        const lock _{};

#ifdef _WIN32
        SetConsoleTitleA(title.data());
#else
        printf("\033]0;%s\007", title.data());
        fflush(stdout);
#endif
    }

    signal_handler::signal_handler(std::function<void()> callback)
    {
        bool value{false};
        if (!signal_installed.compare_exchange_strong(value, true))
        {
            throw std::runtime_error("Global signal handler already installed");
        }

        get_signal_callback() = std::move(callback);

#ifdef _WIN32
        SetConsoleCtrlHandler(handler, TRUE);
#else
        signal(SIGINT, handler);
#endif
    }

    signal_handler::~signal_handler()
    {
#ifdef _WIN32
        SetConsoleCtrlHandler(handler, FALSE);
#else
        signal(SIGINT, SIG_DFL);
#endif

        get_signal_callback() = {};
        signal_installed = false;
    }
}
