#include <print>

namespace {

[[maybe_unused]] constexpr std::string_view smallExample = R"(x00: 1
x01: 1
x02: 1
y00: 0
y01: 1
y02: 0

x00 AND y00 -> z00
x01 XOR y01 -> z01
x02 OR y02 -> z02
)";

[[maybe_unused]] constexpr std::string_view largeExample = R"(x00: 1
x01: 0
x02: 1
x03: 1
x04: 0
y00: 1
y01: 1
y02: 1
y03: 1
y04: 1

ntg XOR fgs -> mjb
y02 OR x01 -> tnw
kwq OR kpj -> z05
x00 OR x03 -> fst
tgd XOR rvg -> z01
vdt OR tnw -> bfw
bfw AND frj -> z10
ffh OR nrd -> bqk
y00 AND y03 -> djm
y03 OR y00 -> psh
bqk OR frj -> z08
tnw OR fst -> frj
gnj AND tgd -> z11
bfw XOR mjb -> z00
x03 OR x00 -> vdt
gnj AND wpb -> z02
x04 AND y00 -> kjc
djm OR pbm -> qhw
nrd AND vdt -> hwm
kjc AND fst -> rvg
y04 OR y02 -> fgs
y01 AND x02 -> pbm
ntg OR kjc -> kwq
psh XOR fgs -> tgd
qhw XOR tgd -> z09
pbm OR djm -> kpj
x03 XOR y03 -> ffh
x00 XOR y04 -> ntg
bfw OR bqk -> z06
nrd XOR fgs -> wpb
frj XOR qhw -> z04
bqk OR frj -> z07
y03 OR x01 -> nrd
hwm AND bqk -> z03
tgd XOR rvg -> z12
tnw OR pbm -> gnj
)";

using WireId = std::array<char, 3>;

enum class Gate : char { IN = 'I', AND = '&', OR = '|', XOR = '^' };
constexpr Gate ParseGate(std::string_view input) {
    if (input == "AND"sv) {
        return Gate::AND;
    } else if (input == "OR"sv) {
        return Gate::OR;
    } else if (input == "XOR"sv) {
        return Gate::XOR;
    } else {
        std::unreachable();
    }
}

struct Wire {
    WireId id;

    WireId lhs;
    WireId rhs;
    Gate gate;

    std::optional<bool> high;
};

struct State {

    using Map = std::map<WireId, Wire>;

    Map wires;

    State(std::string_view input) {
        const size_t split = input.find("\n\n"sv);

        for (std::string_view line : input.substr(0, split + 1) | Split('\n')) {
            WireId id{line[0], line[1], line[2]};
            wires.emplace(id, Wire{
                                  .id   = id,
                                  .lhs = {},
                                  .rhs = {},
                                  .gate = Gate::IN,
                                  .high = line[5] != '0',
                              });
        }

        for (std::string_view line : input.substr(split + 2) | Split('\n')) {
            auto tokens            = line | Split(' ');
            auto tokenIt           = tokens.begin();
            std::string_view token = *tokenIt++;
            WireId lhsId{token[0], token[1], token[2]};
            token     = *tokenIt++;
            Gate gate = ParseGate(token);
            token     = *tokenIt++;
            WireId rhsId{token[0], token[1], token[2]};
            ++tokenIt; // Skip ->
            token = *tokenIt++;
            WireId id{token[0], token[1], token[2]};
            wires.emplace(id, Wire{
                                  .id   = id,
                                  .lhs  = lhsId,
                                  .rhs  = rhsId,
                                  .gate = gate,
                                  .high = std::nullopt,
                              });
        }
    }

    static void evaluateCircuit(Map& wires) {
        bool floating = true;
        while (std::exchange(floating, false)) {
            for (auto& [id, wire] : wires) {
                if (wire.high.has_value()) {
                    continue;
                }
                floating = true;

                Wire& lhs = wires.at(wire.lhs);
                if (!lhs.high.has_value()) {
                    continue;
                }
                Wire& rhs = wires.at(wire.rhs);
                if (!rhs.high.has_value()) {
                    continue;
                }

                switch (wire.gate) {
                    case Gate::AND: wire.high = *lhs.high && *rhs.high; break;
                    case Gate::OR: wire.high = *lhs.high || *rhs.high; break;
                    case Gate::XOR: wire.high = *lhs.high ^ *rhs.high; break;
                    default: std::unreachable();
                }
            }
        }
    }

    uint64_t part1() const {
        Map circuit = wires;
        evaluateCircuit(circuit);
        return getNumber(circuit, 'z');
    }

    static uint64_t getNumber(const Map& wires, char prefix) {
        uint64_t num{0};
        for (size_t bit{0}; bit < 64; ++bit) {
            std::array<char, 3> zId{};
            std::format_to_n(zId.begin(), 3, "{}{:02}", prefix, bit);
            const auto zIt = wires.find(zId);
            if (zIt == wires.end()) {
                break;
            }
            num |= (zIt->second.high.value() ? uint64_t{1} : uint64_t{0}) << bit;
        }
        return num;
    }

    static void setNumber(Map& wires, char prefix, uint64_t num) {
        for (size_t bit{0}; bit < 64; ++bit) {
            std::array<char, 3> zId{};
            std::format_to_n(zId.begin(), 3, "{}{:02}", prefix, bit);
            const auto zIt = wires.find(zId);
            if (zIt == wires.end()) {
                break;
            }
            zIt->second.high = (num & 1) != 0;
            num >>= 1;
        }
    }

    void part2() {
        Map circuit = wires;
        setNumber(circuit, 'x', uint64_t{});
        setNumber(circuit, 'y', ~uint64_t{});

        std::swap(circuit.at({'n', 'b', 'c'}), circuit.at({'s', 'v', 'm'}));
        std::swap(circuit.at({'z', '1', '5'}), circuit.at({'k', 'q', 'k'}));
        std::swap(circuit.at({'z', '3', '9'}), circuit.at({'f', 'n', 'r'}));
        std::swap(circuit.at({'z', '2', '3'}), circuit.at({'c', 'g', 'q'}));
        // cgq,fnr,kqk,nbc,svm,z15,z23,z39

        evaluateCircuit(circuit);
        std::println(R"({:064b} + {:064b} =
666655555555554444444444333333333322222222221111111111
3210987654321098765432109876543210987654321098765432109876543210
----------------------------------------------------------------
{:064b}
{:064b} (expected)
)", getNumber(circuit, 'x'),
                     getNumber(circuit, 'y'), getNumber(circuit, 'z'),
                     getNumber(circuit, 'x') + getNumber(circuit, 'y'));
        // z5 output is wrong
    }
};

} // namespace

void AocMain(std::string_view input) {
    // input = largeExample;
    State state{input};

    // 1972354943 too low
    logger.solution("{}", state.part1());

    state.part2();
}