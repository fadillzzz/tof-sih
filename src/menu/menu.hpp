#pragma once

namespace Menu {
    void init();
    void shutdown();
    void registerMenu(void *func);
    std::vector<void *> getRegisteredMenu();
} // namespace Menu
