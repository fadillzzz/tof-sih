#include "about.hpp"
#include "../logger/logger.hpp"
#include <shellapi.h>

namespace Feats {
    namespace About {
        void menu() {
            ImGui::GetFont()->Scale *= 3;
            ImGui::PushFont(ImGui::GetFont());
            ImGui::Text("TOF-SIH");
            ImGui::GetFont()->Scale /= 3;
            ImGui::PopFont();

            ImGui::Text("Main contributor: fadillzzz");
            ImGui::Text("Special thanks to: ");
            ImGui::Text("- cracksux");
            ImGui::Text("- griffith1deadly");
            ImGui::Text("- Illusions");

            ImGui::Dummy(ImVec2(0, 10.0f));

            if (ImGui::Button("Join the Discord server!")) {
                ShellExecute(NULL, L"open", L"https://discord.gg/5PGzs9zmVy", NULL, NULL, SW_SHOWDEFAULT);
            }

            ImGui::Text("This software is provided for free and is open source.");

            if (ImGui::Button("Source")) {
                ShellExecute(NULL, L"open", L"https://github.com/fadillzzz/tof-sih", NULL, NULL, SW_SHOWDEFAULT);
            }

            ImGui::Text("While it is not mandatory, donations are greatly appreciated.");

            if (ImGui::Button("Sponsor me on GitHub")) {
                ShellExecute(NULL, L"open", L"https://github.com/sponsors/fadillzzz", NULL, NULL, SW_SHOWDEFAULT);
            }

            ImGui::SameLine();

            if (ImGui::Button("Donate via Ko-fi")) {
                ShellExecute(NULL, L"open", L"https://ko-fi.com/fadzilla", NULL, NULL, SW_SHOWDEFAULT);
            }
        }
    } // namespace About
} // namespace Feats