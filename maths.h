// Copyright (c) 2013 Craig Henderson
// Part of the Data Processing Library
// https://github.com/cdmh/dataproc

#include <numeric>          // accumulate
#include <unordered_map>

namespace cdmh {
namespace data_processing {
namespace maths {

template<typename T>
inline T const median(std::vector<T> &&data)
{
    auto median = data.begin() + data.size() / 2;
    std::nth_element(data.begin(), median, data.end());
    return *median;
}

template<typename T>
inline T const mode(std::vector<T> &&data)
{
    // count the occurrence of each value
    std::unordered_map<T, unsigned> counts;
    for (auto const &element : data)
        counts[element]++;

    // find the item with the largest count and return
    return std::max_element(
        counts.cbegin(),
        counts.cend(),
        [](std::pair<T, unsigned> const &a, std::pair<T, unsigned> const &b) {
            return a.second < b.second;
        })->first;
}

template<typename T>
inline double const standard_deviation(std::vector<T> &&data)
{
    double const mean = (double)std::accumulate(data.cbegin(), data.cend(), T()) / data.size();

    std::vector<double> differences;
    differences.reserve(data.size());
    for (auto const &value : data)
        differences.push_back(mean - value);

    double const sum_squared_differences =
        std::accumulate(
            differences.cbegin(),
            differences.cend(),
            double(),
            [](double sum, double value) {
                return sum += value*value;
            });

    return sqrt(sum_squared_differences / differences.size());
}

}   // namespace maths
}   // namespace data_processing
}   // namespace cdmh
