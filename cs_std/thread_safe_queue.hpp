#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

namespace cs_std
{
	template<typename T>
	class thread_safe_queue
	{
	private:
		std::queue<T> queue;
		mutable std::mutex mutex;
		std::condition_variable condition;
		bool unlocked = false;
	public:
		thread_safe_queue() = default;
		~thread_safe_queue() = default;
		thread_safe_queue(const thread_safe_queue<T>& other) = delete;
		thread_safe_queue<T>& operator=(const thread_safe_queue<T>& other) = delete;
		void push(const T& value)
		{
			std::lock_guard<std::mutex> lock(this->mutex);
			this->queue.push(value);
			this->condition.notify_one();
		}
		void push(T&& value)
		{
			std::lock_guard<std::mutex> lock(this->mutex);
			this->queue.push(std::move(value));
			this->condition.notify_one();
		}
		T front() const
		{
			std::lock_guard<std::mutex> lock(this->mutex);
			return this->queue.front();
		}
		// Waits till an element is available then pops it
		std::optional<T> pop()
		{
			std::unique_lock<std::mutex> lock(this->mutex);
			this->condition.wait(lock, [this]() { return !this->queue.empty() || unlocked; });
			
			if (this->queue.empty()) return std::nullopt;

			T value = std::move(this->queue.front());
			this->queue.pop();
			return value;
		}
		// Tries to pop an element, if the queue is empty it returns a nullopt
		std::optional<T> try_pop()
		{
			std::lock_guard<std::mutex> lock(this->mutex);
			if (this->queue.empty()) return std::nullopt;

			T value = std::move(this->queue.front());
			this->queue.pop();
			return value;
		}
		bool empty() const
		{
			std::lock_guard<std::mutex> lock(this->mutex);
			return this->queue.empty();
		}
		// Returns if the queue is empty without locking it
		bool empty_unsafe() const
		{
			return this->queue.empty();
		}
		size_t size() const
		{
			std::lock_guard<std::mutex> lock(this->mutex);
			return this->queue.size();
		}
		// Returns the size of the queue without locking it
		size_t size_unsafe() const
		{
			return this->queue.size();
		}
		// Unlock the queue and allow all threads to continue, all pop operations will return nullopts
		void unlock()
		{
			std::lock_guard<std::mutex> lock(this->mutex);
			this->unlocked = true;
			this->condition.notify_all();
		}
	};
}