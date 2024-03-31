namespace Logger {

    void init();
    void shutdown();

    void info(std::string message);
    void success(std::string message);
    void error(std::string message);
    void warning(std::string message);
    void debug(std::string message);
} // namespace Logger
