namespace {

struct Computer {
    uint64_t aInit;
    std::vector<uint8_t> program;

    Computer(std::string_view input) {
        auto lines  = input | Split('\n');
        auto lineIt = lines.begin();
        aInit       = ParseNumber<uint64_t>((*lineIt++).substr(12));
        // b           = ParseNumber<uint8_t>((*lineIt++).substr(12));
        // c           = ParseNumber<uint8_t>((*lineIt++).substr(12));
        ++lineIt;
        ++lineIt;
        ++lineIt;
        program = (*lineIt++).substr(9) | Split(',') | views::transform(ParseNumber<uint8_t>) | ranges::to_vector;
    }

    std::string run(const uint64_t init) const {
        struct {
            uint64_t a;
            uint8_t b : 3;
            uint8_t c : 3;
        } registers{init, 0, 0};
        size_t pc{0};
        std::vector<uint8_t> output;

        const auto comboOperand = [&registers](const uint8_t operand) -> uint64_t {
            switch (operand) {
                case 0:
                case 1:
                case 2:
                case 3: return operand;
                case 4: return registers.a;
                case 5: return registers.b;
                case 6: return registers.c;
                default: break;
            }
            std::unreachable();
        };

        while (pc != program.size()) {
            const uint8_t opcode{program[pc++]};
            const uint8_t literalOperand{program[pc++]};
            switch (opcode) {
                case 0: registers.a >>= comboOperand(literalOperand); break;
                case 1: registers.b ^= literalOperand; break;
                case 2: registers.b = comboOperand(literalOperand); break;
                case 3:
                    if (registers.a != 0) pc = literalOperand;
                    break;
                case 4: registers.b ^= registers.c; break;
                case 5: output.emplace_back(static_cast<uint8_t>(comboOperand(literalOperand) & 7)); break;
                case 6: registers.b = registers.a >> comboOperand(literalOperand); break;
                case 7: registers.c = registers.a >> comboOperand(literalOperand); break;
            }
        }

        std::string outputStr{std::format("{}", output)};
        std::erase(outputStr, ' ');
        return outputStr;
    }

    std::string run2(const uint64_t init) const {
        uint64_t a{init};
        // uint8_t c{0};
        std::vector<uint8_t> output;

        do {
            const uint8_t b1 = static_cast<uint8_t>(a & 7);
            const uint8_t b2 = b1 ^ 2;
            const uint8_t b3 = b2 ^ static_cast<uint8_t>(a >> b2);
            const uint8_t b4 = b3 ^ 7;
            output.push_back(b4 & 0x7);
            a >>= 3;
        } while (a);

        std::string outputStr{std::format("{}", output)};
        std::erase(outputStr, ' ');
        return outputStr;
    }

    uint64_t reverse() const {
        uint64_t a{0};
        for (const uint8_t outputByte : program | views::reverse) {
            const uint8_t b4 = outputByte;
            const uint8_t b3 = b4 ^ 7;
            uint8_t b1{0};
            for (;; ++b1) {
                assert(b1 < 8);
                uint64_t newA{(a << 3) | b1};
                const uint8_t b2 = b1 ^ 2;
                const uint8_t c  = static_cast<uint8_t>((newA >> b2) & 7);
                if ((b2 ^ c) == b3) {
                    a = newA;
                    break;
                }
            }
        }
        return a;
    }
};

} // namespace

void AocMain(std::string_view input) {
    // input = test;
    Computer computer(input);
    logger.solution("{}", computer.run(computer.aInit));
    logger.info("{}", computer.run2(computer.aInit));
    uint64_t a = computer.reverse();
    logger.solution("{}", a);
    logger.info("{}", computer.run(a));
}