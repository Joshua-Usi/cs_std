#pragma once

#include <vector>
#include <set>
#include <stack>

#define CS_STD_PACKED_VECTOR_CLEAR_ON_ERASE

namespace cs_std
{
	// Vector-like with log(n) insertion and deletion
	// Gaurantees that the indices of the elements will not change
	// Guarantees that the empty blocks closest to the start of the vector will be filled first
	template<typename T>
	class packed_vector
	{
	private:
		struct free_block
		{
			size_t start, length;
			// Used by the set to sort the free blocks
			bool operator<(const free_block& other) const { return start < other.start; }
		};
	private:
		std::vector<T> vector;
		std::set<free_block> free_blocks;
	private:
		size_t remove_first_empty_block()
		{
			auto it = free_blocks.begin();
			size_t index = it->start;
			if (it->length > 1)
			{
				free_block modifiedBlock(it->start + 1, it->length - 1);
				free_blocks.erase(it);
				free_blocks.insert(modifiedBlock);
			}
			else free_blocks.erase(it);
			return index;
		}
	public:
		[[nodiscard]] size_t insert(const T& item)
		{
			if (!free_blocks.empty())
			{
				size_t index = remove_first_empty_block();
				vector[index] = item;
				return index;
			}
			else
			{
				vector.push_back(item);
				return vector.size() - 1;
			}
		}
		[[nodiscard]] size_t insert(T&& item)
		{
			if (!free_blocks.empty())
			{
				size_t index = remove_first_empty_block();
				vector[index] = std::move(item);
				return index;
			}
			else
			{
				vector.push_back(std::move(item));
				return vector.size() - 1;
			}
		}
		template<typename... Args>
		[[nodiscard]] size_t emplace(Args&&... args)
		{
			if (!free_blocks.empty())
			{
				size_t index = remove_first_empty_block();
				vector[index] = T(std::forward<Args>(args)...);
				return index;
			}
			else
			{
				vector.emplace_back(std::forward<Args>(args)...);
				return vector.size() - 1;
			}
		}
		void replace(size_t index, const T& item) { vector[index].~T(); vector[index] = item; }
		void replace(size_t index, T&& item) { vector[index].~T(); vector[index] = std::move(item); }
		void erase(size_t index)
		{
			// clear the value at the index.
			// Could potentially be faster to just leave it as is.
			vector[index].~T();
			#ifdef CS_STD_PACKED_VECTOR_CLEAR_ON_ERASE
				vector[index] = T();
			#endif
			auto next_it = free_blocks.lower_bound(free_block(index, 0));
			auto prev_it = (next_it == free_blocks.begin()) ? free_blocks.end() : std::prev(next_it);
			bool merged = false;
			if (prev_it != free_blocks.end() && prev_it->start + prev_it->length == index)
			{
				free_block largerBlock(prev_it->start, prev_it->length + 1);
				free_blocks.erase(prev_it);
				prev_it = free_blocks.insert(largerBlock).first;
				merged = true;
			}
			if (next_it != free_blocks.end() && index + 1 == next_it->start)
			{
				free_block mergedBlock(merged ? prev_it->start : index, (merged ? prev_it->length : 1) + next_it->length);
				if (merged) free_blocks.erase(prev_it);
				free_blocks.erase(next_it);
				free_blocks.insert(mergedBlock);
				merged = true;
			}
			if (!merged) free_blocks.insert(free_block(index, 1));
		}
		bool is_valid(size_t index) const
		{
			if (index >= vector.size()) return false;
			for (auto& block : free_blocks)
			{
				if (index >= block.start && index < block.start + block.length) return false;
			}
			return true;
		}
		// Return the number of valid elements in the vector
		size_t size() const
		{
			size_t validSize = vector.size();
			for (auto& block : free_blocks) validSize -= block.length;
			return validSize;
		}
		// Return the total size of the vector, including empty slots
		size_t capacity() const { return vector.size(); }
		void clear() { vector.clear(); free_blocks.clear(); }
		// Exception safe
		T& at(size_t index) { return vector.at(index); }
	public:
		// Exception unsafe, but faster
		T& operator[](size_t index) { return vector[index]; }
		const T& operator[](size_t index) const { return vector[index]; }
	};
}