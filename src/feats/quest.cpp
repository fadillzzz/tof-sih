#include "quest.hpp"
#include "../globals.hpp"
#include "../logger/logger.hpp"
#include "hotkey.hpp"
#include <regex>

namespace Feats {
    namespace Quest {
        Config::field<bool> allExceptMainEnabled;

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

        void init() { allExceptMainEnabled = Config::get<bool>(confActivateAllExceptMainEnabled, true); }

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

        void completeCrewMissions() {
            // Crew missions
            completeQuestsWithFilter(std::regex("gh\\d{6}"));
        }

        void completeDaily() {
            // Aesperia daily bounties
            completeQuestsWithFilter(std::regex("q\\d{6}"));
            // Vera daily bounties
            completeQuestsWithFilter(std::regex("rv\\d{6}"));
            // Domain 9 daily bounties
            completeQuestsWithFilter(std::regex("jy\\d{6}"));
            // Gesthos Network daily bounties
            completeQuestsWithFilter(std::regex("no\\d{6}"));
        }

        void completeWeekly() {
            // Weekly activities
            completeQuestsWithFilter(std::regex("[aA]ctivityquest\\d{3}"));
            // Mirroria weekly commissions
            completeQuestsWithFilter(std::regex("SA\\d{6}"));
        }

        void completeAll(bool excludeMain) {
            const auto character = Globals::getCharacter();

            if (character != nullptr) {
                const auto questComponent = character->QuestComponent;

                if (questComponent != nullptr) {
                    const auto quests = questComponent->QuestsInProgress;
                    const auto mainQuests = questComponent->GetCurMainQuest();
                    std::vector<std::string> mainQuestIds = {};

                    for (auto mainQuest : mainQuests) {
                        mainQuestIds.push_back(mainQuest.ToString());
                    }

                    for (auto &quest : quests) {
                        if (excludeMain && std::find(mainQuestIds.begin(), mainQuestIds.end(),
                                                     quest.QuestID.ToString()) != mainQuestIds.end()) {
                            continue;
                        }

                        if (excludeMain && quest.QuestID.ToString().find("v104059") != std::string::npos) {
                            continue;
                        }

                        questComponent->GM_CompleteQuestObject(quest.QuestID);
                    }
                }
            }
        }

        void submitAllPending() {
            const auto character = Globals::getCharacter();

            if (character != nullptr) {
                const auto questComponent = character->QuestComponent;

                if (questComponent != nullptr) {
                    const auto pendingQuests = questComponent->QuestsInProgress;

                    for (auto &quest : pendingQuests) {
                        if (quest.QuestStatus == SDK::EQRSLQuestStatus::EQS_CAN_SUBMIT) {
                            questComponent->SubmitQuest(quest.QuestID);
                        }
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
                completeAll(*allExceptMainEnabled);
            }
        }

        void menu() {
            if (ImGui::Button("Complete main quest(s)")) {
                completeMain();
            }

            if (ImGui::Button("Complete Daily")) {
                completeDaily();
            }

            if (ImGui::Button("Complete Weekly")) {
                completeWeekly();
            }

            if (ImGui::Button("Complete crew missions")) {
                completeCrewMissions();
            }

            if (ImGui::Button("Complete all quests")) {
                completeAll(*allExceptMainEnabled);
            }

            if (ImGui::Button("Submit pending completed quests")) {
                submitAllPending();
            }

            ImGui::SameLine();

            ImGui::Checkbox("Exclude main quest(s)", &allExceptMainEnabled);
        }
    } // namespace Quest
} // namespace Feats
