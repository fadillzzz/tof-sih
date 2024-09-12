#include "early_init.hpp"

namespace Feats {
    namespace EarlyInit {
        Config::field<bool> enabled;

        void init() { enabled = Config::get<bool>(confEnabled, false); }
        void tick() { return; }
        void menu() { ImGui::Checkbox("Initialize menu before login", &enabled); }
    } // namespace EarlyInit
} // namespace Feats