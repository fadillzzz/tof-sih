#include "quest.hpp"
#include "../globals.hpp"

namespace Feats {
    namespace Quest {
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
