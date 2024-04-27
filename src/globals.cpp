namespace Globals {
    static SDK::UHottaGameEngine *engine = nullptr;

    SDK::UHottaGameEngine *getEngine() {
        if (engine == nullptr) {
            engine = (SDK::UHottaGameEngine *)SDK::UEngine::GetEngine();
        }

        return engine;
    }

    SDK::UHottaGameInstance *getInstance() {
        if (getEngine() != nullptr) {
            return (SDK::UHottaGameInstance *)engine->GameInstance;
        }

        return nullptr;
    }

    SDK::UQRSLLocalPlayer *getLocalPlayer() {
        if (getInstance() != nullptr) {
            const auto localPlayers = getInstance()->LocalPlayers;

            if (localPlayers.Num() > 0) {
                return (SDK::UQRSLLocalPlayer *)localPlayers[0];
            }
        }

        return nullptr;
    }

    SDK::UWorld *getWorld() {
        if (getEngine() != nullptr) {
            if (getEngine()->GameViewport != nullptr) {
                return getEngine()->GameViewport->World;
            }
        }

        return nullptr;
    }

    SDK::AQRSLPlayerCharacter *getCharacter() {
        const auto localPlayer = getLocalPlayer();

        if (localPlayer == nullptr) {
            return nullptr;
        }

        if (localPlayer->PlayerController == nullptr) {
            return nullptr;
        }

        const auto playerController = (SDK::AQRSLPlayerController *)localPlayer->PlayerController;

        if (playerController->IsA(SDK::AQRSLPlayerController::StaticClass())) {
            return (SDK::AQRSLPlayerCharacter *)((SDK::AQRSLPlayerController *)playerController)->HottaPlayerOwner;
        }

        const auto character = (SDK::AQRSLPlayerCharacter *)playerController->Character;

        return character;
    }
} // namespace Globals
