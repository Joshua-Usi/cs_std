#pragma once

#include <chrono>

namespace cs_std
{
	class timestamp
	{
	private:
		std::chrono::high_resolution_clock::time_point start;
	public:
		timestamp() : start(std::chrono::high_resolution_clock::now()) {}
		template<typename T = double>
		T elapsed() const
		{
			auto now = std::chrono::high_resolution_clock::now();
			std::chrono::duration<T> duration = std::chrono::duration_cast<std::chrono::nanoseconds>(now - start);
			return duration.count();
		}
	};
}

