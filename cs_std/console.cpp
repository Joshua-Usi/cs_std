#include "console.hpp"

namespace cs_std
{
	static const int desync_io = []() {
		std::ios::sync_with_stdio(false);
		std::cin.tie(nullptr);
		return 0;
	}();

	bool console::enableThreadSafety = true;
	std::mutex console::threadMutex;
	console::severity console::displayedSeverities =
		static_cast<uint8_t>(console::severity_bits::info) |
		static_cast<uint8_t>(console::severity_bits::log) |
		static_cast<uint8_t>(console::severity_bits::warn) |
		static_cast<uint8_t>(console::severity_bits::error) |
		static_cast<uint8_t>(console::severity_bits::fatal);
	bool console::printSeverity = true, console::printTimestamp = true;
}