#include <fstream>

#ifdef WIN32
#include <Windows.h>
#endif

Logger logger;

namespace {

std::string LoadInput() {
    std::ifstream inputFile{"input.txt", std::ios::binary};
    return std::string{std::istreambuf_iterator{inputFile}, {}};
}

inline void AocMainInternal() {
    {
        StopWatch wholeProgramStopWatch{"Whole Program"};
#ifdef WIN32
        SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
        SetConsoleCP(CP_UTF8);
        SetConsoleOutputCP(CP_UTF8);
#endif
        const auto input = StopWatch<std::micro>::Run("LoadInput", LoadInput);
        StopWatch<std::milli>::Run("AocMain", AocMain, std::string_view{input});
    }
    logger.flush();
}
} // namespace

Logger::Logger() {
    logLines.reserve(128);
}

void Logger::flush() {
    std::vector<TypeErasedLogLine> flushedLines;
    {
        const std::lock_guard logLinesLock{logLinesMutex};
        logLines.swap(flushedLines);
    }
    for (const auto& logLine : flushedLines) {
        logLine();
    }
}

int main([[maybe_unused]] const int argc, [[maybe_unused]] const char* argv[]) {
    AocMainInternal();
    return 0;
}