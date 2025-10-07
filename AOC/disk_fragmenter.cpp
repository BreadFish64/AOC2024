#include "Icl.hpp"

#include <boost/pool/pool_alloc.hpp>

namespace {

using FileInterval = boost::icl::right_open_interval<size_t>;
#if true
template <class T>
using Allocator =
    boost::fast_pool_allocator<T, boost::default_user_allocator_new_delete, boost::details::pool::null_mutex>;
#else
template <class T>
using Allocator = std::allocator<T>;
#endif
using FileMap = boost::icl::interval_map<size_t, size_t, boost::icl::partial_enricher, std::less,
                                         boost::icl::inplace_plus, boost::icl::inter_section, FileInterval, Allocator>;

FileMap Parse(std::string_view input) {
    if (input.back() == '\n') {
        input.remove_suffix(1);
    }
    assert(input.size() % 2 == 1);
    FileMap fileMap{};
    size_t back = static_cast<size_t>(input[0] - '0');
    fileMap.add(fileMap.end(), {FileInterval{0, back}, 0});
    size_t fileCount{1};
    for (auto range : input | views::drop(1) | views::transform([](char c) { return static_cast<size_t>(c - '0'); }) |
                          ranges::views::chunk(2)) {
        size_t front = back + range[0];
        back         = front + range[1];
        fileMap.add(fileMap.end(), {FileInterval{front, back}, fileCount++});
    }
    return fileMap;
}

size_t ChunkChecksum(const FileMap::value_type& chunk) {
    const auto& [interval, file] = chunk;
    return file * (interval.lower() + interval.upper() - 1) * boost::icl::length(interval) / 2;
}

size_t Checksum(const FileMap& fileMap) {
    return ranges::fold_left(fileMap | views::transform(ChunkChecksum), size_t{}, std::plus{});
}

size_t FragmentFiles(FileMap fileMap) {
    for (auto it = fileMap.begin(); it != fileMap.end();) {
        auto next = std::next(it);
        if (next == fileMap.end()) {
            break;
        }
        size_t distance = boost::icl::distance(it->first, next->first);
        if (distance == 0) {
            ++it;
            continue;
        }
        auto [backInterval, backFile] = *fileMap.rbegin();
        size_t take                   = std::min(distance, boost::icl::length(backInterval));
        fileMap.erase(FileInterval{backInterval.upper() - take, backInterval.upper()});
        it = fileMap.add(it, {FileInterval{it->first.upper(), it->first.upper() + take}, backFile});
    }
    return Checksum(fileMap);
}

size_t MoveFiles(FileMap fileMap) {
    auto searchBegin = fileMap.rbegin();
    for (size_t currentFile = fileMap.rbegin()->second; currentFile > 0; --currentFile) {
        searchBegin = ranges::find(searchBegin, fileMap.rend(), currentFile, &FileMap::value_type::second);
        auto src    = std::prev(searchBegin.base());
        for (auto before = fileMap.begin(); before != src; ++before) {
            auto after = std::next(before);
            if (boost::icl::distance(before->first, after->first) >= boost::icl::length(src->first)) {
                auto moving = *src;
                fileMap.erase(src);
                fileMap.add(before, {FileInterval{before->first.upper(),
                                                  before->first.upper() + boost::icl::length(moving.first)},
                                     moving.second});
                break;
            }
        }
    }
    return Checksum(fileMap);
}

} // namespace

void AocMain(std::string_view input) {
    FileMap fileMap = StopWatch<std::micro>::Run("Parse", Parse, input);
    logger.solution("FragmentFiles: {}", StopWatch<std::micro>::Run("FragmentFiles", FragmentFiles, fileMap));
    logger.solution("MoveFiles:     {}", StopWatch<std::milli>::Run("MoveFiles", MoveFiles, fileMap));
}