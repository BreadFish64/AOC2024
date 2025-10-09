#include "Icl.hpp"
#include "Mdspan.hpp"

#include <boost/pool/pool_alloc.hpp>

namespace {

template <class T>
using PoolAlloc =
    boost::fast_pool_allocator<T, boost::default_user_allocator_new_delete, boost::details::pool::null_mutex>;

class PlantCrawler {
    const InputGrid<const char> garden;
    const mdspan<uint8_t, dextents<int32_t, 2>> isCrawled;
    const char target;

    size_t area{0};
    size_t perimeter{0};

    using SideSet = boost::icl::interval_set<int32_t, std::less, boost::icl::closed_interval<int32_t>, PoolAlloc>;
    std::array<std::vector<SideSet>, 4> sides;

public:
    PlantCrawler(InputGrid<const char> garden, mdspan<uint8_t, dextents<int32_t, 2>> isCrawled, char target)
        : garden(garden), isCrawled(isCrawled), target(target), sides{
                                                                    std::vector<SideSet>(garden.extent(0) + 2),
                                                                    std::vector<SideSet>(garden.extent(1) + 2),
                                                                    std::vector<SideSet>(garden.extent(0) + 2),
                                                                    std::vector<SideSet>(garden.extent(1) + 2),
                                                                } {}

    size_t countSides() const {
        const auto countSidesInDirection = [](std::span<const SideSet> sideSet) {
            return ranges::accumulate(sideSet, size_t{}, std::plus{}, &SideSet::iterative_size);
        };
        return ranges::accumulate(sides, size_t{}, std::plus{}, countSidesInDirection);
    }

    size_t price1() const { return area * perimeter; }
    size_t price2() const { return area * countSides(); }

    void crawl(const Pos2D pos, const uint8_t direction) {
        if (!InBounds(garden, pos) || garden[pos] != target) {
            ++perimeter;
            if (direction & 1) {
                sides[direction][pos.y() + 1] += pos.x();
            } else {
                sides[direction][pos.x() + 1] += pos.y();
            }
            return;
        }
        if (std::exchange(isCrawled[pos], true)) {
            return;
        }
        ++area;
        crawl(pos + Vec2D{-1, 0}, 0);
        crawl(pos + Vec2D{0, -1}, 1);
        crawl(pos + Vec2D{1, 0}, 2);
        crawl(pos + Vec2D{0, 1}, 3);
    }
};

} // namespace

void AocMain(std::string_view input) {
    mdspan garden = ToGrid(input);
    mdarray<uint8_t, dextents<int32_t, 2>> isCrawled(garden.extents());

    size_t totalPrice1{0};
    size_t totalPrice2{0};
    for (int32_t y = 0; y < garden.extent(0); ++y) {
        for (int32_t x = 0; x < garden.extent(1); ++x) {
            if (isCrawled[y, x]) {
                continue;
            }
            PlantCrawler crawler{garden, isCrawled, garden[y, x]};
            crawler.crawl({y, x}, false);
            totalPrice1 += crawler.price1();
            totalPrice2 += crawler.price2();
        }
    }

    logger.solution("1: {}", totalPrice1);
    logger.solution("2: {}", totalPrice2);
}
