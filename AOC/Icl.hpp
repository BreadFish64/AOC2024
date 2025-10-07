#pragma once

#include <boost/container/flat_set.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/container/small_vector.hpp>

#define ICL_USE_AOC_IMPLEMENTATION
namespace AocIcl {
template <typename Key, typename Compare, typename Allocator>
using set = boost::container::flat_set<Key, Compare, boost::container::small_vector<Key, 16, Allocator>>;
template <typename Key, typename T, typename Compare, typename Allocator>
using map = boost::container::flat_map<Key, T, Compare, Allocator>;
} // namespace AocIcl

#include <boost/icl/interval.hpp>
#include <boost/icl/interval_map.hpp>

template <class T>
struct std::formatter<boost::icl::right_open_interval<T>> : std::formatter<T> {
    auto format(boost::icl::right_open_interval<T> interval, format_context& ctx) const {
        auto it = ctx.out();
        *it++   = '[';
        it      = std::formatter<T>::format(interval.lower(), ctx);
        *it++   = ',';
        *it++   = ' ';
        it      = std::formatter<T>::format(interval.upper(), ctx);
        *it++   = ')';
        return it;
    }
};

template <class T>
struct std::formatter<boost::icl::closed_interval<T>> : std::formatter<T> {
    auto format(boost::icl::closed_interval<T> interval, format_context& ctx) const {
        auto it = ctx.out();
        *it++   = '[';
        it      = std::formatter<T>::format(interval.lower(), ctx);
        *it++   = ',';
        *it++   = ' ';
        it      = std::formatter<T>::format(interval.upper(), ctx);
        *it++   = ']';
        return it;
    }
};