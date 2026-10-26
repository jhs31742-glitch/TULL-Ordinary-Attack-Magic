#pragma once

namespace TOAM::Core::Runtime
{
    enum class GameState
    {
        kCold = 0,
        kDataLoaded,
        kGameReady
    };

    void OnDataLoaded() noexcept;
    void OnPreLoadGame() noexcept;
    void OnPostLoadGame() noexcept;
    void OnNewGame() noexcept;

    [[nodiscard]] GameState GetState() noexcept;
    [[nodiscard]] bool IsGameReady() noexcept;
}