#include <Rosetta/Battlegrounds/Models/Season14.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/CardSets/TrinketBehaviors.hpp>

#include <algorithm>
#include <utility>

namespace RosettaStone::Battlegrounds
{
void Season14State::BeginDecision(
    Season14Decision decision, std::vector<Season14Offering> offerings)
{
    if (decision == Season14Decision::NONE)
    {
        pendingDecision = Season14Decision::NONE;
        choiceOfferings.clear();
        pendingOfferings.clear();
        pendingSourceEntityID = 0;
        pendingSourceCardDbfID = 0;
        chooseOne = {};
        spellModal = {};
        pendingHandLock = false;
        return;
    }

    pendingDecision = decision;
    pendingOfferings = std::move(offerings);
    pendingSourceEntityID = 0;
    pendingSourceCardDbfID = 0;
    chooseOne = {};
    spellModal = {};
    pendingHandLock = false;
    if (decision == Season14Decision::CHOICE ||
        decision == Season14Decision::DISCOVER)
    {
        choiceOfferings = pendingOfferings;
    }
    else
    {
        choiceOfferings.clear();
    }
}

void Season14State::BeginChooseOne(std::uint64_t sourceEntityID, std::uint32_t targetMask,
                                   std::int32_t sourceCardDbfID,
                                   std::vector<Season14Offering> offerings)
{
    pendingDecision = Season14Decision::CHOOSE_ONE;
    pendingOfferings = std::move(offerings);
    choiceOfferings = pendingOfferings;
    pendingSourceEntityID = 0;
    pendingSourceCardDbfID = 0;
    chooseOne = { true, sourceEntityID, targetMask, sourceCardDbfID };
    spellModal = {};
    pendingHandLock = false;
}

void Season14State::BeginSpellTargetChoice(
    std::int32_t sourceCardDbfID, std::int32_t targetIndex,
    std::uint64_t targetEntityID,
    std::int32_t firstAttack, std::int32_t firstHealth,
    std::int32_t secondAttack, std::int32_t secondHealth,
    std::string offeringFilter, Season14SpellModalKind kind)
{
    pendingDecision = Season14Decision::CHOOSE_ONE;
    // Branches are not discoverable cards. Keep two opaque slots so the
    // bridge can expose a deterministic two-way action while the branch
    // payload remains in spellModal.
    pendingOfferings = { Season14Offering{}, Season14Offering{} };
    choiceOfferings = pendingOfferings;
    pendingSourceEntityID = 0;
    pendingSourceCardDbfID = sourceCardDbfID;
    chooseOne = {};
    spellModal = { kind, sourceCardDbfID,
                   targetIndex, targetEntityID, firstAttack, firstHealth, secondAttack,
                   secondHealth, false, std::move(offeringFilter) };
}

void Season14State::BeginSpellAllMinionChoice(
    Season14SpellModalKind kind, std::int32_t sourceCardDbfID,
    std::int32_t firstAttack, std::int32_t firstHealth,
    std::int32_t secondAttack, std::int32_t secondHealth,
    bool secondBranchDelayed)
{
    pendingDecision = Season14Decision::CHOOSE_ONE;
    pendingOfferings = { Season14Offering{}, Season14Offering{} };
    choiceOfferings = pendingOfferings;
    pendingSourceEntityID = 0;
    pendingSourceCardDbfID = sourceCardDbfID;
    chooseOne = {};
    spellModal = { kind, sourceCardDbfID, -1, 0, firstAttack, firstHealth,
                   secondAttack, secondHealth, secondBranchDelayed, {} };
    pendingHandLock = false;
}

bool Season14State::SelectSpellTargetChoice(std::size_t offeringIndex,
                                            std::int32_t& attack,
                                            std::int32_t& health)
{
    if ((spellModal.kind != Season14SpellModalKind::TARGET_STATS &&
         spellModal.kind != Season14SpellModalKind::ALL_MINION_STATS &&
         spellModal.kind != Season14SpellModalKind::TARGET_OR_ALL_STATS) ||
        offeringIndex > 1)
        return false;
    if (offeringIndex == 0) {
        attack = spellModal.firstAttack;
        health = spellModal.firstHealth;
    } else {
        attack = spellModal.secondAttack;
        health = spellModal.secondHealth;
    }
    spellModal = {};
    pendingDecision = Season14Decision::NONE;
    pendingSourceCardDbfID = 0;
    pendingSourceEntityID = 0;
    pendingOfferings.clear();
    choiceOfferings.clear();
    pendingHandLock = false;
    return true;
}

void Season14State::BeginOfferingDecision(
    Season14Decision decision, std::uint64_t sourceEntityID,
    std::int32_t sourceCardDbfID, std::vector<Season14Offering> offerings)
{
    BeginDecision(decision, std::move(offerings));
    pendingSourceEntityID = sourceEntityID;
    pendingSourceCardDbfID = sourceCardDbfID;
}

bool Season14State::SelectDecision(std::size_t offeringIndex)
{
    if (pendingDecision == Season14Decision::NONE ||
        offeringIndex >= pendingOfferings.size())
    {
        return false;
    }

    choiceOfferings.clear();
    pendingOfferings.clear();
    pendingSourceEntityID = 0;
    pendingSourceCardDbfID = 0;
    pendingDecision = Season14Decision::NONE;
    chooseOne = {};
    spellModal = {};
    return true;
}

void Season14State::SetHeroPower(std::int32_t dbfID, std::int32_t cost,
                                 bool available)
{
    heroPowerDbfID = dbfID;
    heroPowerCost = std::max<std::int32_t>(0, cost);
    heroPowerAvailable = available;
    heroPowerUsed = false;
    heroPowerBatch1 = Season14HeroPowerBatch1Modifiers(dbfID);
    heroPowerBatch2 = {};
    heroPowerBatch4 = {};
    heroPowerBatch3State = 0;
    heroPowerBatch5 = {};
    heroPowerBatch6 = {};
    sharpenBladesPurchases = 0;
    cloningGalleryUsed = false;
    buriedTreasureDigs = 0;
    firstKillCopyArmed = false;
    firstKillCopy.reset();
    lastTavernSpellDbfID = 0;
}

Season14HeroPowerBatch2Result Season14State::BeginRecruitTurn()
{
    goldSpentThisTurn = 0;
    if (heroPowerDbfID == 57567)
        sharpenBladesPurchases = 0;
    ResolveSeason14HeroPowerBatch1Event(
        heroPowerDbfID, Season14HeroPowerBatch1Event::BEGIN_TURN,
        heroPowerBatch1);
    ResolveUpbeatHarmonyBeginTurn(heroPowerDbfID, heroPowerBatch1);

    Season14HeroPowerBatch2Result result{};
    ResolveSeason14HeroPowerBatch2Event(
        heroPowerDbfID, Season14HeroPowerBatch2Event::BEGIN_TURN,
        heroPowerBatch2, result);
    if (heroPowerDbfID == 116924)
        heroPowerBatch2.nextHeroPowerDiscount = true;
    BeginRecruitTurnBatch4();
    BeginRecruitTurnBatch5();
    ResolveVoidPowerBeginTurn(heroPowerDbfID, heroPowerBatch6);
    return result;
}

int Season14State::RecordGoldSpent(std::int32_t amount) noexcept
{
    if (amount <= 0) return 0;
    const auto before = goldSpentThisTurn / 5;
    goldSpentThisTurn += amount;
    return goldSpentThisTurn / 5 - before;
}

void Season14State::BeginRecruitTurnBatch4()
{
    Season14HeroPowerBatch4Result result{};
    ResolveSeason14HeroPowerBatch4Event(
        heroPowerDbfID, Season14HeroPowerBatch4Event::BEGIN_TURN,
        heroPowerBatch4, result);
}

void Season14State::BeginRecruitTurnBatch5()
{
    Season14HeroPowerBatch5Result result{};
    ResolveSeason14HeroPowerBatch5Event(
        heroPowerDbfID, Season14HeroPowerBatch5Event::BEGIN_TURN,
        heroPowerBatch5, result);
}

void Season14State::BeginCombatBatch4()
{
    Season14HeroPowerBatch4Result result{};
    ResolveSeason14HeroPowerBatch4Event(
        heroPowerDbfID, Season14HeroPowerBatch4Event::COMBAT_START,
        heroPowerBatch4, result);
}

void Season14State::OnSellMinion()
{
    Season14HeroPowerBatch2Result result{};
    ResolveSeason14HeroPowerBatch2Event(
        heroPowerDbfID, Season14HeroPowerBatch2Event::SELL_MINION,
        heroPowerBatch2, result);
}

std::int32_t Season14State::OnBuyMinion(bool purchasedPirate)
{
    OnBuyMinionSharpenBlades();
    return Season14HeroPowerBatch1PurchaseGold(heroPowerDbfID,
                                               purchasedPirate);
}

void Season14State::OnBuyMinionSharpenBlades() noexcept
{
    if (heroPowerDbfID != 57567) return;
    ++sharpenBladesPurchases;
}

std::pair<std::int32_t, std::int32_t>
Season14State::SharpenBladesStats() const noexcept
{
    return {2 * sharpenBladesPurchases, sharpenBladesPurchases};
}

bool Season14State::CanBuriedTreasureDig() const noexcept
{
    return heroPowerDbfID == 62250 && buriedTreasureDigs < 4;
}

void Season14State::RecordBuriedTreasureDig() noexcept
{
    if (CanBuriedTreasureDig()) ++buriedTreasureDigs;
}

void Season14State::ArmFirstKillCopy() noexcept
{
    firstKillCopyArmed = true;
    firstKillCopy.reset();
}

void Season14State::RecordFirstKillCopy(const Minion& minion)
{
    if (firstKillCopyArmed && !firstKillCopy.has_value())
        firstKillCopy = minion;
}

bool Season14State::TakeFirstKillCopy(Minion& out)
{
    if (!firstKillCopy.has_value()) return false;
    out = *firstKillCopy;
    firstKillCopy.reset();
    firstKillCopyArmed = false;
    return true;
}

void Season14State::ExpireFirstKillCopy() noexcept
{
    firstKillCopy.reset();
    firstKillCopyArmed = false;
}

std::int32_t Season14State::OnBuyMinionBatch4()
{
    Season14HeroPowerBatch4Result result{};
    ResolveSeason14HeroPowerBatch4Event(
        heroPowerDbfID, Season14HeroPowerBatch4Event::BUY_MINION,
        heroPowerBatch4, result);
    return result.purchaseAttack;
}

Season14HeroPowerBatch2Result Season14State::OnPlayElemental()
{
    Season14HeroPowerBatch2Result result{};
    ResolveSeason14HeroPowerBatch2Event(
        heroPowerDbfID, Season14HeroPowerBatch2Event::PLAY_ELEMENTAL,
        heroPowerBatch2, result);
    return result;
}

Season14HeroPowerBatch4Result Season14State::OnPlayMinionBatch4()
{
    Season14HeroPowerBatch4Result result{};
    ResolveSeason14HeroPowerBatch4Event(
        heroPowerDbfID, Season14HeroPowerBatch4Event::PLAY_MINION,
        heroPowerBatch4, result);
    return result;
}

Season14HeroPowerBatch4Result
Season14State::OnFriendlyMinionDiedBatch4()
{
    Season14HeroPowerBatch4Result result{};
    ResolveSeason14HeroPowerBatch4Event(
        heroPowerDbfID, Season14HeroPowerBatch4Event::FRIENDLY_MINION_DIED,
        heroPowerBatch4, result);
    return result;
}

Season14HeroPowerBatch2Result Season14State::OnUpgradeTavern()
{
    Season14HeroPowerBatch2Result result{};
    ResolveSeason14HeroPowerBatch2Event(
        heroPowerDbfID, Season14HeroPowerBatch2Event::UPGRADE_TAVERN,
        heroPowerBatch2, result);
    return result;
}

std::int32_t Season14State::MinionPurchaseCost(std::int32_t baseCost) const
{
    const auto withBatch1 = heroPowerBatch1.MinionCost(baseCost);
    const auto batch2 = Season14HeroPowerBatch2Modifiers(heroPowerDbfID);
    return std::max<std::int32_t>(0, withBatch1 + batch2.minionCost);
}

std::int32_t Season14State::RefreshCost(std::int32_t baseCost) const
{
    if (heroPowerBatch1.freeRefreshAvailable)
    {
        return 0;
    }
    const auto withBatch1 = heroPowerBatch1.RefreshCost(baseCost);
    const auto batch2 = Season14HeroPowerBatch2Modifiers(heroPowerDbfID);
    return std::max<std::int32_t>(0, withBatch1 + batch2.refreshCost);
}

std::int32_t Season14State::UpgradeCost(std::int32_t baseCost) const
{
    return heroPowerBatch1.UpgradeCost(baseCost);
}

std::size_t Season14State::TavernOfferCount(std::size_t baseCount) const
{
    const auto modifiers = Season14HeroPowerBatch2Modifiers(heroPowerDbfID);
    const auto delta = modifiers.tavernSlotsDelta +
                       trinketExtraShopSlots +
                       HeroPowerBatch4PassiveModifiers().tavernSlotsDelta;
    if (delta < 0)
    {
        const auto reduction = static_cast<std::size_t>(-delta);
        return reduction >= baseCount ? 0 : baseCount - reduction;
    }
    return baseCount + static_cast<std::size_t>(delta);
}

Season14HeroPowerBatch4PassiveModifiers
Season14State::HeroPowerBatch4PassiveModifiers() const noexcept
{
    return Season14HeroPowerBatch4Modifiers(heroPowerDbfID);
}

bool Season14State::ResolveHeroPowerBatch4Activation(
    Season14HeroPowerBatch4Result& result) const noexcept
{
    auto candidateState = heroPowerBatch4;
    return ResolveSeason14HeroPowerBatch4Activation(
        heroPowerDbfID, candidateState, result);
}

bool Season14State::ApplyHeroPowerBatch4Activation(
    Season14HeroPowerBatch4Result& result) noexcept
{
    return ResolveSeason14HeroPowerBatch4Activation(
        heroPowerDbfID, heroPowerBatch4, result);
}

bool Season14State::ResolveHeroPowerBatch5Activation(
    Season14HeroPowerBatch5Result& result, std::int32_t tier) const noexcept
{
    auto candidateState = heroPowerBatch5;
    return ResolveSeason14HeroPowerBatch5Activation(
        heroPowerDbfID, candidateState, result, 1, tier);
}

bool Season14State::ApplyHeroPowerBatch5Activation(
    Season14HeroPowerBatch5Result& result, std::int32_t tier) noexcept
{
    const auto roll = result.goldDelta > 0 ? result.goldDelta : 1;
    return ResolveSeason14HeroPowerBatch5Activation(
        heroPowerDbfID, heroPowerBatch5, result, roll, tier);
}

void Season14State::ArmHigherTierRefresh(std::int32_t count)
{
    heroPowerBatch2.higherTierRefreshMinions =
        std::max<std::int32_t>(0, count);
}

std::int32_t Season14State::TakeHigherTierRefresh()
{
    const auto count = heroPowerBatch2.higherTierRefreshMinions;
    heroPowerBatch2.higherTierRefreshMinions = 0;
    return count;
}

bool Season14State::ShouldFreezeRemainingTavern() const
{
    return Season14HeroPowerBatch2Modifiers(heroPowerDbfID)
        .freezeRemainingShopAtEnd;
}

std::int32_t Season14State::TavernSpellCost(std::int32_t baseCost) const
{
    const auto withBatch1 = heroPowerBatch1.TavernSpellCost(baseCost);
    return heroPowerBatch2.TavernSpellCost(withBatch1);
}

std::int32_t Season14State::ConsumeTavernSpellDiscount() noexcept
{
    const auto result = nextTavernSpellDiscount;
    nextTavernSpellDiscount = 0;
    return result;
}

std::pair<std::int32_t, std::int32_t> Season14State::BloodGemStats() const noexcept
{
    return { 1 + bloodGemAttackBonus, 1 + bloodGemHealthBonus };
}

void Season14State::AddTavernSpellHealthBonus(std::int32_t health) noexcept
{
    tavernSpellHealthBonus = std::max<std::int32_t>(
        0, tavernSpellHealthBonus + health);
}

void Season14State::AddTavernSpellAttackBonus(std::int32_t attack) noexcept
{
    tavernSpellAttackBonus = std::max<std::int32_t>(0, tavernSpellAttackBonus + attack);
}

std::int32_t Season14State::TakeGrowingSummonAttack(
    std::int32_t entityIndex, std::int32_t initialAttack,
    std::int32_t increment)
{
    for (auto& bonus : growingSummonBonuses)
    {
        if (bonus.entityIndex == entityIndex)
        {
            const auto result = bonus.nextAttack;
            bonus.nextAttack += bonus.increment;
            return result;
        }
    }
    growingSummonBonuses.push_back(
        {entityIndex, initialAttack + increment, increment});
    return initialAttack;
}

std::pair<std::int32_t, std::int32_t> Season14State::BloodGemStatsFor(
    Race race) const noexcept
{
    auto [attack, health] = BloodGemStats();
    const auto [raceAttack, raceHealth] = BloodGemRaceStatsFor(race);
    return { attack + raceAttack, health + raceHealth };
}

std::pair<std::int32_t, std::int32_t> Season14State::BloodGemRaceStatsFor(
    Race race) const noexcept
{
    std::int32_t attack = 0;
    std::int32_t health = 0;
    for (const auto& bonus : bloodGemRaceBonuses)
    {
        if (bonus.race == race || bonus.race == Race::ALL)
        {
            attack += bonus.attack;
            health += bonus.health;
        }
    }
    return { attack, health };
}

void Season14State::AddBloodGemBonus(std::int32_t attack,
                                     std::int32_t health) noexcept
{
    bloodGemAttackBonus = std::max<std::int32_t>(
        0, bloodGemAttackBonus + attack);
    bloodGemHealthBonus = std::max<std::int32_t>(
        0, bloodGemHealthBonus + health);
}

void Season14State::AddBloodGemRaceBonus(Race race, std::int32_t attack,
                                         std::int32_t health)
{
    // Race-scoped bonuses are persistent state, not a general-purpose stat
    // mutation API.  Reject malformed metadata and preserve the same
    // non-negative invariant used by the global Blood Gem modifiers.
    if (race == Race::INVALID || (attack == 0 && health == 0))
        return;
    auto it = std::find_if(
        bloodGemRaceBonuses.begin(), bloodGemRaceBonuses.end(),
        [race](const Season14RaceShopStats& bonus) {
            return bonus.race == race;
        });
    if (it == bloodGemRaceBonuses.end())
    {
        bloodGemRaceBonuses.push_back(
            { race, std::max<std::int32_t>(0, attack),
              std::max<std::int32_t>(0, health) });
    }
    else
    {
        it->attack = std::max<std::int32_t>(0, it->attack + attack);
        it->health = std::max<std::int32_t>(0, it->health + health);
    }
}

void Season14State::OnRefreshTavern(bool refreshSucceeded)
{
    ResolveSeason14HeroPowerBatch1Event(
        heroPowerDbfID, Season14HeroPowerBatch1Event::REFRESH_TAVERN,
        heroPowerBatch1, refreshSucceeded);
    if (refreshSucceeded && trinketHigherTierRefreshes > 0)
    {
        ArmHigherTierRefresh(trinketHigherTierRefreshes);
    }
}

void Season14State::OnTavernSpellResolved(bool spellResolved,
                                           std::int32_t sourceDbfID)
{
    if (!spellResolved)
        return;
    if (sourceDbfID > 0) lastTavernSpellDbfID = sourceDbfID;
    if (heroPowerDbfID == 105432)
    {
        // Aranna's passive makes every third Tavern spell free.  Arm the
        // discount after the second successful purchase so cost inspection
        // for the third purchase sees zero, then consume it on resolution.
        if (heroPowerBatch2.tavernSpellDiscount > 0)
            heroPowerBatch2.ConsumeTavernSpellDiscount(true);
        ++heroPowerBatch2.tavernSpellPurchases;
        if (heroPowerBatch2.tavernSpellPurchases % 3 == 2)
            heroPowerBatch2.tavernSpellDiscount = 1;
        return;
    }
    heroPowerBatch2.ConsumeTavernSpellDiscount(true);
}

void Season14State::AddNextTurnGold(std::int32_t amount) noexcept
{
    nextTurnGold = std::max<std::int32_t>(0, nextTurnGold + amount);
}

void Season14State::ArmNextCombatReward(std::int32_t sourceCardDbfID) noexcept
{
    if (sourceCardDbfID != 105267)
        return;
    pendingCombatRewardDbfID = sourceCardDbfID;
    ++pendingCombatRewardCount;
}

void Season14State::ResolveNextCombatReward(BattleResult result,
                                             bool playerOne) noexcept
{
    if (pendingCombatRewardDbfID == 0 || pendingCombatRewardCount <= 0)
        return;
    const bool won = (playerOne && result == BattleResult::PLAYER1_WIN) ||
                     (!playerOne && result == BattleResult::PLAYER2_WIN);
    const bool tied = result == BattleResult::DRAW;
    const auto count = pendingCombatRewardCount;
    pendingCombatRewardDbfID = 0;
    pendingCombatRewardCount = 0;
    if (won)
        AddNextTurnGold(3 * count);
    else if (tied)
        AddNextTurnGold(count);
}

void Season14State::ArmNextCombatBuff(std::int32_t sourceCardDbfID,
                                      std::uint64_t targetEntityID,
                                      std::int32_t attack,
                                      std::int32_t health) noexcept
{
    if (sourceCardDbfID != 133369 || targetEntityID == 0)
    {
        pendingCombatBuffs.clear();
        return;
    }
    pendingCombatBuffs.push_back(
        { sourceCardDbfID, targetEntityID, attack, health });
}

bool Season14State::ResolveNextCombatBuff(
    BattleResult result, bool playerOne,
    std::vector<Season14PendingCombatBuff>& resolved) noexcept
{
    if (pendingCombatBuffs.empty())
        return false;
    const bool won = (playerOne && result == BattleResult::PLAYER1_WIN) ||
                     (!playerOne && result == BattleResult::PLAYER2_WIN);
    resolved = std::move(pendingCombatBuffs);
    pendingCombatBuffs.clear();
    return won;
}

void Season14State::ArmCombatStartLeftmostAttackDouble(
    std::int32_t sourceCardDbfID) noexcept
{
    if (sourceCardDbfID == 127503)
        pendingCombatStartEffects.push_back(sourceCardDbfID);
}

void Season14State::ArmCombatStartNearestStats(
    std::int32_t sourceCardDbfID) noexcept
{
    if (sourceCardDbfID == 119599)
        pendingCombatStartEffects.push_back(-sourceCardDbfID);
}

void Season14State::ArmShopBloodGemsOnRefresh(std::int32_t sourceCardDbfID) noexcept
{
    if (sourceCardDbfID == 126676)
        shopBloodGemsOnRefresh = true;
}

bool Season14State::HasShopBloodGemsOnRefresh() const noexcept
{
    return shopBloodGemsOnRefresh;
}

void Season14State::ArmCombatStartBeetles(std::int32_t sourceCardDbfID) noexcept
{
    if (sourceCardDbfID == 110401)
        pendingCombatStartEffects.push_back(sourceCardDbfID);
}

std::size_t Season14State::TakeCombatStartBeetles() noexcept
{
    const auto count = static_cast<std::size_t>(std::count(
        pendingCombatStartEffects.begin(), pendingCombatStartEffects.end(),
        110401));
    pendingCombatStartEffects.erase(
        std::remove(pendingCombatStartEffects.begin(),
                    pendingCombatStartEffects.end(), 110401),
        pendingCombatStartEffects.end());
    return count;
}

void Season14State::ArmCombatStartRandomEnemySetHealth(
    std::int32_t sourceCardDbfID) noexcept
{
    if (sourceCardDbfID == 104560)
        pendingCombatStartEffects.push_back(-sourceCardDbfID);
}

std::size_t Season14State::TakeCombatStartLeftmostAttackDoubles() noexcept
{
    const auto count = static_cast<std::size_t>(std::count(
        pendingCombatStartEffects.begin(), pendingCombatStartEffects.end(),
        127503));
    pendingCombatStartEffects.erase(
        std::remove(pendingCombatStartEffects.begin(),
                    pendingCombatStartEffects.end(), 127503),
        pendingCombatStartEffects.end());
    return count;
}

std::size_t Season14State::TakeCombatStartNearestStats() noexcept
{
    const auto count = static_cast<std::size_t>(std::count(
        pendingCombatStartEffects.begin(), pendingCombatStartEffects.end(),
        -119599));
    pendingCombatStartEffects.erase(
        std::remove(pendingCombatStartEffects.begin(),
                    pendingCombatStartEffects.end(), -119599),
        pendingCombatStartEffects.end());
    return count;
}

std::size_t Season14State::TakeCombatStartRandomEnemySetHealth() noexcept
{
    const auto count = static_cast<std::size_t>(std::count(
        pendingCombatStartEffects.begin(), pendingCombatStartEffects.end(),
        -104560));
    pendingCombatStartEffects.erase(
        std::remove(pendingCombatStartEffects.begin(),
                    pendingCombatStartEffects.end(), -104560),
        pendingCombatStartEffects.end());
    return count;
}

std::int32_t Season14State::TakeNextTurnGold() noexcept
{
    const auto result = nextTurnGold;
    nextTurnGold = 0;
    return result;
}

std::int32_t Season14State::TakeImmediateGold() noexcept
{
    const auto result = trinketImmediateGold;
    trinketImmediateGold = 0;
    return result;
}

std::int32_t Season14State::EffectiveMaxGold(
    std::int32_t baseCap) const noexcept
{
    return std::max<std::int32_t>(0, baseCap + maxGoldDelta +
                                      trinketMaxGoldDelta);
}

void Season14State::IncreaseMaxGold(std::int32_t amount) noexcept
{
    maxGoldDelta = std::max<std::int32_t>(0, maxGoldDelta + amount);
}

void Season14State::AddFreeRefreshes(std::int32_t amount) noexcept
{
    freeRefreshes = std::max<std::int32_t>(0, freeRefreshes + amount);
}

bool Season14State::HasFreeRefresh() const noexcept
{
    return freeRefreshes > 0;
}

bool Season14State::ConsumeFreeRefresh() noexcept
{
    if (!HasFreeRefresh())
    {
        return false;
    }
    --freeRefreshes;
    return true;
}

void Season14State::AddPersistentShopStats(std::int32_t attack,
                                           std::int32_t health) noexcept
{
    persistentShopAttack += attack;
    persistentShopHealth += health;
}

void Season14State::ImproveFutureLobsters(std::int32_t attack,
                                          std::int32_t health) noexcept
{
    futureLobsterAttack += attack;
    futureLobsterHealth += health;
}

std::pair<std::int32_t, std::int32_t>
Season14State::FutureLobsterStats() const noexcept
{
    return { futureLobsterAttack, futureLobsterHealth };
}

void Season14State::ImproveFutureBallers(std::int32_t attack,
                                         std::int32_t health)
{
    futureBallerAttack = std::max<std::int32_t>(0, futureBallerAttack + attack);
    futureBallerHealth = std::max<std::int32_t>(0, futureBallerHealth + health);
}

std::pair<std::int32_t, std::int32_t>
Season14State::FutureBallerStats() const noexcept
{
    return { futureBallerAttack, futureBallerHealth };
}

void Season14State::AddPersistentShopRaceStats(Race race,
                                               std::int32_t attack,
                                               std::int32_t health)
{
    if (race == Race::INVALID || (attack == 0 && health == 0))
    {
        return;
    }

    for (auto& existing : persistentShopRaceStats)
    {
        if (existing.race == race)
        {
            existing.attack += attack;
            existing.health += health;
            return;
        }
    }

    persistentShopRaceStats.push_back({ race, attack, health });
}

void Season14State::AddPersistentRaceStats(Race race, std::int32_t attack,
                                            std::int32_t health)
{
    if (race == Race::INVALID || (attack == 0 && health == 0)) return;
    for (auto& existing : persistentRaceStats)
    {
        if (existing.race == race)
        {
            existing.attack += attack;
            existing.health += health;
            return;
        }
    }
    persistentRaceStats.push_back({ race, attack, health });
}

void Season14State::ArmRefreshRandomShopStats(std::int32_t attack,
                                              std::int32_t health) noexcept
{
    refreshRandomShopAttack += attack;
    refreshRandomShopHealth += health;
}

std::pair<std::int32_t, std::int32_t>
Season14State::RefreshRandomShopStats() noexcept
{
    const auto result = std::make_pair(refreshRandomShopAttack, refreshRandomShopHealth);
    refreshRandomShopAttack = 0;
    refreshRandomShopHealth = 0;
    return result;
}

bool Season14State::ResolveHeroPowerBatch3Activation(
    std::int32_t currentTier,
    Season14HeroPowerBatch3Activation& result) const noexcept
{
    return ResolveSeason14HeroPowerBatch3Activation(
        heroPowerDbfID, currentTier, result);
}

std::int32_t Season14State::HeroPowerBatch3CombatKillAttackBonus() const noexcept
{
    return Season14HeroPowerBatch3CombatKillAttack(
        heroPowerDbfID);
}

bool Season14State::CanUseHeroPower(std::int32_t availableGold) const
{
    const bool bloodboundSecondUse =
        heroPowerDbfID == 71459 && heroPowerBatch2.bloodboundUsesThisTurn < 2;
    return heroPowerAvailable && (bloodboundSecondUse || !heroPowerUsed) &&
           heroPowerDbfID != 0 &&
           availableGold >= EffectiveHeroPowerCost();
}

std::int32_t Season14State::EffectiveHeroPowerCost() const
{
    return std::max<std::int32_t>(
        0, heroPowerCost + heroPowerBatch1.leadExplorerCostDelta -
               (heroPowerBatch2.nextHeroPowerDiscount ? 1 : 0));
}

bool Season14State::UseHeroPower()
{
    if (!heroPowerAvailable ||
        (heroPowerUsed && !(heroPowerDbfID == 71459 &&
                            heroPowerBatch2.bloodboundUsesThisTurn < 2)) ||
        heroPowerDbfID == 0)
    {
        return false;
    }

    if (heroPowerDbfID != 71459)
    {
        heroPowerUsed = true;
    }
    // The Galaxy's Lens grants a one-use discount for the next hero power.
    // Consume it at the successful use site so failed/unauthorized attempts
    // cannot spend the discount and a later turn cannot reuse it indefinitely.
    heroPowerBatch2.nextHeroPowerDiscount = false;
    return true;
}

bool Season14State::CanAddTrinket() const
{
    return trinkets.size() < SEASON14_TRINKET_SLOTS;
}

bool Season14State::CanAddDarkGift() const
{
    return darkGifts.size() < SEASON14_DARK_GIFT_SLOTS;
}

void Season14State::AddTrinket(Season14PersistentEffect effect)
{
    if (CanAddTrinket() && effect.dbfID > 0 && effect.remainingUses > 0 &&
        effect.active &&
        std::none_of(trinkets.begin(), trinkets.end(),
                     [dbfID = effect.dbfID](
                         const Season14PersistentEffect& existing) {
                         return existing.dbfID == dbfID;
                     }))
    {
        trinkets.push_back(effect);
        const auto card = Cards::FindCardByDbfID(effect.dbfID);
        const auto behavior = FindTrinketBehavior(card.id);
        switch (behavior.effect)
        {
            case TrinketEffect::SHOP_STATS:
                AddPersistentShopStats(behavior.attack, behavior.health);
                break;
            case TrinketEffect::EXTRA_SHOP_SLOT:
                trinketExtraShopSlots += behavior.value;
                break;
            case TrinketEffect::HIGHER_TIER_REFRESH:
                ++trinketHigherTierRefreshes;
                break;
            case TrinketEffect::MAX_GOLD:
                trinketMaxGoldDelta += behavior.value;
                break;
            case TrinketEffect::GOLD_AND_MAX_GOLD:
                trinketImmediateGold += behavior.value;
                trinketMaxGoldDelta += behavior.value;
                break;
            case TrinketEffect::IMMEDIATE_GOLD:
                trinketImmediateGold += behavior.value;
                break;
            case TrinketEffect::SHOP_STATS_AND_EXTRA_SLOT:
                AddPersistentShopStats(behavior.attack, behavior.health);
                trinketExtraShopSlots += behavior.value;
                break;
            case TrinketEffect::START_TURN_GOLD_PER_MINION_TYPE:
                // Resolved at recruit start after the board's distinct types
                // are known; acquisition itself has no immediate delta.
                break;
            case TrinketEffect::END_TURN_MAX_GOLD:
                ++trinketEndTurnMaxGold;
                break;
            case TrinketEffect::END_TURN_GOLDEN_STATS:
                // Applied to the concrete board at recruit end; no global
                // aura is installed because only Golden minions qualify.
                break;
            case TrinketEffect::NONE:
                break;
        }
    }
}

void Season14State::AddDarkGift(Season14PersistentEffect effect)
{
    if (CanAddDarkGift() && effect.dbfID > 0 && effect.remainingUses > 0 &&
        effect.active)
    {
        darkGifts.push_back(effect);
    }
}

bool Season14State::ConsumeEffect(
    std::vector<Season14PersistentEffect>& effects, std::size_t slot)
{
    if (slot >= effects.size() || !effects[slot].active ||
        effects[slot].remainingUses == 0)
    {
        return false;
    }

    if (effects[slot].remainingUses > 0)
    {
        --effects[slot].remainingUses;
        if (effects[slot].remainingUses == 0)
        {
            effects[slot].active = false;
        }
    }
    return true;
}

void Season14State::Emit(Season14Event event)
{
    const auto index = static_cast<std::size_t>(event);
    if (index < eventCounts.size())
    {
        ++eventCounts[index];
    }
}

bool Season14State::IsValidBoardTarget(std::int32_t index,
                                       std::int32_t boardCount)
{
    return index >= 0 && index < boardCount;
}
}  // namespace RosettaStone::Battlegrounds
