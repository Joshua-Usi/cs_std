#pragma once
#include <vector>

namespace cs_std
{
    template <typename T, typename... Vectors>
    std::vector<T> combine(const std::vector<T>& first, const Vectors&... vectors) {
        std::vector<T> result;
        result.reserve((first.size() + ... + vectors.size()));

        result.insert(result.end(), first.begin(), first.end());
        (result.insert(result.end(), vectors.begin(), vectors.end()), ...);

        return result;
    }
}