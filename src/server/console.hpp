#pragma once

namespace console
{
	class lock
	{
	public:
		lock();
		~lock();

		lock(lock&&) = delete;
		lock(const lock&) = delete;
		lock& operator=(lock&&) = delete;
		lock& operator=(const lock&) = delete;
	};

	void reset_color();

	void info(const char* message, ...);
	void warn(const char* message, ...);
	void error(const char* message, ...);
	void log(const char* message, ...);

	void set_title(const std::string& title);

	class signal_handler : std::lock_guard<std::mutex>
	{
	public:
		signal_handler(std::function<void()> callback);
		~signal_handler();
	};
}
