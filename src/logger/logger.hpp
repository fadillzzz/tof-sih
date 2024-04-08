namespace Logger {

    void init();
    void shutdown();

    void info(std::string message);
    void success(std::string message);
    void error(std::string message);
    void warning(std::string message);
    void debug(std::string message);

    void info(std::wstring message);
    void success(std::wstring message);
    void error(std::wstring message);
    void warning(std::wstring message);
    void debug(std::wstring message);
} // namespace Logger
