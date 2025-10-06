namespace {

std::pmr::unsynchronized_pool_resource pool;

using NodeId = std::array<char, 2>;
struct CompareNodeId {
    constexpr bool operator()(NodeId lhs, NodeId rhs) const noexcept {
        return std::bit_cast<uint16_t>(lhs) < std::bit_cast<uint16_t>(rhs);
    }
};

using IdSet         = boost::container::flat_set<NodeId, CompareNodeId, boost::container::static_vector<NodeId, 13>>;
using ConnectionMap = boost::unordered_flat_map<NodeId, IdSet>;

ConnectionMap Parse(std::string_view input) {
    ConnectionMap connectionMap;
    for (std::string_view line : input | Split('\n')) {
        NodeId nodeA{line[0], line[1]};
        NodeId nodeB{line[3], line[4]};
        connectionMap[nodeA].emplace(nodeB);
        connectionMap[nodeB].emplace(nodeA);
    }
    return connectionMap;
}

size_t TriGroupCount(const ConnectionMap& connectionMap) {
    boost::container::flat_set<std::array<NodeId, 3>> groups;
    for (const auto& [aId, aConn] : connectionMap) {
        if (aId[0] != 't' || aConn.size() < 2) {
            continue;
        }
        for (const auto& [bId, cId] : views::cartesian_product(aConn, aConn)) {
            const auto& bConn = connectionMap.at(bId);
            const auto& cConn = connectionMap.at(cId);
            if (bConn.contains(cId) && cConn.contains(bId)) {
                std::array<NodeId, 3> group{aId, bId, cId};
                ranges::sort(group);
                groups.emplace(group);
            }
        }
    }
    return groups.size();
};

std::string Password(const ConnectionMap& connectionMap) {
    IdSet maxGroup;
    for (const auto& [aId, aConn] : connectionMap) {
        if ((aConn.size() + 1) < maxGroup.size()) {
            continue;
        }
        using IntersectionMap = boost::container::flat_map<IdSet, IdSet, std::less<IdSet>,
                                                           std::pmr::polymorphic_allocator<std::pair<IdSet, IdSet>>>;
        IntersectionMap prevIntersectionMap{{{aId}, aConn}};
        IntersectionMap nextIntersectionMap{};
        while (!prevIntersectionMap.empty()) {
            for (const auto& [keySet, valSet] : prevIntersectionMap) {
                if (keySet.size() > maxGroup.size()) {
                    maxGroup = keySet;
                }
                for (const NodeId toAdd : valSet) {
                    IdSet newKeySet = keySet;
                    newKeySet.emplace(toAdd);
                    if (nextIntersectionMap.contains(newKeySet)) {
                        continue;
                    }

                    const auto& addSet = connectionMap.at(toAdd);
                    IdSet intersection;
                    ranges::set_intersection(valSet, addSet, std::inserter(intersection, intersection.end()));
                    if (newKeySet.size() + intersection.size() <= maxGroup.size()) {
                        continue;
                    }
                    nextIntersectionMap.emplace(std::move(newKeySet), std::move(intersection));
                }
            }
            prevIntersectionMap.swap(nextIntersectionMap);
            nextIntersectionMap.clear();
        }
    }
    return maxGroup | views::transform(Constructor<std::string_view>{}) | views::join(',') | ranges::to<std::string>;
};

} // namespace

void AocMain(std::string_view input) {
    std::pmr::set_default_resource(&pool);
    const ConnectionMap connectionMap = StopWatch<std::micro>::Run("Parse", Parse, input);
    logger.solution("Group of 3 Count: {}", StopWatch<std::micro>::Run("TriGroupCount", TriGroupCount, connectionMap));
    for (int i{0}; i < 1000; ++i) {
        logger.solution("Password: {}", StopWatch<std::milli>::Run("Password", Password, connectionMap));
    }
}