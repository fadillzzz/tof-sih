#include "quest.hpp"
#include "../globals.hpp"
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
        void tick() { return; }
        void menu() {
            if (ImGui::Button("Complete main quest(s)")) {
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

            ImGui::SameLine();

            if (ImGui::Button("Complete Daily")) {
                completeQuestsWithFilter(std::regex("rv\\d{6}"));
            }

            ImGui::SameLine();

            if (ImGui::Button("Complete Weekly")) {
                completeQuestsWithFilter(std::regex("[aA]ctivityquest\\d{3}"));
            }

            ImGui::SameLine();

            if (ImGui::Button("Complete all quests")) {
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
        }
    } // namespace Quest
} // namespace Feats
