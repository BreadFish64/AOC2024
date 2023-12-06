#define ICL_USE_AOC_IMPLEMENTATION
namespace AocIcl {
template <typename Key, typename Compare, typename Allocator>
using set = boost::container::flat_set<Key, Compare, boost::container::small_vector<Key, 16, Allocator>>;
template <typename Key, typename T, typename Compare, typename Allocator>
using map = boost::container::flat_map<Key, T, Compare, Allocator>;
} // namespace AocIcl

#include <boost/icl/interval_map.hpp>

template <>
struct fmt::formatter<boost::icl::right_open_interval<s64>> : fmt::formatter<s64> {
    // parse is inherited from formatter<string_view>.

    auto format(boost::icl::right_open_interval<s64> seedRange, format_context& ctx) const {
        auto it = ctx.out();
        *it++   = '[';
        it      = fmt::formatter<s64>::format(seedRange.lower(), ctx);
        *it++   = ',';
        *it++   = ' ';
        it      = fmt::formatter<s64>::format(seedRange.upper(), ctx);
        *it++   = ')';
        return it;
    }
};