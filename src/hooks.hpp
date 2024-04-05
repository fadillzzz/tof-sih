namespace Hooks {
    enum ExecutionFlag {
        CONTINUE_EXECUTION = 0,
        STOP_EXECUTION = 1,
    };

    enum Type {
        PRE,
        POST,
    };

    typedef uint8_t (*Callback)(SDK::UObject *, SDK::UFunction *, void *);

    void init();
    void shutdown();
    void registerHook(std::string functionName, Callback handler, Type type = Type::PRE);
} // namespace Hooks
