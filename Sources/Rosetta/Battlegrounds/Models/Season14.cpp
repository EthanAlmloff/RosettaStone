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
        chooseOne = {};
        return;
    }

    pendingDecision = decision;
    pendingOfferings = std::move(offerings);
    chooseOne = {};
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
    chooseOne = { true, sourceEntityID, targetMask, sourceCardDbfID };
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
    pendingDecision = Season14Decision::NONE;
    chooseOne = {};
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
}

Season14HeroPowerBatch2Result Season14State::BeginRecruitTurn()
{
    ResolveSeason14HeroPowerBatch1Event(
        heroPowerDbfID, Season14HeroPowerBatch1Event::BEGIN_TURN,
        heroPowerBatch1);

    Season14HeroPowerBatch2Result result{};
    ResolveSeason14HeroPowerBatch2Event(
        heroPowerDbfID, Season14HeroPowerBatch2Event::BEGIN_TURN,
        heroPowerBatch2, result);
    BeginRecruitTurnBatch4();
    return result;
}

void Season14State::BeginRecruitTurnBatch4()
{
    Season14HeroPowerBatch4Result result{};
    ResolveSeason14HeroPowerBatch4Event(
        heroPowerDbfID, Season14HeroPowerBatch4Event::BEGIN_TURN,
        heroPowerBatch4, result);
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

std::int32_t Season14State::OnBuyMinion(bool purchasedPirate) const
{
    return Season14HeroPowerBatch1PurchaseGold(heroPowerDbfID,
                                               purchasedPirate);
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

std::pair<std::int32_t, std::int32_t> Season14State::BloodGemStats() const noexcept
{
    return { 1 + bloodGemAttackBonus, 1 + bloodGemHealthBonus };
}

void Season14State::AddBloodGemBonus(std::int32_t attack,
                                     std::int32_t health) noexcept
{
    bloodGemAttackBonus = std::max<std::int32_t>(
        0, bloodGemAttackBonus + attack);
    bloodGemHealthBonus = std::max<std::int32_t>(
        0, bloodGemHealthBonus + health);
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

void Season14State::OnTavernSpellResolved(bool spellResolved)
{
    heroPowerBatch2.ConsumeTavernSpellDiscount(spellResolved);
}

void Season14State::AddNextTurnGold(std::int32_t amount) noexcept
{
    nextTurnGold = std::max<std::int32_t>(0, nextTurnGold + amount);
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

void Season14State::ArmRefreshRandomShopStats(std::int32_t attack,
                                              std::int32_t health) noexcept
{
    refreshRandomShopAttack += attack;
    refreshRandomShopHealth += health;
}

std::pair<std::int32_t, std::int32_t>
Season14State::RefreshRandomShopStats() const noexcept
{
    return { refreshRandomShopAttack, refreshRandomShopHealth };
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
        0, heroPowerCost - (heroPowerBatch2.nextHeroPowerDiscount ? 1 : 0));
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
        effect.active)
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
            case TrinketEffect::SHOP_STATS_AND_EXTRA_SLOT:
                AddPersistentShopStats(behavior.attack, behavior.health);
                trinketExtraShopSlots += behavior.value;
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
