#pragma once
#include <cstdint>
#include <mutex>
#include <chrono>
#include <iostream>
#include <type_traits>

namespace cs_std
{
	template <typename T>
	concept TimeUnit = std::is_convertible_v<T, std::chrono::nanoseconds> ||
		std::is_convertible_v<T, std::chrono::microseconds> ||
		std::is_convertible_v<T, std::chrono::milliseconds> ||
		std::is_convertible_v<T, std::chrono::seconds> ||
		std::is_convertible_v<T, std::chrono::minutes> ||
		std::is_convertible_v<T, std::chrono::hours>;

	// Javascript-like logger for C++
	class console
	{
	public:
		// Severity flags
		typedef uint8_t severity;
		enum class severity_bits: uint8_t
		{
			verbose	= 0b00000001,
			info	= 0b00000010,
			log		= 0b00000100,
			warn	= 0b00001000,
			error	= 0b00010000,
			fatal	= 0b00100000,
		};
	private:
		static constexpr const char* const SEVERITY_STRINGS[6] { "Verbose", "Info", "Log", "Warn", "Error", "Critical" };
		static bool enableThreadSafety;
		static std::mutex threadMutex;
		static severity displayedSeverities;
		static bool printSeverity, printTimestamp;
		static std::chrono::time_point<std::chrono::steady_clock> timePoint;

		template<typename... Ts>
		static void base_log(severity_bits severity, Ts&&... args)
		{
			if (enableThreadSafety)
			{
				std::scoped_lock lock(threadMutex);
				internal_log(severity, std::forward<Ts>(args)...);
			}
			else internal_log(severity, std::forward<Ts>(args)...);
		}

		template<typename... Ts>
		static void internal_log(severity_bits severity, Ts&&... args)
		{
			if ((displayedSeverities & static_cast<uint8_t>(severity)) == 0) return;
			if (printTimestamp)
			{
				int64_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
				std::cout << '[' << std::put_time(std::localtime(&time), "%T") << "] ";
			}
			if (printSeverity) std::cout << '[' << SEVERITY_STRINGS[static_cast<size_t>(std::log2(static_cast<double>(severity)))] << "] ";
			([&] {
				if constexpr (std::is_same_v<std::decay_t<Ts>, bool>) std::cout << (args ? "true" : "false");
				else std::cout << args;
			} (), ...);
			std::cout << "\n";
		}
		template<typename... Ts> static void raw_base(Ts&&... args) { ([&] { std::cout << args; } (), ...); }
	public:
		// Verbose, usually not displayed
		template <typename... Ts>
		static void verbose(Ts&&... args) { base_log(severity_bits::verbose, args...); }
		// Info, provides useful information alongside other logs
		template <typename... Ts>
		static void info(Ts&&... args) { base_log(severity_bits::info, args...); }
		// Log, standard logging method
		template <typename... Ts>
		static void log(Ts&&... args) { base_log(severity_bits::log, args...); }
		// Warn, denotes potential side-effects or errors
		template <typename... Ts>
		static void warn(Ts&&... args) { base_log(severity_bits::warn, args...); }
		// Error, denotes a non-fatal error
		template <typename... Ts>
		static void error(Ts&&... args) { base_log(severity_bits::error, args...); }
		// Fatal, denotes a fatal error that requires a program crash
		template <typename... Ts>
		static void fatal(Ts&&... args) { base_log(severity_bits::fatal, args...); }

		template<typename... Ts>
		static void raw(Ts&&... args)
		{
			if (enableThreadSafety)
			{
				std::scoped_lock lock(threadMutex);
				raw_base(std::forward<Ts>(args)...);
			}
			else raw_base(std::forward<Ts>(args)...);
		}

		// Set the severity flags, by default allows info, log, warn, error and fatal
		static void severity_flags(severity severityFlag) { displayedSeverities = severityFlag; }
		// Enables thread safety, on by default, can improve performance slightly if turned off
		// However, if turned off does not gaurantee order or correctness
		static void thread_safety(bool enable) { enableThreadSafety = enable; }
		// Whether or not to print the HH:MM::SS timestamps next to the logs
		static void print_timestamps(bool enable) { printTimestamp = enable; }
		// Whether or not to print the severity of the log	
		static void print_severity(bool enable) { printSeverity = enable; }

		static void begin() { timePoint = std::chrono::high_resolution_clock::now(); }
		template<typename TimeUnit = std::chrono::seconds> static size_t end() { return std::chrono::duration_cast<TimeUnit>(std::chrono::high_resolution_clock::now() - timePoint).count(); }
	};
}