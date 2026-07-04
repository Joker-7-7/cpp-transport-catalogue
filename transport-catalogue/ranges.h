#pragma once

/**
 * @file ranges.h
 * Simple range wrapper for iterator-based iteration.
 *
 * Provides a lightweight Range class that wraps a pair of iterators and
 * exposes begin()/end() for use in range-for loops. The AsRange() helper
 * deduces the iterator type from any container.
 */

#include <iterator>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

/**
 * Utility range types and helpers.
 */
namespace ranges {

/**
 * Wraps an iterator pair [begin, end) into a range object.
 *
 * Enables range-for iteration and standard algorithm usage over
 * a sub-range or a subset of a container.
 *
 * @tparam It Iterator type.
 */
template <typename It>
class Range {
public:
    /**
     * The value type obtained by dereferencing the iterator.
     */    using ValueType = typename std::iterator_traits<It>::value_type;

    /**
     * Constructs a range from a pair of iterators.
     * @param begin Start iterator.
     * @param end   Past-the-end iterator.
     */
    Range(It begin, It end)
        : begin_(begin)
        , end_(end) {
    }

    /**
     * Returns the start iterator.
     */
    It begin() const {
        return begin_;
    }

    /**
     * Returns the past-the-end iterator.
     */
    It end() const {
        return end_;
    }

private:
    It begin_;
    It end_;
};

/**
 * Creates a Range from a container's begin() and end() iterators.
 *
 * @tparam C Container type (must have begin() and end()).
 * @param container The container to wrap.
 * @return A Range iterating over the container's elements.
 */
template <typename C>
auto AsRange(const C& container) {
    return Range{container.begin(), container.end()};
}

}  // namespace ranges