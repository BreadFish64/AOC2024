namespace {
const std::string INPUT_FILE_NAME{"input.txt"s};

auto LoadInput() {
    return mio::mmap_source{INPUT_FILE_NAME};
}

inline void AocMainInternal() {
    {
        StopWatch wholeProgramStopWatch{"Whole Program"};
        SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
        SetConsoleCP(CP_UTF8);
        SetConsoleOutputCP(CP_UTF8);
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

Logger logger;

int main([[maybe_unused]] const int argc, [[maybe_unused]] const char* argv[]) {
    AocMainInternal();
    return 0;
}