// Copyright (c) 2017-2024 Chris Ohk

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Games/Game.hpp>
#include <Rosetta/Battlegrounds/Managers/GameManager.hpp>
#include <Rosetta/Battlegrounds/Models/Battle.hpp>

#include <effolkronium/random.hpp>

#include <sstream>
#include <stdexcept>
#include <utility>

using Random = effolkronium::random_thread_local;

namespace
{
class OpponentNotFound final : public std::logic_error
{
 public:
    using std::logic_error::logic_error;
};
}  // namespace

namespace RosettaStone::Battlegrounds
{
Game::Game(std::uint64_t seed) : m_seed(seed)
{
}

Game::Game(std::uint64_t seed, std::vector<std::string> supportedCardIDs)
    : m_seed(seed), m_supportedCardIDs(std::move(supportedCardIDs))
{
}

GameState& Game::GetGameState()
{
    return m_gameState;
}

std::string Game::CaptureRandomState() const
{
    std::ostringstream output;
    output << Random::engine();
    return output.str();
}

void Game::RestoreRandomState(const std::string& state) const
{
    std::istringstream input(state);
    input >> Random::engine();
    if (!input)
    {
        throw std::invalid_argument("Invalid Battlegrounds random state");
    }
}

void Game::Start()
{
    // A Game owns its full lobby state. Reset the scalar lobby state before
    // rebuilding the seeded lobby; Player/zone storage is initialized by the
    // setup loop below and is intentionally not copy-assigned (HandZone has a
    // const zone type and is therefore non-assignable).
    m_playerCount = 0;
    m_cardIndex = 0;
    m_playerFightPair.clear();
    m_gameState.phase = Phase::INVALID;
    m_gameState.nextPhase = Phase::INVALID;
    m_gameState.numRemainPlayer = NUM_BATTLEGROUNDS_PLAYERS;
    m_gameState.ghostPlayerIdx = std::numeric_limits<std::size_t>::max();

    if (m_seed.has_value())
    {
        Random::seed(m_seed.value());
    }

    // Card arrays must be populated before the minion pool reads them. Do not
    // rely on another test or game having initialized the singleton first.
    static_cast<void>(Cards::GetInstance());

    // Choose a race to exclude from the minion pool at random
    const auto raceIdx =
        Random::get<std::size_t>(0, RACES_IN_BATTLEGROUNDS.size() - 1);
    m_excludeRace = RACES_IN_BATTLEGROUNDS.at(raceIdx);

    // Initialize the minion pool
    if (m_supportedCardIDs.empty())
    {
        m_gameState.minionPool.Initialize(m_excludeRace);
    }
    else
    {
        m_gameState.minionPool.InitializeSupported(m_supportedCardIDs);
    }
    m_playerFightPair.reserve(NUM_BATTLEGROUNDS_PLAYERS / 2);

    // Create callback to increase player count and process next phase
    auto selectHeroCallback = [this](Player& player) {
        ++m_playerCount;

        player.hero.health = player.season14.heroPowerBatch1.StartingHealth(
            player.hero.card.GetHealth());

        if (m_playerCount >= NUM_BATTLEGROUNDS_PLAYERS)
        {
            // Set next phase
            m_gameState.nextPhase = Phase::RECRUIT;
            GameManager::ProcessNextPhase(*this, m_gameState.nextPhase);
        }
    };

    // Create callback to prepare a list of minions for purchase
    auto prepareTavernMinionsCallback = [this](Player& player) {
        m_gameState.minionPool.AddMinionsToTavern(player, player.tavern);
    };

    // Create callback to purchase a minion in Tavern
    auto purchaseMinionCallback = [](Player& player, std::size_t tavernIdx) {
        Minion minion =
            player.tavern.fieldZone.Remove(player.tavern.fieldZone[tavernIdx]);
        player.hand.Add(minion, -1);
    };

    // Create callback to get next card index
    auto getNextCardIndexCallback = [this]() -> int { return m_cardIndex++; };

    // Create callback to return a minion to the minion pool
    auto returnMinionCallback = [this](int poolIdx) {
        m_gameState.minionPool.ReturnMinion(poolIdx);
    };

    // Create callback to clear a list of minions in Tavern's field
    auto clearTavernMinionsCallback = [this](Player& player) {
        for (int i = player.tavern.fieldZone.GetCount() - 1; i >= 0; --i)
        {
            if (player.tavern.fieldZone[i].IsFrozen())
            {
                continue;
            }

            Minion minion = player.tavern.fieldZone.Remove(
                player.tavern.fieldZone[static_cast<std::size_t>(i)]);
            m_gameState.minionPool.ReturnMinion(minion.GetPoolIndex());
        }
    };

    // Create callback to upgrade player's Tavern to the next tier
    auto upgradeTavernCallback = [](Player& player) {
        ++player.currentTier;

        // Set the value of coin to upgrade player's Tavern to the next tier
        switch (player.currentTier)
        {
            case 2:
                player.coinToUpgradeTavern = NUM_COIN_UPGRADE_TAVERN_TIER_3;
                break;
            case 3:
                player.coinToUpgradeTavern = NUM_COIN_UPGRADE_TAVERN_TIER_4;
                break;
            case 4:
                player.coinToUpgradeTavern = NUM_COIN_UPGRADE_TAVERN_TIER_5;
                break;
            case 5:
                player.coinToUpgradeTavern = NUM_COIN_UPGRADE_TAVERN_TIER_6;
                break;
            case 6:
                player.coinToUpgradeTavern = 0;
                break;
            case 7:
                player.coinToUpgradeTavern = 0;
                break;
            default:
                throw std::logic_error("Invalid player's current tier");
        }
    };

    // Create callback to complete recruit and process next phase
    auto completeRecruitCallback = [this]() {
        ++m_playerCount;

        if (m_playerCount >= m_gameState.numRemainPlayer)
        {
            // Set next phase
            m_gameState.nextPhase = Phase::COMBAT;
            GameManager::ProcessNextPhase(*this, m_gameState.nextPhase);
        }
    };

    // Create callback to get opponent player
    auto getOpponentPlayerCallback = [this](Player& player) -> Player& {
        const std::size_t idx = FindPlayerNextFight(player.idx);
        return m_gameState.players.at(idx);
    };

    // Create callback to process the tasks related to defeat
    auto processDefeatCallback = [this](Player& player) {
        player.playState = PlayState::LOST;

        // Determine player's rank
        player.rank = m_gameState.numRemainPlayer;
        --m_gameState.numRemainPlayer;

        player.tavern.fieldZone.ForEach([this](MinionData& minion) {
            m_gameState.minionPool.ReturnMinion(minion.value().GetPoolIndex());
        });

        player.hand.ForEach([this](std::optional<CardData>& card) {
            if (std::holds_alternative<Minion>(card.value()))
            {
                const auto minion = std::get<Minion>(card.value());
                m_gameState.minionPool.ReturnMinion(minion.GetPoolIndex());
            }
        });

        player.recruitField.ForEach([this](MinionData& minion) {
            m_gameState.minionPool.ReturnMinion(minion.value().GetPoolIndex());
        });

        m_gameState.ghostPlayerIdx = player.idx;
    };

    std::size_t playerIdx = 0;

    // Initialize variables and callbacks
    for (auto& player : m_gameState.players)
    {
        player.playState = PlayState::PLAYING;
        player.idx = playerIdx;

        player.remainCoin = 0;
        player.totalCoin = 2;
        player.currentTier = 1;
        player.coinToUpgradeTavern = NUM_COIN_UPGRADE_TAVERN_TIER_2 + 1;

        player.selectHeroCallback = selectHeroCallback;
        player.prepareTavernMinionsCallback = prepareTavernMinionsCallback;
        player.purchaseMinionCallback = purchaseMinionCallback;
        player.getNextCardIndexCallback = getNextCardIndexCallback;
        player.returnMinionCallback = returnMinionCallback;
        player.clearTavernMinionsCallback = clearTavernMinionsCallback;
        player.upgradeTavernCallback = upgradeTavernCallback;
        player.completeRecruitCallback = completeRecruitCallback;
        player.getOpponentPlayerCallback = getOpponentPlayerCallback;
        player.processDefeatCallback = processDefeatCallback;

        ++playerIdx;
    }

    // Set next phase
    m_gameState.nextPhase = Phase::SELECT_HERO;
    GameManager::ProcessNextPhase(*this, m_gameState.nextPhase);
}

void Game::SelectHero()
{
    // Shuffle current heroes
    auto currentHeroes = Cards::GetInstance().GetCurrentHeroes();
    Random::shuffle(currentHeroes.begin(), currentHeroes.end());

    // Assign 4 heroes to each player
    std::size_t heroIdx = 0;
    for (auto& player : m_gameState.players)
    {
        for (std::size_t i = 0; i < NUM_HEROES_ON_SELECTION_LIST; ++i)
        {
            player.heroChoices.at(i) = currentHeroes.at(heroIdx + i).dbfID;
        }

        heroIdx += NUM_HEROES_ON_SELECTION_LIST;
    }
}

void Game::Recruit()
{
    // Check this game is over
    if (m_gameState.numRemainPlayer == 1)
    {
        // Set next phase
        m_gameState.nextPhase = Phase::GAMEOVER;
        GameManager::ProcessNextPhase(*this, m_gameState.nextPhase);
        return;
    }

    // Initialize player count for callback
    m_playerCount = 0;

    // Determine each player's opponent
    DetermineOpponent();

    for (auto& player : m_gameState.players)
    {
        if (player.playState != PlayState::PLAYING)
        {
            continue;
        }

        // Recruit actions and recruit-phase task selectors operate on the
        // recruit field.  Combat sets this flag true; clear it here before
        // any turn-start effects or Tavern work so GetField() cannot route
        // recruit summons/effects into the copied battle field.
        player.isInCombat = false;

        player.season14.Emit(Season14Event::RECRUIT_START);
        player.season14.heroPowerUsed = false;

        // Assign the index of the player to fight next.
        player.playerIdxNextFight = FindPlayerNextFight(player.idx);

        // Set the value of coin (remain/total)
        if (player.totalCoin <
            player.season14.EffectiveMaxGold(COIN_UPPER_LIMIT))
        {
            ++player.totalCoin;
        }
        player.remainCoin = player.totalCoin +
                            player.season14.TakeNextTurnGold();

        const auto heroPowerResult = player.season14.BeginRecruitTurn();
        player.remainCoin += heroPowerResult.goldDelta;

        // Decrease the value of coin to upgrade player's Tavern to next tier
        if (player.currentTier < TIER_UPPER_LIMIT)
        {
            player.coinToUpgradeTavern =
                std::max(0, player.coinToUpgradeTavern - 1);
        }

        const bool manuallyFrozen = player.freezeTavern;

        // Return only cards that were not preserved. Frozen state belongs to
        // each Tavern entity rather than to the Tavern as a whole.
        for (int i = player.tavern.fieldZone.GetCount() - 1; i >= 0; --i)
        {
            if (player.tavern.fieldZone[i].IsFrozen())
            {
                continue;
            }

            Minion minion = player.tavern.fieldZone.Remove(
                player.tavern.fieldZone[static_cast<std::size_t>(i)]);
            m_gameState.minionPool.ReturnMinion(minion.GetPoolIndex());
        }

        // A manual whole-shop freeze preserves exactly the remaining cards.
        // Independently frozen entities do not prevent ordinary vacancies
        // from being filled on a normal end turn.
        if (!manuallyFrozen)
        {
            player.PrepareTavern();
        }

        // Consume the one-turn frozen state.
        player.tavern.fieldZone.ForEach(
            [](MinionData& minion) { minion.value().SetFrozen(false); });
        player.freezeTavern = false;
    }
}

void Game::CompleteRecruitPhase()
{
    if (m_gameState.phase != Phase::RECRUIT)
    {
        throw std::logic_error("Cannot complete recruit outside recruit phase");
    }

    for (auto& player : m_gameState.players)
    {
        if (player.playState == PlayState::PLAYING)
        {
            if (player.season14.ShouldFreezeRemainingTavern())
            {
                player.tavern.fieldZone.ForEach(
                    [](MinionData& minion) {
                        minion.value().SetFrozen(true);
                    });
            }
            player.season14.Emit(Season14Event::RECRUIT_END);
        }
    }
    m_playerCount = 0;
    m_gameState.nextPhase = Phase::COMBAT;
    GameManager::ProcessNextPhase(*this, m_gameState.nextPhase);
}

void Game::Combat()
{
    for (auto& player : m_gameState.players)
    {
        // Set the flag
        player.isInCombat = true;
        if (player.playState == PlayState::PLAYING)
        {
            player.season14.Emit(Season14Event::COMBAT_START);
        }
    }

    // Simulates a battle for each pair
    for (const auto& pair : m_playerFightPair)
    {
        Player& player1 = m_gameState.players.at(std::get<0>(pair));
        Player& player2 = m_gameState.players.at(std::get<1>(pair));

        Battle battle(player1, player2);

        // Create callback to get battle
        player1.getBattleCallback = [&battle]() -> Battle& { return battle; };
        player2.getBattleCallback = [&battle]() -> Battle& { return battle; };

        battle.Run();

        if (player1.playState == PlayState::PLAYING)
        {
            player1.season14.Emit(Season14Event::COMBAT_END);
        }
        if (player2.playState == PlayState::PLAYING)
        {
            player2.season14.Emit(Season14Event::COMBAT_END);
        }

        const auto player1Idx = std::get<0>(pair);
        const auto player2Idx = std::get<1>(pair);
        const bool player1Ghost = player1.playState != PlayState::PLAYING;
        const bool player2Ghost = player2.playState != PlayState::PLAYING;
        if (!player1Ghost)
        {
            player1.playerIdxFoughtLastTurn = player2Idx;
            player1.isFoughtGhostLastTurn = player2Ghost;
        }
        if (!player2Ghost)
        {
            player2.playerIdxFoughtLastTurn = player1Idx;
            player2.isFoughtGhostLastTurn = player1Ghost;
        }
    }

    // Set next phase
    m_gameState.nextPhase = Phase::RECRUIT;
    GameManager::ProcessNextPhase(*this, m_gameState.nextPhase);
}

void Game::GameOver()
{
    for (auto& player : m_gameState.players)
    {
        if (player.playState == PlayState::PLAYING)
        {
            player.playState = PlayState::WON;
            player.rank = 1;
        }
    }
    m_gameState.phase = Phase::COMPLETE;
}

void Game::DetermineOpponent()
{
    // NOTE: Random player that you didn't fight. If there is an odd number of
    // players alive, bottom 3 have a chance to play the ghost. Can't fight a
    // ghost 2 turns in a row. Ghost is the 1 of the most recent players to die.
    m_playerFightPair.clear();

    auto playerData = CalculateRank();

    // Check there is an odd number of players alive
    if (playerData.size() % 2 == 1)
    {
        // Determine player to fight the ghost
        const std::size_t playerIdx = DeterminePlayerToFightGhost(playerData);
        m_playerFightPair.emplace_back(
            std::make_tuple(playerIdx, m_gameState.ghostPlayerIdx));

        // Pair a list of players
        PairPlayers(playerData);
    }
    else
    {
        // Pair a list of players
        PairPlayers(playerData);
    }
}

std::vector<std::tuple<int, int>> Game::CalculateRank()
{
    std::vector<std::tuple<int, int>> playerData;
    playerData.reserve(NUM_BATTLEGROUNDS_PLAYERS);

    for (const auto& player : m_gameState.players)
    {
        if (player.playState != PlayState::PLAYING)
        {
            continue;
        }

        playerData.emplace_back(
            std::make_tuple(player.idx, player.hero.health));
    }

    std::sort(playerData.begin(), playerData.end(),
              [](std::tuple<int, int> a, std::tuple<int, int> b) {
                  return std::get<1>(a) > std::get<1>(b);
              });

    return playerData;
}

std::size_t Game::DeterminePlayerToFightGhost(
    std::vector<std::tuple<int, int>>& playerData)
{
    // Bottom 3 have a chance to play the ghost
    std::vector<int> ghostCandidates;

    const std::size_t firstCandidate =
        playerData.size() > 3 ? playerData.size() - 3 : 0;
    for (std::size_t i = firstCandidate; i < playerData.size(); ++i)
    {
        const int playerIdx = std::get<0>(playerData.at(i));
        // Can't fight a ghost 2 turns in a row
        if (m_gameState.players.at(playerIdx).isFoughtGhostLastTurn)
        {
            continue;
        }

        ghostCandidates.emplace_back(playerIdx);
    }

    // With very few players, the no-repeat constraint may exclude everyone.
    if (ghostCandidates.empty())
    {
        for (std::size_t i = firstCandidate; i < playerData.size(); ++i)
        {
            ghostCandidates.emplace_back(std::get<0>(playerData.at(i)));
        }
    }

    // Fight randomly selected player and the ghost
    const std::size_t choice =
        Random::get<std::size_t>(0, ghostCandidates.size() - 1);
    const int playerIdx = ghostCandidates.at(choice);

    // Remove the index of randomly selected player from player data
    playerData.erase(std::remove_if(playerData.begin(), playerData.end(),
                                    [playerIdx](std::tuple<int, int> data) {
                                        return std::get<0>(data) == playerIdx;
                                    }),
                     playerData.end());

    return static_cast<std::size_t>(playerIdx);
}

void Game::SetPlayerPair(int player1Idx, int player2Idx)
{
    m_playerFightPair.emplace_back(std::make_tuple(player1Idx, player2Idx));
}

void Game::PairPlayers(std::vector<std::tuple<int, int>>& playerData)
{
    Random::shuffle(playerData.begin(), playerData.end());
    for (std::size_t i = 0; i < playerData.size(); i += 2)
    {
        const int player1Idx = std::get<0>(playerData.at(i));
        const int player2Idx = std::get<0>(playerData.at(i + 1));
        m_playerFightPair.emplace_back(std::make_tuple(player1Idx, player2Idx));
    }
}

std::size_t Game::FindPlayerNextFight(std::size_t playerIdx)
{
    for (const auto& fightPair : m_playerFightPair)
    {
        if (std::get<0>(fightPair) == playerIdx)
        {
            return std::get<1>(fightPair);
        }

        if (std::get<1>(fightPair) == playerIdx)
        {
            return std::get<0>(fightPair);
        }
    }

    throw OpponentNotFound("Opponent player not found for player " +
                           std::to_string(playerIdx));
}
}  // namespace RosettaStone::Battlegrounds
