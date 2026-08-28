// Copyright (c) 2019 Chris Ohk, Youngjoong Kim, SeungHyun Jeon

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#ifndef ROSETTASTONE_BATTLEGROUNDS_GAME_HPP
#define ROSETTASTONE_BATTLEGROUNDS_GAME_HPP

#include <Rosetta/Battlegrounds/Games/GameState.hpp>

#include <atomic>
#include <cstdint>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

namespace RosettaStone::Battlegrounds
{
//!
//! \brief Game class.
//!
//! This class processes the game, Hearthstone: Battlegrounds. Battlegrounds is
//! a game mode where eight players face off in 1v1 rounds, with the goal to be
//! the last player standing. Each round consists of two phases.
//!
class Game
{
 public:
    //! Default constructor using a nondeterministic process seed.
    Game() = default;

    //! Constructs a reproducible game.
    //! \param seed The seed used by Battlegrounds random operations.
    explicit Game(std::uint64_t seed);

    //! Constructs a seeded game with an explicitly supported minion pool.
    Game(std::uint64_t seed, std::vector<std::string> supportedCardIDs);

    //! Gets the game state.
    //! \return The game state.
    GameState& GetGameState();

    //! Captures the Battlegrounds random engine for deterministic interleaving.
    //! \return A serialized random engine state.
    std::string CaptureRandomState() const;

    //! Restores a previously captured Battlegrounds random engine state.
    //! \param state The serialized random engine state.
    void RestoreRandomState(const std::string& state) const;

    //! Starts the game.
    void Start();

    //! Processes the select hero phase.
    void SelectHero();

    //! Processes the recruit phase.
    void Recruit();

    //! Completes the current recruit phase after all living players have
    //! submitted their decisions.
    void CompleteRecruitPhase();

    //! Processes the combat phase.
    void Combat();

    //! Processes the game over phase.
    void GameOver();

    //! Determines each player's opponent.
    void DetermineOpponent();

    //! Calculates the rank of players according to their health.
    //! \return A container that stores the player index and the rank.
    std::vector<std::tuple<int, int>> CalculateRank();

    //! Determines the player to fight the ghost.
    //! \param playerData The player data that stores index and rank.
    //! \return The index of player to fight the ghost.
    std::size_t DeterminePlayerToFightGhost(
        std::vector<std::tuple<int, int>>& playerData);

    //! Sets a player pair to fight next.
    //! \param player1Idx The index of first player.
    //! \param player2Idx The index of second player.
    void SetPlayerPair(int player1Idx, int player2Idx);

    //! Pairs a list of players.
    //! \param playerData The player data that stores index and rank.
    void PairPlayers(std::vector<std::tuple<int, int>>& playerData);

    //! Finds the index of the opponent player to fight next.
    //! \param playerIdx The index of the player to find opponent.
    //! \return The index of the opponent player to fight next.
    std::size_t FindPlayerNextFight(std::size_t playerIdx);

 private:
    std::optional<std::uint64_t> m_seed;
    std::vector<std::string> m_supportedCardIDs;
    GameState m_gameState{};

    Race m_excludeRace = Race::INVALID;
    std::vector<std::tuple<std::size_t, std::size_t>> m_playerFightPair;
    std::atomic<int> m_playerCount = 0;
    std::atomic<int> m_cardIndex = 0;
};
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_GAME_HPP
