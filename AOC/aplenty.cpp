#include <boost/icl/closed_interval.hpp>

namespace {

using PropertyIdx   = u8;
using Property      = s16;
using WorkflowIdx   = u32;
using MatchInterval = boost::icl::closed_interval<Property>;

const MatchInterval ALL_PASS{1, 4000};

constexpr WorkflowIdx ACCEPTED = 'A', REJECTED = 'R', IN = 'i' | ('n' << 8);

constexpr PropertyIdx ToPropertyIdx(char c) {
    switch (c) {
        case 'x': return 0;
        case 'm': return 1;
        case 'a': return 2;
        case 's': return 3;
        default: Assume(false);
    }
    return 0;
}

struct ItemRange {
    std::array<MatchInterval, 4> propertyRanges{ALL_PASS, ALL_PASS, ALL_PASS, ALL_PASS};

    s64 combinations() const {
        return ranges::accumulate(propertyRanges, s64{1}, std::multiplies{}, boost::icl::length<MatchInterval>);
    }
};

struct Item {
    std::array<Property, 4> properties{};

    Property rating() const { return ranges::accumulate(properties, Property{}); }

    static Item Parse(std::string_view str) {
        Item item{};
        str.remove_prefix(1);
        str.remove_suffix(1);
        for (std::string_view propertyStr : str | Split(',')) {
            item.properties[ToPropertyIdx(propertyStr.front())] = ParseNumber<Property>(propertyStr.substr(2));
        }
        return item;
    }
};

struct Rule {
    MatchInterval matchInterval{ALL_PASS};
    WorkflowIdx workflowIdx{};
    PropertyIdx propertyIdx{};

    bool matches(const Item& item) const { return boost::icl::contains(matchInterval, item.properties[propertyIdx]); }

    static Rule Parse(std::string_view str) {
        Rule rule{};
        const size_t conditionEnd  = str.find(':');
        std::string_view dstIdxStr = str;
        if (conditionEnd != str.npos) {
            dstIdxStr.remove_prefix(conditionEnd + 1);
            rule.propertyIdx = ToPropertyIdx(str.front());
            const auto bound = ParseNumber<Property>(str.substr(2, conditionEnd - 2));
            if (str[1] == '>') {
                rule.matchInterval = MatchInterval{static_cast<Property>(bound + 1), ALL_PASS.upper()};
            } else if (str[1] == '<') {
                rule.matchInterval = MatchInterval{ALL_PASS.lower(), static_cast<Property>(bound - 1)};
            } else {
                Assume(false);
            }
        }
        Assume(dstIdxStr.size() <= 3);
        std::memcpy(&rule.workflowIdx, dstIdxStr.data(), dstIdxStr.size());
        return rule;
    }
};

struct Workflow {
    boost::container::static_vector<Rule, 4> rules;

    WorkflowIdx nextWorkflow(const Item& item) const {
        for (const Rule& rule : rules) {
            if (rule.matches(item)) {
                return rule.workflowIdx;
            }
        }
        Assume(false);
        return REJECTED;
    }

    static std::pair<WorkflowIdx, Workflow> Parse(std::string_view str) {
        Workflow workflow{};
        WorkflowIdx idx{};

        const size_t leftBracePos = str.find('{');
        Assume(leftBracePos <= 3);
        std::memcpy(&idx, str.data(), leftBracePos);

        str.remove_prefix(leftBracePos + 1);
        str.remove_suffix(1);
        for (const std::string_view ruleStr : str | Split(',')) {
            workflow.rules.emplace_back(Rule::Parse(ruleStr));
        }
        return {idx, workflow};
    }
};
using WorkflowCollection = std::vector<Workflow>;

s64 Part1(const WorkflowCollection& workflows, std::span<const Item> items) {
    s64 totalAcceptedRating{};
    for (const Item& item : items) {
        WorkflowIdx workflowIdx = IN;
        while (true) {
            const Workflow& workflow = workflows.at(workflowIdx);
            workflowIdx              = workflow.nextWorkflow(item);
            if (workflowIdx == ACCEPTED) {
                totalAcceptedRating += item.rating();
                break;
            }
            if (workflowIdx == REJECTED) {
                break;
            }
        }
    }
    return totalAcceptedRating;
}

s64 TraverseWorkflows(const WorkflowCollection& workflows, const ItemRange& itemRange, WorkflowIdx workflowIdx);

s64 TraverseRules(const WorkflowCollection& workflows, const ItemRange& sourceItemRange, std::span<const Rule> rules) {
    Assume(!rules.empty());

    s64 combinations{};
    ItemRange splitItemRange          = sourceItemRange;
    const MatchInterval matchInterval = rules.front().matchInterval;

    const u8 propertyIdx                = rules.front().propertyIdx;
    const MatchInterval& sourceProperty = sourceItemRange.propertyRanges[propertyIdx];
    MatchInterval& splitProperty        = splitItemRange.propertyRanges[propertyIdx];

    splitProperty = boost::icl::right_subtract(sourceItemRange.propertyRanges[propertyIdx], matchInterval);
    if (!boost::icl::is_empty(splitProperty)) {
        combinations += TraverseRules(workflows, splitItemRange, rules.subspan<1>());
    }
    splitProperty = boost::icl::left_subtract(sourceItemRange.propertyRanges[propertyIdx], matchInterval);
    if (!boost::icl::is_empty(splitProperty)) {
        combinations += TraverseRules(workflows, splitItemRange, rules.subspan<1>());
    }
    splitItemRange.propertyRanges[propertyIdx] = sourceItemRange.propertyRanges[propertyIdx] & matchInterval;
    combinations += TraverseWorkflows(workflows, splitItemRange, rules.front().workflowIdx);
    return combinations;
}

s64 TraverseWorkflows(const WorkflowCollection& workflows, const ItemRange& itemRange, WorkflowIdx workflowIdx) {
    const s64 combinations = itemRange.combinations();
    if (workflowIdx == REJECTED || combinations == 0) return 0;
    if (workflowIdx == ACCEPTED) return combinations;
    return TraverseRules(workflows, itemRange, workflows.at(workflowIdx).rules);
}

std::tuple<WorkflowCollection, std::vector<Item>> Parse(std::string_view input) {
    const size_t inputSplit              = input.find("\n\n"sv);
    const std::string_view workflowInput = input.substr(0, inputSplit);
    const std::string_view itemInput     = input.substr(inputSplit + 2);

    WorkflowCollection workflows(1_sz << 24);
    for (auto&& [idx, workflow] : workflowInput | Split('\n') | views::transform(Workflow::Parse)) {
        workflows.at(idx) = std::move(workflow);
    }
    std::vector<Item> items = itemInput | Split('\n') | views::transform(Item::Parse) | ranges::to<std::vector>;
    return {std::move(workflows), std::move(items)};
}

} // namespace

void AocMain(std::string_view input) {
    const auto& [workflows, items] = StopWatch<std::milli>::Run("Parsing", Parse, input);
    logger.solution("Part 1: {}", StopWatch<std::micro>::Run("Part 1", Part1, workflows, items));
    logger.solution("Part 2: {}", StopWatch<std::micro>::Run("Part 2", TraverseWorkflows, workflows, ItemRange{}, IN));
}