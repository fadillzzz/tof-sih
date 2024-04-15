#include "quest.hpp"
#include "../globals.hpp"
#include "../logger/logger.hpp"
#include "hotkey.hpp"
#include <regex>

namespace Feats {
    namespace Quest {
        void completeQuestsWithFilter(std::regex filter) {
            const auto character = Globals::getCharacter();

            if (character != nullptr) {
                const auto questComponent = character->QuestComponent;

                if (questComponent != nullptr) {
                    const auto quests = questComponent->QuestsInProgress;

                    for (auto &quest : quests) {
                        if (std::regex_match(quest.QuestID.ToString(), filter)) {
                            questComponent->GM_CompleteQuestObject(quest.QuestID);
                        }
                    }
                }
            }
        }

        void init() { return; }

        void completeMain() {
            const auto character = Globals::getCharacter();

            if (character != nullptr) {
                const auto questComponent = character->QuestComponent;

                if (questComponent != nullptr) {
                    const auto mainQuests = questComponent->GetCurMainQuest();

                    for (auto mainQuest : mainQuests) {
                        questComponent->GM_CompleteQuestObject(mainQuest);
                    }
                }
            }
        }

        void completeDaily() {
            // Aesperia daily bounties
            completeQuestsWithFilter(std::regex("q\\d{6}"));
            // Vera daily bounties
            completeQuestsWithFilter(std::regex("rv\\d{6}"));
            // Domain 9 daily bounties
            completeQuestsWithFilter(std::regex("jy\\d{6}"));
        }

        void completeWeekly() {
            // Weekly activities
            completeQuestsWithFilter(std::regex("[aA]ctivityquest\\d{3}"));
            // Crew missions
            completeQuestsWithFilter(std::regex("gh\\d{6}"));
            // Mirroria weekly commissions
            completeQuestsWithFilter(std::regex("SA\\d{6}"));
        }

        void completeAll() {
            const auto character = Globals::getCharacter();

            if (character != nullptr) {
                const auto questComponent = character->QuestComponent;

                if (questComponent != nullptr) {
                    const auto quests = questComponent->QuestsInProgress;

                    for (auto &quest : quests) {
                        questComponent->GM_CompleteQuestObject(quest.QuestID);
                    }
                }
            }
        }

        void tick() {
            if (Feats::Hotkey::hotkeyPressed(confActivateMain)) {
                completeMain();
            }

            if (Feats::Hotkey::hotkeyPressed(confActivateDaily)) {
                completeDaily();
            }

            if (Feats::Hotkey::hotkeyPressed(confActivateWeekly)) {
                completeWeekly();
            }

            if (Feats::Hotkey::hotkeyPressed(confActivateAll)) {
                completeAll();
            }
        }

        void menu() {
            if (ImGui::Button("Complete main quest(s)")) {
                completeMain();
            }

            ImGui::SameLine();

            if (ImGui::Button("Complete Daily")) {
                completeDaily();
            }

            ImGui::SameLine();

            if (ImGui::Button("Complete Weekly")) {
                completeWeekly();
            }

            ImGui::SameLine();

            if (ImGui::Button("Complete all quests")) {
                completeAll();
            }
        }
    } // namespace Quest
} // namespace Feats
