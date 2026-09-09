// Copyright (c) 2017-2024 Chris Ohk

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Cards/CardDefs.hpp>
#include <Rosetta/Battlegrounds/Games/Game.hpp>
#include <Rosetta/Battlegrounds/CardSets/TrinketBehaviors.hpp>
#include <Rosetta/Battlegrounds/Managers/GameManager.hpp>
#include <Rosetta/Battlegrounds/Models/Battle.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomCardToHandTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ActivateRandomTavernSpellsTask.hpp>

#include <effolkronium/random.hpp>

#include <sstream>
#include <stdexcept>
#include <cassert>
#include <algorithm>
#include <limits>
#include <unordered_set>
#include <utility>

using Random = effolkronium::random_thread_local;

namespace
{
class OpponentNotFound final : public std::logic_error
{
 public:
    using std::logic_error::logic_error;
};

// Rank/pairing keeps its historical int-based tuple representation, while
// Player::idx is size_t.  Player indices are bounded by the fixed lobby size;
// keep the conversion explicit and make that invariant checkable in debug
// builds instead of relying on an implicit narrowing conversion.
int PlayerIndexForRank(std::size_t index)
{
    assert(index <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
    return static_cast<int>(index);
}

int ZonePositionForIndex(std::size_t index)
{
    assert(index <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
    return static_cast<int>(index);
}
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
        const auto preferredRace = player.season14.TakeRefreshRace();
        m_gameState.minionPool.AddMinionsToTavern(player, player.tavern,
                                                  preferredRace);
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

    auto replaceTavernMinionWithRaceCallback = [this](
        Player& player, Tavern& tavern, std::size_t slot, Race race) {
        if (slot >= static_cast<std::size_t>(tavern.fieldZone.GetCount()) ||
            tavern.fieldZone[slot].IsFrozen())
            return false;
        auto candidates = m_gameState.minionPool.GetMinions(1, player.currentTier, true);
        candidates.erase(std::remove_if(candidates.begin(), candidates.end(),
            [race](const Minion& minion) { return !minion.HasRace(race); }),
            candidates.end());
        if (candidates.empty()) return false;
        Random::shuffle(candidates.begin(), candidates.end());
        auto replacement = std::move(candidates.front());
        if (!m_gameState.minionPool.TakeMinion(replacement.GetPoolIndex()))
            return false;
        auto old = tavern.fieldZone.Remove(tavern.fieldZone[slot]);
        m_gameState.minionPool.ReturnMinion(old.GetPoolIndex());
        player.ApplyFreshTavernMinionModifiers(replacement);
        tavern.fieldZone.Add(replacement, ZonePositionForIndex(slot));
        return true;
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
        for (int i = static_cast<int>(player.tavern.spellSlots.size()) - 1; i >= 0; --i)
        {
            if (player.tavern.spellSlots[static_cast<std::size_t>(i)].IsFrozen()) continue;
            player.tavern.spellSlots.erase(player.tavern.spellSlots.begin() + i);
        }
    };
    auto addRandomTavernMinionCallback = [this](Player& player, int tier) {
        return m_gameState.minionPool.AddRandomMinionToTavern(player, player.tavern, tier);
    };
    auto addRandomRaceTavernMinionCallback = [this](
        Player& player, int tier, Race race) {
        return m_gameState.minionPool.AddRandomRaceMinionToTavern(
            player, player.tavern, tier, race);
    };
    auto addRandomMinionToHandCallback = [this](Player& player, int tier) {
        if (player.hand.IsFull()) return false;
        auto candidates = m_gameState.minionPool.GetMinions(tier, tier, true);
        if (candidates.empty()) return false;
        const auto pick = Random::get<std::size_t>(0, candidates.size() - 1);
        return player.AddMinionCopyToHand(candidates[pick]);
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

    auto getLowestHealthOpponentPlayerCallback = [this](Player& player) -> Player& {
        Player* selected = nullptr;
        for (auto& candidate : m_gameState.players) {
            if (&candidate == &player || candidate.playState != PlayState::PLAYING)
                continue;
            if (selected == nullptr || candidate.hero.health < selected->hero.health)
                selected = &candidate;
        }
        return selected != nullptr ? *selected : player;
    };

    // Create callback to process the tasks related to defeat
    auto processDefeatCallback = [this](Player& player) {
        // Kel'Thuzad's Kitty resolves against the defeated player's public
        // warband before the pool/board cleanup below removes its entities.
        for (auto& observer : m_gameState.players) {
            if (observer.season14.heroPowerDbfID != 63320 ||
                observer.hand.IsFull() ||
                observer.season14.pendingDecision != Season14Decision::NONE)
                continue;
            std::vector<Season14Offering> offerings;
            player.recruitField.ForEachAlive([&offerings](MinionData& data) {
                const auto& minion = data.value();
                const auto card = Cards::FindCardByID(minion.GetCardID());
                if (card.dbfID != 0 && card.normalDbfID == 0 &&
                    card.GetCardType() == CardType::MINION && card.hasBehavior)
                    offerings.push_back({card.dbfID, 0});
            });
            if (!offerings.empty()) {
                Random::shuffle(offerings.begin(), offerings.end());
                if (offerings.size() > 3) offerings.resize(3);
                observer.season14.BeginOfferingDecision(
                    Season14Decision::DISCOVER, 0, 63320,
                    std::move(offerings));
            }
        }
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
        // Start() is also the deterministic replay/reset boundary.  Season 14
        // counters are player-owned (including BG28_884's pending combat
        // reward), so reconstruct them before the new lobby is initialized;
        // otherwise a replay can leak queued gold or a consumed arm from the
        // previous run.
        player.season14 = Season14State{};
        player.playState = PlayState::PLAYING;
        player.idx = playerIdx;

        player.remainCoin = 0;
        player.totalCoin = 2;
        player.currentTier = 1;
        player.coinToUpgradeTavern = NUM_COIN_UPGRADE_TAVERN_TIER_2 + 1;

        player.selectHeroCallback = selectHeroCallback;
        player.prepareTavernMinionsCallback = prepareTavernMinionsCallback;
        player.purchaseMinionCallback = purchaseMinionCallback;
        player.addRandomTavernMinionCallback = addRandomTavernMinionCallback;
        player.addRandomRaceTavernMinionCallback =
            addRandomRaceTavernMinionCallback;
        player.addRandomMinionToHandCallback = addRandomMinionToHandCallback;
        player.getNextCardIndexCallback = getNextCardIndexCallback;
        player.returnMinionCallback = returnMinionCallback;
        player.replaceTavernMinionWithRaceCallback =
            replaceTavernMinionWithRaceCallback;
        player.clearTavernMinionsCallback = clearTavernMinionsCallback;
        player.hand.SetAddCallback([playerPtr = &player](const CardData& card) {
            playerPtr->OnCardAcquired(card);
        });
        player.hand.SetMinionAddCallback([playerPtr = &player](Minion& minion) {
            const bool retainedBySamePlayer =
                minion.GetOwnerToken() == static_cast<const void*>(playerPtr);
            minion.getPlayerCallback = [playerPtr]() -> Player& { return *playerPtr; };
            minion.SetOwnerToken(static_cast<const void*>(playerPtr));
            // Retain identity for a same-player zone move, but never let a
            // card copied/transferred from another player's hand retain the
            // old player's entity ID.
            if ((!retainedBySamePlayer || minion.GetIndex() < 0) &&
                playerPtr->getNextCardIndexCallback)
                minion.SetIndex(playerPtr->getNextCardIndexCallback());
        });
        player.upgradeTavernCallback = upgradeTavernCallback;
        player.completeRecruitCallback = completeRecruitCallback;
        player.getOpponentPlayerCallback = getOpponentPlayerCallback;
        player.getLowestHealthOpponentPlayerCallback =
            getLowestHealthOpponentPlayerCallback;
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
    // A generated reward such as Purified Shard can end the game during the
    // recruit phase without going through combat elimination. Finalize that
    // terminal winner before pairing opponents; otherwise numRemainPlayer
    // still reflects the pre-reward lobby and the game can continue forever
    // with only non-playing seats left.
    if (std::any_of(m_gameState.players.begin(), m_gameState.players.end(),
                    [](const Player& player) {
                        return player.playState == PlayState::WON;
                    }))
    {
        m_gameState.nextPhase = Phase::GAMEOVER;
        GameManager::ProcessNextPhase(*this, m_gameState.nextPhase);
        return;
    }
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
        player.ApplyDeferredTavernSpellStats();
        player.season14.firstMinionPlayedThisTurn = false;
        player.season14.battlecryBuysThisTurn = 0;
        player.season14.minionsPlayedThisTurn = 0;
        player.season14.repeatedPlayCardIDs.clear();
        // Cliffdiver Sticker scales from Battlecries triggered this recruit
        // turn, never from the lifetime Battlecry counter.
        player.season14.battlecriesTriggered = 0;
        player.season14.heroPowerUsed = false;
        player.season14.heroPowerBatch3State = 0;
        player.recruitField.ForEach([](MinionData& minion) {
            minion.value().ResetActivateUses();
            minion.value().ResetBuyTriggerUses();
        });
        // Per-turn purchase observers own their cadence on the persistent
        // effect, not on a transient board Minion. Reset Magicfin's cap only
        // at the recruit boundary so duplicate Stickers remain independent.
        for (auto& trinket : player.season14.trinkets) {
            if (!trinket.active || trinket.remainingUses == 0) continue;
            const auto card = Cards::FindCardByDbfID(trinket.dbfID);
            if (FindTrinketBehavior(card.id).effect ==
                TrinketEffect::AFTER_BUY_TAVERN_SPELL_MURLOC ||
                FindTrinketBehavior(card.id).effect ==
                    TrinketEffect::AFTER_BUY_MINION_MAGNETIC_SATELLITE)
                trinket.triggerProgress = 0;
        }

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
        player.remainCoin += player.season14.TakeImmediateGold();

        // Glowing Crystal grants one Gold per distinct friendly minion type
        // at the start of each recruit turn.  Count only concrete types; an
        // ALL/multi-type minion contributes once for each type it has.
        for (const auto& trinket : player.season14.trinkets)
        {
            if (!trinket.active || trinket.remainingUses == 0) continue;
            const auto card = Cards::FindCardByDbfID(trinket.dbfID);
            if (FindTrinketBehavior(card.id).effect !=
                TrinketEffect::START_TURN_GOLD_PER_MINION_TYPE)
                continue;
            std::unordered_set<Race> types;
            player.recruitField.ForEachAlive([&](const MinionData& data) {
                for (const Race race : RACES_IN_BATTLEGROUNDS)
                    if (data.value().HasRace(race)) types.insert(race);
            });
            player.remainCoin += static_cast<int>(types.size());
        }
        // Start-turn random Trinket grants use the same seeded pool and
        // hand-capacity path as acquisition-time grants.
        player.GrantTrinketStartTurnCards();

        player.RefreshSpellcraft();
        player.hand.ForEach([](std::optional<CardData>& card) {
            if (card.has_value() && std::holds_alternative<Minion>(card.value()))
                std::get<Minion>(card.value()).SetHandLocked(false);
        });

        const auto heroPowerResult = player.season14.BeginRecruitTurn();
        player.RefreshSousChefHeroPowerUses();
        player.remainCoin += heroPowerResult.goldDelta;
        // Nether Portal is a passive start-of-turn reward.  Use the shared
        // seeded pool task so hand capacity, executable-pool filtering, and
        // replay RNG semantics remain identical to other random acquisitions.
        if (player.season14.heroPowerDbfID == 123087)
            (void)SimpleTasks::RandomCardToHandTask{Race::DEMON, 0, 2}.Run(player);
        // Puzzle Box unlocks on Turn 3. Target-free executable Tavern spells
        // are safe to resolve automatically; target-requiring spells remain
        // fail-closed until a public target modal exists.
        if (player.season14.heroPowerDbfID == 122958 &&
            player.season14.recruitTurnNumber >= 3)
            (void)SimpleTasks::ActivateRandomTavernSpellsTask{1}.Run(player);
        player.ResolveStartTurnTrinkets();
        player.TryResolveWarpGateReward();
        player.BeginSpawningPoolMorphChoice();
        player.BeginOrnateClockOffer();
        player.BeginFantasticTreasureOffer();
        player.recruitField.ForEachAlive([](MinionData& data) {
            if (data.value().HasTimeTurning())
                data.value().ActivateTrigger(TriggerType::TURN_END, data.value());
        });
        player.remainCoin += player.season14.ResolveDelayedTrinketGold();
        player.ResolveGeneratedQuestRewardStartTurn();
        player.ResolveRelicsOfTheDeepStartTurn();
        if (player.season14.imprisonedTurns == 0) {
            int imprisonedSlot = -1;
            for (int slot = 0; slot < player.tavern.fieldZone.GetCount(); ++slot) {
                const auto& card = player.tavern.fieldZone[static_cast<std::size_t>(slot)];
                if (!card.IsDestroyed() && card.GetIndex() == player.season14.imprisonedEntityID) {
                    imprisonedSlot = slot;
                    break;
                }
            }
            // Older snapshots may have no entity identity; retain the slot
            // fallback for compatibility with those states.
            if (imprisonedSlot < 0 && player.season14.imprisonedEntityID < 0 &&
                player.season14.imprisonedSlot >= 0 &&
                player.season14.imprisonedSlot < player.tavern.fieldZone.GetCount())
                imprisonedSlot = player.season14.imprisonedSlot;
            if (imprisonedSlot >= 0)
                player.tavern.fieldZone[static_cast<std::size_t>(imprisonedSlot)].SetFrozen(false);
            player.season14.imprisonedSlot = -1;
            player.season14.imprisonedEntityID = -1;
        }
        // Power of the Storm presents two new public hero-power options at
        // every recruit start; the modal remains pending until selected.
        player.BeginPowerOfStormChoice();
        player.ResolveWardenBuddy();
        player.ResolveHunterOfOldBuddy();
        player.ResolveLilKTMinions();
        // Dungar flightpaths advance once per recruit turn and resolve before
        // the player receives the next decision.  Ironforge intentionally
        // leaves its Discover modal pending for the policy to choose.
        player.season14.AdvanceFlightpath();
        player.ResolveFlightpathCompletion();
        if (player.season14.TakeVoidPowerDiscoverReady() &&
            !player.BeginVoidPowerDiscover())
            player.season14.RestoreVoidPowerDiscoverReady();
        if (player.season14.TakeFeelDevastationDiscoverReady() &&
            !player.BeginFeelDevastationDiscover())
            player.season14.RestoreFeelDevastationDiscoverReady();

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
            for (int i = static_cast<int>(player.tavern.spellSlots.size()) - 1; i >= 0; --i)
            {
                if (player.tavern.spellSlots[static_cast<std::size_t>(i)].IsFrozen()) continue;
                player.tavern.spellSlots.erase(player.tavern.spellSlots.begin() + i);
            }
            player.PrepareTavern();
        }
        player.MaybeBeginExpeditionDiscovery();

        // Consume the one-turn frozen state.
        player.tavern.fieldZone.ForEach(
            [](MinionData& minion) { minion.value().SetFrozen(false); });
        for (auto &slot : player.tavern.spellSlots) slot.SetFrozen(false);
        player.freezeTavern = false;
        player.season14.ConsumeFirstRecruitTurnSkip();
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
            player.ResolveRecruitEndDeaths();
            player.ResolveLiftOffEndTurn();
            player.ResolveFodderDefilerEndTurn();
            player.ResolveEnigmaticHeadstoneEndTurn();
            player.ResolveTrinketEndTurn();
            player.ResolveGeneratedQuestRewardSnickerSnacks();
            // Advance persistent end-of-turn counters (including Patient
            // Scout's tier improvement) before combat begins.
            // Drakkari Enchanter multiplies minion end-of-turn effects. The
            // strongest copy controls the multiplier: normal is 2x and golden
            // is 3x; duplicate copies do not compound the same aura.
            int drakkariPasses = 1;
            player.recruitField.ForEachAlive([&drakkariPasses](MinionData& data) {
                if (data.value().GetCardID() == "BG26_ICC_901") drakkariPasses = std::max(drakkariPasses, 2);
                else if (data.value().GetCardID() == "BG26_ICC_901_G") drakkariPasses = std::max(drakkariPasses, 3);
            });
            for (int pass = 0; pass < drakkariPasses; ++pass)
            {
                player.ResolveGeneratedQuestRewardEndTurn();
                player.ResolveDarkGiftEndTurnTriggers();
                // Tiny Henchmen is itself an end-of-turn effect, so it must
                // participate in the same Drakkari repeat boundary as other
                // end-of-turn triggers.
                player.ResolveGeneratedQuestRewardTinyHenchmen();
            }
            player.ResolveSulfurasEndTurn();
            player.ResolveCthunEndTurn();
            player.AdvanceCthunUpgrade();
            if (player.season14.ShouldFreezeRemainingTavern())
            {
                player.tavern.fieldZone.ForEach(
                    [](MinionData& minion) {
                        minion.value().SetFrozen(true);
                    });
                for (auto &slot : player.tavern.spellSlots) slot.SetFrozen(true);
            }
            // End-of-turn Trinkets resolve after the final recruit action.
            // Wallet increases the cap for subsequent turns, while Gilded
            // Anchor buffs only Golden minions currently in the warband.
            const auto endTurnMaxGold = player.season14.TakeEndTurnMaxGold();
            // trinketEndTurnMaxGold is consumed here before RECRUIT_END.
            if (endTurnMaxGold > 0)
                player.season14.trinketMaxGoldDelta += endTurnMaxGold;
            for (const auto& trinket : player.season14.trinkets)
            {
                if (!trinket.active || trinket.remainingUses == 0) continue;
                const auto behavior = FindTrinketBehavior(
                    Cards::FindCardByDbfID(trinket.dbfID).id);
                if (behavior.effect != TrinketEffect::END_TURN_GOLDEN_STATS)
                {
                    if (behavior.effect == TrinketEffect::END_TURN_BLOOD_GEMS_PER_TYPE)
                    {
                        // Splinter of Aurum's linked end-turn effect chooses
                        // one random friendly minion for every concrete type.
                        // A dual-type minion may therefore receive both typed
                        // applications, matching the visible type model.
                        for (const auto race : RACES_IN_BATTLEGROUNDS)
                        {
                            std::vector<Minion*> candidates;
                            player.recruitField.ForEachAlive([&](MinionData& data) {
                                if (data.value().HasRace(race))
                                    candidates.push_back(&data.value());
                            });
                            if (!candidates.empty())
                            {
                                auto* target = candidates[Random::get<std::size_t>(
                                    0, candidates.size() - 1)];
                                for (int i = 0; i < behavior.attack; ++i)
                                    player.ApplyBloodGemTo(*target);
                            }
                        }
                        continue;
                    }
                    if (behavior.effect == TrinketEffect::END_TURN_MURLOC_STATS)
                    {
                        std::vector<Minion*> candidates;
                        player.recruitField.ForEachAlive([&](MinionData& data) {
                            if (data.value().HasRace(Race::MURLOC))
                                candidates.push_back(&data.value());
                        });
                        if (!candidates.empty())
                        {
                            auto& target = *candidates[Random::get<std::size_t>(
                                0, candidates.size() - 1)];
                            target.SetAttack(target.GetAttack() + behavior.attack);
                            target.SetHealth(target.GetHealth() + behavior.health);
                            target.SetGameTag(GameTag::VENOMOUS, 1);
                        }
                        continue;
                    }
                    if (behavior.effect == TrinketEffect::END_TURN_MINION_STATS)
                    {
                        const int goldenScale = player.season14.goldenMinionsPlayed;
                        player.recruitField.ForEachAlive([&behavior, goldenScale](MinionData& data) {
                            auto& minion = data.value();
                            // Gold Mallet improves once per Golden minion
                            // played this game; ordinary/golden Mallet values
                            // remain the base +2/+1 and +4/+2 respectively.
                            minion.SetAttack(minion.GetAttack() + behavior.attack + goldenScale);
                            minion.SetHealth(minion.GetHealth() + behavior.health + goldenScale);
                        });
                        continue;
                    }
                    if (behavior.effect ==
                        TrinketEffect::END_TURN_LEFTMOST_MINION_STATS_PER_SPELL)
                    {
                        // Charming Panpipes always selects the left-most
                        // warband minion.  Its payload improves permanently
                        // once per successful Tavern spell; each Trinket
                        // copy carries its own progress counter.
                        if (player.recruitField.GetCount() > 0) {
                            auto& minion = player.recruitField[0];
                            minion.SetAttack(minion.GetAttack() +
                                             behavior.attack +
                                             trinket.triggerProgress);
                            minion.SetHealth(minion.GetHealth() +
                                             behavior.health +
                                             trinket.triggerProgress);
                        }
                        continue;
                    }
                    if (behavior.effect == TrinketEffect::END_TURN_LEFT_STATS_PER_BATTLECRY)
                    {
                        const int scale = player.season14.battlecriesTriggered;
                        int selected = 0;
                        player.recruitField.ForEachAlive([&](MinionData& data) {
                            if (selected >= 2) return;
                            auto& minion = data.value();
                            minion.SetAttack(minion.GetAttack() + behavior.attack + scale);
                            minion.SetHealth(minion.GetHealth() + behavior.health + scale);
                            ++selected;
                        });
                        continue;
                    }
                    if (behavior.effect == TrinketEffect::END_TURN_LEFTMOST_STATS_PER_BATTLECRY)
                    {
                        const int scale = player.season14.battlecriesTriggered;
                        bool selected = false;
                        player.recruitField.ForEachAlive([&behavior, scale, &selected](MinionData& data) {
                            if (selected) return;
                            auto& minion = data.value();
                            minion.SetAttack(minion.GetAttack() + behavior.attack + scale);
                            minion.SetHealth(minion.GetHealth() + behavior.health + scale);
                            selected = true;
                        });
                        continue;
                    }
                    if (behavior.effect == TrinketEffect::END_TURN_UNDEAD_ATTACK)
                    {
                        player.ApplyPersistentRaceStats(behavior.race,
                                                        behavior.attack,
                                                        behavior.health);
                        continue;
                    }
                    if (behavior.effect != TrinketEffect::END_TURN_DIVINE_SHIELD_ATTACK)
                        continue;
                    player.recruitField.ForEachAlive([&](MinionData& data) {
                        auto& minion = data.value();
                        if (minion.HasDivineShield())
                            minion.SetAttack(minion.GetAttack() + behavior.attack);
                    });
                    continue;
                }
                player.recruitField.ForEachAlive([&](MinionData& data) {
                    auto& minion = data.value();
                    if (minion.IsGolden()) {
                        minion.SetAttack(minion.GetAttack() + behavior.attack);
                        minion.SetHealth(minion.GetHealth() + behavior.health);
                    }
                    });
            }
            // Wolfhead Flail and Insurrectionist's Blade resolve against
            // stable recruit-board positions. Reacquire each entity before
            // execution because a deathrattle/Rally task may mutate the field.
            for (const auto& trinket : player.season14.trinkets)
            {
                if (!trinket.active || trinket.remainingUses == 0) continue;
                const auto behavior = FindTrinketBehavior(
                    Cards::FindCardByDbfID(trinket.dbfID).id);
                if (behavior.effect != TrinketEffect::END_TURN_LEFT_DEATHRATTLES &&
                    behavior.effect != TrinketEffect::END_TURN_RALLY_TRIGGERS)
                    continue;
                std::vector<std::uint64_t> selected;
                player.recruitField.ForEachAlive([&](MinionData& data) {
                    if (behavior.effect == TrinketEffect::END_TURN_RALLY_TRIGGERS ||
                        selected.size() < 2)
                        selected.push_back(static_cast<std::uint64_t>(data.value().GetIndex()));
                });
                for (const auto entityID : selected) {
                    for (int i = 0; i < player.recruitField.GetCount(); ++i) {
                        auto& minion = player.recruitField[static_cast<std::size_t>(i)];
                        if (static_cast<std::uint64_t>(minion.GetIndex()) != entityID ||
                            minion.IsDestroyed()) continue;
                        if (behavior.effect == TrinketEffect::END_TURN_LEFT_DEATHRATTLES)
                            minion.ActivateTask(PowerType::DEATHRATTLE, player);
                        else
                            minion.ActivateTrigger(TriggerType::RALLY, minion);
                        break;
                    }
                }
            }
            // Young Murk-Eye Sticker repeats the Battlecries of the two
            // edge minions once at recruit end. Snapshot entity identities so
            // a Battlecry that mutates the board cannot retarget the pass.
            for (const auto& trinket : player.season14.trinkets)
            {
                if (!trinket.active || trinket.remainingUses == 0) continue;
                if (FindTrinketBehavior(
                        Cards::FindCardByDbfID(trinket.dbfID).id).effect !=
                    TrinketEffect::END_TURN_BATTLECRY_TRIGGER)
                    continue;
                std::vector<std::uint64_t> selected;
                player.recruitField.ForEachAlive([&selected](MinionData& data) {
                    if (selected.size() < 2)
                        selected.push_back(static_cast<std::uint64_t>(data.value().GetIndex()));
                });
                if (player.recruitField.GetCount() > 2) {
                    std::uint64_t right = selected.back();
                    player.recruitField.ForEachAlive([&right](MinionData& data) {
                        right = static_cast<std::uint64_t>(data.value().GetIndex());
                    });
                    selected[1] = right;
                }
                for (const auto entityID : selected) {
                    for (int i = 0; i < player.recruitField.GetCount(); ++i) {
                        auto& minion = player.recruitField[static_cast<std::size_t>(i)];
                        if (static_cast<std::uint64_t>(minion.GetIndex()) != entityID ||
                            minion.IsDestroyed()) continue;
                        // Battlegrounds uses POWER for a minion's Battlecry
                        // task; PowerType has no separate BATTLECRY value.
                        minion.ActivateTask(PowerType::POWER, player);
                        break;
                    }
                }
                break;
            }
            // Resolve ordinary minion end-of-turn triggers after the final
            // recruit action and before combat.  Trigger dispatch is kept on
            // the authoritative board instances so generated effects (such
            // as Cataclysmic Harbinger's last-spell copy) cannot be skipped.
            const int endTurnPasses = player.season14.HasGeneratedRewardGhastlyMask() ? 2 : 1;
            int resolvedEndTurnPasses = endTurnPasses;
            int lucifronExtraPasses = 0;
            player.recruitField.ForEachAlive([&lucifronExtraPasses](const MinionData& data) {
                const auto& id = data.value().GetCardID();
                if (CardDefs::FindCardDefByID(id).lifecycle ==
                    CardLifecycle::BUDDY_LUCIFRON)
                    lucifronExtraPasses += id.ends_with("_G") ? 2 : 1;
            });
            resolvedEndTurnPasses = std::max(resolvedEndTurnPasses,
                1 + lucifronExtraPasses);
            for (const auto& trinket : player.season14.trinkets)
            {
                if (!trinket.active || trinket.remainingUses == 0) continue;
                if (FindTrinketBehavior(
                        Cards::FindCardByDbfID(trinket.dbfID).id).effect ==
                    TrinketEffect::END_TURN_EXTRA_TRIGGER)
                {
                    resolvedEndTurnPasses = std::max(resolvedEndTurnPasses, 2);
                    break;
                }
            }
            for (int pass = 0; pass < resolvedEndTurnPasses; ++pass) {
                player.recruitField.ForEachAlive([](MinionData& data) {
                    auto& minion = data.value();
                    minion.ActivateTrigger(TriggerType::TURN_END, minion);
                });
            }
            // Master Gadrin resolves from the final recruit-board positions.
            // Normal copies Attack to the minion on its left; golden copies
            // to both adjacent minions. This is a copy, not an additive buff.
            std::vector<std::pair<int, int>> gadrinEffects;
            player.recruitField.ForEachAlive([&gadrinEffects](const MinionData& data) {
                const auto& source = data.value();
                if (source.GetCardID() == "BG20_HERO_201_Buddy")
                    gadrinEffects.emplace_back(source.GetZonePosition(), source.GetAttack());
                else if (source.GetCardID() == "BG20_HERO_201_Buddy_G") {
                    gadrinEffects.emplace_back(source.GetZonePosition(), source.GetAttack());
                    // Encode the right-side target as -(position + 1), so a
                    // Gadrin at slot zero remains distinguishable.
                    gadrinEffects.emplace_back(-(source.GetZonePosition() + 1), source.GetAttack());
                }
            });
            for (const auto& [position, attack] : gadrinEffects) {
                const int target = position >= 0 ? position - 1 : -position;
                if (target < 0 || target >= player.recruitField.GetCount()) continue;
                auto& minion = player.recruitField[static_cast<std::size_t>(target)];
                if (!minion.IsDestroyed()) minion.SetAttack(attack);
            }
            // Bilgewater Mogul permanently increases the owner's maximum Gold
            // at the end of each recruit turn. Count every surviving copy;
            // the delta is consumed by EffectiveMaxGold on later turns and is
            // intentionally not granted as immediate spendable Gold.
            int maxGoldIncrease = 0;
            player.recruitField.ForEachAlive([&maxGoldIncrease](const MinionData& data) {
                const auto& id = data.value().GetCardID();
                if (id == "TB_BaconShop_HERO_10_Buddy") ++maxGoldIncrease;
                else if (id == "TB_BaconShop_HERO_10_Buddy_G") maxGoldIncrease += 2;
            });
            if (maxGoldIncrease > 0)
                player.season14.IncreaseMaxGold(maxGoldIncrease);
            // Snack Vendor copies its current stats onto random Tier 3
            // Tavern minions. Golden copies resolve twice; targets are
            // sampled without replacement for this trigger.
            std::vector<std::pair<int, int>> snackVendors;
            player.recruitField.ForEachAlive([&](const MinionData& data) {
                const auto& id = data.value().GetCardID();
                if (id == "TB_BaconShop_HERO_16_Buddy") {
                    snackVendors.emplace_back(data.value().GetAttack(), data.value().GetHealth());
                } else if (id == "TB_BaconShop_HERO_16_Buddy_G") {
                    snackVendors.emplace_back(data.value().GetAttack(), data.value().GetHealth());
                    snackVendors.emplace_back(data.value().GetAttack(), data.value().GetHealth());
                }
            });
            std::vector<std::size_t> snackTargets;
            for (std::size_t i = 0; i < static_cast<std::size_t>(player.tavern.fieldZone.GetCount()); ++i) {
                const auto& offer = player.tavern.fieldZone[i];
                if (!offer.IsDestroyed() && offer.GetGameTag(GameTag::TECH_LEVEL) == 3)
                    snackTargets.push_back(i);
            }
            Random::shuffle(snackTargets.begin(), snackTargets.end());
            const auto snackCount = std::min(snackVendors.size(), snackTargets.size());
            for (std::size_t i = 0; i < snackCount; ++i) {
                auto& offer = player.tavern.fieldZone[snackTargets[i]];
                offer.SetAttack(snackVendors[i].first);
                offer.SetHealth(snackVendors[i].second);
            }
            int thawedCopies = 0;
            player.recruitField.ForEachAlive([&](const MinionData& data) {
                const auto& id = data.value().GetCardID();
                if (id == "TB_BaconShop_HERO_27_Buddy") thawedCopies += 1;
                else if (id == "TB_BaconShop_HERO_27_Buddy_G") thawedCopies += 2;
            });
            if (thawedCopies > 0) {
                std::vector<std::size_t> frozenOffers;
                for (std::size_t i = 0; i < static_cast<std::size_t>(player.tavern.fieldZone.GetCount()); ++i) {
                    const auto& offer = player.tavern.fieldZone[i];
                    if (!offer.IsDestroyed() && offer.IsFrozen()) frozenOffers.push_back(i);
                }
                for (int copy = 0; copy < thawedCopies && !frozenOffers.empty(); ++copy) {
                    const auto pick = Random::get<std::size_t>(0, frozenOffers.size() - 1);
                    const auto slot = frozenOffers[pick];
                    player.AddMinionCopyToHand(player.tavern.fieldZone[slot]);
                    frozenOffers.erase(frozenOffers.begin() + static_cast<std::ptrdiff_t>(pick));
                }
            }
            int botaniCopies = 0;
            int lanternCopies = 0;
            player.recruitField.ForEachAlive([&](const MinionData& data) {
                const auto& id = data.value().GetCardID();
                if (id == "TB_BaconShop_HERO_74_Buddy") botaniCopies += 1;
                else if (id == "TB_BaconShop_HERO_74_Buddy_G") botaniCopies += 2;
                else if (id == "TB_BaconShop_HERO_75_Buddy") lanternCopies += 1;
                else if (id == "TB_BaconShop_HERO_75_Buddy_G") lanternCopies += 2;
            });
            for (int copy = 0; copy < botaniCopies; ++copy) {
                auto candidates = m_gameState.minionPool.GetMinions(
                    player.currentTier, player.currentTier, true);
                if (candidates.empty() || player.hand.IsFull()) break;
                const auto pick = Random::get<std::size_t>(0, candidates.size() - 1);
                // The generated hand minion is a fresh entity drawn from the
                // live Tavern pool.  Mark the selected pool entry unavailable
                // before copying its canonical card definition; otherwise
                // Botani can repeatedly mint the same pool copy without
                // changing pool/entity state.
                const auto poolIndex = candidates[pick].GetPoolIndex();
                if (!m_gameState.minionPool.TakeMinion(poolIndex)) break;
                player.AddMinionCopyToHand(candidates[pick]);
            }
            for (int copy = 0; copy < lanternCopies; ++copy) {
                std::vector<std::size_t> friendly;
                for (std::size_t i = 0; i < static_cast<std::size_t>(player.recruitField.GetCount()); ++i) {
                    if (!player.recruitField[i].IsDestroyed()) friendly.push_back(i);
                }
                if (!friendly.empty()) {
                    const auto pick = Random::get<std::size_t>(0, friendly.size() - 1);
                    auto& target = player.recruitField[friendly[pick]];
                    const int amount = player.currentTier;
                    target.SetAttack(target.GetAttack() + amount);
                    target.SetHealth(target.GetHealth() + amount);
                }
            }
            std::vector<std::pair<int, int>> dagwikBuffs;
            player.recruitField.ForEachAlive([&](const MinionData& data) {
                const auto& id = data.value().GetCardID();
                if (id == "TB_BaconShop_HERO_64_Buddy") {
                    dagwikBuffs.emplace_back(5, 5);
                } else if (id == "TB_BaconShop_HERO_64_Buddy_G") {
                    dagwikBuffs.emplace_back(10, 10);
                }
            });
            for (const auto& [dagwikAttack, dagwikHealth] : dagwikBuffs) {
                std::vector<std::size_t> golden;
                for (std::size_t i = 0; i < static_cast<std::size_t>(player.recruitField.GetCount()); ++i) {
                    if (!player.recruitField[i].IsDestroyed() && player.recruitField[i].IsGolden()) golden.push_back(i);
                }
                if (!golden.empty()) {
                    const auto pick = Random::get<std::size_t>(0, golden.size() - 1);
                    auto& target = player.recruitField[golden[pick]];
                    target.SetAttack(target.GetAttack() + dagwikAttack);
                    target.SetHealth(target.GetHealth() + dagwikHealth);
                }
            }
            int flightpathTurns = 0;
            player.recruitField.ForEachAlive([&flightpathTurns](const MinionData& data) {
                const auto& id = data.value().GetCardID();
                if (id == "BG20_HERO_283_Buddy") flightpathTurns += 1;
                else if (id == "BG20_HERO_283_Buddy_G") flightpathTurns += 2;
            });
            for (int i = 0; i < flightpathTurns; ++i)
                player.season14.AdvanceFlightpath();
            // Upbeat Harmony resolves at the end of every third recruit turn.
            // Consume the schedule only after the copy is attempted; a full
            // hand still consumes the triggered reward, as in-game.
            if (player.season14.TakeUpbeatHarmonyCopyReady())
                player.AddPlainCopyOfLeftmostHandCard();
            player.season14.Emit(Season14Event::RECRUIT_END);
        }
    }
    // Bananarama gives its owner two Bananas immediately, then gives every
    // other living player one at recruit end.  Resolve this after all normal
    // end-of-turn effects so the generated cards are observable before the
    // combat copies are made.
    const auto banana = Cards::FindCardByID("EX1_014t");
    if (!banana.id.empty()) {
        for (auto& owner : m_gameState.players) {
            if (owner.playState != PlayState::PLAYING ||
                !owner.season14.heroPowerBatch8.bananaramaPending)
                continue;
            const auto copies = owner.season14.heroPowerBatch8.bananaramaActivations > 0
                                    ? owner.season14.heroPowerBatch8.bananaramaActivations
                                    : 1;
            for (auto& recipient : m_gameState.players) {
                if (&recipient == &owner || recipient.playState != PlayState::PLAYING ||
                    recipient.hand.IsFull())
                    continue;
                for (int i = 0; i < copies && !recipient.hand.IsFull(); ++i)
                    recipient.hand.Add(CardData{Spell(banana)});
            }
            owner.season14.heroPowerBatch8.bananaramaPending = false;
            owner.season14.heroPowerBatch8.bananaramaActivations = 0;
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

        // Teron's Rapid Reanimation resolves before combat copies are made;
        // this preserves recruit ordering and gives the resurrected instance
        // its fresh entity identity.
        player1.ResolveRapidReanimationStartCombat();
        player2.ResolveRapidReanimationStartCombat();

        Battle battle(player1, player2);

        // Detective for Hire may reveal only the public minion identities
        // from the next opponent's most recent combat.  Snapshot before the
        // combat mutates either field; no hand, RNG, or hidden pool state is
        // copied into the information set.
        auto snapshotPublicBoard = [](const Player& player) {
            std::vector<Minion> minions;
            player.recruitField.ForEachAlive([&minions](const MinionData& data) {
                minions.push_back(data.value());
            });
            return minions;
        };
        player1.season14.RecordLastOpponentCombatMinionSnapshots(
            snapshotPublicBoard(player2));
        player2.season14.RecordLastOpponentCombatMinionSnapshots(
            snapshotPublicBoard(player1));
        player1.season14.RecordLastOpponentBuddy(player2.hero.card.relatedDbfID);
        player2.season14.RecordLastOpponentBuddy(player1.hero.card.relatedDbfID);

        // Create callback to get battle
        player1.getBattleCallback = [&battle]() -> Battle& { return battle; };
        player2.getBattleCallback = [&battle]() -> Battle& { return battle; };

        const CombatResult result = battle.Run();
        player1.season14.lastCombatLost = result.outcome == BattleResult::PLAYER2_WIN;
        player2.season14.lastCombatLost = result.outcome == BattleResult::PLAYER1_WIN;
        // Friendly Wager resolves exactly once, after the guessed seat's
        // combat outcome is authoritative.  A Wishbone replay preserves the
        // single guess but pays the effect once per committed resolution.
        for (auto& observer : m_gameState.players) {
            auto& wager = observer.season14.heroPowerBatch9;
            if (!wager.wagerPending ||
                (wager.wagerTargetPlayer != static_cast<int>(std::get<0>(pair)) &&
                 wager.wagerTargetPlayer != static_cast<int>(std::get<1>(pair))))
                continue;
            const bool guessedWon =
                (result.outcome == BattleResult::PLAYER1_WIN &&
                 wager.wagerTargetPlayer == static_cast<int>(std::get<0>(pair))) ||
                (result.outcome == BattleResult::PLAYER2_WIN &&
                 wager.wagerTargetPlayer == static_cast<int>(std::get<1>(pair)));
            const auto payout = wager.wagerResolutionCount;
            if (ResolveSeason14FriendlyWager(wager, guessedWon) && guessedWon)
                observer.AddTavernCoins(3 * std::max(1, payout));
        }
        // Combat fields are copies.  Commit only deltas explicitly marked as
        // permanent by combat effects before post-combat rewards resolve.
        battle.CommitPersistentState();
        // Resolve the owner-side first-kill copy after combat, using the
        // canonical normal card definition so temporary/golden combat state
        // cannot leak into the plain hand copy.
        auto resolveFirstKillCopy = [](Player& owner) {
            Minion snapshot;
            if (!owner.season14.TakeFirstKillCopy(snapshot)) return;
            if (owner.hand.IsFull()) return;
            Card card = Cards::FindCardByDbfID(snapshot.GetDbfID());
            if (card.normalDbfID != 0)
                card = Cards::FindCardByDbfID(card.normalDbfID);
            if (card.dbfID == 0) return;
            Minion plain(card);
            owner.ApplyFreshMinionModifiers(plain);
            owner.hand.Add(CardData{std::move(plain)});
        };
        resolveFirstKillCopy(player1);
        resolveFirstKillCopy(player2);
        // The power lasts for one combat only, including a combat in which no
        // qualifying kill occurred or the reward could not fit in hand.
        player1.season14.ExpireFirstKillCopy();
        player2.season14.ExpireFirstKillCopy();
        player1.ResolveGeneratedQuestRewardAfterCombat();
        player2.ResolveGeneratedQuestRewardAfterCombat();
        player1.season14.ResolveNextCombatReward(result.outcome, true);
        player2.season14.ResolveNextCombatReward(result.outcome, false);
        std::vector<Season14PendingCombatBuff> player1Buffs;
        if (player1.season14.ResolveNextCombatBuff(result.outcome, true,
                                                   player1Buffs))
        {
            for (const auto& player1Buff : player1Buffs)
                player1.recruitField.ForEachAlive([&](MinionData& data) {
                    if (static_cast<std::uint64_t>(data.value().GetIndex()) ==
                        player1Buff.targetEntityID)
                    {
                        data.value().SetAttack(data.value().GetAttack() +
                                               player1Buff.attack);
                        data.value().SetHealth(data.value().GetHealth() +
                                               player1Buff.health);
                    }
                });
        }
        std::vector<Season14PendingCombatBuff> player2Buffs;
        if (player2.season14.ResolveNextCombatBuff(result.outcome, false,
                                                   player2Buffs))
        {
            for (const auto& player2Buff : player2Buffs)
                player2.recruitField.ForEachAlive([&](MinionData& data) {
                    if (static_cast<std::uint64_t>(data.value().GetIndex()) ==
                        player2Buff.targetEntityID)
                    {
                        data.value().SetAttack(data.value().GetAttack() +
                                               player2Buff.attack);
                        data.value().SetHealth(data.value().GetHealth() +
                                               player2Buff.health);
                    }
                });
        }

        if (player1.playState == PlayState::PLAYING)
        {
            player1.season14.Emit(Season14Event::COMBAT_END);
        }
        if (player2.playState == PlayState::PLAYING)
        {
            player2.season14.Emit(Season14Event::COMBAT_END);
        }

        // Fairmount's Conviction improvement is a player decision after
        // combat, never a random recruit-end mutation. Queueing is performed
        // by Battle on each qualifying kill; expose one replayable modal now.
        if (player1.playState == PlayState::PLAYING)
            player1.season14.BeginConvictionImprovementChoice();
        if (player2.playState == PlayState::PLAYING)
            player2.season14.BeginConvictionImprovementChoice();

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
    const auto winner = std::find_if(
        m_gameState.players.begin(), m_gameState.players.end(),
        [](const Player& player) { return player.playState == PlayState::WON; });
    if (winner != m_gameState.players.end())
    {
        // Purified Shard (or another immediate-win reward) already assigned
        // rank 1. Keep the first winner deterministic, and assign every other
        // seat a distinct descending placement. This also handles a second
        // immediate-win selection that was queued before this phase ran.
        // Seats eliminated before the immediate win already own the lower
        // placement numbers from ProcessDefeat; continue at the current live
        // seat count so those ranks are not duplicated.
        int nextRank = PlayerIndexForRank(m_gameState.numRemainPlayer);
        winner->rank = 1;
        for (auto& player : m_gameState.players)
        {
            if (&player != &*winner &&
                (player.playState == PlayState::PLAYING ||
                 player.playState == PlayState::WON))
            {
                player.playState = PlayState::LOST;
                player.rank = nextRank--;
            }
        }
        m_gameState.numRemainPlayer = 1;
        m_gameState.phase = Phase::COMPLETE;
        return;
    }
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
            std::make_tuple(PlayerIndexForRank(playerIdx),
                            PlayerIndexForRank(m_gameState.ghostPlayerIdx)));

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
            std::make_tuple(PlayerIndexForRank(player.idx), player.hero.health));
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
