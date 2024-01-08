#pragma once

#include <thread>
#include <functional>
#include <atomic>

#include "thread_safe_queue.hpp"

namespace cs_std
{
	/// <summary>
	/// Safe multithreaded task queue designed to automatically assign tasks to threads
	/// </summary>
	class task_queue
	{
	private:
		cs_std::thread_safe_queue<std::function<void()>> tasks;
		std::vector<std::jthread> threads;
		std::atomic<bool> isRunning;
		std::atomic<size_t> activeThreads;
	public:
		explicit task_queue(size_t threadOverride = std::numeric_limits<size_t>::max()) { this->wake(threadOverride); }
		~task_queue() { this->sleep(); }
		// Delete move
		task_queue(task_queue&&) = delete;
		task_queue& operator=(task_queue&&) = delete;
		// Delete copy
		task_queue(const task_queue&) = delete;
		task_queue& operator=(const task_queue&) = delete;
		// Threads will finish their tasks and the queue will stop
		void sleep()
		{
			if (!this->isRunning) return;
			this->isRunning = false;
			this->tasks.unlock();
			this->threads.clear();
		}
		// Threads will be awoken and begin executing tasks
		void wake(size_t threadOverride = std::numeric_limits<size_t>::max())
		{
			if (this->isRunning) return;
			this->isRunning = true;

			this->threads.clear();
			size_t threadCount = std::min(threadOverride, static_cast<size_t>(std::thread::hardware_concurrency()));
			for (size_t i = 0; i < threadCount; i++)
			{
				this->threads.emplace_back([this]() {
					while (this->isRunning)
					{
						auto func = tasks.pop();
						if (func.has_value())
						{
							activeThreads++;
							func.value()();
							activeThreads--;
						}
						else std::this_thread::yield();
					}
				});
			}
		}
		void push_back(const std::function<void()>& function) { this->tasks.push(function); }
		// Blocks calling thread until all tasks are finished
		void wait_till_finished() { while (!this->finished()) std::this_thread::yield(); }
		bool finished() const { return this->tasks.empty_unsafe() && this->activeThreads == 0; }
		size_t thread_count() const { return this->threads.size(); }
		// Number of threads currently executing tasks
		size_t active_thread_count() const { return this->activeThreads; }
		size_t pending_task_count() const { return this->tasks.size_unsafe(); }
	};
}