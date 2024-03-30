namespace Hooks {
    enum {
        CONTINUE_EXECUTION = 0,
        STOP_EXECUTION = 1,
    };

    typedef uint8_t (*Callback)(SDK::UObject *, SDK::UFunction *, void *);

    void init();
    void shutdown();
    void registerHook(std::string functionName, Callback handler);
} // namespace Hooks
