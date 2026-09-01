// Copyright (c) 2017-2024 Chris Ohk

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Cards/CardDefs.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch21.hpp>
#include <Rosetta/Battlegrounds/CardSets/TavernSpellBehaviors.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/CardSets/TrinketBehaviors.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomCardToHandTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomMagneticMechToTargetTask.hpp>
#include <Rosetta/Battlegrounds/CardSets/DarkGiftBehaviors.hpp>
#include <Rosetta/Battlegrounds/CardSets/FodderBehaviors.hpp>
#include <Rosetta/Battlegrounds/CardSets/GiantSpellcraftBehaviors.hpp>
#include <Rosetta/Battlegrounds/CardSets/BuddyBehaviors.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomSpellcraftToHandTask.hpp>

#include <effolkronium/random.hpp>

#include <algorithm>
#include <iterator>
#include <set>
#include <vector>

namespace RosettaStone::Battlegrounds
{
namespace
{
using Random = effolkronium::random_thread_local;

// These pool helpers are defined with the other supported-card predicates
// below, but are also used by modal legality/commit paths earlier in this
// translation unit.  Keep declarations here so every compiler sees the same
// fail-closed candidate filtering.
std::vector<Card> SupportedTierMinions(const Player& player);
std::vector<Card> SupportedMinionsForRace(Race race);
std::vector<Card> SupportedCombinedChooseOneMinions();
bool AddRandomMinionToHand(Player& player, std::vector<Card> candidates);

// Blue Whelp modifies stat-bearing Tavern spells.  It must not turn a
// keyword-only, economy, transform, or consume-the-Tavern spell into a
// hidden stat buff merely because TavernSpellBehavior has a health field.
bool TavernSpellReceivesHealthBonus(TavernSpellEffect effect)
{
    switch (effect)
    {
        case TavernSpellEffect::BLOOD_GEM:
        case TavernSpellEffect::ALL_STATS:
        case TavernSpellEffect::ALL_STATS_AND_GOLDEN:
        case TavernSpellEffect::LEFTMOST_STATS:
        case TavernSpellEffect::ALL_AND_RACE:
        case TavernSpellEffect::ALL_RACE_AND_DIVINE_SHIELD:
        case TavernSpellEffect::RANDOM_STATS:
        case TavernSpellEffect::MENAGERIE_STATS:
        case TavernSpellEffect::ONE_PER_RACE_STATS:
        case TavernSpellEffect::SHOP_STATS:
        case TavernSpellEffect::TARGET_STATS:
        case TavernSpellEffect::TARGET_AND_RACE:
        case TavernSpellEffect::TARGET_STATS_REPEAT:
        case TavernSpellEffect::TARGET_STATS_AND_TAUNT:
        case TavernSpellEffect::TARGET_STATS_AND_WINDFURY:
        case TavernSpellEffect::TARGET_STATS_AND_REBORN:
        case TavernSpellEffect::TARGET_STATS_TOGGLE_TAUNT:
        case TavernSpellEffect::TARGET_SHARED_RACE_STATS:
        case TavernSpellEffect::TARGET_NEXT_COMBAT_BUFF:
        case TavernSpellEffect::TARGET_CHOOSE_ONE_STATS:
        case TavernSpellEffect::ALL_MINION_CHOOSE_ONE_STATS:
        case TavernSpellEffect::TARGET_OR_ALL_CHOOSE_ONE_STATS:
        case TavernSpellEffect::TARGET_RACE_SHOP_STATS_PERSISTENT:
        case TavernSpellEffect::SHOP_STATS_PERSISTENT:
        case TavernSpellEffect::RANDOM_SHOP_STATS_ON_REFRESH:
            return true;
        default:
            return false;
    }
}

bool TavernSpellReceivesAttackBonus(TavernSpellEffect effect)
{
    // Attack-only Tavern spells (currently Sanctify) must receive the
    // Synthesizer bonus even though they are intentionally excluded from the
    // health whitelist above.
    if (effect == TavernSpellEffect::DIVINE_SHIELD_ATTACK)
    {
        return true;
    }
    return TavernSpellReceivesHealthBonus(effect);
}

// Enchanted Sentinel and Humong'oz are continuous Tavern-spell auras.  Keep
// this derived from the live recruit field so copies, triples, and removals
// take effect immediately without a stale player counter.
std::pair<int, int> TavernSpellAuraBonus(const FieldZone& field)
{
    int attack = 0;
    int health = 0;
    field.ForEachAlive([&](const MinionData& data) {
        const auto& minion = data.value();
        const auto id = minion.GetCardID();
        const int scale = minion.IsGolden() ? 2 : 1;
        if (id == "BG32_341" || id == "BG32_341_G")
        {
            attack += scale;
            health += 2 * scale;
        }
        else if (id == "BG35_341" || id == "BG35_341_G")
        {
            attack += scale;
            health += scale;
        }
    });
    return {attack, health};
}
}

FieldZone& Player::GetField()
{
    return isInCombat ? battleField : recruitField;
}

void Player::ArmNextBoughtStats(int sourceIndex, int multiplier)
{
    if (multiplier > 0)
        nextBoughtStatsArms.emplace_back(sourceIndex, multiplier);
}

void Player::DispatchHeroDamage(const HeroDamageEvent& event)
{
    // Tichondrius-style recruit effects react to damage paid by the player
    // during the recruit phase.  Combat damage is resolved against a copied
    // battle field and must not create a non-persistent buff (or fire the
    // recruit trigger after combat has already ended).
    if (event.healthLost <= 0 ||
        event.source != HeroDamageSource::RECRUIT_SELF)
    {
        return;
    }
    season14.heroDamageThisTurn += event.healthLost;

    // Nether Pendant-style progress is driven only by recruit self-damage;
    // combat damage is excluded above.  Apply the newly earned aura to the
    // live Tavern immediately as well as to future fills.
    const auto [shopAttack, shopHealth] =
        season14.OnRecruitHeroDamage(event.healthLost);
    if (shopAttack != 0 || shopHealth != 0)
    {
        tavern.fieldZone.ForEachAlive([shopAttack, shopHealth](MinionData& data) {
            data.value().SetAttack(data.value().GetAttack() + shopAttack);
            data.value().SetHealth(data.value().GetHealth() + shopHealth);
        });
    }

    // Recruit self-damage is authoritative on recruitField.  Do not dispatch
    // into hand/Tavern, and do not use a combat copy or the opponent's field.
    recruitField.ForEachAlive([](MinionData& data) {
        data.value().ActivateHeroDamageTrigger();
    });
}

void Player::DispatchHeroPowerDamage(int damage)
{
    if (damage <= 0) return;
    std::vector<std::pair<int, int>> effects;
    recruitField.ForEachAlive([&](const MinionData& data) {
        const auto& minion = data.value();
        if (minion.GetCardID() == "BG22_HERO_000_Buddy")
            effects.emplace_back(minion.GetZonePosition(), damage);
        else if (minion.GetCardID() == "BG22_HERO_000_Buddy_G")
            effects.emplace_back(minion.GetZonePosition(), damage * 2);
    });
    for (const auto& [position, amount] : effects) {
        for (const int target : {position - 1, position + 1}) {
            if (target < 0 || target >= recruitField.GetCount()) continue;
            auto& minion = recruitField[static_cast<std::size_t>(target)];
            if (!minion.IsDestroyed()) {
                minion.SetAttack(minion.GetAttack() + amount);
                minion.SetHealth(minion.GetHealth() + amount);
            }
        }
    }
}

bool Player::ResolveDamagingHeroPower(int actualDamage)
{
    if (actualDamage < 0 || !season14.UseHeroPower()) return false;
    DispatchHeroPowerDamage(actualDamage);
    return true;
}

void Player::ApplyFreshMinionModifiers(Minion& minion)
{
    if (minion.GetCardID() == "BG32_HERO_001_Buddy" ||
        minion.GetCardID() == "BG32_HERO_001_Buddy_G")
    {
        const int multiplier = minion.GetCardID().ends_with("_G") ? 2 : 1;
        const int bonus = (season14.GoldSpentThisGame() / 3) * multiplier;
        minion.ApplyPersistentMinionStats(bonus, bonus);
    }
    if (minion.GetCardID() == "BG21_HERO_020_Buddy" || minion.GetCardID() == "BG21_HERO_020_Buddy_G")
        season14.EnableBuddyExtraHeroPowerUses(minion.GetCardID().ends_with("_G") ? 2 : 1);
    if (season14.persistentMinionAttack != 0 ||
        season14.persistentMinionHealth != 0)
    {
        minion.ApplyPersistentMinionStats(season14.persistentMinionAttack,
                                          season14.persistentMinionHealth);
    }
    const auto tierAura = FindTrinketBehavior("BG30_MagicItem_843t");
    if (minion.GetTier() <= tierAura.value && tierAura.value > 0)
    {
        // This lookup is only a compact way to keep the tier boundary in one
        // place; the actual aura is installed below from owned Trinkets.
        for (const auto& trinket : season14.trinkets)
        {
            const auto behavior = FindTrinketBehavior(
                Cards::FindCardByDbfID(trinket.dbfID).id);
            if (trinket.active && trinket.remainingUses > 0 &&
                behavior.effect == TrinketEffect::STATIC_TIER_MINION_STATS)
                minion.ApplyPersistentTierMinionStats(behavior.value, behavior.attack,
                                                      behavior.health);
        }
    }
    if (minion.GetCardID() == "BG35_342" ||
        minion.GetCardID() == "BG35_342_G")
        minion.ApplySkyGolemDeathrattleCount(season14.deathrattlesTriggered);
    if (minion.GetCardID() == "BG32_HERO_001_Buddy" ||
        minion.GetCardID() == "BG32_HERO_001_Buddy_G")
    {
        const int multiplier = minion.GetCardID().ends_with("_G") ? 2 : 1;
        minion.ApplyPersistentMinionStats(
            multiplier * (season14.GoldSpentThisGame() / 3),
            multiplier * (season14.GoldSpentThisGame() / 3));
    }
    if (minion.GetCardID() == "BG28_603t" || minion.GetCardID() == "BG28_603t_G") {
        minion.SetAttack(minion.GetAttack() + season14.persistentBeetleAttack);
        minion.SetHealth(minion.GetHealth() + season14.persistentBeetleHealth);
    }
    for (const auto& bonus : season14.persistentRaceStats)
        minion.ApplyPersistentRaceStats(bonus.race, bonus.attack, bonus.health);
    const auto batch4 = season14.HeroPowerBatch4PassiveModifiers();
    if (batch4.globalMinionAttack != 0)
    {
        minion.ApplyGlobalMinionAttack(batch4.globalMinionAttack);
    }

    // Tasty Lobster's deathrattle improves only copies created afterwards.
    // Apply the player-owned cumulative aura at every fresh-instance boundary
    // (Tavern, purchase, hand play, and summon), including generated normal
    // and golden Lobsters.  Existing instances are intentionally untouched.
    if (minion.GetCardID() == "BG36_202" ||
        minion.GetCardID() == "BG36_202_G")
    {
        const auto [attack, health] = season14.FutureLobsterStats();
        minion.ApplyFutureLobsterStats(attack, health);
    }
    if (minion.GetCardID() == "BG36_181" ||
        minion.GetCardID() == "BG36_181_G" ||
        minion.GetCardID() == "BG31_816" ||
        minion.GetCardID() == "BG31_816_G" ||
        minion.GetCardID() == "BG31_818" ||
        minion.GetCardID() == "BG31_818_G")
    {
        const auto [attack, health] = season14.FutureBallerStats();
        minion.ApplyFutureBallerStats(attack, health);
    }
    if (minion.GetCardID() == "BG36_524" || minion.GetCardID() == "BG36_524_G") {
        const int scale = minion.GetCardID().ends_with("_G") ? 14 : 7;
        minion.SetAttack(minion.GetAttack() + scale * season14.goldenMinionsPlayed);
        minion.SetHealth(minion.GetHealth() + scale * season14.goldenMinionsPlayed);
    }
    if (season14.GeneratedRewardGlobalAttack() != 0)
        minion.ApplyGlobalMinionAttack(season14.GeneratedRewardGlobalAttack());
}

void Player::ApplyTamuzoCombatSummon(Minion& summoned)
{
    int multiplier = 1;
    // Read the active combat field: a Tamuzo that died earlier in combat no
    // longer grants an aura, while multiple copies stack multiplicatively.
    GetField().ForEachAlive([&multiplier](MinionData& data) {
        const auto& id = data.value().GetCardID();
        if (id == "BG23_HERO_201_Buddy") multiplier *= 2;
        if (id == "BG23_HERO_201_Buddy_G") multiplier *= 3;
    });
    if (multiplier > 1) {
        summoned.SetAttack(summoned.GetAttack() * multiplier);
        summoned.SetHealth(summoned.GetHealth() * multiplier);
    }
}

void Player::RefreshSousChefHeroPowerUses()
{
    season14.ResetBuddyExtraHeroPowerUses();
    recruitField.ForEachAlive([this](MinionData& data) {
        const auto& id = data.value().GetCardID();
        if (id == "BG21_HERO_020_Buddy" || id == "BG21_HERO_020_Buddy_G")
            season14.EnableBuddyExtraHeroPowerUses(id.ends_with("_G") ? 2 : 1);
    });
}

void Player::ResolveWatfinGuess(bool correct, const Card& guessedMinion)
{
    if (!correct || guessedMinion.dbfID == 0) return;
    int copies = 0;
    recruitField.ForEachAlive([&copies](MinionData& data) {
        if (data.value().GetCardID() == "BG23_HERO_303_Buddy") copies = std::max(copies, 1);
        if (data.value().GetCardID() == "BG23_HERO_303_Buddy_G") copies = std::max(copies, 2);
    });
    for (int i = 0; i < copies; ++i) AddGeneratedDiscoverCopy(guessedMinion);
}

bool Player::ResolveZippersDeathrattle()
{
    // The pinned card text names a server-defined pool not present in the
    // authoritative card metadata.  Fail closed until that pool is supplied.
    return false;
}

void Player::ApplyFreshTavernMinionModifiers(Minion& minion)
{
    ApplyFreshMinionModifiers(minion);
    for (const auto& bonus : season14.persistentShopRaceStats)
    {
        if (minion.HasRace(bonus.race))
        {
            minion.SetAttack(minion.GetAttack() + bonus.attack);
            minion.SetHealth(minion.GetHealth() + bonus.health);
        }
    }
}

bool Player::ApplyGeneratedQuestReward(std::int32_t dbfID)
{
    if (!season14.ApplyGeneratedQuestReward(dbfID)) return false;
    if (season14.GeneratedRewardGlobalAttack() != 0)
    {
        recruitField.ForEachAlive([this](MinionData& data) {
            ApplyFreshMinionModifiers(data.value());
        });
        tavern.fieldZone.ForEachAlive([this](MinionData& data) {
            ApplyFreshMinionModifiers(data.value());
        });
        hand.ForEach([this](std::optional<CardData>& data) {
            if (data.has_value() && std::holds_alternative<Minion>(data.value()))
                ApplyFreshMinionModifiers(std::get<Minion>(data.value()));
        });
    }
    return true;
}

void Player::ResolveGeneratedQuestRewardEndTurn()
{
    if (!season14.HasGeneratedRewardParasol()) return;
    Minion* rightmost = nullptr;
    recruitField.ForEachAlive([&rightmost](MinionData& data) {
        rightmost = &data.value();
    });
    if (rightmost == nullptr) return;
    rightmost->SetHealth(rightmost->GetHealth() + 8);
    rightmost->SetGameTag(GameTag::STEALTH, 1);
    season14.generatedRewardStealthEntityID =
        static_cast<std::uint64_t>(rightmost->GetIndex());
}

void Player::ResolveFodderDefilerEndTurn()
{
    recruitField.ForEachAlive([this](MinionData& data) {
        const auto* behavior = FindFodderBehavior(data.value().GetCardID());
        if (behavior == nullptr ||
            behavior->kind != FodderBehaviorDefinition::Kind::DEFILER)
            return;
        // Each Defiler independently arms three successful refreshes. The
        // state object combines simultaneous arms using the strongest count.
        season14.ArmFodderDefilerRefreshes(3, behavior->foddersPerRefresh);
    });
}

void Player::ResolveEnigmaticHeadstoneEndTurn()
{
    for (const auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect == TrinketEffect::END_TURN_UNDEAD_ATTACK)
            ApplyPersistentRaceStats(behavior.race, behavior.attack,
                                     behavior.health);
    }
}

void Player::ResolveGeneratedQuestRewardStartCombat(FieldZone& combatField)
{
    if (season14.HasGeneratedRewardStolenGold())
    {
        std::vector<Minion*> alive;
        combatField.ForEachAlive([&alive](MinionData& data) {
            alive.push_back(&data.value());
        });
        if (!alive.empty()) alive.front()->MakeGolden();
        if (alive.size() > 1) alive.back()->MakeGolden();
    }
    if (season14.HasGeneratedRewardEvilTwin() && !combatField.IsFull())
    {
        Minion* highest = nullptr;
        combatField.ForEachAlive([&highest](MinionData& data) {
            if (highest == nullptr || data.value().GetHealth() > highest->GetHealth())
                highest = &data.value();
        });
        if (highest != nullptr)
        {
            Minion copy = *highest;
            if (getNextCardIndexCallback) copy.SetIndex(getNextCardIndexCallback());
            copy.getPlayerCallback = [this]() -> Player& { return *this; };
            combatField.Add(copy, combatField.GetCount());
        }
    }
}

void Player::ResolveGeneratedQuestRewardDeath(Minion& deadMinion)
{
    if (!season14.HasGeneratedRewardRitualDagger()) return;
    // The entity has already been removed from the combat field when this
    // hook runs.  Mutate that removed instance so Reborn and any later
    // permanent reconciliation retain the reward; searching recruitField
    // cannot find a dead minion and silently over-credits nothing.
    deadMinion.SetAttack(deadMinion.GetAttack() + 5);
    deadMinion.SetHealth(deadMinion.GetHealth() + 5);
}

void Player::ResolveGeneratedQuestRewardSnickerSnacks()
{
    if (!season14.HasGeneratedRewardSnickerSnacks()) return;
    std::vector<Minion*> candidates;
    recruitField.ForEachAlive([&candidates](MinionData& data) {
        if (data.value().HasBattlecry()) candidates.push_back(&data.value());
    });
    Random::shuffle(candidates.begin(), candidates.end());
    const auto count = std::min<std::size_t>(2, candidates.size());
    for (std::size_t i = 0; i < count; ++i)
        candidates[i]->ActivateTask(PowerType::POWER, *this);
}

void Player::ResolveGeneratedQuestRewardStartTurn()
{
    if (!season14.HasGeneratedRewardRedHand() || hand.IsFull()) return;
    std::vector<Minion*> candidates;
    hand.ForEach([&candidates](std::optional<CardData>& data) {
        if (data.has_value() && std::holds_alternative<Minion>(data.value()))
            candidates.push_back(&std::get<Minion>(data.value()));
    });
    if (candidates.empty()) return;
    const auto index = Random::get<std::size_t>(0, candidates.size() - 1);
    candidates[index]->SetAttack(candidates[index]->GetAttack() + 12);
    candidates[index]->SetHealth(candidates[index]->GetHealth() + 12);
}

bool Player::AddGeneratedDiscoverCopy(const Card& card)
{
    if (!season14.HasGeneratedRewardSecretSinstone() || hand.IsFull()) return false;
    if (card.GetCardType() == CardType::MINION)
    {
        Card plain = card.normalDbfID != 0
                         ? Cards::FindCardByDbfID(card.normalDbfID)
                         : card;
        Minion copy(plain);
        ApplyFreshMinionModifiers(copy);
        hand.Add(CardData{std::move(copy)});
        return true;
    }
    if (card.GetCardType() == CardType::SPELL ||
        card.GetCardType() == CardType::BATTLEGROUND_SPELL)
    {
        hand.Add(CardData{Spell(card)});
        return true;
    }
    return false;
}

void Player::ApplyPersistentRaceStats(Race race, int attack, int health)
{
    season14.AddPersistentRaceStats(race, attack, health);
    auto apply = [race, attack, health](Minion& minion) {
        minion.ApplyPersistentRaceStats(race, attack, health);
    };
    recruitField.ForEachAlive([&](MinionData& d) { apply(d.value()); });
    tavern.fieldZone.ForEachAlive([&](MinionData& d) { apply(d.value()); });
    hand.ForEach([&](std::optional<CardData>& d) {
        if (d.has_value() && std::holds_alternative<Minion>(d.value()))
            apply(std::get<Minion>(d.value()));
    });
}

void Player::ApplyPersistentRaceStatsExcept(Race race, int attack, int health,
                                             std::uint64_t excludedEntityID)
{
    season14.AddPersistentRaceStats(race, attack, health);
    auto apply = [race, attack, health](Minion& minion) {
        minion.ApplyPersistentRaceStats(race, attack, health);
    };
    recruitField.ForEachAlive([&](MinionData& d) {
        if (static_cast<std::uint64_t>(d.value().GetIndex()) != excludedEntityID)
            apply(d.value());
    });
    tavern.fieldZone.ForEachAlive([&](MinionData& d) {
        if (static_cast<std::uint64_t>(d.value().GetIndex()) != excludedEntityID)
            apply(d.value());
    });
    hand.ForEach([&](std::optional<CardData>& d) {
        if (d.has_value() && std::holds_alternative<Minion>(d.value()) &&
            static_cast<std::uint64_t>(std::get<Minion>(d.value()).GetIndex()) !=
                excludedEntityID)
            apply(std::get<Minion>(d.value()));
    });
}

void Player::ResolveDiscoverTriggers()
{
    std::vector<std::pair<std::uint64_t, int>> hooktusks;
    recruitField.ForEachAlive([&](const MinionData& data) {
        const auto& minion = data.value();
        if (minion.GetCardID() == "BG36_344" ||
            minion.GetCardID() == "BG36_344_G")
        {
            const int base = minion.GetCardID().ends_with("_G") ? 2 : 1;
            hooktusks.emplace_back(static_cast<std::uint64_t>(minion.GetIndex()),
                                   base + season14.goldenMinionsPlayed);
        }
    });
    for (const auto [entityID, amount] : hooktusks)
        ApplyPersistentRaceStatsExcept(Race::PIRATE, amount, amount, entityID);
}

void Player::ApplySpellRaceBuff(Race race, int attack, int health, bool includeHand)
{
    auto apply = [race, attack, health](Minion& minion) {
        if (race == Race::INVALID || minion.HasRace(race)) {
            minion.SetAttack(minion.GetAttack() + attack);
            minion.SetHealth(minion.GetHealth() + health);
        }
    };
    recruitField.ForEachAlive([&](MinionData& d) { apply(d.value()); });
    if (includeHand) hand.ForEach([&](std::optional<CardData>& d) {
        if (d.has_value() && std::holds_alternative<Minion>(d.value())) apply(std::get<Minion>(d.value()));
    });
}

void Player::ApplySpellSpecialBuff(int mode, int attack, int health)
{
    if (mode == 1) {
        for (int i = 0; i < hand.GetCount(); ++i)
            if (std::holds_alternative<Minion>(hand[i])) { auto& m = std::get<Minion>(hand[i]); m.SetAttack(m.GetAttack()+attack); m.SetHealth(m.GetHealth()+health); break; }
    } else if (mode == 2) {
        season14.AddPersistentShopStats(attack, health);
        tavern.fieldZone.ForEachAlive([&](MinionData& d) { d.value().SetAttack(d.value().GetAttack()+attack); d.value().SetHealth(d.value().GetHealth()+health); });
    }
}

void Player::ApplyTavernRaceBuff(Race race, int attack, int health)
{
    season14.AddPersistentShopRaceStats(race, attack, health);
    tavern.fieldZone.ForEachAlive([&](MinionData& d) {
        if (race == Race::INVALID || d.value().HasRace(race)) {
            d.value().SetAttack(d.value().GetAttack() + attack);
            d.value().SetHealth(d.value().GetHealth() + health);
        }
    });
}

void Player::SelectHero(std::size_t idx)
{
    const auto heroCard = Cards::FindCardByDbfID(heroChoices.at(idx));
    hero.Initialize(heroCard);

    // Hero powers are metadata-only cards in RosettaStone.  Install their
    // cost and lifecycle state on the owning player at selection time; the
    // bridge still decides whether a target-dependent power is exposed.
    const auto* batch4 =
        FindSeason14HeroPowerBehaviorBatch4(hero.card.heroPowerDbfID);
    const auto* batch1 =
        FindSeason14HeroPowerBehavior(hero.card.heroPowerDbfID);
    const auto* batch2 =
        FindSeason14HeroPowerBehaviorBatch2(hero.card.heroPowerDbfID);
    const auto* batch3 =
        FindSeason14HeroPowerBehaviorBatch3(hero.card.heroPowerDbfID);
    const auto* batch5 =
        FindSeason14HeroPowerBehaviorBatch5(hero.card.heroPowerDbfID);
    const int heroPowerCost = batch1 != nullptr
                                  ? (batch4 != nullptr
                                         ? batch4->cost
                                         : batch1->cost)
                                  : (batch2 != nullptr
                                         ? batch2->cost
                                         : (batch3 != nullptr
                                                ? batch3->cost
                                                : (batch4 != nullptr
                                                       ? batch4->cost
                                                       : (batch5 != nullptr
                                                              ? batch5->cost
                                                              : 0))));
    season14.SetHeroPower(hero.card.heroPowerDbfID, heroPowerCost,
                          hero.card.heroPowerDbfID != 0);

    // Tests and bridge callers may pre-seed a player's hand/board before
    // selecting a hero.  Install an aura on those existing instances too;
    // the per-instance operation is idempotent and therefore safe for the
    // normal empty-at-selection path.
    tavern.fieldZone.ForEach([this](MinionData& data) {
        ApplyFreshMinionModifiers(data.value());
    });
    recruitField.ForEach([this](MinionData& data) {
        ApplyFreshMinionModifiers(data.value());
    });
    hand.ForEach([this](std::optional<CardData>& data) {
        if (std::holds_alternative<Minion>(data.value()))
        {
            ApplyFreshMinionModifiers(std::get<Minion>(data.value()));
        }
    });

    selectHeroCallback(*this);
}

void Player::PrepareTavern()
{
    // Preserve the identity of cards already in the Tavern.  Independently
    // frozen cards survive a normal fill and must not receive a persistent
    // spell bonus more than once on every subsequent turn.
    std::set<int> existingPoolIndices;
    tavern.fieldZone.ForEach([&existingPoolIndices](MinionData& minion) {
        existingPoolIndices.insert(minion.value().GetPoolIndex());
    });
    prepareTavernMinionsCallback(*this);
    const auto batch4 = season14.HeroPowerBatch4PassiveModifiers();
    if (season14.persistentShopAttack != 0 ||
        season14.persistentShopHealth != 0 ||
        !season14.persistentShopRaceStats.empty() ||
        batch4.globalMinionAttack != 0 || batch4.mechShopAttack != 0 ||
        batch4.mechShopHealth != 0 || season14.persistentTavernTierMax != 0 ||
        season14.temporaryRefreshShopAttack != 0 ||
        season14.temporaryRefreshShopHealth != 0)
    {
        tavern.fieldZone.ForEach(
            [this, &existingPoolIndices, batch4](MinionData& minion) {
                if (existingPoolIndices.contains(
                        minion.value().GetPoolIndex()))
                {
                    return;
                }
                minion.value().SetAttack(minion.value().GetAttack() +
                                         season14.persistentShopAttack);
                minion.value().SetHealth(minion.value().GetHealth() +
                                         season14.persistentShopHealth);
                for (const auto& raceBonus : season14.persistentShopRaceStats)
                {
                    if (minion.value().HasRace(raceBonus.race))
                    {
                        minion.value().SetAttack(
                            minion.value().GetAttack() + raceBonus.attack);
                        minion.value().SetHealth(
                            minion.value().GetHealth() + raceBonus.health);
                    }
                }
                ApplyFreshMinionModifiers(minion.value());
                minion.value().SetAttack(minion.value().GetAttack() +
                                         season14.temporaryRefreshShopAttack);
                minion.value().SetHealth(minion.value().GetHealth() +
                                         season14.temporaryRefreshShopHealth);
                if (minion.value().GetCardID() == "BG35_150t" &&
                    (season14.persistentFodderAttack != 0 ||
                     season14.persistentFodderHealth != 0))
                {
                    minion.value().SetAttack(minion.value().GetAttack() +
                                             season14.persistentFodderAttack);
                    minion.value().SetHealth(minion.value().GetHealth() +
                                             season14.persistentFodderHealth);
                }
                if (season14.persistentTavernTierMax != 0 && minion.value().GetTier() <= season14.persistentTavernTierMax) {
                    minion.value().SetAttack(minion.value().GetAttack() + season14.persistentTavernTierAttack);
                    minion.value().SetHealth(minion.value().GetHealth() + season14.persistentTavernTierHealth);
                }
                if (batch4.mechShopAttack != 0 &&
                    minion.value().HasRace(Race::MECHANICAL))
                {
                    minion.value().SetAttack(
                        minion.value().GetAttack() + batch4.mechShopAttack);
                    minion.value().SetHealth(
                        minion.value().GetHealth() + batch4.mechShopHealth);
                }
            });
    }
}

void Player::ApplyNaturalBalance()
{
    for (int tier = 1; tier <= TIER_UPPER_LIMIT; ++tier)
    {
        std::vector<Minion*> candidates;
        recruitField.ForEachAlive([&](MinionData& data) {
            if (data.value().GetTier() == tier)
                candidates.push_back(&data.value());
        });
        if (candidates.empty()) continue;
        auto& target = *candidates[Random::get<std::size_t>(
            0, candidates.size() - 1)];
        target.SetAttack(target.GetAttack() + 2);
        target.SetHealth(target.GetHealth() + 2);
    }
}

bool Player::AddPlainCopyOfLeftmostHandCard()
{
    if (hand.IsFull() || hand.GetCount() == 0) return false;
    const CardData& source = hand[0];
    Card card;
    if (std::holds_alternative<Minion>(source)) {
        card = Cards::FindCardByID(std::get<Minion>(source).GetCardID());
        // "Plain" strips golden/enchantment instance state while retaining
        // the source card identity and current ruleset definition.
        if (card.normalDbfID != 0)
            card = Cards::FindCardByDbfID(card.normalDbfID);
    } else
        card = Cards::FindCardByID(std::get<Spell>(source).GetID());
    if (card.id.empty()) return false;
    if (card.GetCardType() == CardType::MINION) {
        Minion plain(card);
        ApplyFreshMinionModifiers(plain);
        hand.Add(CardData{std::move(plain)});
    }
    else if (card.GetCardType() == CardType::SPELL)
        hand.Add(CardData{Spell(card)});
    else
        return false;
    return true;
}

bool Player::AddMinionCopyToHand(const Minion& source)
{
    if (hand.IsFull()) return false;
    auto card = Cards::FindCardByID(source.GetCardID());
    if (card.id.empty() || card.GetCardType() != CardType::MINION)
        return false;
    // Double Vision grants a plain copy: a golden target must resolve to its
    // normal card definition, with no copied instance enchantments/state.
    if (card.normalDbfID != 0)
        card = Cards::FindCardByDbfID(card.normalDbfID);
    Minion plain(card);
    ApplyFreshMinionModifiers(plain);
    hand.Add(CardData{std::move(plain)});
    return true;
}

bool Player::BeginVoidPowerDiscover()
{
    if (season14.pendingDecision != Season14Decision::NONE || hand.IsFull())
        return false;
    std::vector<Card> candidates;
    for (const auto& card : Cards::GetTier5Minions())
        if (card.normalDbfID == 0 && card.hasBehavior)
            candidates.push_back(card);
    if (candidates.empty()) return false;
    Random::shuffle(candidates.begin(), candidates.end());
    std::vector<Card> gifts;
    for (const auto& gift : Cards::GetAllCards())
        if (gift.isBattlegroundsDarkGift &&
            FindDarkGiftBehavior(gift.id).effect != DarkGiftEffect::NONE)
            gifts.push_back(gift);
    if (gifts.empty()) return false;
    std::vector<Season14Offering> offerings;
    for (std::size_t i = 0; i < std::min<std::size_t>(3, candidates.size()); ++i)
    {
        Random::shuffle(gifts.begin(), gifts.end());
        const Minion preview(candidates[i]);
        const auto gift = std::find_if(
            gifts.begin(), gifts.end(), [&preview](const Card& candidate) {
                return DarkGiftTargetIsLegal(
                    preview, FindDarkGiftBehavior(candidate.id));
            });
        if (gift == gifts.end()) continue;
        offerings.push_back({candidates[i].dbfID, 0, gift->dbfID});
    }
    if (offerings.empty()) return false;
    season14.BeginOfferingDecision(Season14Decision::DISCOVER, 0, 132581,
                                   std::move(offerings));
    return true;
}

void Player::RefreshSpellcraft()
{
    recruitField.ForEach([](MinionData& data) {
        data.value().ExpireTemporaryEffects();
        data.value().ResetSpellcraftUses();
    });
    // Spellcraft cards expire at the next recruit start. Remove only cards
    // marked by this lifecycle, preserving ordinary copies of the same spell.
    for (int i = hand.GetCount() - 1; i >= 0; --i)
    {
        if (std::holds_alternative<Spell>(hand[i]) &&
            std::get<Spell>(hand[i]).IsTemporary())
        {
            hand.Remove(hand[i]);
        }
    }

    struct SpellcraftSpec
    {
        const char* minion;
        const char* spell;
        int copies;
    };
    constexpr SpellcraftSpec specs[] = {
        { "BG30_MagicItem_416", "BG30_MagicItem_416t", 1 },
        { "BG23_000", "BG23_000t", 1 },
        { "BG23_000_G", "BG23_000_Gt", 1 },
        { "BG23_004", "BG23_004t", 1 },
        { "BG23_004_G", "BG23_004_Gt", 1 },
        { "BG23_007", "BG23_007t", 1 },
        { "BG23_007_G", "BG23_007_Gt", 1 },
        { "BG23_008", "BG23_008t", 1 },
        { "BG23_008_G", "BG23_008_Gt", 1 },
        { "BG31_830", "BG31_830t", 1 },
        { "BG31_830_G", "BG31_830_Gt", 1 },
        { "BG34_Giant_035", "BG34_Giant_035t", 1 },
        { "BG34_Giant_035_G", "BG34_Giant_035t_G", 1 },
        { "BGS_200", "BG28_810", 1 },
        { "TB_BaconUps_256", "BG33_815", 1 },
    };
    for (const auto& spec : specs)
    {
        bool present = false;
        recruitField.ForEach([&](MinionData& data) {
            if (data.value().GetCardID() == spec.minion)
                present = true;
        });
        if (!present || hand.IsFull())
            continue;
        const auto card = Cards::FindCardByID(spec.spell);
        if (card.id.empty())
            continue;
        Spell spell(card);
        spell.SetTemporary(true);
        for (int i = 0; i < spec.copies && !hand.IsFull(); ++i)
            hand.Add(CardData{ spell });
    }
    constexpr const char* randomSpellcraft[] = {
        "BG23_000t", "BG23_004t", "BG23_007t", "BG23_008t", "BG31_830t"};
    recruitField.ForEach([&](MinionData& data) {
        const auto id = data.value().GetCardID();
        const int copies = id == "BG33_319_G" ? 2 : (id == "BG33_319" ? 1 : 0);
        for (int i = 0; i < copies && !hand.IsFull(); ++i) {
            const auto spellID = randomSpellcraft[Random::get<std::size_t>(
                0, std::size(randomSpellcraft) - 1)];
            const auto card = Cards::FindCardByID(spellID);
            if (card.id.empty()) continue;
            Spell spell(card);
            spell.SetTemporary(true);
            hand.Add(CardData{spell});
        }
    });
}

void Player::PurchaseMinion(std::size_t idx)
{
    if (idx >= static_cast<std::size_t>(tavern.fieldZone.GetCount()))
    {
        return;
    }

    const bool battlecryDiscount =
        tavern.fieldZone[idx].HasBattlecry() &&
        season14.battlecryBuysThisTurn < 2 &&
        std::any_of(season14.trinkets.begin(), season14.trinkets.end(),
                    [](const Season14PersistentEffect& trinket) {
                        return trinket.active && trinket.remainingUses > 0 &&
                               FindTrinketBehavior(
                                   Cards::FindCardByDbfID(trinket.dbfID).id)
                                   .effect == TrinketEffect::BATTLECRY_BUY_DISCOUNT;
                    });
    const int cost = battlecryDiscount
                         ? 0
                         : season14.MinionPurchaseCost(NUM_COIN_PURCHASE_MINION);
    if (remainCoin < cost)
    {
        return;
    }

    const bool purchasedPirate =
        tavern.fieldZone[idx].GetRace() == Race::PIRATE;
    const auto handCountBeforePurchase = hand.GetCount();
    purchaseMinionCallback(*this, idx);

    if (hand.GetCount() > handCountBeforePurchase)
    {
        if (battlecryDiscount) ++season14.battlecryBuysThisTurn;
        auto& purchased = std::get<Minion>(hand[hand.GetCount() - 1]);
        ApplyFreshMinionModifiers(purchased);
        const auto attack = season14.OnBuyMinionBatch4();
        purchased.SetAttack(purchased.GetAttack() + attack);
        if (attack > 0 && season14.heroPowerDbfID == 71455) {
            int healthMultiplier = 0;
            recruitField.ForEachAlive([&healthMultiplier](const MinionData& data) {
                const auto& id = data.value().GetCardID();
                if (id == "BG20_HERO_102_Buddy") healthMultiplier += 1;
                else if (id == "BG20_HERO_102_Buddy_G") healthMultiplier += 2;
            });
            purchased.SetHealth(purchased.GetHealth() + attack * healthMultiplier);
        }
        if (!nextBoughtStatsArms.empty())
        {
            recruitField.ForEachAlive([&purchased, this](MinionData& data) {
                auto& source = data.value();
                for (const auto& [sourceIndex, multiplier] : nextBoughtStatsArms)
                {
                    if (source.GetIndex() == sourceIndex)
                    {
                        source.SetAttack(source.GetAttack() + purchased.GetAttack() * multiplier);
                        source.SetHealth(source.GetHealth() + purchased.GetHealth() * multiplier);
                    }
                }
            });
            nextBoughtStatsArms.clear();
        }
    }

    // Payment is committed before after-buy observers run.  The purchased
    // entity is already in hand, and remains the event source for every
    // friendly board trigger below.
    remainCoin -= cost;
    RecordGoldSpent(cost);
    if (hand.GetCount() > handCountBeforePurchase)
    {
        auto& purchased = std::get<Minion>(hand[hand.GetCount() - 1]);
        recruitField.ForEachAlive([&purchased](MinionData& data) {
            data.value().ActivateTrigger(TriggerType::BUY_MINION, purchased);
        });
    }
    remainCoin += season14.OnBuyMinion(purchasedPirate);
    if (hand.GetCount() > handCountBeforePurchase)
    {
        int livingNightmareBonus = 0;
        recruitField.ForEachAlive([&livingNightmareBonus](const MinionData& data) {
            const auto& id = data.value().GetCardID();
            if (id == "BG20_HERO_280_Buddy") livingNightmareBonus += 2;
            else if (id == "BG20_HERO_280_Buddy_G") livingNightmareBonus += 4;
        });
        if (livingNightmareBonus > 0) {
            season14.AddTemporaryRefreshShopStats(livingNightmareBonus,
                                                  livingNightmareBonus);
            tavern.fieldZone.ForEachAlive([livingNightmareBonus](MinionData& data) {
                data.value().SetAttack(data.value().GetAttack() + livingNightmareBonus);
                data.value().SetHealth(data.value().GetHealth() + livingNightmareBonus);
            });
        }
        if (addRandomTavernMinionCallback) {
            int magnusCopies = 0;
            recruitField.ForEachAlive([&](const MinionData& data) {
                const auto& id = data.value().GetCardID();
                if (id == "TB_BaconShop_HERO_49_Buddy") magnusCopies += 1;
                else if (id == "TB_BaconShop_HERO_49_Buddy_G") magnusCopies += 2;
            });
            const auto& purchased = std::get<Minion>(hand[hand.GetCount() - 1]);
            const int tier = purchased.GetGameTag(GameTag::TECH_LEVEL);
            for (int copy = 0; copy < magnusCopies && tier > 0; ++copy)
                if (!addRandomTavernMinionCallback(*this, tier)) break;
        }
        // SI:7 Scout gains stats for each minion bought.  The generated
        // mapping historically covered only the golden card (and with an
        // outdated +4/+4 value), so apply both pinned variants explicitly.
        recruitField.ForEachAlive([](MinionData& data) {
            auto& scout = data.value();
            const auto& id = scout.GetCardID();
            const int bonus = id == "TB_BaconShop_HERO_01_Buddy" ? 2
                              : id == "TB_BaconShop_HERO_01_Buddy_G" ? 4 : 0;
            if (bonus > 0) {
                scout.SetAttack(scout.GetAttack() + bonus);
                scout.SetHealth(scout.GetHealth() + bonus);
            }
        });
        for (const auto& trinket : season14.trinkets)
        {
            if (!trinket.active || trinket.remainingUses == 0) continue;
            const auto behavior = FindTrinketBehavior(
                Cards::FindCardByDbfID(trinket.dbfID).id);
            if (behavior.effect != TrinketEffect::AFTER_BUY_RANDOM_FRIENDLY_BUFF)
                continue;
            std::vector<int> candidates;
            recruitField.ForEachAlive([&](MinionData& data) {
                candidates.push_back(data.value().GetIndex());
            });
            const int amount = std::min(behavior.amount,
                                        static_cast<int>(candidates.size()));
            for (int i = 0; i < amount; ++i)
            {
                const auto pick = Random::get<std::size_t>(0, candidates.size() - 1);
                const int entityIndex = candidates[pick];
                candidates.erase(candidates.begin() + pick);
                recruitField.ForEachAlive([&](MinionData& data) {
                    if (data.value().GetIndex() == entityIndex) {
                        data.value().SetAttack(data.value().GetAttack() + behavior.attack);
                        data.value().SetHealth(data.value().GetHealth() + behavior.health);
                    }
                });
            }
        }
    }
}

bool Player::TakeTavernMinionToHand(std::size_t idx, int attack, int health)
{
    if (hand.IsFull() || idx >= static_cast<std::size_t>(tavern.fieldZone.GetCount()) ||
        tavern.fieldZone[idx].IsDestroyed())
        return false;
    Minion minion = tavern.fieldZone.Remove(tavern.fieldZone[idx]);
    minion.SetAttack(attack);
    minion.SetHealth(health);
    hand.Add(minion, -1);
    return true;
}

bool Player::DevourRandomTavernForDemons(int multiplier)
{
    if (multiplier <= 0) return false;
    bool consumedAny = false;
    recruitField.ForEachAlive([&](MinionData& data) {
        auto& demon = data.value();
        if (!demon.HasRace(Race::DEMON)) return;
        std::vector<int> candidates;
        tavern.fieldZone.ForEach([&candidates](const MinionData& entry) {
            if (!entry.value().IsDestroyed())
                candidates.push_back(entry.value().GetZonePosition());
        });
        if (candidates.empty()) return;
        const auto slot = candidates[Random::get<std::size_t>(0, candidates.size() - 1)];
        auto& consumed = tavern.fieldZone[static_cast<std::size_t>(slot)];
        const int attack = consumed.GetAttack();
        const int health = consumed.GetHealth();
        const int poolIndex = consumed.GetPoolIndex();
        tavern.fieldZone.Remove(consumed);
        returnMinionCallback(poolIndex);
        demon.SetAttack(demon.GetAttack() + attack * multiplier);
        demon.SetHealth(demon.GetHealth() + health * multiplier);
        consumedAny = true;
    });
    return consumedAny;
}

void Player::UpdateSkyGolemsForDeathrattle()
{
    const auto apply = [this](Minion& minion) {
        if (minion.GetCardID() == "BG35_342" ||
            minion.GetCardID() == "BG35_342_G")
            minion.ApplySkyGolemDeathrattleCount(
                season14.deathrattlesTriggered);
    };
    recruitField.ForEachAlive([&apply](MinionData& data) { apply(data.value()); });
    battleField.ForEachAlive([&apply](MinionData& data) { apply(data.value()); });
    tavern.fieldZone.ForEachAlive([&apply](MinionData& data) { apply(data.value()); });
    hand.ForEach([&apply](std::optional<CardData>& data) {
        if (std::holds_alternative<Minion>(data.value()))
            apply(std::get<Minion>(data.value()));
    });
}

bool Player::SummonExactMinionCopy(std::size_t idx)
{
    if (recruitField.IsFull() || idx >= static_cast<std::size_t>(recruitField.GetCount()) ||
        recruitField[idx].IsDestroyed())
        return false;
    Minion copy = recruitField[idx];
    copy.SetIndex(getNextCardIndexCallback());
    copy.getPlayerCallback = [this]() -> Player& { return *this; };
    const auto summonPosition = recruitField.GetCount();
    recruitField.Add(copy, summonPosition);
    // Gallery summons into the recruit board, so normal SUMMON observers
    // must see the fresh entity just as they do for a played minion.  Do not
    // apply fresh-instance auras: this is an exact copy of current state.
    recruitField.ForEachAlive([&copy](MinionData& data) {
        data.value().ActivateTrigger(TriggerType::SUMMON, copy);
    });
    ApplySummonTrinkets(recruitField[recruitField.GetCount() - 1]);
    return true;
}

bool Player::SummonCombatSnapshot(Minion snapshot)
{
    if (battleField.IsFull()) return false;
    snapshot.SetIndex(getNextCardIndexCallback());
    snapshot.getPlayerCallback = [this]() -> Player& { return *this; };
    battleField.Add(snapshot, battleField.GetCount());
    battleField.ForEachAlive([&snapshot](MinionData& data) {
        data.value().ActivateTrigger(TriggerType::SUMMON, snapshot);
    });
    return true;
}

bool Player::CanPurchaseTavernSlot(std::size_t idx) const
{
    if (hand.IsFull() || idx >= tavern.SlotCount()) return false;
    if (idx < static_cast<std::size_t>(tavern.fieldZone.GetCount()))
    {
        const bool battlecryDiscount =
            tavern.fieldZone[idx].HasBattlecry() &&
            season14.battlecryBuysThisTurn < 2 &&
            std::any_of(season14.trinkets.begin(), season14.trinkets.end(),
                        [](const Season14PersistentEffect &trinket) {
                            return trinket.active && trinket.remainingUses > 0 &&
                                   FindTrinketBehavior(
                                       Cards::FindCardByDbfID(trinket.dbfID).id)
                                           .effect == TrinketEffect::BATTLECRY_BUY_DISCOUNT;
                        });
        const int cost = battlecryDiscount
                             ? 0
                             : season14.MinionPurchaseCost(NUM_COIN_PURCHASE_MINION);
        return remainCoin >= cost;
    }
    const auto &slot = tavern.spellSlots[idx - tavern.fieldZone.GetCount()];
    return slot.IsSpell() && remainCoin >= slot.AsSpell().GetCost();
}

bool Player::PurchaseTavernSlot(std::size_t idx)
{
    if (!CanPurchaseTavernSlot(idx)) return false;
    const std::size_t minionCount = static_cast<std::size_t>(tavern.fieldZone.GetCount());
    if (idx < minionCount)
    {
        PurchaseMinion(idx);
        return true;
    }
    auto &slot = tavern.spellSlots[idx - minionCount];
    Spell spell = slot.AsSpell();
    const int cost = spell.GetCost();
    tavern.spellSlots.erase(tavern.spellSlots.begin() + static_cast<std::ptrdiff_t>(idx - minionCount));
    hand.Add(CardData{spell});
    remainCoin -= cost;
    RecordGoldSpent(cost);
    return true;
}

void Player::PlayCard(std::size_t handIdx, std::size_t fieldIdx, int targetIdx)
{
    if (handIdx >= static_cast<std::size_t>(hand.GetCount()))
        return;
    if (std::holds_alternative<Minion>(hand[handIdx]))
    {
        Minion& handMinion = std::get<Minion>(hand[handIdx]);
        if (handMinion.IsHandLocked()) return;
        const bool magnetic = handMinion.IsMagnetic();
        // Magnetic cards consume a hand slot but do not consume board space;
        // their target must be a legal friendly Mech (or the explicit
        // Prosthetic Hand Undead exception).
        // Magnetic is an optional attachment mode.  With no target the card
        // is still a normal minion play (and consumes a board slot), as in
        // Battlegrounds; only a supplied target selects the attachment path.
        if (magnetic && targetIdx != -1)
        {
            if (targetIdx < 0 || targetIdx >= recruitField.GetCount() ||
                !handMinion.CanMagnetizeTo(
                    recruitField[static_cast<std::size_t>(targetIdx)]))
                return;
            Minion attachment = std::get<Minion>(hand.Remove(hand[handIdx]));
            attachment.MagnetizeOnto(
                recruitField[static_cast<std::size_t>(targetIdx)]);
            if (attachment.GetCardID() == "BG31_HERO_802_Buddy" ||
                attachment.GetCardID() == "BG31_HERO_802_Buddy_G")
                recruitField[static_cast<std::size_t>(targetIdx)].MakeGolden();
            ApplyFirstMinionDivineShield(
                recruitField[static_cast<std::size_t>(targetIdx)]);
            ApplyAfterPlayCardTrinkets(attachment.GetRace());
            return;
        }

        // Check the field is full
        if (recruitField.IsFull())
            return;

        // Check if we can play this card and the target is valid
        if (!handMinion.IsPlayableByCardReq(*this) ||
            !handMinion.IsValidPlayTarget(*this, targetIdx))
        {
            return;
        }

    CardData card = hand.Remove(hand[handIdx]);
        auto minion = std::get<Minion>(card);
        if (minion.IsGolden()) ++season14.goldenMinionsPlayed;
        if (minion.GetRace() == Race::PIRATE) ++season14.piratesPlayedThisGame;
        season14.RecordMinionPlay(false);
        ApplyFreshMinionModifiers(minion);
        minion.getPlayerCallback = [this]() -> Player& { return *this; };
        minion.SetIndex(getNextCardIndexCallback());

        const auto batch4 = season14.OnPlayMinionBatch4();
        minion.SetAttack(minion.GetAttack() + batch4.attack);
        minion.SetHealth(minion.GetHealth() + batch4.health);

        if (targetIdx == -1)
        {
            recruitField.Add(minion, fieldIdx);
            ApplyFirstMinionDivineShield(
                recruitField[static_cast<std::size_t>(minion.GetZonePosition())]);

            recruitField.ForEachAlive([&minion](MinionData& aliveMinion) {
                aliveMinion.value().ActivateTrigger(TriggerType::SUMMON,
                                                    minion);
            });
            ApplySummonTrinkets(recruitField[static_cast<std::size_t>(minion.GetZonePosition())]);
            ApplyAfterPlayCardTrinkets(minion.GetRace());

            minion.ActivateTask(PowerType::POWER, *this);
            if ((minion.GetCardID() == "BG22_HERO_201_Buddy" ||
                 minion.GetCardID() == "BG22_HERO_201_Buddy_G") &&
                addRandomMinionToHandCallback) {
                const int copies = minion.GetCardID().ends_with("_G") ? 2 : 1;
                for (int copy = 0; copy < copies; ++copy)
                    for (const int tier : {1, 3, 5})
                        if (!addRandomMinionToHandCallback(*this, tier)) break;
            }
            if ((minion.GetCardID() == "BG20_HERO_242_Buddy" ||
                 minion.GetCardID() == "BG20_HERO_242_Buddy_G") &&
                addRandomTavernMinionCallback) {
                const int copies = minion.GetCardID().ends_with("_G") ? 2 : 1;
                for (int copy = 0; copy < copies; ++copy)
                    for (int tier = 1; tier <= currentTier; ++tier)
                        if (!addRandomTavernMinionCallback(*this, tier)) break;
            }
            if (minion.HasBattlecry() && season14.ConsumeGeneratedRewardConch())
            {
                minion.ActivateTask(PowerType::POWER, *this);
                minion.ActivateTask(PowerType::POWER, *this);
            }
            if (minion.GetRace() == Race::DRAGON &&
                ShouldDuplicateDragonBattlecry())
                // PlayCard's POWER activation is the minion Battlecry path;
                // preserve the same source/target for the extra resolution.
                minion.ActivateTask(PowerType::POWER, *this);
        }
        else
        {
            Minion& target = recruitField[targetIdx];

            recruitField.Add(minion, fieldIdx);
            ApplyFirstMinionDivineShield(
                recruitField[static_cast<std::size_t>(minion.GetZonePosition())]);

            recruitField.ForEachAlive([&minion](MinionData& aliveMinion) {
                aliveMinion.value().ActivateTrigger(TriggerType::SUMMON,
                                                    minion);
            });
            ApplySummonTrinkets(recruitField[static_cast<std::size_t>(minion.GetZonePosition())]);
            ApplyAfterPlayCardTrinkets(minion.GetRace());

            minion.ActivateTask(PowerType::POWER, *this, target);
            if (minion.GetCardID() == "TB_BaconShop_HERO_18_Buddy" ||
                minion.GetCardID() == "TB_BaconShop_HERO_18_Buddy_G") {
                const int multiplier = minion.GetCardID().ends_with("_G") ? 2 : 1;
                const int amount = season14.piratesPlayedThisGame * multiplier;
                target.SetAttack(target.GetAttack() + amount);
                target.SetHealth(target.GetHealth() + amount);
            }
            if (minion.HasBattlecry() && season14.ConsumeGeneratedRewardConch())
            {
                minion.ActivateTask(PowerType::POWER, *this, target);
                minion.ActivateTask(PowerType::POWER, *this, target);
            }
            if (minion.GetRace() == Race::DRAGON &&
                ShouldDuplicateDragonBattlecry())
                // Targeted Battlecries receive the identical target again.
                minion.ActivateTask(PowerType::POWER, *this, target);
        }

        // Choose One is a public modal after the minion has been committed;
        // payment/board insertion above therefore remains atomic.  Only the
        // deterministic Sprightly Scarab branches are staged here.
        if (minion.GetCardID() == "BG27_084" ||
            minion.GetCardID() == "BG27_084_G" ||
            minion.GetCardID() == "BG30_123" ||
            minion.GetCardID() == "BG30_123_G" ||
            minion.GetCardID() == "BG36_330" ||
            minion.GetCardID() == "BG36_330_G" ||
            minion.GetCardID() == "BG36_332" ||
            minion.GetCardID() == "BG36_332_G")
        {
            std::uint32_t targetMask = 0;
            std::size_t targetSlot = 0;
            recruitField.ForEach([&targetMask, &targetSlot](MinionData& data) {
                const auto& candidate = data.value();
                if (!candidate.IsDestroyed() && candidate.HasRace(Race::BEAST) &&
                    targetSlot < 32)
                    targetMask |= std::uint32_t{1} << targetSlot;
                ++targetSlot;
            });
            const auto option0 = Cards::FindCardByID(
                minion.GetCardID() == "BG27_084_G" ? "BG27_084_Gt" :
                minion.GetCardID() == "BG30_123_G" ? "BG30_123_Gt" :
                minion.GetCardID() == "BG36_330_G" ? "BG36_330_Gt" :
                minion.GetCardID() == "BG30_123" ? "BG30_123t" :
                minion.GetCardID() == "BG36_330" ? "BG36_330t" :
                minion.GetCardID() == "BG36_332" ? "BG36_332" : "BG27_084t");
            const auto option1 = Cards::FindCardByID(
                minion.GetCardID() == "BG27_084_G" ? "BG27_084_Gt2" :
                minion.GetCardID() == "BG30_123_G" ? "BG30_123_Gt2" :
                minion.GetCardID() == "BG36_330_G" ? "BG36_330_Gt2" :
                minion.GetCardID() == "BG30_123" ? "BG30_123t2" :
                minion.GetCardID() == "BG36_330" ? "BG36_330t2" :
                minion.GetCardID() == "BG36_332" ? "BG36_332_G" : "BG27_084t2");
            // A target-dependent modal is only exposed when both generated
            // options and at least one Beast target are available; otherwise
            // do not leave the recruit phase permanently locked.
            const bool targetless = minion.GetCardID() != "BG27_084" &&
                                    minion.GetCardID() != "BG27_084_G";
            if (targetless)
                targetMask = 0;
            if (minion.HasCombinedChooseOne() && targetless) {
                const bool golden = minion.GetCardID().ends_with("_G");
                if (minion.GetCardID().starts_with("BG30_123")) {
                    season14.AddBloodGemBonus(golden ? 2 : 1, golden ? 2 : 1);
                    AddBloodGems(golden ? 8 : 4);
                } else if (minion.GetCardID().starts_with("BG36_330")) {
                    season14.AddFreeRefreshes(golden ? 4 : 2);
                    AddBloodGems(golden ? 6 : 3);
                }
            } else if ((targetless || targetMask != 0) && option0.dbfID != 0 && option1.dbfID != 0)
                season14.BeginChooseOne(static_cast<std::uint64_t>(minion.GetIndex()),
                                        targetMask, minion.GetDbfID(),
                                        {{option0.dbfID, 0}, {option1.dbfID, 0}});
        }

        // These simple Quilboar battlecries are intentionally resolved here
        // until the simulator's full generated-card task graph is available.
        // They still use the canonical AddBloodGems path and therefore retain
        // exact card identity, hand capacity, and replay semantics.
        if (minion.GetCardID() == "BG20_100")
        {
            AddBloodGems(2);
        }
        else if (minion.GetCardID() == "BG20_100_G")
        {
            AddBloodGems(4);
        }
        if (minion.GetRace() == Race::ELEMENTAL)
        {
            ++season14.unboundElementals;
            const auto result = season14.OnPlayElemental();
            coinToUpgradeTavern = std::max(
                0, coinToUpgradeTavern + result.upgradeCostDelta);
            for (const auto& trinket : season14.trinkets)
            {
                if (!trinket.active || trinket.remainingUses == 0) continue;
                const auto behavior = FindTrinketBehavior(
                    Cards::FindCardByDbfID(trinket.dbfID).id);
                if (behavior.effect != TrinketEffect::AFTER_PLAY_ELEMENTAL_SHOP_BUFF)
                {
                    if (behavior.effect == TrinketEffect::NEXT_TAVERN_SPELL_DISCOUNT)
                        season14.nextTavernSpellDiscount += behavior.value;
                    continue;
                }
                season14.AddPersistentShopStats(behavior.attack, behavior.health);
                tavern.fieldZone.ForEachAlive([&](MinionData& data) {
                    auto& shopMinion = data.value();
                    if (shopMinion.HasRace(Race::ELEMENTAL)) {
                        shopMinion.SetAttack(shopMinion.GetAttack() + behavior.attack);
                        shopMinion.SetHealth(shopMinion.GetHealth() + behavior.health);
                    }
                });
            }
            if (season14.unboundElementals >= 3) {
                season14.unboundElementals -= 3;
                int attack = 0, health = 0;
                tavern.fieldZone.ForEachAlive([&](MinionData& d) { if (d.value().GetHealth() > health) { health = d.value().GetHealth(); attack = d.value().GetAttack(); } });
                const int scale = minion.GetCardID() == "BG36_352_G" ? 2 : 1;
                minion.SetAttack(minion.GetAttack() + attack * scale);
                minion.SetHealth(minion.GetHealth() + health * scale);
            }
        }

        recruitField.ForEachAlive([&minion](MinionData& aliveMinion) {
            aliveMinion.value().ActivateTrigger(TriggerType::AFTER_PLAY_MINION,
                                                minion);
        });
        hand.ForEach([&minion](std::optional<CardData>& data) {
            if (data.has_value() && std::holds_alternative<Minion>(data.value()))
            {
                auto& observer = std::get<Minion>(data.value());
                observer.ActivateTrigger(TriggerType::AFTER_PLAY_MINION, minion);
            }
        });
        // Dark Gifts with a play-card stat trigger affect each gifted board
        // minion after the played card has committed successfully.
        recruitField.ForEachAlive([](MinionData& data) {
            data.value().ApplyPlayCardStatBonus();
        });

        // Baby Elekk's authoritative 36.4 text checks whether the played
        // minion has less Attack than this Buddy, then buffs both the played
        // minion and the Buddy. Snapshot the played Attack so multiple
        // Buddies evaluate the same committed value before any buffs land.
        const int playedAttackBeforeElekk = minion.GetAttack();
        recruitField.ForEachAlive([&minion, playedAttackBeforeElekk](MinionData& data) {
            auto& buddy = data.value();
            int bonus = 0;
            if (buddy.GetCardID() == "BG20_HERO_101_Buddy") bonus = 1;
            else if (buddy.GetCardID() == "BG20_HERO_101_Buddy_G") bonus = 2;
            if (bonus == 0 || playedAttackBeforeElekk >= buddy.GetAttack()) return;
            minion.SetAttack(minion.GetAttack() + bonus);
            minion.SetHealth(minion.GetHealth() + bonus);
            buddy.SetAttack(buddy.GetAttack() + bonus);
            buddy.SetHealth(buddy.GetHealth() + bonus);
        });

        // Prophet of the Boar triggers after a Quilboar is played (including
        // a golden one), rather than being a Battlecry on the Prophet itself.
        if (minion.GetRace() == Race::QUILBOAR)
        {
            recruitField.ForEachAlive([this](const MinionData& data) {
                const auto& observer = data.value();
                if (observer.GetCardID() == "BG20_203")
                {
                    AddBloodGems(1);
                }
                else if (observer.GetCardID() == "BG20_203_G")
                {
                    AddBloodGems(2);
                }
            });
        }
        ApplyAfterPlayCardTrinkets();
    }
    else
    {
        if (targetIdx == -1)
        {
            static_cast<void>(PlaySpell(handIdx));
        }
    }
}

bool Player::BeginTavernSpellDiscover(int amount, std::uint64_t sourceEntityID,
                                      std::int32_t sourceCardDbfID)
{
    if (amount <= 0 || hand.IsFull()) return false;
    // Golden battlecries produce sequential Discover modals.  Keep the
    // remaining count in Season14State so only one choice is pending at a
    // time and hand-cap/source-lifetime validation is reapplied between
    // choices.
    std::vector<Card> candidates;
    for (const auto& card : Cards::GetAllCards())
        if (card.isBattlegroundsPoolSpell && card.normalDbfID == 0 &&
            FindTavernSpellBehavior(card.id).effect != TavernSpellEffect::NONE)
            candidates.push_back(card);
    if (candidates.empty()) return false;
    season14.tavernSpellDiscoverRemaining = amount > 1 ? amount - 1 : 0;
    Random::shuffle(candidates.begin(), candidates.end());
    std::vector<Season14Offering> offerings;
    for (std::size_t i = 0; i < std::min<std::size_t>(3, candidates.size()); ++i)
        offerings.push_back({candidates[i].dbfID, 0});
    season14.BeginOfferingDecision(Season14Decision::DISCOVER, sourceEntityID,
                                   sourceCardDbfID, std::move(offerings));
    return true;
}

bool Player::ApplyChoice(std::size_t offeringIdx)
{
    if (season14.pendingDecision != Season14Decision::CHOICE &&
        season14.pendingDecision != Season14Decision::DISCOVER)
    {
        return false;
    }
    const bool wasDiscover = season14.pendingDecision == Season14Decision::DISCOVER;
    const bool galakrondGreed =
        season14.pendingTavernReplacementSlot >= 0 &&
        season14.pendingSourceCardDbfID == Cards::FindCardByID("TB_BaconShop_HP_011").dbfID;
    const bool dungarFlightpath = season14.pendingSourceCardDbfID == 75703;
    const bool ironforgeFlightpath = season14.pendingSourceCardDbfID == 75705;
    const bool powerOfStorm = season14.pendingSourceCardDbfID == 71909;
    const bool nagaConquest = season14.pendingSourceCardDbfID == 80007;
    const bool convictionImprovement = season14.pendingSourceCardDbfID == 73941;
    if (offeringIdx >= season14.pendingOfferings.size() ||
        (!galakrondGreed && !dungarFlightpath && !convictionImprovement && hand.IsFull()))
    {
        return false;
    }

    if (season14.pendingSourceEntityID != 0)
    {
        bool sourceStillPresent = false;
        recruitField.ForEachAlive([&](const MinionData& data) {
            const auto& source = data.value();
            if (static_cast<std::uint64_t>(source.GetIndex()) ==
                    season14.pendingSourceEntityID &&
                (season14.pendingSourceCardDbfID == 0 ||
                 source.GetDbfID() == season14.pendingSourceCardDbfID))
                sourceStillPresent = true;
        });
        if (!sourceStillPresent) return false;
    }

    const auto offering = season14.pendingOfferings[offeringIdx];

    // Conviction improvements are typed options, not cards.  Resolve them
    // before Card lookup so the negative sentinel IDs can never enter a hand
    // or be mistaken for an executable generated entity.
    if (season14.pendingSourceCardDbfID == 73941)
        return season14.ApplyConvictionImprovement(offeringIdx);
    // Naga Conquest is a real three-option Discover, not a generic choice
    // modal. Replayed or stale state must retain exactly three distinct,
    // supported normal Naga minions before any selected card is committed.
    if (nagaConquest)
    {
        if (season14.pendingOfferings.size() != 3)
            return false;
        std::set<std::int32_t> nagaOfferings;
        for (const auto& pending : season14.pendingOfferings)
        {
            const auto candidate = Cards::FindCardByDbfID(pending.dbfID);
            if (candidate.GetCardType() != CardType::MINION ||
                !candidate.isBattlegroundsPoolMinion ||
                candidate.normalDbfID != 0 || !candidate.hasBehavior ||
                !candidate.HasRace(Race::NAGA) ||
                !nagaOfferings.insert(candidate.dbfID).second)
                return false;
        }
    }
    const auto card = Cards::FindCardByDbfID(offering.dbfID);
    if (card.dbfID == 0 ||
         (card.GetCardType() != CardType::MINION &&
          card.GetCardType() != CardType::SPELL &&
          card.GetCardType() != CardType::BATTLEGROUND_SPELL &&
          card.GetCardType() != CardType::HERO_POWER &&
          !(card.GetCardType() == CardType::BATTLEGROUND_QUEST_REWARD &&
            IsSeason14GeneratedQuestReward(card.dbfID))))
    {
        return false;
    }

    if (powerOfStorm &&
        (card.GetCardType() != CardType::HERO_POWER ||
         card.normalDbfID != 0 || card.dbfID == 71909 ||
         FindSeason14HeroPowerBehavior(card.dbfID) == nullptr))
        return false;

    if (nagaConquest &&
        (card.GetCardType() != CardType::MINION ||
         !card.isBattlegroundsPoolMinion || card.normalDbfID != 0 ||
         !card.hasBehavior || !card.HasRace(Race::NAGA)))
        return false;

    if (galakrondGreed)
    {
        const auto slot = season14.pendingTavernReplacementSlot;
        if (card.GetCardType() != CardType::MINION ||
            !card.isBattlegroundsPoolMinion || card.normalDbfID != 0 ||
            !card.hasBehavior || card.GetTier() <= season14.pendingTavernReplacementTier ||
            card.GetTier() > 6 ||
            slot < 0 || slot >= tavern.fieldZone.GetCount() ||
            tavern.fieldZone[static_cast<std::size_t>(slot)].IsDestroyed())
            return false;
        auto replaced = tavern.fieldZone.Remove(
            tavern.fieldZone[static_cast<std::size_t>(slot)]);
        const bool wasFrozen = replaced.IsFrozen();
        if (replaced.GetPoolIndex() >= 0)
            returnMinionCallback(replaced.GetPoolIndex());
        Minion replacement(card);
        ApplyFreshMinionModifiers(replacement);
        replacement.SetFrozen(wasFrozen);
        tavern.fieldZone.Add(replacement, slot);
        return season14.SelectDecision(offeringIdx);
    }

    if (dungarFlightpath)
    {
        if (card.GetCardType() != CardType::HERO_POWER ||
            !season14.SelectFlightpath(card.dbfID))
            return false;
        return season14.SelectDecision(offeringIdx);
    }
    if (ironforgeFlightpath &&
        (card.GetCardType() != CardType::MINION ||
         !card.isBattlegroundsPoolMinion || card.normalDbfID != 0 ||
         !card.hasBehavior || card.GetTier() != currentTier))
        return false;
    const bool voidPower = season14.pendingSourceCardDbfID == 132581;
    const auto voidGift = voidPower
        ? FindDarkGiftBehavior(Cards::FindCardByDbfID(offering.darkGiftDbfID).id)
        : DarkGiftBehavior{};
    if (voidPower &&
        (offering.darkGiftDbfID == 0 || voidGift.effect == DarkGiftEffect::NONE))
        return false;
    if (voidPower &&
        (card.GetCardType() != CardType::MINION || card.normalDbfID != 0 ||
         card.GetTier() != 5 || !card.hasBehavior ||
         !DarkGiftTargetIsLegal(Minion(card), voidGift)))
        return false;

    // Recheck the constrained Hired Headhunter pool at commit time so a
    // stale/replayed modal cannot offer an ordinary or golden minion.
    if (season14.pendingDecision == Season14Decision::DISCOVER &&
        season14.pendingSourceCardDbfID != 0 &&
        FindTavernSpellBehavior(
            Cards::FindCardByDbfID(season14.pendingSourceCardDbfID).id)
                .effect == TavernSpellEffect::DISCOVER_BATTLECRY_MINION &&
        (card.GetCardType() != CardType::MINION || card.normalDbfID != 0 ||
         !CardDefs::FindCardDefByID(card.id).HasBattlecry()))
    {
        return false;
    }
    if (season14.pendingDecision == Season14Decision::DISCOVER &&
        season14.pendingSourceCardDbfID != 0 &&
        Cards::FindCardByDbfID(season14.pendingSourceCardDbfID).id ==
            "BG31_890" &&
        ((card.GetCardType() != CardType::MINION &&
          card.GetCardType() != CardType::SPELL) ||
         card.GetTier() != currentTier ||
         (card.GetCardType() == CardType::SPELL &&
          FindTavernSpellBehavior(card.id).effect == TavernSpellEffect::NONE)))
    {
        return false;
    }
    if (season14.pendingDecision == Season14Decision::DISCOVER &&
        Cards::FindCardByDbfID(season14.pendingSourceCardDbfID).id ==
            "BG31_892")
    {
        const auto supported = SupportedCombinedChooseOneMinions();
        const bool offered = std::any_of(
            supported.begin(), supported.end(),
            [&card](const Card& candidate) { return candidate.dbfID == card.dbfID; });
        if (!offered || hand.IsFull()) return false;
        Minion minion(card);
        minion.SetCombinedChooseOne(true);
        ApplyFreshMinionModifiers(minion);
        hand.Add(CardData{std::move(minion)});
        const bool selected = season14.SelectDecision(offeringIdx);
        if (selected && wasDiscover) ResolveDiscoverTriggers();
        return selected;
    }
    if (season14.pendingDecision == Season14Decision::DISCOVER &&
        Cards::FindCardByDbfID(season14.pendingSourceCardDbfID).id ==
            "BG34_888")
    {
        const bool offered = card.GetCardType() == CardType::MINION &&
            card.normalDbfID == 0 && card.HasRace(Race::UNDEAD) &&
            card.hasBehavior;
        if (!offered || hand.IsFull()) return false;
        Minion minion(card);
        minion.SetDiesAtRecruitEnd(true);
        ApplyFreshMinionModifiers(minion);
        hand.Add(CardData{std::move(minion)});
        const bool selected = season14.SelectDecision(offeringIdx);
        if (selected && wasDiscover) ResolveDiscoverTriggers();
        return selected;
    }

    const auto pendingSourceID =
        Cards::FindCardByDbfID(season14.pendingSourceCardDbfID).id;
    if (season14.pendingDecision == Season14Decision::DISCOVER &&
        (pendingSourceID == "BG36_342" || pendingSourceID == "BG36_342_G"))
    {
        const bool offered =
            (card.GetCardType() == CardType::SPELL ||
             card.GetCardType() == CardType::BATTLEGROUND_SPELL) &&
            card.isBattlegroundsPoolSpell && card.normalDbfID == 0 &&
            FindTavernSpellBehavior(card.id).effect != TavernSpellEffect::NONE;
        if (!offered || hand.IsFull()) return false;
    }

    if (card.GetCardType() == CardType::BATTLEGROUND_QUEST_REWARD)
    {
        if (!IsSeason14GeneratedQuestReward(card.dbfID)) return false;
        // Commit the modal before installing any effect.  This prevents an
        // invalid/stale selection from mutating player state.  The selected
        // DBF is retained for replay, while ApplyGeneratedQuestReward itself
        // remains the sole source of executable reward credit.
        const bool selected = season14.SelectDecision(offeringIdx);
        if (!selected) return false;
        season14.generatedQuestRewards.push_back(card.dbfID);
        ApplyGeneratedQuestReward(card.dbfID);
        if (selected && wasDiscover) ResolveDiscoverTriggers();
        return selected;
    }

    if (card.GetCardType() == CardType::MINION)
    {
        Minion minion(card);
        ApplyFreshMinionModifiers(minion);
        if (voidPower && !ApplyDarkGift(*this, minion, voidGift)) return false;
        hand.Add(CardData{ std::move(minion) });
        if (season14.pendingHandLock)
            std::get<Minion>(hand[hand.GetCount() - 1]).SetHandLocked(true);
    }
    else if (card.GetCardType() == CardType::HERO_POWER)
    {
        // Hero-power choice cards do not expose a generic Card::GetCost;
        // their costs are resolved by the selected hero-power registry.
        const auto* behavior = FindSeason14HeroPowerBehavior(card.dbfID);
        season14.SetHeroPower(card.dbfID,
                              behavior != nullptr ? behavior->cost :
                                  card.gameTags.contains(GameTag::COST) ?
                                      card.gameTags.at(GameTag::COST) : 0,
                              true);
        if (powerOfStorm) season14.powerOfStormActive = true;
    }
    else
    {
        hand.Add(CardData{ Spell(card) });
    }
    const auto sourceCardDbfID = season14.pendingSourceCardDbfID;
    const auto sourceEntityID = season14.pendingSourceEntityID;
    const bool selected = season14.SelectDecision(offeringIdx);
    if (selected && wasDiscover)
        AddGeneratedDiscoverCopy(card);
    if (selected && wasDiscover)
        ResolveDiscoverTriggers();
    if (selected && sourceCardDbfID == 89294)
        season14.reclaimedSoulsDeaths.clear();
    if (selected && season14.tavernSpellDiscoverRemaining > 0)
    {
        --season14.tavernSpellDiscoverRemaining;
        BeginTavernSpellDiscover(1, sourceEntityID, sourceCardDbfID);
    }
    season14.pendingHandLock = false;
    return selected;
}

bool Player::ResolveFlightpathCompletion()
{
    const auto completed = season14.flightpath.completedDbfID;
    if (completed == 0) return false;
    if (completed == 75704) {
        if (hand.IsFull()) return false;
        std::vector<Card> candidates;
        for (const auto& card : Cards::GetAllCards())
            if (card.isBattlegroundsPoolSpell && card.normalDbfID == 0 &&
                card.GetCardType() == CardType::BATTLEGROUND_SPELL &&
                card.gameTags.contains(GameTag::COST) &&
                card.gameTags.at(GameTag::COST) == 1 &&
                FindTavernSpellBehavior(card.id).effect != TavernSpellEffect::NONE)
                candidates.push_back(card);
        if (candidates.empty()) return false;
        Random::shuffle(candidates.begin(), candidates.end());
        hand.Add(CardData{Spell(candidates.front())});
        season14.TakeCompletedFlightpath();
        return true;
    }
    if (completed == 75705) {
        season14.TakeCompletedFlightpath();
        remainCoin += 2;
        return true;
    }
    if (completed != 75706) return false;

    // Ironforge is a current-tier minion Discover.  The offering is public
    // and deterministic under the simulator RNG; the selected card is later
    // committed by ApplyChoice, so a full hand leaves the modal retryable.
    if (hand.IsFull() || season14.pendingDecision != Season14Decision::NONE)
        return false;
    std::vector<Card> candidates;
    for (const auto& card : Cards::GetAllCards())
        if (card.isBattlegroundsPoolMinion && card.normalDbfID == 0 &&
            card.hasBehavior && card.GetCardType() == CardType::MINION &&
            card.GetTier() == currentTier)
            candidates.push_back(card);
    if (candidates.empty()) return false;
    Random::shuffle(candidates.begin(), candidates.end());
    std::vector<Season14Offering> offerings;
    for (std::size_t i = 0; i < std::min<std::size_t>(3, candidates.size()); ++i)
        offerings.push_back({candidates[i].dbfID, 0});
    season14.BeginOfferingDecision(Season14Decision::DISCOVER, 0, 75706,
                                   std::move(offerings));
    season14.TakeCompletedFlightpath();
    return true;
}

void Player::ApplyTavernSpellTrinkets()
{
    for (const auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect == TrinketEffect::AFTER_TAVERN_SPELL_SHOP_BUFF)
        {
            season14.AddPersistentShopStats(behavior.attack, behavior.health);
            tavern.fieldZone.ForEachAlive([&](MinionData& data) {
                auto& shopMinion = data.value();
                shopMinion.SetAttack(shopMinion.GetAttack() + behavior.attack);
                shopMinion.SetHealth(shopMinion.GetHealth() + behavior.health);
            });
        }
        else if (behavior.effect == TrinketEffect::AFTER_TAVERN_SPELL_RACE_BUFF)
        {
            ApplyPersistentRaceStats(behavior.race, behavior.attack,
                                     behavior.health);
        }
    }
    ApplyAfterPlayCardTrinkets();
}

void Player::ApplyAfterPlayCardTrinkets(Race playedRace)
{
    for (const auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect == TrinketEffect::AFTER_PLAY_NAGA_SPELLCRAFT)
        {
            if (playedRace == Race::NAGA)
                (void)SimpleTasks::RandomSpellcraftToHandTask{}.Run(*this);
            continue;
        }
        if (behavior.effect != TrinketEffect::AFTER_PLAY_CARD_RANDOM_RACE_BUFF) continue;
        std::vector<int> candidates;
        recruitField.ForEachAlive([&](MinionData& data) {
            if (data.value().HasRace(behavior.race)) candidates.push_back(data.value().GetIndex());
        });
        if (candidates.empty()) continue;
        const int selected = candidates[Random::get<std::size_t>(0, candidates.size() - 1)];
        recruitField.ForEachAlive([&](MinionData& data) {
            if (data.value().GetIndex() == selected) {
                data.value().SetAttack(data.value().GetAttack() + behavior.attack);
                data.value().SetHealth(data.value().GetHealth() + behavior.health);
            }
        });
    }
}

void Player::ApplyAfterRebornTrinkets()
{
    for (const auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect != TrinketEffect::AFTER_REBORN_STATS) continue;
        // Deathwhisper fires from combat Reborn. Apply the temporary combat
        // buff to the active field; never mutate the recruit copy while a
        // battle is resolving.
        GetField().ForEachAlive([&](MinionData& data) {
            auto& minion = data.value();
            minion.SetAttack(minion.GetAttack() + behavior.attack);
            minion.SetHealth(minion.GetHealth() + behavior.health);
        });
    }
}

void Player::ApplyStartCombatTrinkets()
{
    // Piloted Whirl-O-Tron copies the two left-most friendly Deathrattles at
    // combat start. This is a Buddy effect, not a targeted play effect;
    // exclude every Whirl-O-Tron from the source list and preserve golden
    // twice-copy semantics.
    std::vector<Minion*> deathrattleSources;
    battleField.ForEachAlive([&](MinionData& data) {
        auto& candidate = data.value();
        if (candidate.HasDeathrattle() &&
            candidate.GetCardID() != "BG21_HERO_030_Buddy" &&
            candidate.GetCardID() != "BG21_HERO_030_Buddy_G")
            deathrattleSources.push_back(&candidate);
    });
    const auto sourceCount = std::min<std::size_t>(2, deathrattleSources.size());
    battleField.ForEachAlive([&](MinionData& data) {
        auto& buddy = data.value();
        int copies = buddy.GetCardID() == "BG21_HERO_030_Buddy_G" ? 2 :
                     buddy.GetCardID() == "BG21_HERO_030_Buddy" ? 1 : 0;
        for (int copy = 0; copy < copies; ++copy)
            for (std::size_t i = 0; i < sourceCount; ++i)
                deathrattleSources[i]->CopyDeathrattleTo(buddy);
    });
    for (const auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect == TrinketEffect::START_COMBAT_HEALTH_FROM_ATTACK)
        {
            battleField.ForEachAlive([](MinionData& data) {
                auto& minion = data.value();
                minion.SetHealth(minion.GetHealth() + minion.GetAttack() / 2);
            });
            continue;
        }
        if (behavior.effect == TrinketEffect::START_COMBAT_NAGA_SPELLCRAFT)
        {
            battleField.ForEachAlive([&](MinionData& data) {
                auto& minion = data.value();
                if (minion.HasRace(Race::NAGA))
                    minion.AddDarkGiftDeathrattleTask(
                        SimpleTasks::RandomSpellcraftToHandTask{});
            });
            continue;
        }
        if (behavior.effect == TrinketEffect::START_COMBAT_QUILBOAR_BLOOD_GEMS)
        {
            battleField.ForEachAlive([&](MinionData& data) {
                auto& minion = data.value();
                if (minion.HasRace(Race::QUILBOAR))
                    minion.AddDarkGiftDeathrattleTask(
                        SimpleTasks::GenerateBloodGemsTask{behavior.value});
            });
            continue;
        }
        if (behavior.effect == TrinketEffect::START_COMBAT_ELEMENTAL_FROSTLING)
        {
            // The pinned text says *two* friendly Elementals, not every
            // Elemental. Select a deterministic seeded-random subset once at
            // combat start; later summons must not inherit this one-shot
            // start-of-combat grant.
            std::vector<Minion*> candidates;
            battleField.ForEachAlive([&candidates](MinionData& data) {
                if (data.value().HasRace(Race::ELEMENTAL))
                    candidates.push_back(&data.value());
            });
            Random::shuffle(candidates.begin(), candidates.end());
            const auto count = std::min<std::size_t>(2, candidates.size());
            for (std::size_t i = 0; i < count; ++i)
                candidates[i]->AddDarkGiftDeathrattleTask(
                    SimpleTasks::SummonTask{"BG26_537", 1});
            continue;
        }
        if (behavior.effect == TrinketEffect::START_COMBAT_BEAST_SCALING)
        {
            const int scale = behavior.attack + trinket.triggerProgress;
            battleField.ForEachAlive([&](MinionData& data) {
                if (data.value().HasRace(Race::BEAST))
                {
                    data.value().SetAttack(data.value().GetAttack() + scale);
                    data.value().SetHealth(data.value().GetHealth() + scale);
                }
            });
            continue;
        }
        // Blood Golem Sticker is a death observer, not a start-of-combat
        // deathrattle grant. It is resolved from Battle::ProcessDestroy so
        // summoned Quilboars and their final Blood Gem counts are included.
        if (behavior.effect == TrinketEffect::START_COMBAT_EDGE_SHIELDS)
        {
            if (battleField.GetCount() > 0)
            {
                battleField[0].SetGameTag(GameTag::DIVINE_SHIELD, 1);
                battleField[battleField.GetCount() - 1].SetGameTag(
                    GameTag::DIVINE_SHIELD, 1);
            }
            continue;
        }
        if (behavior.effect == TrinketEffect::START_COMBAT_LEFT_COPY)
        {
            if (battleField.IsFull() || battleField.GetCount() == 0) continue;
            Minion copy = battleField[0];
            copy.SetIndex(getNextCardIndexCallback ? getNextCardIndexCallback() : copy.GetIndex());
            copy.getPlayerCallback = [this]() -> Player& { return *this; };
            battleField.Add(copy, battleField.GetCount());
            Minion& added = battleField[battleField.GetCount() - 1];
            battleField.ForEachAlive([&added](MinionData& data) {
                data.value().ActivateTrigger(TriggerType::SUMMON, added);
            });
            ApplySummonTrinkets(added);
            continue;
        }
        if (behavior.effect == TrinketEffect::START_COMBAT_UNDEAD_EDGE_REBORN)
        {
            std::vector<Minion*> undead;
            battleField.ForEachAlive([&](MinionData& data) {
                if (data.value().HasRace(Race::UNDEAD)) undead.push_back(&data.value());
            });
            if (!undead.empty()) undead.front()->SetReborn(true);
            if (undead.size() > 1) undead.back()->SetReborn(true);
            continue;
        }
        if (behavior.effect == TrinketEffect::START_COMBAT_TRIGGER_DEATHRATTLES)
        {
            std::vector<std::uint64_t> ids;
            battleField.ForEachAlive([&](MinionData& data) {
                if (data.value().HasDeathrattle()) ids.push_back(data.value().GetIndex());
            });
            for (const auto id : ids)
                battleField.ForEachAlive([&](MinionData& data) {
                    if (data.value().GetIndex() == id)
                        data.value().ActivateTask(PowerType::DEATHRATTLE, *this);
                });
            continue;
        }
        if (behavior.effect == TrinketEffect::START_COMBAT_HIGHEST_HAND_MINION)
        {
            SimpleTasks::StartCombatHighestHandMinionSummonTask task{
                behavior.attack, behavior.health};
            Minion source;
            task.Run(*this, source);
            continue;
        }
        if (behavior.effect == TrinketEffect::START_COMBAT_NEUTRAL_TRIPLE)
        {
            battleField.ForEachAlive([](MinionData& data) {
                auto& minion = data.value();
                if (minion.GetRace() == Race::INVALID)
                {
                    minion.SetAttack(minion.GetAttack() * 3);
                    minion.SetHealth(minion.GetHealth() * 3);
                }
            });
            continue;
        }
        if (behavior.effect == TrinketEffect::START_COMBAT_DRAGON_MAX_ATTACK)
        {
            int highest = 0;
            battleField.ForEachAlive([&highest](MinionData& data) {
                if (data.value().HasRace(Race::DRAGON))
                    highest = std::max(highest, data.value().GetAttack());
            });
            battleField.ForEachAlive([highest](MinionData& data) {
                auto& minion = data.value();
                if (minion.HasRace(Race::DRAGON)) minion.SetAttack(highest);
            });
            continue;
        }
        if (behavior.effect == TrinketEffect::START_COMBAT_LEFTMOST_HAND_STATS)
        {
            const Minion* highest = nullptr;
            hand.ForEach([&](const std::optional<CardData>& entry) {
                if (!entry || !std::holds_alternative<Minion>(*entry)) return;
                const auto& candidate = std::get<Minion>(*entry);
                if (!highest || candidate.GetHealth() > highest->GetHealth()) highest = &candidate;
            });
            if (highest && battleField.GetCount() > 0)
            {
                battleField[0].SetAttack(battleField[0].GetAttack() + highest->GetAttack());
                battleField[0].SetHealth(battleField[0].GetHealth() + highest->GetHealth());
            }
            continue;
        }
        if (behavior.effect == TrinketEffect::START_COMBAT_LOWEST_ATTACK_DOUBLE)
        {
            std::vector<Minion*> candidates;
            battleField.ForEachAlive([&candidates](MinionData& data) { candidates.push_back(&data.value()); });
            std::stable_sort(candidates.begin(), candidates.end(), [](const Minion* a, const Minion* b) {
                return a->GetAttack() < b->GetAttack();
            });
            const auto count = std::min<std::size_t>(2, candidates.size());
            for (std::size_t i = 0; i < count; ++i)
            {
                candidates[i]->SetAttack(candidates[i]->GetAttack() * 2);
                candidates[i]->SetHealth(candidates[i]->GetHealth() * 2);
            }
            continue;
        }
        if (behavior.effect == TrinketEffect::START_COMBAT_LEFT_BEAST_SHIELDS)
        {
            int granted = 0;
            battleField.ForEachAlive([&granted](MinionData& data) {
                auto& minion = data.value();
                if (granted < 2 && minion.HasRace(Race::BEAST))
                {
                    minion.SetGameTag(GameTag::DIVINE_SHIELD, 1);
                    ++granted;
                }
            });
            continue;
        }
        if (behavior.effect == TrinketEffect::START_COMBAT_HIGHEST_TIER_DRAGON_GOLDEN)
        {
            Minion* best = nullptr;
            battleField.ForEachAlive([&best](MinionData& data) {
                auto& minion = data.value();
                if (!minion.HasRace(Race::DRAGON)) return;
                if (!best || minion.GetTier() > best->GetTier()) best = &minion;
            });
            if (best && best->CanMakeGolden()) best->MakeGolden();
            continue;
        }
        if (behavior.effect == TrinketEffect::START_COMBAT_THREE_BLOOD_GEMS)
        {
            battleField.ForEachAlive([](MinionData& data) {
                auto& minion = data.value();
                minion.ApplyBloodGem(1, 1);
                minion.ApplyBloodGem(1, 1);
                minion.ApplyBloodGem(1, 1);
            });
            continue;
        }
        if (behavior.effect == TrinketEffect::START_COMBAT_TYPE_STATS)
        {
            std::set<Race> seen;
            battleField.ForEachAlive([&](MinionData& data) {
                auto& minion = data.value();
                if (seen.insert(minion.GetRace()).second && minion.GetRace() != Race::INVALID)
                {
                    minion.SetAttack(minion.GetAttack() + behavior.attack);
                    minion.SetHealth(minion.GetHealth() + behavior.health);
                }
            });
            continue;
        }
        if (behavior.effect == TrinketEffect::START_COMBAT_NAGA_HEALTH)
        {
            const int bonus = behavior.health + season14.SuccessfulSpellCount() / 4;
            battleField.ForEachAlive([&](MinionData& data) {
                if (data.value().HasRace(Race::NAGA))
                    data.value().SetHealth(data.value().GetHealth() + bonus);
            });
            continue;
        }
        if (behavior.effect == TrinketEffect::START_COMBAT_RANDOM_PIRATE_SHIELDS)
        {
            std::vector<Minion*> pirates;
            battleField.ForEachAlive([&](MinionData& data) {
                if (data.value().HasRace(Race::PIRATE)) pirates.push_back(&data.value());
            });
            Random::shuffle(pirates.begin(), pirates.end());
            const auto count = std::min<std::size_t>(behavior.value, pirates.size());
            for (std::size_t i = 0; i < count; ++i) pirates[i]->SetGameTag(GameTag::DIVINE_SHIELD, 1);
            continue;
        }
        if (behavior.effect == TrinketEffect::START_COMBAT_MURLOC_MAX_ATTACK)
        {
            int highest = 0;
            hand.ForEach([&highest](const std::optional<CardData>& entry) {
                if (entry && std::holds_alternative<Minion>(*entry) &&
                    std::get<Minion>(*entry).GetAttack() > highest)
                    highest = std::get<Minion>(*entry).GetAttack();
            });
            battleField.ForEachAlive([highest](MinionData& data) {
                if (data.value().HasRace(Race::MURLOC)) data.value().SetAttack(data.value().GetAttack() + highest);
            });
            continue;
        }
        if (behavior.effect == TrinketEffect::START_COMBAT_RALLY_SHIELDS)
        {
            battleField.ForEachAlive([](MinionData& data) {
                if (data.value().GetGameTag(GameTag::BACON_RALLY) != 0)
                    data.value().SetGameTag(GameTag::DIVINE_SHIELD, 1);
            });
            continue;
        }
        if (behavior.effect != TrinketEffect::START_COMBAT_MINION_STATS) continue;
        battleField.ForEachAlive([&behavior](MinionData& data) {
            data.value().SetAttack(data.value().GetAttack() + behavior.attack);
            data.value().SetHealth(data.value().GetHealth() + behavior.health);
        });
    }
}

void Player::ApplySummonTrinkets(Minion& summoned)
{
    if (summoned.IsDestroyed()) return;
    for (auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect == TrinketEffect::SUMMON_DIVINE_SHIELD)
        {
            if (!isInCombat) continue;
            if (behavior.value <= 0 || trinket.triggerProgress >= behavior.value)
                continue;
            summoned.SetGameTag(GameTag::DIVINE_SHIELD, 1);
            ++trinket.triggerProgress;
            continue;
        }
        if (behavior.effect == TrinketEffect::SUMMON_MECH_RANDOM_DIVINE_SHIELD &&
            isInCombat && summoned.HasRace(Race::MECHANICAL))
        {
            std::vector<Minion*> candidates;
            battleField.ForEachAlive([&](MinionData& data) {
                if (data.value().HasRace(Race::MECHANICAL))
                    candidates.push_back(&data.value());
            });
            if (!candidates.empty())
                candidates[Random::get<std::size_t>(0, candidates.size() - 1)]
                ->SetGameTag(GameTag::DIVINE_SHIELD, 1);
        }
        if (behavior.effect == TrinketEffect::SUMMON_BEAST_DOUBLE_ATTACK &&
            isInCombat && summoned.HasRace(Race::BEAST))
            summoned.SetAttack(summoned.GetAttack() * 2);
        if (behavior.effect == TrinketEffect::SUMMON_BEAST_STATS &&
            summoned.HasRace(Race::BEAST))
        {
            summoned.SetAttack(summoned.GetAttack() + behavior.attack);
            summoned.SetHealth(summoned.GetHealth() + behavior.health);
        }
        if (behavior.effect == TrinketEffect::START_COMBAT_BEAST_SCALING &&
            summoned.HasRace(Race::BEAST))
            ++trinket.triggerProgress;
        if (behavior.effect == TrinketEffect::SUMMON_BEAST_RANDOM_MINION &&
            summoned.HasRace(Race::BEAST) &&
            behavior.value > 0 && ++trinket.triggerProgress >= behavior.value)
        {
            trinket.triggerProgress = 0;
            (void)SimpleTasks::RandomCardToHandTask{
                Race::BEAST, 0, 1}.Run(*this);
        }
    }
}

void Player::ResolveStartTurnTrinkets()
{
    for (const auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect != TrinketEffect::START_TURN_GOLD_DAMAGE) continue;
        remainCoin += behavior.attack;
        // Wax Imprinter's payment is real recruit self-damage.  Do not gate
        // it on remaining health: lethal damage must still resolve through
        // Hero::TakeDamage so defeat and downstream health-loss triggers fire.
        if (behavior.value > 0)
            hero.TakeDamage(*this, behavior.value,
                            HeroDamageSource::RECRUIT_SELF);
    }
}

bool Player::ShouldDuplicateDragonBattlecry() const noexcept
{
    for (const auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect == TrinketEffect::DUPLICATE_DRAGON_BATTLECRY)
            return true;
    }
    return false;
}

void Player::ApplyFirstMinionDivineShield(Minion& minion)
{
    if (season14.firstMinionPlayedThisTurn) return;
    for (const auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        if (FindTrinketBehavior(Cards::FindCardByDbfID(trinket.dbfID).id).effect ==
            TrinketEffect::FIRST_MINION_DIVINE_SHIELD)
        {
            minion.SetGameTag(GameTag::DIVINE_SHIELD, 1);
            season14.firstMinionPlayedThisTurn = true;
            return;
        }
    }
}

bool Player::ApplySpellChoice(std::size_t offeringIdx)
{
    if (offeringIdx > 1 || season14.spellModal.kind == Season14SpellModalKind::NONE)
        return false;
    if (season14.pendingDecision != Season14Decision::CHOOSE_ONE)
        return false;
    if (season14.spellModal.kind ==
        Season14SpellModalKind::DISCOVER_TIER_MINION_OR_SPELL)
    {
        const auto source = Cards::FindCardByDbfID(
            season14.pendingSourceCardDbfID);
        if (source.id != "BG31_890") return false;
        std::vector<Card> candidates;
        if (offeringIdx == 0)
            candidates = SupportedTierMinions(*this);
        else
            for (const auto& card : Cards::GetAllCards())
                if (card.isBattlegroundsPoolSpell && card.normalDbfID == 0 &&
                    card.GetTier() == currentTier &&
                    FindTavernSpellBehavior(card.id).effect !=
                        TavernSpellEffect::NONE)
                    candidates.push_back(card);
        if (candidates.empty() || hand.IsFull()) return false;
        Random::shuffle(candidates.begin(), candidates.end());
        const auto count = std::min<std::size_t>(3, candidates.size());
        std::vector<Season14Offering> offerings;
        offerings.reserve(count);
        for (std::size_t i = 0; i < count; ++i)
            offerings.push_back({candidates[i].dbfID, 0});
        season14.BeginOfferingDecision(Season14Decision::DISCOVER, 0,
                                       source.dbfID, std::move(offerings));
        return true;
    }
    if (season14.spellModal.kind == Season14SpellModalKind::ALL_MINION_STATS)
    {
        int attack = 2;
        int health = 2;
        if (offeringIdx == 1)
        {
            season14.deferredMinionAttack += 4;
            season14.deferredMinionHealth += 4;
            season14.deferredMinionStatTurns = 1;
        }
        else
        {
            recruitField.ForEachAlive([&](MinionData& data) {
                auto& minion = data.value();
                minion.SetAttack(minion.GetAttack() + attack);
                minion.SetHealth(minion.GetHealth() + health);
            });
        }
        const auto sourceDbfID = season14.spellModal.sourceCardDbfID;
        if (!season14.SelectSpellTargetChoice(offeringIdx, attack, health))
            return false;
        season14.OnTavernSpellResolved(true, sourceDbfID);
        ApplyTavernSpellTrinkets();
        AdvanceDarkGiftCounters(3);
        return true;
    }
    const auto modalKind = season14.spellModal.kind;
    if (modalKind == Season14SpellModalKind::TARGET_STATS) {
        if (season14.pendingSourceCardDbfID <= 0)
            return false;
        const auto sourceCard = Cards::FindCardByDbfID(season14.pendingSourceCardDbfID);
        if (sourceCard.dbfID == 0 ||
            FindTavernSpellBehavior(sourceCard.id).effect !=
                TavernSpellEffect::TARGET_CHOOSE_ONE_STATS)
            return false;
    }
    const int targetIdx = season14.spellModal.targetIndex;
    if (modalKind == Season14SpellModalKind::TARGET_OR_ALL_STATS &&
        offeringIdx == 1)
    {
        int attack = 0, health = 0;
        const auto sourceDbfID = season14.spellModal.sourceCardDbfID;
        if (!season14.SelectSpellTargetChoice(offeringIdx, attack, health)) return false;
        recruitField.ForEachAlive([&](MinionData& data) {
            auto& minion = data.value();
            minion.SetAttack(minion.GetAttack() + attack);
            minion.SetHealth(minion.GetHealth() + health);
        });
        season14.OnTavernSpellResolved(true, sourceDbfID);
        ApplyTavernSpellTrinkets();
        AdvanceDarkGiftCounters(3);
        return true;
    }
    if (targetIdx < 0 || targetIdx >= recruitField.GetCount()) return false;
    Minion& target = recruitField[static_cast<std::size_t>(targetIdx)];
    if (target.IsDestroyed()) return false;
    if (season14.spellModal.targetEntityID != 0 &&
        static_cast<std::uint64_t>(target.GetIndex()) !=
            season14.spellModal.targetEntityID)
        return false;
    int attack = 0, health = 0;
    const auto sourceDbfID = season14.spellModal.sourceCardDbfID;
    if (!season14.SelectSpellTargetChoice(offeringIdx, attack, health)) return false;
    target.SetAttack(target.GetAttack() + attack);
    target.SetHealth(target.GetHealth() + health);
    if (modalKind == Season14SpellModalKind::TARGET_OR_ALL_STATS &&
        offeringIdx == 0)
    {
        target.SetAttack(target.GetAttack() + 6);
        target.SetHealth(target.GetHealth() + 6);
    }
    recruitField.ForEachAlive([&target](MinionData& data) {
        data.value().ActivateTrigger(TriggerType::AFTER_CAST_SPELL, target);
    });
    season14.OnTavernSpellResolved(true, sourceDbfID);
    ApplyTavernSpellTrinkets();
    AdvanceDarkGiftCounters(3);
    return true;
}

void Player::ApplyDeferredTavernSpellStats()
{
    if (season14.deferredMinionStatTurns == 0) return;
    const int attack = season14.deferredMinionAttack;
    const int health = season14.deferredMinionHealth;
    recruitField.ForEachAlive([&](MinionData& data) {
        auto& minion = data.value();
        minion.SetAttack(minion.GetAttack() + attack);
        minion.SetHealth(minion.GetHealth() + health);
    });
    season14.deferredMinionAttack = 0;
    season14.deferredMinionHealth = 0;
    season14.deferredMinionStatTurns = 0;
}

bool Player::ApplyChooseOne(std::size_t offeringIdx, std::size_t targetIdx)
{
    if (season14.pendingDecision != Season14Decision::CHOOSE_ONE ||
        !season14.chooseOne.pending || offeringIdx >= 2 ||
        (!season14.chooseOne.targetMask && targetIdx != static_cast<std::size_t>(-1)) ||
        (season14.chooseOne.targetMask &&
         (targetIdx >= static_cast<std::size_t>(recruitField.GetCount()) ||
          (season14.chooseOne.targetMask & (std::uint32_t{1} << targetIdx)) == 0)))
        return false;
    // The source is part of the public modal identity.  Refuse stale/replayed
    // decisions even if a caller presents a currently valid Beast slot.
    bool sourceStillOnBoard = false;
    recruitField.ForEachAlive([&](MinionData& data) {
        const auto& source = data.value();
        if (static_cast<std::uint64_t>(source.GetIndex()) ==
                season14.chooseOne.sourceEntityID &&
            (season14.chooseOne.sourceCardDbfID == 0 ||
             source.GetDbfID() == season14.chooseOne.sourceCardDbfID))
            sourceStillOnBoard = true;
    });
    if (!sourceStillOnBoard) return false;

    if (!season14.chooseOne.targetMask)
    {
        const bool golden = season14.chooseOne.sourceCardDbfID ==
                            Cards::FindCardByID("BG30_123_G").dbfID ||
                            season14.chooseOne.sourceCardDbfID ==
                            Cards::FindCardByID("BG36_330_G").dbfID;
        if (season14.chooseOne.sourceCardDbfID == Cards::FindCardByID("BG30_123").dbfID ||
            season14.chooseOne.sourceCardDbfID == Cards::FindCardByID("BG30_123_G").dbfID)
        {
            if (offeringIdx == 0)
                season14.AddBloodGemBonus(golden ? 2 : 1, golden ? 2 : 1);
            else
                AddBloodGems(golden ? 8 : 4);
        }
        else if (season14.chooseOne.sourceCardDbfID == Cards::FindCardByID("BG36_330").dbfID ||
                 season14.chooseOne.sourceCardDbfID == Cards::FindCardByID("BG36_330_G").dbfID)
        {
            if (offeringIdx == 0)
                season14.AddFreeRefreshes(golden ? 4 : 2);
            else
                AddBloodGems(golden ? 6 : 3);
        }
        else if (season14.chooseOne.sourceCardDbfID == Cards::FindCardByID("BG36_332").dbfID ||
                 season14.chooseOne.sourceCardDbfID == Cards::FindCardByID("BG36_332_G").dbfID)
        {
            if (offeringIdx == 0)
                AddRandomMinionToHand(*this, SupportedMinionsForRace(Race::QUILBOAR));
            else
                season14.IncreaseMaxGold(golden ? 2 : 1);
        }
        else
            return false;
        return season14.SelectDecision(offeringIdx);
    }

    Minion& target = recruitField[targetIdx];
    if (target.IsDestroyed() || !target.HasRace(Race::BEAST)) return false;
    const bool golden = season14.chooseOne.sourceCardDbfID ==
                        Cards::FindCardByID("BG27_084_G").dbfID;
    if (offeringIdx == 0) {
        target.SetAttack(target.GetAttack() + (golden ? 2 : 1));
        target.SetHealth(target.GetHealth() + (golden ? 2 : 1));
        target.SetReborn(true);
    } else {
        target.SetAttack(target.GetAttack() + (golden ? 8 : 4));
        target.SetGameTag(GameTag::WINDFURY, 1);
    }
    // A combined Fandral copy receives both branch effects in the one
    // selected target decision, while retaining the normal source identity.
    bool combined = false;
    recruitField.ForEachAlive([&](MinionData& data) {
        if (static_cast<std::uint64_t>(data.value().GetIndex()) ==
                season14.chooseOne.sourceEntityID &&
            data.value().HasCombinedChooseOne())
            combined = true;
    });
    if (combined) {
        target.SetAttack(target.GetAttack() + (golden ? 8 : 4));
        target.SetGameTag(GameTag::WINDFURY, 1);
        if (offeringIdx == 1) {
            target.SetAttack(target.GetAttack() + (golden ? 2 : 1));
            target.SetHealth(target.GetHealth() + (golden ? 2 : 1));
            target.SetReborn(true);
        }
    }
    return season14.SelectDecision(offeringIdx);
}

int Player::AddTavernCoins(int count)
{
    if (count <= 0 || hand.IsFull())
        return 0;
    // BG28_810 is the canonical Battlegrounds Tavern Coin entity. Resolve it
    // by stable ID so this path cannot accidentally generate a normal-mode
    // Coin if DBF assignments are refreshed.
    const Card coin = Cards::FindCardByID("BG28_810");
    if (coin.id != "BG28_810" ||
        coin.GetCardType() != CardType::BATTLEGROUND_SPELL)
        return 0;
    int added = 0;
    while (added < count && !hand.IsFull())
    {
        hand.Add(CardData{ Spell(coin) });
        ++added;
    }
    return added;
}

bool Player::AcquireTrinket(Season14PersistentEffect effect)
{
    if (!season14.CanAddTrinket() || effect.dbfID <= 0 || !effect.active ||
        effect.remainingUses == 0)
        return false;
    // Resolve the entity before mutating persistent state.  Generated or
    // stale DBF ids must not consume a Trinket slot, and all acquisition-time
    // effects are keyed by the canonical card id.
    const auto card = Cards::FindCardByDbfID(effect.dbfID);
    if (card.id.empty() || card.GetCardType() != CardType::BATTLEGROUND_TRINKET)
        return false;
    const auto behavior = FindTrinketBehavior(card.id);
    if (behavior.effect == TrinketEffect::ACQUIRE_FIXED_CARD) {
        const Card generated = Cards::FindCardByID(behavior.cardID);
        if (behavior.cardID.empty() || generated.dbfID == 0 ||
            (generated.GetCardType() != CardType::SPELL &&
             generated.GetCardType() != CardType::MINION))
            return false;
    }
    const auto before = season14.trinkets.size();
    season14.AddTrinket(effect);
    // Static Tavern auras take effect on cards already offered as well as on
    // future fills.  The persistent state above covers future cards; apply
    // this acquisition-time delta to the live mixed/frozen shop exactly once.
    if (behavior.effect == TrinketEffect::SHOP_STATS ||
        behavior.effect == TrinketEffect::SHOP_STATS_AND_EXTRA_SLOT ||
        behavior.effect == TrinketEffect::REFRESH_SHOP_STATS ||
        behavior.effect == TrinketEffect::HERO_DAMAGE_SHOP_STATS)
    {
        tavern.fieldZone.ForEachAlive([&behavior](MinionData& data) {
            data.value().SetAttack(data.value().GetAttack() + behavior.attack);
            data.value().SetHealth(data.value().GetHealth() + behavior.health);
        });
    }
    if (behavior.effect == TrinketEffect::TAVERN_SPELL_STATS) {
        season14.AddTavernSpellAttackBonus(behavior.attack);
        season14.AddTavernSpellHealthBonus(behavior.health);
    }
    if (behavior.effect == TrinketEffect::SPELL_COUNT_MINION_ATTACK)
    {
        season14.persistentMinionAttack += behavior.attack;
        recruitField.ForEachAlive([&behavior](MinionData& data) {
            data.value().SetAttack(data.value().GetAttack() + behavior.attack);
        });
        hand.ForEach([&behavior](std::optional<CardData>& data) {
            if (data.has_value() && std::holds_alternative<Minion>(data.value()))
                std::get<Minion>(data.value()).SetAttack(
                    std::get<Minion>(data.value()).GetAttack() + behavior.attack);
        });
    }
    if (season14.trinkets.size() == before) return false;
    if (behavior.effect == TrinketEffect::STATIC_MINION_STATS)
    {
        recruitField.ForEachAlive([this](MinionData& data) {
            data.value().ApplyPersistentMinionStats(
                season14.persistentMinionAttack,
                season14.persistentMinionHealth);
        });
        hand.ForEach([this](std::optional<CardData>& data) {
            if (data.has_value() && std::holds_alternative<Minion>(data.value()))
            {
                auto& minion = std::get<Minion>(data.value());
                minion.ApplyPersistentMinionStats(
                    season14.persistentMinionAttack,
                    season14.persistentMinionHealth);
            }
        });
    }
    if (behavior.effect == TrinketEffect::STATIC_TIER_MINION_STATS)
    {
        recruitField.ForEachAlive([&behavior](MinionData& data) {
            data.value().ApplyPersistentTierMinionStats(
                behavior.value, behavior.attack, behavior.health);
        });
        tavern.fieldZone.ForEachAlive([&behavior](MinionData& data) {
            data.value().ApplyPersistentTierMinionStats(
                behavior.value, behavior.attack, behavior.health);
        });
        hand.ForEach([&behavior](std::optional<CardData>& data) {
            if (data.has_value() && std::holds_alternative<Minion>(data.value()))
                std::get<Minion>(data.value()).ApplyPersistentTierMinionStats(
                    behavior.value, behavior.attack, behavior.health);
        });
    }
    if (behavior.effect == TrinketEffect::STATIC_TIER_MINION_STATS)
    {
        recruitField.ForEachAlive([&behavior](MinionData& data) {
            if (data.value().GetTier() <= behavior.value)
                data.value().ApplyPersistentMinionStats(
                    behavior.attack, behavior.health);
        });
        hand.ForEach([&behavior](std::optional<CardData>& data) {
            if (!data.has_value() || !std::holds_alternative<Minion>(data.value()))
                return;
            auto& minion = std::get<Minion>(data.value());
            if (minion.GetTier() <= behavior.value)
                minion.ApplyPersistentTierMinionStats(
                    behavior.value, behavior.attack, behavior.health);
        });
    }
    if (behavior.effect == TrinketEffect::STATIC_FODDER_SHOP_STATS)
    {
        tavern.fieldZone.ForEachAlive([&behavior](MinionData& data) {
            if (data.value().GetCardID() == "BG35_150t")
            {
                data.value().SetAttack(data.value().GetAttack() + behavior.attack);
                data.value().SetHealth(data.value().GetHealth() + behavior.health);
            }
        });
    }
    if (behavior.effect == TrinketEffect::TAVERN_STATS_PER_SOLD &&
        season14.SoldMinionsThisTurn() > 0)
    {
        const int attack = behavior.attack * season14.SoldMinionsThisTurn();
        const int health = behavior.health * season14.SoldMinionsThisTurn();
        tavern.fieldZone.ForEachAlive([attack, health](MinionData& data) {
            data.value().SetAttack(data.value().GetAttack() + attack);
            data.value().SetHealth(data.value().GetHealth() + health);
        });
    }
    if (behavior.effect == TrinketEffect::BLOOD_GEM_BONUS &&
        behavior.amount > 0)
        AddBloodGems(behavior.amount);
    if (behavior.effect == TrinketEffect::STATIC_RACE_STATS)
        ApplyPersistentRaceStats(behavior.race, behavior.attack, behavior.health);
    if (behavior.effect == TrinketEffect::ACQUIRE_RANDOM_MINIONS)
        (void)SimpleTasks::RandomCardToHandTask{behavior.race, behavior.tier,
                                                 behavior.amount,
                                                 behavior.magneticOnly,
                                                 behavior.battlecryOnly}.Run(*this);
    else if (behavior.effect == TrinketEffect::ACQUIRE_FIXED_CARD &&
             behavior.cardID.size() != 0 && hand.GetCount() < MAX_HAND_SIZE)
    {
        const Card generated = Cards::FindCardByID(behavior.cardID);
        if (generated.dbfID != 0 && generated.GetCardType() != CardType::INVALID)
        {
            if (generated.GetCardType() == CardType::SPELL)
                hand.Add(CardData{Spell(generated)});
            else if (generated.GetCardType() == CardType::MINION)
                hand.Add(CardData{Minion(generated)});
        }
        // Nerglish Phrasebook buffs the left-most minion still in hand after
        // every successful minion play.  Resolve from the post-play hand so
        // the played card can never be selected as its own recipient.
        for (const auto& trinket : season14.trinkets)
        {
            if (!trinket.active || trinket.remainingUses == 0) continue;
            const auto followupBehavior = FindTrinketBehavior(
                Cards::FindCardByDbfID(trinket.dbfID).id);
            if (followupBehavior.effect != TrinketEffect::AFTER_PLAY_HAND_BUFF)
                continue;
            for (int handIdx = 0; handIdx < hand.GetCount(); ++handIdx)
            {
                if (!std::holds_alternative<Minion>(hand[handIdx])) continue;
                auto& recipient = std::get<Minion>(hand[handIdx]);
                recipient.SetAttack(recipient.GetAttack() + followupBehavior.attack);
                recipient.SetHealth(recipient.GetHealth() + followupBehavior.health);
                break;
            }
        }
    }
    return true;
}

int Player::GrantTrinketStartTurnCards()
{
    int added = 0;
    for (const auto& trinket : season14.trinkets) {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect != TrinketEffect::START_TURN_RANDOM_MINIONS &&
            !(behavior.effect == TrinketEffect::ACQUIRE_RANDOM_MINIONS &&
              behavior.repeatAtStartTurn) &&
            !(behavior.effect == TrinketEffect::ACQUIRE_FIXED_CARD &&
              behavior.repeatAtStartTurn)) continue;
        if (behavior.effect == TrinketEffect::ACQUIRE_FIXED_CARD) {
            if (behavior.cardID.empty() || hand.IsFull()) continue;
            const auto before = hand.GetCount();
            const Card generated = Cards::FindCardByID(behavior.cardID);
            if (generated.dbfID == 0) continue;
            if (generated.GetCardType() == CardType::SPELL)
                hand.Add(CardData{Spell(generated)});
            else if (generated.GetCardType() == CardType::MINION)
                hand.Add(CardData{Minion(generated)});
            added += hand.GetCount() - before;
            continue;
        }
        const auto before = hand.GetCount();
        (void)SimpleTasks::RandomCardToHandTask{behavior.race, behavior.tier,
                                                 behavior.amount,
                                                 behavior.magneticOnly,
                                                 behavior.battlecryOnly}.Run(*this);
        added += hand.GetCount() - before;
    }
    return added;
}

namespace
{
bool ValidFriendlyBoardTarget(const Player& player, int targetIdx)
{
    return targetIdx >= 0 && targetIdx < player.recruitField.GetCount() &&
           !player.recruitField[static_cast<std::size_t>(targetIdx)]
                .IsDestroyed();
}

template <std::size_t N>
void AppendSupportedNormalMinions(const std::array<Card, N>& cards,
                                  std::vector<Card>& result, Race race)
{
    for (const auto& card : cards)
    {
        if (card.id.empty() || !card.hasBehavior ||
            card.normalDbfID != 0 || card.GetCardType() != CardType::MINION ||
            (race != Race::INVALID && !card.HasRace(race)))
        {
            continue;
        }
        result.push_back(card);
    }
}

bool HasSupportedTier1Minion()
{
    std::vector<Card> candidates;
    AppendSupportedNormalMinions(Cards::GetTier1Minions(), candidates,
                                 Race::INVALID);
    return !candidates.empty();
}

std::vector<Card> SupportedMinionsForRace(Race race)
{
    std::vector<Card> result;
    AppendSupportedNormalMinions(Cards::GetTier1Minions(), result, race);
    AppendSupportedNormalMinions(Cards::GetTier2Minions(), result, race);
    AppendSupportedNormalMinions(Cards::GetTier3Minions(), result, race);
    AppendSupportedNormalMinions(Cards::GetTier4Minions(), result, race);
    AppendSupportedNormalMinions(Cards::GetTier5Minions(), result, race);
    AppendSupportedNormalMinions(Cards::GetTier6Minions(), result, race);
    AppendSupportedNormalMinions(Cards::GetTier7Minions(), result, race);
    return result;
}

std::vector<Card> SupportedDeathrattleMinions()
{
    std::vector<Card> result;
    const auto append = [&result](const auto& cards) {
        for (const auto& card : cards)
            if (card.hasBehavior && !card.power.GetDeathrattleTask().empty()) result.push_back(card);
    };
    append(Cards::GetTier1Minions()); append(Cards::GetTier2Minions()); append(Cards::GetTier3Minions());
    append(Cards::GetTier4Minions()); append(Cards::GetTier5Minions()); append(Cards::GetTier6Minions()); append(Cards::GetTier7Minions());
    return result;
}

std::vector<Card> SupportedBattlecryMinions()
{
    std::vector<Card> result;
    const auto append = [&result](const auto& cards) {
        for (const auto& card : cards)
            if (card.hasBehavior && card.normalDbfID == 0 &&
                card.GetCardType() == CardType::MINION &&
                CardDefs::FindCardDefByID(card.id).HasBattlecry())
                result.push_back(card);
    };
    append(Cards::GetTier1Minions()); append(Cards::GetTier2Minions());
    append(Cards::GetTier3Minions()); append(Cards::GetTier4Minions());
    append(Cards::GetTier5Minions()); append(Cards::GetTier6Minions());
    append(Cards::GetTier7Minions());
    return result;
}

std::vector<Card> SupportedTierMinions(const Player& player)
{
    std::vector<Card> result;
    if (player.currentTier == 1) AppendSupportedNormalMinions(Cards::GetTier1Minions(), result, Race::INVALID);
    else if (player.currentTier == 2) AppendSupportedNormalMinions(Cards::GetTier2Minions(), result, Race::INVALID);
    else if (player.currentTier == 3) AppendSupportedNormalMinions(Cards::GetTier3Minions(), result, Race::INVALID);
    else if (player.currentTier == 4) AppendSupportedNormalMinions(Cards::GetTier4Minions(), result, Race::INVALID);
    else if (player.currentTier == 5) AppendSupportedNormalMinions(Cards::GetTier5Minions(), result, Race::INVALID);
    else if (player.currentTier == 6) AppendSupportedNormalMinions(Cards::GetTier6Minions(), result, Race::INVALID);
    else if (player.currentTier == 7) AppendSupportedNormalMinions(Cards::GetTier7Minions(), result, Race::INVALID);
    return result;
}

// Registry of Choose-One minions whose two branch tasks have an executable
// combined form.  Keeping this explicit prevents metadata-only Choose-One
// cards from entering the Discover pool.
std::vector<Card> SupportedCombinedChooseOneMinions()
{
    std::vector<Card> result;
    for (const auto id : {"BG27_084", "BG30_123", "BG36_330"}) {
        const auto card = Cards::FindCardByID(id);
        if (card.dbfID != 0 && card.hasBehavior && card.normalDbfID == 0)
            result.push_back(card);
    }
    return result;
}

bool AddRandomMinionToHand(Player& player, std::vector<Card> candidates)
{
    if (candidates.empty() || player.hand.IsFull())
    {
        return false;
    }
    Random::shuffle(candidates.begin(), candidates.end());
    Minion minion(candidates.front());
    player.ApplyFreshMinionModifiers(minion);
    player.hand.Add(CardData{ std::move(minion) });
    return true;
}

bool BeginMinionDiscover(Player& player, std::vector<Card> candidates,
                         std::int32_t sourceCardDbfID, bool lockHand = false)
{
    if (candidates.empty() || player.hand.IsFull()) return false;
    Random::shuffle(candidates.begin(), candidates.end());
    const auto count = std::min<std::size_t>(3, candidates.size());
    std::vector<Season14Offering> offerings;
    offerings.reserve(count);
    for (std::size_t i = 0; i < count; ++i)
        offerings.push_back({candidates[i].dbfID, 0});
    player.season14.BeginOfferingDecision(
        Season14Decision::DISCOVER, 0, sourceCardDbfID, std::move(offerings));
    player.season14.pendingHandLock = lockHand;
    return true;
}

Race MostCommonFriendlyRace(const Player& player)
{
    Race result = Race::INVALID;
    int highest = 0;
    for (const Race race : RACES_IN_BATTLEGROUNDS)
    {
        int count = 0;
        player.recruitField.ForEachAlive(
            [race, &count](const MinionData& minion) {
                if (minion.value().HasRace(race))
                {
                    ++count;
                }
            });
        if (count > highest)
        {
            highest = count;
            result = race;
        }
    }
    return result;
}

bool HasRandomGoldenShopTarget(const Player& player)
{
    bool found = false;
    player.tavern.fieldZone.ForEach([&found](const MinionData& minion) {
        if (!minion.value().IsDestroyed() &&
            minion.value().GetPoolIndex() >= 0 &&
            minion.value().CanMakeGolden())
        {
            found = true;
        }
    });
    return found;
}

bool HasSupportedRaceMinion(Race race)
{
    return !SupportedMinionsForRace(race).empty();
}

std::size_t AliveFriendlyMinionCount(const Player& player)
{
    std::size_t count = 0;
    player.recruitField.ForEachAlive(
        [&count](const MinionData&) { ++count; });
    return count;
}

void ApplySpellBoardEffect(Player& player, const TavernSpellBehavior& effect,
                           int targetIdx, bool temporary,
                           std::int32_t sourceCardDbfID = 0)
{
    const auto addStats = [&effect](MinionData& aliveMinion) {
        Minion& minion = aliveMinion.value();
        minion.SetAttack(minion.GetAttack() + effect.attack);
        minion.SetHealth(minion.GetHealth() + effect.health);
    };

    switch (effect.effect)
    {
        case TavernSpellEffect::NONE:
            return;
        case TavernSpellEffect::BLOOD_GEM:
        {
            if (targetIdx < 0 || targetIdx >= player.recruitField.GetCount())
            {
                return;
            }
            Minion& target =
                player.recruitField[static_cast<std::size_t>(targetIdx)];
            auto [scaledAttack, scaledHealth] =
                player.season14.BloodGemStats();
            // A Blood Gem aura is satisfied by every concrete type the
            // target has; Card::HasRace also makes ALL minions match every
            // concrete Battlegrounds race.  This preserves multitype and
            // ALL semantics instead of consulting only GetRace().
            for (const Race race : RACES_IN_BATTLEGROUNDS)
            {
                if (!target.HasRace(race))
                    continue;
                const auto [raceAttack, raceHealth] =
                    player.season14.BloodGemRaceStatsFor(race);
                scaledAttack += raceAttack;
                scaledHealth += raceHealth;
            }
            if (target.GetBloodGemsThisTurn() == 0)
            {
                if (target.GetCardID() == "BG20_103")
                {
                    scaledAttack += 3;
                    scaledHealth += 3;
                }
                else if (target.GetCardID() == "BG20_103_G")
                {
                    scaledAttack += 6;
                    scaledHealth += 6;
                }
            }
            // Agamaggan's aura modifies every Blood Gem, including the one
            // being resolved.  It is deliberately derived from the visible
            // board, never from hidden pool/card text state.
            player.recruitField.ForEachAlive([&](const MinionData& data) {
                const auto id = data.value().GetCardID();
                if (id == "BG20_205")
                {
                    ++scaledAttack;
                    ++scaledHealth;
                }
                else if (id == "BG20_205_G")
                {
                    scaledAttack += 2;
                    scaledHealth += 2;
                }
            });
            target.ApplyBloodGem(scaledAttack, scaledHealth);

            // Tough Tusk gains a shield from the first gem played on it each
            // recruit turn.  The normal copy is temporary; the golden copy
            // keeps the keyword permanently.  The per-minion counter is
            // incremented by ApplyBloodGem before this hook runs.
            if (target.GetBloodGemsThisTurn() == 1)
            {
                if (target.GetCardID() == "BG20_102")
                {
                    target.ApplyTemporaryKeyword(GameTag::DIVINE_SHIELD);
                }
                else if (target.GetCardID() == "BG20_102_G")
                {
                    target.SetGameTag(GameTag::DIVINE_SHIELD, 1);
                }
            }

            // Dynamic Duo is a persistent +attack/+health response on other
            // Quilboar.  Resolve it from the post-gem public board.
            player.recruitField.ForEachAlive([&](MinionData& data) {
                Minion& observer = data.value();
                if (&observer == &target || !observer.HasRace(Race::QUILBOAR) ||
                    !target.HasRace(Race::QUILBOAR))
                {
                    return;
                }
                if (observer.GetCardID() == "BG20_207")
                {
                    observer.SetAttack(observer.GetAttack() + 1);
                    observer.SetHealth(observer.GetHealth() + 1);
                }
                else if (observer.GetCardID() == "BG20_207_G")
                {
                    observer.SetAttack(observer.GetAttack() + 2);
                    observer.SetHealth(observer.GetHealth() + 2);
                }
            });

            return;
        }
        case TavernSpellEffect::ALL_STATS:
            player.GetField().ForEachAlive(addStats);
            return;
        case TavernSpellEffect::ALL_STATS_AND_GOLDEN:
            player.GetField().ForEachAlive(
                [&effect, &addStats](MinionData& aliveMinion) {
                    addStats(aliveMinion);
                    if (aliveMinion.value().IsGolden())
                    {
                        addStats(aliveMinion);
                    }
                });
            return;
        case TavernSpellEffect::LEFTMOST_STATS:
        {
            bool applied = false;
            player.recruitField.ForEachAlive(
                [&applied, &addStats](MinionData& aliveMinion) {
                    if (!applied)
                    {
                        addStats(aliveMinion);
                        applied = true;
                    }
                });
            return;
        }
        case TavernSpellEffect::DIVINE_SHIELD_ATTACK:
            player.recruitField.ForEachAlive(
                [&effect](MinionData& aliveMinion) {
                    Minion& minion = aliveMinion.value();
                    if (minion.HasDivineShield())
                    {
                        minion.SetAttack(minion.GetAttack() + effect.attack);
                    }
                });
            return;
        case TavernSpellEffect::ALL_AND_RACE:
            player.GetField().ForEachAlive(
                [&effect, &addStats](MinionData& aliveMinion) {
                    addStats(aliveMinion);
                    Minion& minion = aliveMinion.value();
                    // Card metadata can contain multiple gameplay tribes (or
                    // ALL for an amalgam).  Effects that say "Naga" must use
                    // the card's complete tribe predicate, not only its
                    // primary race field.
                    if (minion.HasRace(effect.race))
                    {
                        addStats(aliveMinion);
                    }
                });
            return;
        case TavernSpellEffect::ALL_RACE_AND_DIVINE_SHIELD:
            player.GetField().ForEachAlive(
                [&effect, &addStats](MinionData& aliveMinion) {
                    addStats(aliveMinion);
                    Minion& minion = aliveMinion.value();
                    if (minion.HasRace(effect.race))
                    {
                        addStats(aliveMinion);
                    }
                    if (minion.HasDivineShield())
                    {
                        addStats(aliveMinion);
                    }
                });
            return;
        case TavernSpellEffect::RANDOM_STATS:
        {
            std::vector<Minion*> candidates;
            player.recruitField.ForEachAlive(
                [&candidates](MinionData& aliveMinion) {
                    candidates.push_back(&aliveMinion.value());
                });
            Random::shuffle(candidates.begin(), candidates.end());
            const auto count = std::min<std::size_t>(
                static_cast<std::size_t>(std::max(0, effect.randomCount)),
                candidates.size());
            for (std::size_t i = 0; i < count; ++i)
            {
                candidates[i]->SetAttack(candidates[i]->GetAttack() +
                                         effect.attack);
                candidates[i]->SetHealth(candidates[i]->GetHealth() +
                                         effect.health);
            }
            return;
        }
        case TavernSpellEffect::MENAGERIE_STATS:
        {
            std::set<Race> races;
            player.recruitField.ForEachAlive(
                [&races](MinionData& aliveMinion) {
                    const Minion& minion = aliveMinion.value();
                    // A dual-tribe or ALL minion contributes every concrete
                    // Battlegrounds type it belongs to.  Counting only
                    // GetRace() under-counts these cards and changes the
                    // number of Menagerie Tableware repeats.
                    for (const Race race : RACES_IN_BATTLEGROUNDS)
                    {
                        if (minion.HasRace(race))
                        {
                            races.insert(race);
                        }
                    }
                });
            const auto repeats = MenagerieTablewareRepeatCount(races.size());
            for (std::size_t i = 0; i < repeats; ++i)
            {
                player.recruitField.ForEachAlive(addStats);
            }
            return;
        }
        case TavernSpellEffect::ONE_PER_RACE_STATS:
        {
            // Resolve one friendly minion independently for every concrete
            // tribe.  A dual-tribe/ALL minion may therefore satisfy more
            // than one type, matching the game's per-type targeting model.
            for (const Race race : RACES_IN_BATTLEGROUNDS)
            {
                bool applied = false;
                player.recruitField.ForEachAlive(
                    [&effect, race, &applied](MinionData& aliveMinion) {
                        Minion& minion = aliveMinion.value();
                        if (!applied && minion.HasRace(race))
                        {
                            minion.SetAttack(minion.GetAttack() +
                                             effect.attack);
                            minion.SetHealth(minion.GetHealth() +
                                             effect.health);
                            applied = true;
                        }
                    });
            }
            return;
        }
        case TavernSpellEffect::SHOP_STATS:
            player.tavern.fieldZone.ForEach(
                [&effect](MinionData& minion) {
                    minion.value().SetAttack(minion.value().GetAttack() +
                                             effect.attack);
                    minion.value().SetHealth(minion.value().GetHealth() +
                                             effect.health);
                });
            return;
        case TavernSpellEffect::TARGET_STATS:
        {
            Minion& minion =
                player.recruitField[static_cast<std::size_t>(targetIdx)];
            if (temporary)
                minion.ApplyTemporaryStats(effect.attack, effect.health);
            else
            {
                minion.SetAttack(minion.GetAttack() + effect.attack);
                minion.SetHealth(minion.GetHealth() + effect.health);
            }
            return;
        }
        case TavernSpellEffect::SET_TARGET_STATS:
        {
            Minion& minion =
                player.recruitField[static_cast<std::size_t>(targetIdx)];
            minion.SetAttack(effect.attack);
            minion.SetHealth(effect.health);
            return;
        }
        case TavernSpellEffect::TARGET_AND_RACE:
        {
            Minion& target =
                player.recruitField[static_cast<std::size_t>(targetIdx)];
            target.SetAttack(target.GetAttack() + effect.attack);
            target.SetHealth(target.GetHealth() + effect.health);
            player.GetField().ForEachAlive(
                [&effect, &addStats](MinionData& aliveMinion) {
                    if (aliveMinion.value().HasRace(effect.race))
                    {
                        addStats(aliveMinion);
                    }
                });
            return;
        }
        case TavernSpellEffect::TARGET_STATS_REPEAT:
        {
            Minion& minion =
                player.recruitField[static_cast<std::size_t>(targetIdx)];
            const int repeats = minion.HasRace(effect.race) ? 3 : 2;
            minion.SetAttack(minion.GetAttack() + repeats * effect.attack);
            minion.SetHealth(minion.GetHealth() + repeats * effect.health);
            return;
        }
        case TavernSpellEffect::TARGET_STATS_AND_TAUNT:
        {
            Minion& minion =
                player.recruitField[static_cast<std::size_t>(targetIdx)];
            if (temporary)
                minion.ApplyTemporaryStats(effect.attack, effect.health, true);
            else
            {
                minion.SetAttack(minion.GetAttack() + effect.attack);
                minion.SetHealth(minion.GetHealth() + effect.health);
                minion.SetTaunt(true);
            }
            return;
        }
        case TavernSpellEffect::TARGET_STATS_AND_WINDFURY:
        case TavernSpellEffect::TARGET_STATS_AND_REBORN:
        {
            Minion& minion = player.recruitField[static_cast<std::size_t>(targetIdx)];
            if (temporary)
            {
                minion.ApplyTemporaryStats(effect.attack, effect.health);
                if (minion.HasRace(effect.race))
                    minion.ApplyTemporaryKeyword(effect.effect == TavernSpellEffect::TARGET_STATS_AND_WINDFURY ? GameTag::WINDFURY : GameTag::REBORN);
            }
            else
            {
                minion.SetAttack(minion.GetAttack() + effect.attack);
                minion.SetHealth(minion.GetHealth() + effect.health);
            }
            return;
        }
        case TavernSpellEffect::TARGET_DIVINE_SHIELD_TEMP:
        {
            Minion& minion = player.recruitField[static_cast<std::size_t>(targetIdx)];
            if (temporary) minion.ApplyTemporaryKeyword(GameTag::DIVINE_SHIELD);
            else minion.SetGameTag(GameTag::DIVINE_SHIELD, 1);
            return;
        }
        case TavernSpellEffect::TARGET_DIVINE_SHIELD:
        {
            Minion& minion =
                player.recruitField[static_cast<std::size_t>(targetIdx)];
            minion.SetGameTag(GameTag::DIVINE_SHIELD, 1);
            return;
        }
        case TavernSpellEffect::TARGET_STATS_TOGGLE_TAUNT:
        {
            Minion& minion =
                player.recruitField[static_cast<std::size_t>(targetIdx)];
            const bool alreadyTaunted = minion.HasTaunt();
            minion.SetAttack(minion.GetAttack() + effect.attack);
            minion.SetHealth(minion.GetHealth() + effect.health);
            minion.SetTaunt(!alreadyTaunted);
            return;
        }
        case TavernSpellEffect::TARGET_SHARED_RACE_STATS:
        {
            const Minion& target =
                player.recruitField[static_cast<std::size_t>(targetIdx)];
            player.recruitField.ForEachAlive(
                [&target, &effect, &addStats](MinionData& aliveMinion) {
                    for (const Race race : RACES_IN_BATTLEGROUNDS)
                    {
                        if (target.HasRace(race) &&
                            aliveMinion.value().HasRace(race))
                        {
                            addStats(aliveMinion);
                            break;
                        }
                    }
                });
            return;
        }
        case TavernSpellEffect::TARGET_RACE_SHOP_STATS_PERSISTENT:
        {
            const Minion& target =
                player.recruitField[static_cast<std::size_t>(targetIdx)];
            for (const Race race : RACES_IN_BATTLEGROUNDS)
            {
                if (!target.HasRace(race))
                {
                    continue;
                }
                player.tavern.fieldZone.ForEach(
                    [&effect, race](MinionData& minion) {
                        if (minion.value().HasRace(race))
                        {
                            minion.value().SetAttack(
                                minion.value().GetAttack() + effect.attack);
                            minion.value().SetHealth(
                                minion.value().GetHealth() + effect.health);
                        }
                    });
                player.season14.AddPersistentShopRaceStats(
                    race, effect.attack, effect.health);
            }
            return;
        }
        case TavernSpellEffect::TARGET_GOLDEN:
        {
            static_cast<void>(player.recruitField[
                static_cast<std::size_t>(targetIdx)].MakeGolden());
            return;
        }
        case TavernSpellEffect::RANDOM_SHOP_GOLDEN:
        {
            std::vector<Minion*> candidates;
            player.tavern.fieldZone.ForEach(
                [&candidates](MinionData& minion) {
                    if (!minion.value().IsDestroyed() &&
                        minion.value().GetPoolIndex() >= 0 &&
                        minion.value().CanMakeGolden())
                    {
                        candidates.push_back(&minion.value());
                    }
                });
            Random::shuffle(candidates.begin(), candidates.end());
            for (Minion* candidate : candidates)
            {
                if (candidate->MakeGolden())
                {
                    break;
                }
            }
            return;
        }
        case TavernSpellEffect::RANDOM_MINION_TO_HAND:
            static_cast<void>(AddRandomMinionToHand(
                player, [&] {
                    std::vector<Card> result;
                    AppendSupportedNormalMinions(
                        Cards::GetTier1Minions(), result, Race::INVALID);
                    return result;
                }()));
            return;
        case TavernSpellEffect::RANDOM_COMMON_RACE_MINION_TO_HAND:
        {
            const Race race = MostCommonFriendlyRace(player);
            static_cast<void>(AddRandomMinionToHand(
                player, SupportedMinionsForRace(race)));
            return;
        }
        case TavernSpellEffect::DISCOVER_MINION:
        {
            std::vector<Card> candidates;
            if (effect.race != Race::INVALID)
                candidates = SupportedMinionsForRace(effect.race);
            else if (effect.value == 1)
                AppendSupportedNormalMinions(Cards::GetTier1Minions(), candidates, Race::INVALID);
            else if (effect.value == 7)
                AppendSupportedNormalMinions(Cards::GetTier7Minions(), candidates, Race::INVALID);
            else if (effect.value == 8)
                candidates = SupportedDeathrattleMinions();
            else if (effect.lockHand) {
                if (player.currentTier == 1) AppendSupportedNormalMinions(Cards::GetTier1Minions(), candidates, Race::INVALID);
                else if (player.currentTier == 2) AppendSupportedNormalMinions(Cards::GetTier2Minions(), candidates, Race::INVALID);
                else if (player.currentTier == 3) AppendSupportedNormalMinions(Cards::GetTier3Minions(), candidates, Race::INVALID);
                else if (player.currentTier == 4) AppendSupportedNormalMinions(Cards::GetTier4Minions(), candidates, Race::INVALID);
                else if (player.currentTier == 5) AppendSupportedNormalMinions(Cards::GetTier5Minions(), candidates, Race::INVALID);
                else if (player.currentTier == 6) AppendSupportedNormalMinions(Cards::GetTier6Minions(), candidates, Race::INVALID);
                else if (player.currentTier == 7) AppendSupportedNormalMinions(Cards::GetTier7Minions(), candidates, Race::INVALID);
            }
            else
                candidates = SupportedMinionsForRace(MostCommonFriendlyRace(player));
            static_cast<void>(BeginMinionDiscover(player, std::move(candidates), sourceCardDbfID, effect.lockHand));
            return;
        }
        case TavernSpellEffect::DISCOVER_BATTLECRY_MINION:
            static_cast<void>(BeginMinionDiscover(
                player, SupportedBattlecryMinions(), sourceCardDbfID));
            return;
        case TavernSpellEffect::TRANSFORM_HIGHER_TIER:
        {
            std::vector<Card> candidates;
            for (int tier = player.recruitField[static_cast<std::size_t>(targetIdx)].GetTier() + 1;
                 tier <= TIER_UPPER_LIMIT; ++tier) {
                const auto append = [&candidates](const auto& cards) {
                    AppendSupportedNormalMinions(cards, candidates, Race::INVALID);
                };
                if (tier == 1) append(Cards::GetTier1Minions());
                else if (tier == 2) append(Cards::GetTier2Minions());
                else if (tier == 3) append(Cards::GetTier3Minions());
                else if (tier == 4) append(Cards::GetTier4Minions());
                else if (tier == 5) append(Cards::GetTier5Minions());
                else if (tier == 6) append(Cards::GetTier6Minions());
                else if (tier == 7) append(Cards::GetTier7Minions());
            }
            if (candidates.empty()) return;
            Random::shuffle(candidates.begin(), candidates.end());
            auto& target = player.recruitField[static_cast<std::size_t>(targetIdx)];
            const int attack = target.GetAttack(), health = target.GetHealth();
            if (target.TransformTo(candidates.front())) {
                target.SetAttack(attack); target.SetHealth(health);
            }
            return;
        }
        case TavernSpellEffect::DISCOVER_DIFFERENT_RACE:
        {
            const auto& target = player.recruitField[static_cast<std::size_t>(targetIdx)];
            std::vector<Card> candidates = SupportedMinionsForRace(target.GetRace());
            candidates.erase(std::remove_if(candidates.begin(), candidates.end(),
                [&target](const Card& card) { return card.id == target.GetCardID(); }), candidates.end());
            static_cast<void>(BeginMinionDiscover(player, std::move(candidates), sourceCardDbfID));
            return;
        }
        case TavernSpellEffect::RANDOM_MINION_AND_COPY:
        {
            auto candidates = SupportedMinionsForRace(effect.race);
            if (candidates.empty() || player.hand.GetCount() + 2 > MAX_HAND_SIZE)
                return;
            Random::shuffle(candidates.begin(), candidates.end());
            for (int copy = 0; copy < 2; ++copy) {
                Minion minion(candidates.front());
                player.ApplyFreshMinionModifiers(minion);
                player.hand.Add(CardData{std::move(minion)});
            }
            return;
        }
        case TavernSpellEffect::FIXED_CARDS:
        {
            const int count = effect.cardB.empty() ? effect.randomCount : 2;
            if (player.hand.GetCount() + count > MAX_HAND_SIZE) return;
            const Card first = Cards::FindCardByID(effect.cardA);
            const Card second = effect.cardB.empty() ? Card{} : Cards::FindCardByID(effect.cardB);
            if (first.dbfID == 0 || (!effect.cardB.empty() && second.dbfID == 0)) return;
            for (int i = 0; i < count; ++i) {
                const Card card = (i == 0 || effect.cardB.empty()) ? first : second;
                if (card.GetCardType() == CardType::SPELL)
                    player.hand.Add(CardData{Spell(card)});
                else
                    player.hand.Add(CardData{Minion(card)});
            }
            return;
        }
        case TavernSpellEffect::BLOOD_GEM_TRANSFER:
        {
            auto& target = player.recruitField[static_cast<std::size_t>(targetIdx)];
            int attack = 0, health = 0;
            if (targetIdx > 0) { auto stats = player.recruitField[static_cast<std::size_t>(targetIdx - 1)].RemoveBloodGems(); attack += stats.first; health += stats.second; }
            if (targetIdx + 1 < player.recruitField.GetCount()) { auto stats = player.recruitField[static_cast<std::size_t>(targetIdx + 1)].RemoveBloodGems(); attack += stats.first; health += stats.second; }
            target.SetAttack(target.GetAttack() + attack);
            target.SetHealth(target.GetHealth() + health);
            auto [gemAttack, gemHealth] = player.season14.BloodGemStats();
            target.ApplyBloodGem(gemAttack, gemHealth);
            target.ApplyBloodGem(gemAttack, gemHealth);
            return;
        }
        case TavernSpellEffect::RANDOM_SPELLCRAFT:
        {
            static constexpr std::array ids{"BG23_000t", "BG23_004t", "BG23_007t", "BG23_008t", "BG23_015t", "BG31_830t"};
            if (player.hand.GetCount() + 3 > MAX_HAND_SIZE) return;
            std::vector<std::string_view> choices(ids.begin(), ids.end());
            Random::shuffle(choices.begin(), choices.end());
            for (int i = 0; i < 3; ++i) {
                const Card card = Cards::FindCardByID(choices[static_cast<std::size_t>(i)]);
                if (card.dbfID == 0) return;
                player.hand.Add(CardData{Spell(card)});
            }
            return;
        }
        case TavernSpellEffect::DISCOVER_HERO_POWER:
        {
            std::vector<Card> candidates;
            for (const auto& card : Cards::GetHeroPowerMetadata())
                if (card.dbfID != 0 && card.hasBehavior) candidates.push_back(card);
            if (candidates.empty()) return;
            Random::shuffle(candidates.begin(), candidates.end());
            const auto count = std::min<std::size_t>(3, candidates.size());
            std::vector<Season14Offering> offerings;
            for (std::size_t i = 0; i < count; ++i) offerings.push_back({candidates[i].dbfID, 0});
            player.season14.BeginOfferingDecision(Season14Decision::DISCOVER, 0, sourceCardDbfID, std::move(offerings));
            return;
        }
        case TavernSpellEffect::STEAL_RANDOM_SHOP_MINION:
        {
            std::vector<int> candidates;
            player.tavern.fieldZone.ForEach(
                [&candidates](const MinionData& minion) {
                    candidates.push_back(minion.value().GetZonePosition());
                });
            Random::shuffle(candidates.begin(), candidates.end());
            if (!candidates.empty() && !player.hand.IsFull())
            {
                Minion& source = player.tavern.fieldZone[
                    static_cast<std::size_t>(candidates.front())];
                Minion stolen = player.tavern.fieldZone.Remove(source);
                player.hand.Add(CardData{ std::move(stolen) });
            }
            return;
        }
        case TavernSpellEffect::RANDOM_SHOP_STATS_ON_REFRESH:
            player.season14.ArmRefreshRandomShopStats(effect.attack,
                                                       effect.health);
            return;
        case TavernSpellEffect::SELL_TARGET_GIVE_RANDOM_STATS:
        {
            const Minion& target =
                player.recruitField[static_cast<std::size_t>(targetIdx)];
            const int soldAttack = target.GetAttack();
            const int soldHealth = target.GetHealth();
            Minion sold = player.recruitField.Remove(
                player.recruitField[static_cast<std::size_t>(targetIdx)]);
            player.returnMinionCallback(sold.GetPoolIndex());
            player.remainCoin += 1;
            player.season14.OnSellMinion();

            std::vector<Minion*> candidates;
            player.recruitField.ForEachAlive(
                [&candidates](MinionData& minion) {
                    candidates.push_back(&minion.value());
                });
            Random::shuffle(candidates.begin(), candidates.end());
            if (!candidates.empty())
            {
                candidates.front()->SetAttack(candidates.front()->GetAttack() +
                                              soldAttack);
                candidates.front()->SetHealth(candidates.front()->GetHealth() +
                                              soldHealth);
            }
            return;
        }
        case TavernSpellEffect::TARGET_CONSUME_SHOP_STATS:
        {
            Minion& target =
                player.recruitField[static_cast<std::size_t>(targetIdx)];
            int attack = 0;
            int health = 0;
            // Rebuild positions after every removal.  FieldZone::Remove
            // compacts the array, so retaining a shuffled list of positions
            // would consume the wrong cards (or an out-of-range slot).
            for (int i = 0; i < effect.randomCount; ++i)
            {
                std::vector<int> candidates;
                player.tavern.fieldZone.ForEach(
                    [&candidates](const MinionData& minion) {
                        if (!minion.value().IsDestroyed() &&
                            minion.value().GetPoolIndex() >= 0)
                        {
                            candidates.push_back(
                                minion.value().GetZonePosition());
                        }
                    });
                if (candidates.empty())
                {
                    break;
                }
                Random::shuffle(candidates.begin(), candidates.end());
                Minion& consumed = player.tavern.fieldZone[
                    static_cast<std::size_t>(candidates.front())];
                attack += consumed.GetAttack();
                health += consumed.GetHealth();
                if (effect.copyKeywords) {
                    if (consumed.HasTaunt()) target.SetTaunt(true);
                    if (consumed.HasDivineShield()) target.SetGameTag(GameTag::DIVINE_SHIELD, 1);
                    if (consumed.HasReborn()) target.SetReborn(true);
                    if (consumed.HasWindfury()) target.SetGameTag(GameTag::WINDFURY, 1);
                    if (consumed.HasVenomous()) target.SetGameTag(GameTag::VENOMOUS, 1);
                    if (consumed.HasStealth()) target.SetGameTag(GameTag::STEALTH, 1);
                }
                const int poolIndex = consumed.GetPoolIndex();
                player.tavern.fieldZone.Remove(consumed);
                player.returnMinionCallback(poolIndex);
            }
            target.SetAttack(target.GetAttack() + attack);
            target.SetHealth(target.GetHealth() + health);
            return;
        }
        case TavernSpellEffect::SELL_TARGET_GIVE_LEFTMOST_RACE_STATS:
        {
            const Minion& soldTarget =
                player.recruitField[static_cast<std::size_t>(targetIdx)];
            const int attack = soldTarget.GetAttack();
            const int health = soldTarget.GetHealth();
            Minion sold = player.recruitField.Remove(
                player.recruitField[static_cast<std::size_t>(targetIdx)]);
            player.returnMinionCallback(sold.GetPoolIndex());
            player.remainCoin += 1;
            player.season14.OnSellMinion();
            bool applied = false;
            player.recruitField.ForEachAlive(
                [&applied, &effect, attack, health](MinionData& minion) {
                    if (!applied && minion.value().HasRace(effect.race))
                    {
                        minion.value().SetAttack(minion.value().GetAttack() +
                                                 attack);
                        minion.value().SetHealth(minion.value().GetHealth() +
                                                 health);
                        applied = true;
                    }
                });
            return;
        }
        case TavernSpellEffect::SET_PLAYER_ARMOR:
            player.armor = effect.value;
            return;
        case TavernSpellEffect::NEXT_TURN_GOLD:
            player.season14.AddNextTurnGold(effect.value);
            return;
        case TavernSpellEffect::NEXT_COMBAT_REWARD:
            player.season14.ArmNextCombatReward(105267);
            return;
        case TavernSpellEffect::TARGET_NEXT_COMBAT_BUFF:
        {
            if (targetIdx < 0 || targetIdx >= player.recruitField.GetCount())
                return;
            auto& target = player.recruitField[static_cast<std::size_t>(targetIdx)];
            target.SetAttack(target.GetAttack() + effect.attack);
            target.SetHealth(target.GetHealth() + effect.health);
            player.season14.ArmNextCombatBuff(
                sourceCardDbfID, static_cast<std::uint64_t>(target.GetIndex()),
                4, 6);
            return;
        }
        case TavernSpellEffect::COMBAT_START_LEFTMOST_ATTACK_DOUBLE:
            player.season14.ArmCombatStartLeftmostAttackDouble(sourceCardDbfID);
            return;
        case TavernSpellEffect::COMBAT_START_LEFTMOST_NEAREST_STATS:
            player.season14.ArmCombatStartNearestStats(sourceCardDbfID);
            return;
        case TavernSpellEffect::COMBAT_START_RANDOM_ENEMY_SET_HEALTH:
            player.season14.ArmCombatStartRandomEnemySetHealth(sourceCardDbfID);
            return;
        case TavernSpellEffect::DESTROY_UNDEAD_GIVE_PERSISTENT_ATTACK:
        {
            if (targetIdx < 0 || targetIdx >= player.recruitField.GetCount())
                return;
            Minion& target = player.recruitField[static_cast<std::size_t>(targetIdx)];
            if (!target.HasRace(Race::UNDEAD))
                return;
            // Destroy is distinct from selling: resolve the target's owned
            // deathrattle while it is still on the recruit board, then return
            // the spent instance to the pool.
            if (target.HasDeathrattle())
                target.ActivateTask(PowerType::DEATHRATTLE, player);
            const int poolIndex = target.GetPoolIndex();
            player.recruitField.Remove(target);
            player.returnMinionCallback(poolIndex);
            player.ApplyPersistentRaceStats(Race::UNDEAD, effect.attack, 0);
            return;
        }
        case TavernSpellEffect::SHOP_BLOOD_GEMS_ON_REFRESH:
            player.season14.ArmShopBloodGemsOnRefresh(sourceCardDbfID);
            return;
        case TavernSpellEffect::COMBAT_START_SUMMON_BEETLES:
            player.season14.ArmCombatStartBeetles(sourceCardDbfID);
            return;
        case TavernSpellEffect::INCREASE_MAX_GOLD:
            player.season14.IncreaseMaxGold(effect.value);
            return;
        case TavernSpellEffect::FREE_REFRESHES:
            player.season14.AddFreeRefreshes(effect.value);
            return;
        case TavernSpellEffect::SHOP_STATS_PERSISTENT:
            player.tavern.fieldZone.ForEach(
                [&effect](MinionData& minion) {
                    minion.value().SetAttack(minion.value().GetAttack() +
                                             effect.attack);
                    minion.value().SetHealth(minion.value().GetHealth() +
                                             effect.health);
                });
            player.season14.AddPersistentShopStats(effect.attack,
                                                   effect.health);
            return;
        case TavernSpellEffect::SPELL_COSTS_HEALTH:
            return;
    }
}
}  // namespace

bool Player::CastTavernSpellFree(const std::string& cardID, int amount,
                                 int targetIdx)
{
    if (amount <= 0)
        return true;
    const Card card = Cards::FindCardByID(cardID);
    if (card.id.empty())
        return false;
    const TavernSpellBehavior behavior = FindTavernSpellBehavior(cardID);
    if (behavior.effect == TavernSpellEffect::NONE)
        return false;
    for (int i = 0; i < amount; ++i)
    {
        ApplySpellBoardEffect(*this, behavior, targetIdx, false, card.dbfID);
        season14.Emit(Season14Event::SPELL_CAST);
        // A target-aware trigger only fires when this free cast was resolved
        // on an explicit minion.  Untargeted Tavern spells (including Rally
        // casts) must not masquerade as casts on every friendly minion.
        if (targetIdx >= 0 && targetIdx < GetField().GetCount())
        {
            auto& target = GetField()[static_cast<std::size_t>(targetIdx)];
            GetField().ForEachAlive([&](MinionData& data) {
                data.value().ActivateTrigger(TriggerType::AFTER_CAST_SPELL, target);
            });
        }
        ApplyTavernSpellTrinkets();
    }
    return true;
}

void Player::ApplyBloodGemTo(Minion& target)
{
    if (target.IsDestroyed())
        return;
    const int targetIdx = target.GetZonePosition();
    if (targetIdx < 0 || targetIdx >= recruitField.GetCount())
        return;
    // Rally-generated gems are free and do not masquerade as a spell cast;
    // use the canonical board-effect executor so race auras, Agamaggan,
    // Tough Tusk, and Dynamic Duo all resolve identically to a real gem.
    ApplySpellBoardEffect(*this, FindTavernSpellBehavior("BG20_GEM"),
                          targetIdx, false);
}

bool Player::CanPlaySpell(std::size_t handIdx) const
{
    return CanPlaySpell(handIdx, -1);
}

int Player::AddBloodGems(int count)
{
    if (count <= 0)
    {
        return 0;
    }
    const Card gemCard = Cards::FindCardByDbfID(70136);
    if (gemCard.id != "BG20_GEM")
    {
        return 0;
    }
    // Death's Head Sage adds extra copies for each Blood Gem gained. Compute
    // the multiplier once per gain event; the generated copies do not recurse
    // through this path, and hand capacity remains authoritative.
    int extraPerGem = 0;
    // Blood Gems generated during combat must observe the combat field: a
    // Sage that died earlier in combat no longer owns the trigger. During
    // recruit, GetField() resolves to recruitField as usual.
    GetField().ForEachAlive([&extraPerGem](const MinionData& data) {
        const auto& id = data.value().GetCardID();
        if (id == "BG20_HERO_103_Buddy") extraPerGem += 1;
        else if (id == "BG20_HERO_103_Buddy_G") extraPerGem += 2;
    });
    count *= 1 + extraPerGem;
    int added = 0;
    while (added < count && !hand.IsFull())
    {
        hand.Add(CardData{ Spell(gemCard) });
        ++added;
    }
    return added;
}

bool Player::CanPlaySpell(std::size_t handIdx, int targetIdx) const
{
    if (handIdx >= static_cast<std::size_t>(hand.GetCount()))
    {
        return false;
    }
    const CardData& card = hand[static_cast<int>(handIdx)];
    if (!std::holds_alternative<Spell>(card))
    {
        return false;
    }
    const Spell& spell = std::get<Spell>(card);
    if (spell.GetID() == "BG30_MagicItem_416t")
    {
        if (targetIdx < 0 || !ValidFriendlyBoardTarget(*this, targetIdx)) return false;
        const auto& target = recruitField[static_cast<std::size_t>(targetIdx)];
        if (target.GetTier() >= 6) return false;
        return std::any_of(Cards::GetAllCards().begin(), Cards::GetAllCards().end(),
            [&target](const Card& candidate) {
                return candidate.isBattlegroundsPoolMinion && candidate.hasBehavior &&
                       candidate.GetCardType() == CardType::MINION && candidate.normalDbfID == 0 &&
                       candidate.GetTier() == target.GetTier() + 1;
            });
    }
    const TavernSpellBehavior behavior = FindTavernSpellBehavior(spell.GetID());
    if (behavior.gold < 0 ||
        TavernSpellRequiresTarget(behavior.effect) != (targetIdx >= 0))
    {
        return false;
    }
    if (targetIdx >= 0 && !ValidFriendlyBoardTarget(*this, targetIdx))
    {
        return false;
    }
    if (targetIdx >= 0)
    {
        const Minion& target = recruitField[static_cast<std::size_t>(targetIdx)];
        if (behavior.effect == TavernSpellEffect::TARGET_CONSUME_SHOP_STATS &&
            !target.HasRace(behavior.race))
        {
            return false;
        }
        if (behavior.effect ==
                TavernSpellEffect::DESTROY_UNDEAD_GIVE_PERSISTENT_ATTACK &&
            !target.HasRace(Race::UNDEAD))
        {
            return false;
        }
        if (!TavernSpellTargetIsLegal(behavior.effect, target.GetTier(),
                                      target.IsGolden()))
        {
            return false;
        }
        if (behavior.effect == TavernSpellEffect::TARGET_GOLDEN &&
            !target.CanMakeGolden())
        {
            return false;
        }
    }
    if (behavior.effect == TavernSpellEffect::RANDOM_SHOP_GOLDEN &&
        !HasRandomGoldenShopTarget(*this))
    {
        return false;
    }
    if ((behavior.effect == TavernSpellEffect::RANDOM_MINION_TO_HAND ||
         behavior.effect == TavernSpellEffect::RANDOM_COMMON_RACE_MINION_TO_HAND ||
         behavior.effect == TavernSpellEffect::STEAL_RANDOM_SHOP_MINION) &&
        hand.IsFull())
    {
        return false;
    }
    if (behavior.effect == TavernSpellEffect::RANDOM_MINION_TO_HAND &&
        !HasSupportedTier1Minion())
    {
        return false;
    }
    if (behavior.effect == TavernSpellEffect::DISCOVER_MINION) {
        if (hand.IsFull()) return false;
        std::vector<Card> candidates;
        if (behavior.value == 1)
            AppendSupportedNormalMinions(Cards::GetTier1Minions(), candidates, Race::INVALID);
        else if (behavior.value == 7)
            AppendSupportedNormalMinions(Cards::GetTier7Minions(), candidates, Race::INVALID);
        else if (behavior.value == 8)
            candidates = SupportedDeathrattleMinions();
        else if (behavior.lockHand) {
            if (currentTier == 1) AppendSupportedNormalMinions(Cards::GetTier1Minions(), candidates, Race::INVALID);
            else if (currentTier == 2) AppendSupportedNormalMinions(Cards::GetTier2Minions(), candidates, Race::INVALID);
            else if (currentTier == 3) AppendSupportedNormalMinions(Cards::GetTier3Minions(), candidates, Race::INVALID);
            else if (currentTier == 4) AppendSupportedNormalMinions(Cards::GetTier4Minions(), candidates, Race::INVALID);
            else if (currentTier == 5) AppendSupportedNormalMinions(Cards::GetTier5Minions(), candidates, Race::INVALID);
            else if (currentTier == 6) AppendSupportedNormalMinions(Cards::GetTier6Minions(), candidates, Race::INVALID);
            else if (currentTier == 7) AppendSupportedNormalMinions(Cards::GetTier7Minions(), candidates, Race::INVALID);
        }
        else
            candidates = SupportedMinionsForRace(MostCommonFriendlyRace(*this));
        if (candidates.empty()) return false;
    }
    if (behavior.effect == TavernSpellEffect::DISCOVER_BATTLECRY_MINION &&
        SupportedBattlecryMinions().empty())
    {
        return false;
    }
    if (behavior.effect == TavernSpellEffect::DISCOVER_TIER_MINION_OR_SPELL &&
        (SupportedTierMinions(*this).empty() || currentTier <= 0))
    {
        return false;
    }
    if (behavior.effect == TavernSpellEffect::DISCOVER_CHOOSE_ONE_COMBINED &&
        SupportedCombinedChooseOneMinions().empty())
    {
        return false;
    }
    if (behavior.effect == TavernSpellEffect::DISCOVER_UNDEAD_DIES_THIS_TURN &&
        SupportedMinionsForRace(Race::UNDEAD).empty())
    {
        return false;
    }
    if (behavior.effect == TavernSpellEffect::REFRESH_TAVERN_SPELLS &&
        std::none_of(Cards::GetAllCards().begin(), Cards::GetAllCards().end(),
            [](const Card& card) {
                return card.isBattlegroundsPoolSpell && card.normalDbfID == 0 &&
                    FindTavernSpellBehavior(card.id).effect != TavernSpellEffect::NONE;
            }))
        return false;
    if (behavior.effect == TavernSpellEffect::TRANSFORM_HIGHER_TIER) {
        if (targetIdx < 0 || targetIdx >= recruitField.GetCount()) return false;
        const int tier = recruitField[static_cast<std::size_t>(targetIdx)].GetTier();
        if (tier >= TIER_UPPER_LIMIT) return false;
    }
    if (behavior.effect == TavernSpellEffect::DISCOVER_DIFFERENT_RACE) {
        if (targetIdx < 0 || targetIdx >= recruitField.GetCount()) return false;
        const auto& target = recruitField[static_cast<std::size_t>(targetIdx)];
        auto candidates = SupportedMinionsForRace(target.GetRace());
        candidates.erase(std::remove_if(candidates.begin(), candidates.end(),
            [&target](const Card& card) { return card.id == target.GetCardID(); }), candidates.end());
        if (candidates.empty()) return false;
    }
    if (behavior.effect == TavernSpellEffect::RANDOM_MINION_AND_COPY) {
        if (hand.GetCount() + 2 > MAX_HAND_SIZE ||
            SupportedMinionsForRace(behavior.race).empty()) return false;
    }
    if (behavior.effect == TavernSpellEffect::FIXED_CARDS) {
        const int count = behavior.cardB.empty() ? behavior.randomCount : 2;
        if (hand.GetCount() + count > MAX_HAND_SIZE ||
            Cards::FindCardByID(behavior.cardA).dbfID == 0 ||
            (!behavior.cardB.empty() && Cards::FindCardByID(behavior.cardB).dbfID == 0)) return false;
    }
    if (behavior.effect == TavernSpellEffect::RANDOM_SPELLCRAFT &&
        hand.GetCount() + 3 > MAX_HAND_SIZE)
        return false;
    if (behavior.effect == TavernSpellEffect::RANDOM_COMMON_RACE_MINION_TO_HAND)
    {
        const Race race = MostCommonFriendlyRace(*this);
        if (race == Race::INVALID || !HasSupportedRaceMinion(race))
        {
            return false;
        }
    }
    if (behavior.effect == TavernSpellEffect::STEAL_RANDOM_SHOP_MINION &&
        tavern.fieldZone.IsEmpty())
    {
        return false;
    }
    if (behavior.effect == TavernSpellEffect::TARGET_CONSUME_SHOP_STATS)
    {
        if (behavior.race != Race::INVALID &&
            !recruitField[static_cast<std::size_t>(targetIdx)].HasRace(behavior.race))
            return false;
        std::size_t available = 0;
        tavern.fieldZone.ForEach([&available](const MinionData& minion) {
            if (!minion.value().IsDestroyed() &&
                minion.value().GetPoolIndex() >= 0)
            {
                ++available;
            }
        });
        if (available < static_cast<std::size_t>(behavior.randomCount))
        {
            return false;
        }
    }
    if (behavior.effect == TavernSpellEffect::SELL_TARGET_GIVE_LEFTMOST_RACE_STATS)
    {
        bool recipient = false;
        for (int i = 0; i < recruitField.GetCount(); ++i)
        {
            if (i != targetIdx && !recruitField[static_cast<std::size_t>(i)]
                                      .IsDestroyed() &&
                recruitField[static_cast<std::size_t>(i)].HasRace(
                    behavior.race))
            {
                recipient = true;
                break;
            }
        }
        if (!recipient || AliveFriendlyMinionCount(*this) < 2)
        {
            return false;
        }
    }
    if (behavior.effect == TavernSpellEffect::SELL_TARGET_GIVE_RANDOM_STATS &&
        AliveFriendlyMinionCount(*this) < 2)
    {
        // The spell must leave at least one friendly minion to receive the
        // sold minion's stats.  Reject an unresolvable target before paying
        // the spell cost or mutating the board.
        return false;
    }
    const int baseCost = spell.GetCost();
    int trinketStatDiscount = 0;
    for (const auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto tb = FindTrinketBehavior(Cards::FindCardByDbfID(trinket.dbfID).id);
        if (tb.effect == TrinketEffect::STAT_TAVERN_SPELL_DISCOUNT)
            trinketStatDiscount += tb.value;
    }
    const int costBeforeFree = std::max(
        0, season14.TavernSpellCost(baseCost) -
               season14.nextTavernSpellDiscount -
               (TavernSpellReceivesHealthBonus(behavior.effect) ||
                TavernSpellReceivesAttackBonus(behavior.effect)
                    ? trinketStatDiscount : 0));
    const int cost = season14.trinketFreeSpellUses > 0 ? 0 : costBeforeFree;
    if (baseCost < 0)
    {
        return false;
    }
    if (behavior.effect == TavernSpellEffect::SPELL_COSTS_HEALTH)
    {
        // This is a health payment, not damage: Armor must not absorb it.
        // It also cannot reduce the hero to zero; a lethal payment is not a
        // legal purchase and must not partially resolve the spell.
        return hero.health > cost;
    }
    return remainCoin >= cost;
}

bool Player::PlaySpell(std::size_t handIdx)
{
    return PlaySpell(handIdx, -1);
}

bool Player::PlaySpell(std::size_t handIdx, int targetIdx)
{
    if (!CanPlaySpell(handIdx, targetIdx))
    {
        return false;
    }

    CardData& card = hand[static_cast<int>(handIdx)];
    const Spell& spell = std::get<Spell>(card);
    if (spell.GetID() == "BG30_MagicItem_416t")
    {
        auto& target = recruitField[static_cast<std::size_t>(targetIdx)];
        if (!season14.BeginTransformDecision(0, spell.GetDbfID(),
                                             static_cast<std::uint64_t>(target.GetIndex()),
                                             targetIdx, target.GetTier()))
            return false;
        hand.Remove(card);
        return true;
    }
    const auto previewEffect = FindTavernSpellBehavior(spell.GetID());
    int trinketStatDiscount = 0;
    for (const auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto tb = FindTrinketBehavior(Cards::FindCardByDbfID(trinket.dbfID).id);
        if (tb.effect == TrinketEffect::STAT_TAVERN_SPELL_DISCOUNT)
            trinketStatDiscount += tb.value;
    }
    const int costBeforeFree = std::max(
        0, season14.TavernSpellCost(spell.GetCost()) -
               season14.nextTavernSpellDiscount -
               (TavernSpellReceivesHealthBonus(previewEffect.effect) ||
                TavernSpellReceivesAttackBonus(previewEffect.effect)
                    ? trinketStatDiscount : 0));
    const int cost = season14.trinketFreeSpellUses > 0 ? 0 : costBeforeFree;
    season14.ConsumeTavernSpellDiscount();
    if (season14.trinketFreeSpellUses > 0)
        --season14.trinketFreeSpellUses;
    const int sourceSpellDbfID = spell.GetDbfID();
    bool temporarySpell = spell.IsTemporary();
    TavernSpellBehavior effect = FindTavernSpellBehavior(spell.GetID());
    if (IsBountySpell(spell.GetID())) {
        int repeats = 1;
        recruitField.ForEachAlive([&](MinionData& data) {
            if (data.value().GetCardID() == "BG33_825") repeats = std::max(repeats, 2);
            if (data.value().GetCardID() == "BG33_825_G") repeats = std::max(repeats, 3);
        });
        if (repeats > 1) {
            effect.attack *= repeats; effect.health *= repeats;
            effect.gold *= repeats; effect.randomCount *= repeats;
        }
    }
    // Lava Lurker does not generate a Spellcraft card.  It instead makes the
    // first one (two when golden) actually cast on that entity permanent for
    // this recruit turn.  Consume the allowance only after all play legality
    // checks above have succeeded.
    if (temporarySpell && targetIdx >= 0 &&
        targetIdx < recruitField.GetCount() &&
        (recruitField[static_cast<std::size_t>(targetIdx)].HasPermanentSpellcraft() ||
         (recruitField[static_cast<std::size_t>(targetIdx)].IsLavaLurker() &&
          recruitField[static_cast<std::size_t>(targetIdx)].ConsumeSpellcraftUse())))
        temporarySpell = false;
    const auto [auraAttack, auraHealth] = TavernSpellAuraBonus(recruitField);
    if (TavernSpellReceivesHealthBonus(effect.effect))
    {
        effect.health += auraHealth;
    }
    if (TavernSpellReceivesAttackBonus(effect.effect))
    {
        effect.attack += auraAttack;
    }
    // Blue Whelp's Rally is player-owned and cumulative.  Apply it at
    // resolution so spells already in hand, refreshed spells, and generated
    // copies all receive the same bonus exactly once.
    if (TavernSpellReceivesHealthBonus(effect.effect))
    {
        effect.health += season14.tavernSpellHealthBonus;
    }
    if (TavernSpellReceivesAttackBonus(effect.effect))
    {
        effect.attack += season14.tavernSpellAttackBonus;
        effect.attack += season14.TemporaryTavernSpellStats().first;
    }
    if (TavernSpellReceivesHealthBonus(effect.effect))
        effect.health += season14.TemporaryTavernSpellStats().second;
    hand.Remove(card);
    if (effect.effect == TavernSpellEffect::SPELL_COSTS_HEALTH)
    {
        // Hasty Excavation explicitly costs Health instead of Gold, so do
        // not route this through Hero::TakeDamage (which would consume Armor).
        hero.health -= cost;
    }
    else
    {
        remainCoin -= cost;
        RecordGoldSpent(cost);
    }
    remainCoin += effect.gold;
    season14.Emit(Season14Event::SPELL_CAST);
    if (effect.effect == TavernSpellEffect::REFRESH_TAVERN_SPELLS)
    {
        std::vector<Card> candidates;
        for (const auto& candidate : Cards::GetAllCards())
            if (candidate.isBattlegroundsPoolSpell &&
                candidate.normalDbfID == 0 &&
                FindTavernSpellBehavior(candidate.id).effect !=
                    TavernSpellEffect::NONE)
                candidates.push_back(candidate);
        if (candidates.empty()) return false;
        Random::shuffle(candidates.begin(), candidates.end());
        tavern.ClearSpellSlots();
        const auto count = std::min<std::size_t>(
            season14.TavernOfferCount(MAX_FIELD_SIZE), candidates.size());
        for (std::size_t i = 0; i < count; ++i)
            tavern.spellSlots.emplace_back(Spell(candidates[i]));
        season14.OnRefreshTavern(true);
        ApplyTavernSpellTrinkets();
        AdvanceDarkGiftCounters(3);
        return true;
    }
    if (effect.effect == TavernSpellEffect::DISCOVER_TIER_MINION_OR_SPELL)
    {
        season14.BeginSpellAllMinionChoice(
            Season14SpellModalKind::DISCOVER_TIER_MINION_OR_SPELL,
            sourceSpellDbfID, 0, 0, 0, 0, false);
        return true;
    }
    if (effect.effect == TavernSpellEffect::DISCOVER_CHOOSE_ONE_COMBINED)
    {
        auto candidates = SupportedCombinedChooseOneMinions();
        if (candidates.empty() || hand.IsFull()) return false;
        Random::shuffle(candidates.begin(), candidates.end());
        std::vector<Season14Offering> offerings;
        for (std::size_t i = 0; i < std::min<std::size_t>(3, candidates.size()); ++i)
            offerings.push_back({candidates[i].dbfID, 0});
        season14.BeginOfferingDecision(Season14Decision::DISCOVER, 0,
                                       sourceSpellDbfID, std::move(offerings));
        return true;
    }
    if (effect.effect == TavernSpellEffect::DISCOVER_UNDEAD_DIES_THIS_TURN)
    {
        auto candidates = SupportedMinionsForRace(Race::UNDEAD);
        if (candidates.empty() || hand.IsFull()) return false;
        Random::shuffle(candidates.begin(), candidates.end());
        std::vector<Season14Offering> offerings;
        for (std::size_t i = 0; i < std::min<std::size_t>(3, candidates.size()); ++i)
            offerings.push_back({candidates[i].dbfID, 0});
        season14.BeginOfferingDecision(Season14Decision::DISCOVER, 0,
                                       sourceSpellDbfID, std::move(offerings));
        return true;
    }
    if (effect.effect == TavernSpellEffect::REFRESH_RACE)
    {
        if (targetIdx < 0 || targetIdx >= recruitField.GetCount()) return false;
        const auto race = recruitField[static_cast<std::size_t>(targetIdx)].GetRace();
        if (race == Race::INVALID) return false;
        clearTavernMinionsCallback(*this);
        season14.ArmRefreshRace(race);
        PrepareTavern();
        season14.OnRefreshTavern(true);
        ApplyTavernSpellTrinkets();
        AdvanceDarkGiftCounters(3);
        return true;
    }
    if (effect.effect == TavernSpellEffect::TARGET_CHOOSE_ONE_STATS)
    {
        season14.BeginSpellTargetChoice(
            sourceSpellDbfID, targetIdx,
            static_cast<std::uint64_t>(recruitField[targetIdx].GetIndex()),
                                        3 + auraAttack, 1 + auraHealth,
                                        1 + auraAttack, 3 + auraHealth,
                                        "friendly_minion_target");
        return true;
    }
    if (effect.effect == TavernSpellEffect::ALL_MINION_CHOOSE_ONE_STATS)
    {
        season14.BeginSpellAllMinionChoice(
            Season14SpellModalKind::ALL_MINION_STATS, sourceSpellDbfID,
            2 + auraAttack, 2 + auraHealth,
            2 + auraAttack, 2 + auraHealth, true);
        return true;
    }
    if (effect.effect == TavernSpellEffect::TARGET_OR_ALL_CHOOSE_ONE_STATS)
    {
        season14.BeginSpellTargetChoice(
            sourceSpellDbfID, targetIdx,
            static_cast<std::uint64_t>(recruitField[targetIdx].GetIndex()),
            6 + auraAttack, 6 + auraHealth,
            2 + auraAttack, 2 + auraHealth, "friendly_minion_target",
            Season14SpellModalKind::TARGET_OR_ALL_STATS);
        return true;
    }
    ApplySpellBoardEffect(*this, effect, targetIdx, temporarySpell,
                          sourceSpellDbfID);
    if (targetIdx >= 0 && targetIdx < recruitField.GetCount()) {
        auto& target = recruitField[static_cast<std::size_t>(targetIdx)];
        recruitField.ForEachAlive([&target](MinionData& data) {
            data.value().ActivateTrigger(TriggerType::AFTER_CAST_SPELL, target);
        });
    } else {
        recruitField.ForEachAlive([](MinionData& data) {
            auto& minion = data.value();
            minion.ActivateTrigger(TriggerType::AFTER_CAST_SPELL, minion);
        });
    }
    // Arcane Knowledge and other one-shot Tavern-spell discounts are
    // consumed only after a supported spell has actually resolved.  The
    // legality check above ensures unaffordable/unsupported attempts leave
    // the discount untouched.
    season14.OnTavernSpellResolved(true, sourceSpellDbfID);
    const auto spellAttack = season14.TakeSpellMinionAttackDelta();
    if (spellAttack != 0)
    {
        recruitField.ForEachAlive([spellAttack](MinionData& data) {
            data.value().SetAttack(data.value().GetAttack() + spellAttack);
        });
        hand.ForEach([spellAttack](std::optional<CardData>& data) {
            if (data.has_value() && std::holds_alternative<Minion>(data.value()))
                std::get<Minion>(data.value()).SetAttack(
                    std::get<Minion>(data.value()).GetAttack() + spellAttack);
        });
    }
    const auto [castAttack, castHealth] = season14.TakeSpellCastMinionStats();
    if (castAttack != 0 || castHealth != 0)
    {
        season14.persistentMinionAttack += castAttack;
        season14.persistentMinionHealth += castHealth;
        recruitField.ForEachAlive([castAttack, castHealth](MinionData& data) {
            data.value().SetAttack(data.value().GetAttack() + castAttack);
            data.value().SetHealth(data.value().GetHealth() + castHealth);
        });
        hand.ForEach([castAttack, castHealth](std::optional<CardData>& data) {
            if (data.has_value() && std::holds_alternative<Minion>(data.value())) {
                auto& minion = std::get<Minion>(data.value());
                minion.SetAttack(minion.GetAttack() + castAttack);
                minion.SetHealth(minion.GetHealth() + castHealth);
            }
        });
    }
    ApplyTavernSpellTrinkets();
    AdvanceDarkGiftCounters(3);
    return true;
}

bool Player::ApplySeason14HeroPowerBatch3Activation(
    const Season14HeroPowerBatch3Activation& activation)
{
    std::vector<Minion*> candidates;
    recruitField.ForEachAlive([&candidates](MinionData& minion) {
        candidates.push_back(&minion.value());
    });
    if (candidates.empty())
    {
        return false;
    }

    Random::shuffle(candidates.begin(), candidates.end());
    const auto count = activation.randomCount <= 0
                           ? candidates.size()
                           : std::min<std::size_t>(
                                 static_cast<std::size_t>(activation.randomCount),
                                 candidates.size());
    for (std::size_t i = 0; i < count; ++i)
    {
        candidates[i]->SetAttack(candidates[i]->GetAttack() +
                                 activation.attack);
        candidates[i]->SetHealth(candidates[i]->GetHealth() +
                                 activation.health);
    }
    return true;
}

bool Player::ApplyConvictionHeroPower()
{
    if (season14.heroPowerDbfID != 73941) return false;
    std::vector<Minion*> candidates;
    recruitField.ForEachAlive([&candidates](MinionData& data) {
        candidates.push_back(&data.value());
    });
    if (candidates.empty()) return false;

    Random::shuffle(candidates.begin(), candidates.end());
    const auto count = std::min<std::size_t>(
        static_cast<std::size_t>(2 + season14.ConvictionExtraTargets()),
        candidates.size());
    const int attack = 1 + season14.ConvictionAttackBonus();
    const int health = 1 + season14.ConvictionHealthBonus();
    for (std::size_t i = 0; i < count; ++i) {
        candidates[i]->SetAttack(candidates[i]->GetAttack() + attack);
        candidates[i]->SetHealth(candidates[i]->GetHealth() + health);
    }
    return true;
}

bool Player::ActivateMinion(std::size_t boardIdx, int targetIdx)
{
    if (boardIdx >= static_cast<std::size_t>(recruitField.GetCount()))
    {
        return false;
    }
    return recruitField[boardIdx].Activate(*this, targetIdx);
}

bool Player::ApplyDevour(std::size_t sourceIdx, std::size_t targetIdx)
{
    if (sourceIdx >= static_cast<std::size_t>(recruitField.GetCount()) ||
        targetIdx >= static_cast<std::size_t>(recruitField.GetCount()) ||
        sourceIdx == targetIdx || recruitField[sourceIdx].IsDestroyed() ||
        recruitField[targetIdx].IsDestroyed())
        return false;
    const auto attack = recruitField[sourceIdx].GetAttack();
    const auto health = recruitField[sourceIdx].GetHealth();
    const auto sourceID = recruitField[sourceIdx].GetCardID();
    SellMinion(sourceIdx);
    if (sourceIdx < targetIdx) --targetIdx;
    if (targetIdx >= static_cast<std::size_t>(recruitField.GetCount()) ||
        recruitField[targetIdx].IsDestroyed()) return false;
    recruitField[targetIdx].SetAttack(recruitField[targetIdx].GetAttack() + attack);
    recruitField[targetIdx].SetHealth(recruitField[targetIdx].GetHealth() + health);
    const int extras = sourceID == "BG20_HERO_301_Buddy" ? 2 :
                       sourceID == "BG20_HERO_301_Buddy_G" ? 4 : 0;
    if (extras > 0 && (attack != 0 || health != 0)) {
        std::vector<std::size_t> candidates;
        for (std::size_t i = 0; i < static_cast<std::size_t>(recruitField.GetCount()); ++i)
            if (i != targetIdx && !recruitField[i].IsDestroyed())
                candidates.push_back(i);
        Random::shuffle(candidates.begin(), candidates.end());
        const auto count = std::min<std::size_t>(extras, candidates.size());
        for (std::size_t i = 0; i < count; ++i) {
            auto& extra = recruitField[candidates[i]];
            extra.SetAttack(extra.GetAttack() + attack);
            extra.SetHealth(extra.GetHealth() + health);
        }
    }
    return true;
}

bool Player::BeginISpyDiscover()
{
    if (hand.IsFull() || !getOpponentPlayerCallback) return false;
    // The callback is the simulator's pairing source of truth.  Only the
    // opponent's currently revealed recruit field is sampled; hand, Tavern,
    // pool and RNG state never enter the offering.
    Player& opponent = getOpponentPlayerCallback(*this);
    std::vector<Card> candidates;
    opponent.recruitField.ForEachAlive([&](MinionData& data) {
        const auto& observed = data.value();
        Card card = Cards::FindCardByID(observed.GetCardID());
        if (card.normalDbfID != 0)
            card = Cards::FindCardByDbfID(card.normalDbfID);
        if (card.dbfID != 0 && card.GetCardType() == CardType::MINION &&
            card.isBattlegroundsPoolMinion && card.normalDbfID == 0 &&
            card.hasBehavior &&
            std::none_of(candidates.begin(), candidates.end(),
                         [&card](const Card& existing) {
                             return existing.dbfID == card.dbfID;
                         }))
            candidates.push_back(card);
    });
    if (candidates.empty()) return false;
    Random::shuffle(candidates.begin(), candidates.end());
    std::vector<Season14Offering> offerings;
    for (std::size_t i = 0; i < std::min<std::size_t>(3, candidates.size()); ++i)
        offerings.push_back({candidates[i].dbfID, 0});
    season14.BeginOfferingDecision(Season14Decision::DISCOVER, 0, 76563,
                                   std::move(offerings));
    return true;
}

int Player::ResolveWardenBuddy()
{
    if (!getOpponentPlayerCallback || hand.IsFull()) return 0;
    int copies = 0;
    recruitField.ForEachAlive([&copies](const MinionData& data) {
        const auto& id = data.value().GetCardID();
        if (id == "BG21_HERO_010_Buddy") copies = std::max(copies, 1);
        else if (id == "BG21_HERO_010_Buddy_G") copies = std::max(copies, 2);
    });
    if (copies == 0) return 0;
    const auto& opponent = getOpponentPlayerCallback(*this);
    const auto buddyDbfID = opponent.hero.card.relatedDbfID;
    if (buddyDbfID == 0) return 0;
    const auto buddy = Cards::FindCardByDbfID(buddyDbfID);
    if (buddy.GetCardType() != CardType::MINION || buddy.dbfID == 0)
        return 0;
    int added = 0;
    for (; added < copies && !hand.IsFull(); ++added) {
        Minion generated(buddy);
        ApplyFreshMinionModifiers(generated);
        hand.Add(CardData{std::move(generated)});
    }
    return added;
}

bool Player::BeginPowerOfStormChoice()
{
    if (!season14.powerOfStormActive ||
        season14.pendingDecision != Season14Decision::NONE)
        return false;
    std::vector<Card> candidates;
    for (const auto& card : Cards::GetAllCards())
    {
        const auto* behavior = FindSeason14HeroPowerBehavior(card.dbfID);
        if (card.GetCardType() != CardType::HERO_POWER ||
            card.normalDbfID != 0 || card.dbfID == 71909 ||
            behavior == nullptr || behavior->passive ||
            std::any_of(candidates.begin(), candidates.end(),
                        [&card](const Card& existing) {
                            return existing.dbfID == card.dbfID;
                        }))
            continue;
        candidates.push_back(card);
    }
    if (candidates.size() < 2) return false;
    Random::shuffle(candidates.begin(), candidates.end());
    std::size_t optionCount = 2;
    recruitField.ForEachAlive([&optionCount](const MinionData& data) {
        const auto& id = data.value().GetCardID();
        if (id == "BG20_HERO_202_Buddy") optionCount = std::max(optionCount, std::size_t{3});
        else if (id == "BG20_HERO_202_Buddy_G") optionCount = std::max(optionCount, std::size_t{4});
    });
    optionCount = std::min(optionCount, candidates.size());
    std::vector<Season14Offering> offerings;
    offerings.reserve(optionCount);
    for (std::size_t i = 0; i < optionCount; ++i)
        offerings.push_back({candidates[i].dbfID, 0});
    season14.BeginOfferingDecision(Season14Decision::CHOICE, 0, 71909,
                                   std::move(offerings));
    return true;
}

void Player::SellMinion(std::size_t idx)
{
    if (idx >= static_cast<std::size_t>(recruitField.GetCount()))
    {
        return;
    }
    const auto soldID = recruitField[idx].GetCardID();
    auto minion = recruitField.Remove(recruitField[idx]);
    returnMinionCallback(minion.GetPoolIndex());

    remainCoin += 1;
    // The sold entity is no longer in recruitField, so it cannot be reached
    // by the observer loop below.  Resolve its self-scoped SELL_MINION
    // trigger explicitly after removal; this is the lifecycle used by
    // Twisted Wrathguard while preserving observers on surviving minions.
    minion.ActivateTrigger(TriggerType::SELL_MINION, minion);
    recruitField.ForEachAlive([&minion](MinionData& data) {
        data.value().ActivateTrigger(TriggerType::SELL_MINION,
                                      minion);
    });
    if (soldID == "BG20_301")
    {
        AddBloodGems(2);
    }
    else if (soldID == "BG20_301_G")
    {
        AddBloodGems(4);
    }
    else if (const auto* fishbait = FindFishbaitSellBehavior(minion.GetDbfID());
             fishbait != nullptr && fishbait->kind == FishbaitSellKind::AIR_BALLER)
    {
        const int attack = fishbait->stat;
        const int health = fishbait->stat;
        recruitField.ForEachAlive([attack, health](MinionData& data) {
            data.value().SetAttack(data.value().GetAttack() + attack);
            data.value().SetHealth(data.value().GetHealth() + health);
        });
        season14.ImproveFutureBallers(attack, health);
    }
    else if (const auto* sharkBehavior = FindFishbaitSellBehavior(minion.GetDbfID());
             sharkBehavior != nullptr && sharkBehavior->kind == FishbaitSellKind::SNARKY_SHARK)
    {
        // Snarky Shark refreshes immediately using the normal Tavern refresh
        // lifecycle (including free-refresh hooks and persistent shop auras),
        // then replaces the left-most offer with the normal/golden Fishbait
        // generated by its definition.  The sale has already removed the
        // Shark, so all state needed by this generated card is retained on
        // Season14State rather than on the sold entity.
        season14.fishbaitActive = true;
        season14.fishbaitDbfID = sharkBehavior->generatedFishbaitDbfID;
        RefreshTavern(true);
        if (tavern.fieldZone.GetCount() > 0) {
            auto replaced = tavern.fieldZone.Remove(tavern.fieldZone[0]);
            returnMinionCallback(replaced.GetPoolIndex());
        }
        const auto baitCard = Cards::FindCardByDbfID(season14.fishbaitDbfID);
        const auto* baitBehavior = FindFishbaitTokenBehavior(baitCard.dbfID);
        if (baitBehavior != nullptr && !tavern.fieldZone.IsFull()) {
            Minion bait(baitCard);
            const int reward = baitBehavior->killerStat;
            bool attacked = false;
            recruitField.ForEachAlive([&](MinionData& data) {
                if (attacked || !data.value().HasRace(Race::BEAST)) return;
                // A left-most Beast attacks the 0/1 (or 0/2 golden) bait and
                // receives its deathrattle reward immediately.
                data.value().SetAttack(data.value().GetAttack() + reward);
                data.value().SetHealth(data.value().GetHealth() + reward);
                attacked = true;
            });
            if (!attacked) {
                bait.SetDeathrattleStatTransfer(reward, reward);
                tavern.fieldZone.Add(bait, 0);
            }
        }
    }
    else if (soldID == "BG31_816" || soldID == "BG31_816_G" ||
             soldID == "BG31_818" || soldID == "BG31_818_G")
    {
        const int amount = soldID.ends_with("_G") ? 2 : 1;
        const int attack = soldID.starts_with("BG31_816") ? amount : 0;
        const int health = soldID.starts_with("BG31_818") ? amount : 0;
        recruitField.ForEachAlive([attack, health](MinionData& data) {
            data.value().SetAttack(data.value().GetAttack() + attack);
            data.value().SetHealth(data.value().GetHealth() + health);
        });
        season14.ImproveFutureBallers(attack, health);
    }
    else
    {
        AddTavernCoins(FindSellBehaviorBatch21(soldID).tavernCoins);
    }
    if ((soldID == "BG24_018" || soldID == "BG24_018_G") &&
        season14.lastCombatLost)
    {
        // Blue Shell pays five (normal) or ten (golden) total Gold after a
        // loss.  The ordinary sale coin was already granted above.
        remainCoin += soldID.ends_with("_G") ? 9 : 4;
    }
    season14.OnSellMinion();
    int soldAttack = 0;
    int soldHealth = 0;
    for (const auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect == TrinketEffect::TAVERN_STATS_PER_SOLD)
        {
            soldAttack += behavior.attack;
            soldHealth += behavior.health;
        }
    }
    if (soldAttack != 0 || soldHealth != 0)
    {
        tavern.fieldZone.ForEachAlive([soldAttack, soldHealth](MinionData& data) {
            data.value().SetAttack(data.value().GetAttack() + soldAttack);
            data.value().SetHealth(data.value().GetHealth() + soldHealth);
        });
    }
}

void Player::UpgradeTavern()
{
    const int cost = season14.UpgradeCost(coinToUpgradeTavern);
    if (currentTier == TIER_UPPER_LIMIT || remainCoin < cost)
    {
        return;
    }

    remainCoin -= cost;
    RecordGoldSpent(cost);
    upgradeTavernCallback(*this);
    const auto result = season14.OnUpgradeTavern();
    remainCoin += result.goldDelta;
    if (currentTier >= 6)
        remainCoin += season14.ResolveTierSixTrinketGold();
}

void Player::RefreshTavern(bool freeRefresh)
{
    const bool allowanceRefresh = !freeRefresh && season14.HasFreeRefresh();
    const int cost = freeRefresh || allowanceRefresh
                         ? 0
                         : season14.RefreshCost(NUM_COIN_REFRESH_TAVERN);
    if (remainCoin < cost)
    {
        return;
    }

    clearTavernMinionsCallback(*this);
    season14.refreshExtraShopSlots = 0;
    for (const auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect == TrinketEffect::REFRESH_EXTRA_SHOP_SLOTS)
            season14.refreshExtraShopSlots += behavior.value;
    }
    remainCoin -= cost;
    RecordGoldSpent(cost);

    if (allowanceRefresh)
    {
        season14.ConsumeFreeRefresh();
    }

    PrepareTavern();
    season14.refreshExtraShopSlots = 0;
    // Sold-count Tavern auras apply to every newly filled offer as well as
    // the offers present when each minion was sold.
    if (season14.SoldMinionsThisTurn() > 0)
    {
        int attack = 0;
        int health = 0;
        for (const auto& trinket : season14.trinkets)
        {
            if (!trinket.active || trinket.remainingUses == 0) continue;
            const auto behavior = FindTrinketBehavior(
                Cards::FindCardByDbfID(trinket.dbfID).id);
            if (behavior.effect == TrinketEffect::TAVERN_STATS_PER_SOLD)
            {
                attack += behavior.attack * season14.SoldMinionsThisTurn();
                health += behavior.health * season14.SoldMinionsThisTurn();
            }
        }
        tavern.fieldZone.ForEachAlive([attack, health](MinionData& data) {
            data.value().SetAttack(data.value().GetAttack() + attack);
            data.value().SetHealth(data.value().GetHealth() + health);
        });
    }
    // Laboratory Assistant counts successful refreshes, not turn-start Tavern
    // preparation.  Consume after PrepareTavern has completed so the normal
    // refresh lifecycle and mixed spell/minion slots remain authoritative.
    const auto fodders = season14.ConsumeFodderRefresh();
    if (fodders > 0) {
        const auto* fodderBehavior = FindFodderBehavior("BG35_150t");
        const auto fodder = Cards::FindCardByDbfID(
            fodderBehavior == nullptr ? 0 : fodderBehavior->dbfID);
        if (fodder.dbfID != 0 && fodder.GetCardType() == CardType::MINION) {
            for (int i = 0; i < fodders && !tavern.fieldZone.IsFull(); ++i) {
                Minion generated{fodder};
                ApplyFreshTavernMinionModifiers(generated);
                generated.SetAttack(generated.GetAttack() +
                                    season14.persistentFodderAttack);
                generated.SetHealth(generated.GetHealth() +
                                    season14.persistentFodderHealth);
                tavern.fieldZone.Add(generated);
            }
        }
    }
    if (season14.HasShopBloodGemsOnRefresh())
    {
        tavern.fieldZone.ForEachAlive([](MinionData& data) {
            data.value().SetAttack(data.value().GetAttack() + 1);
            data.value().SetHealth(data.value().GetHealth() + 1);
        });
    }
    const auto [randomAttack, randomHealth] =
        season14.RefreshRandomShopStats();
    if (randomAttack != 0 || randomHealth != 0)
    {
        std::vector<Minion*> candidates;
        tavern.fieldZone.ForEach(
            [&candidates](MinionData& minion) {
                if (!minion.value().IsDestroyed())
                {
                    candidates.push_back(&minion.value());
                }
            });
        Random::shuffle(candidates.begin(), candidates.end());
        if (!candidates.empty())
        {
            candidates.front()->SetAttack(candidates.front()->GetAttack() +
                                          randomAttack);
            candidates.front()->SetHealth(candidates.front()->GetHealth() +
                                          randomHealth);
        }
    }
    if (season14.HasGeneratedRewardMirrorShield())
    {
        std::vector<Minion*> candidates;
        tavern.fieldZone.ForEachAlive([&candidates](MinionData& data) {
            if (!data.value().IsDestroyed()) candidates.push_back(&data.value());
        });
        Random::shuffle(candidates.begin(), candidates.end());
        if (!candidates.empty())
        {
            candidates.front()->SetAttack(candidates.front()->GetAttack() + 6);
            candidates.front()->SetHealth(candidates.front()->GetHealth() + 6);
            candidates.front()->SetGameTag(GameTag::DIVINE_SHIELD, 1);
        }
    }
    const auto oldShopAttack = season14.persistentShopAttack;
    const auto oldShopHealth = season14.persistentShopHealth;
    season14.OnRefreshTavern(true);
    const auto refreshAttack = season14.persistentShopAttack - oldShopAttack;
    const auto refreshHealth = season14.persistentShopHealth - oldShopHealth;
    if (refreshAttack != 0 || refreshHealth != 0)
    {
        tavern.fieldZone.ForEachAlive([refreshAttack, refreshHealth](MinionData& data) {
            data.value().SetAttack(data.value().GetAttack() + refreshAttack);
            data.value().SetHealth(data.value().GetHealth() + refreshHealth);
        });
    }
    // Cursed Crystal's refresh aura is temporary, but applies to every offer
    // in the newly authoritative Tavern and must not become persistent.
    const auto [temporaryAttack, temporaryHealth] =
        season14.TakeRefreshShopStatsDelta();
    if (temporaryAttack != 0 || temporaryHealth != 0)
    {
        tavern.fieldZone.ForEachAlive([temporaryAttack, temporaryHealth](MinionData& data) {
            data.value().SetAttack(data.value().GetAttack() + temporaryAttack);
            data.value().SetHealth(data.value().GetHealth() + temporaryHealth);
        });
    }
    int vardenMultiplier = 0;
    recruitField.ForEachAlive([&](const MinionData& data) {
        const auto& id = data.value().GetCardID();
        if (id == "BG22_HERO_004_Buddy") vardenMultiplier = std::max(vardenMultiplier, 1);
        else if (id == "BG22_HERO_004_Buddy_G") vardenMultiplier = std::max(vardenMultiplier, 2);
    });
    if (vardenMultiplier > 0 && !tavern.fieldZone.IsFull()) {
        int bestTier = -1; int bestSlot = -1;
        for (int i = 0; i < tavern.fieldZone.GetCount(); ++i) {
            const auto& offer = tavern.fieldZone[static_cast<std::size_t>(i)];
            const int tier = offer.GetGameTag(GameTag::TECH_LEVEL);
            if (!offer.IsDestroyed() && tier > bestTier) { bestTier = tier; bestSlot = i; }
        }
        if (bestSlot >= 0) {
            const auto& source = tavern.fieldZone[static_cast<std::size_t>(bestSlot)];
            Minion copy(Cards::FindCardByID(source.GetCardID()));
            ApplyFreshMinionModifiers(copy);
            const int amount = bestTier * vardenMultiplier;
            const int sourceAttack = source.GetAttack();
            const int sourceHealth = source.GetHealth();
            tavern.fieldZone[static_cast<std::size_t>(bestSlot)].SetAttack(sourceAttack + amount);
            tavern.fieldZone[static_cast<std::size_t>(bestSlot)].SetHealth(sourceHealth + amount);
            copy.SetAttack(sourceAttack + amount); copy.SetHealth(sourceHealth + amount);
            copy.SetFrozen(true); tavern.fieldZone[static_cast<std::size_t>(bestSlot)].SetFrozen(true);
            tavern.fieldZone.Add(copy);
        }
    }
}

void Player::RecordGoldSpent(std::int32_t amount)
{
    const int malorneBefore = season14.GoldSpentThisGame() / 3;
    const int thresholds = season14.RecordGoldSpent(amount);
    const int malorneDelta = season14.GoldSpentThisGame() / 3 - malorneBefore;
    if (malorneDelta > 0)
        recruitField.ForEachAlive([malorneDelta](MinionData& data) {
            const auto& id = data.value().GetCardID();
            if (id == "BG32_HERO_001_Buddy" || id == "BG32_HERO_001_Buddy_G") {
                const int multiplier = id.ends_with("_G") ? 2 : 1;
                data.value().ApplyPersistentMinionStats(malorneDelta * multiplier,
                                                        malorneDelta * multiplier);
            }
        });
    for (int i = 0; i < thresholds; ++i)
    {
        bool hasEscapee = false; bool goldenEscapee = false;
        recruitField.ForEachAlive([&](MinionData& d) { if (d.value().GetCardID() == "BG36_523" || d.value().GetCardID() == "BG36_523_G") { hasEscapee = true; goldenEscapee = goldenEscapee || d.value().GetCardID() == "BG36_523_G"; } });
        if (!hasEscapee) continue;
        if (season14.lockboxActive == false) {
            const auto lockbox = Cards::FindCardByID("BG36_520t");
            if (!lockbox.id.empty() && !hand.IsFull()) { hand.Add(CardData{Minion{lockbox}}); season14.lockboxActive = true; }
        } else { season14.lockboxAdvance += goldenEscapee ? 2 : 1; }
        recruitField.ForEachAlive([this](MinionData& data) {
            auto& minion = data.value();
            minion.ActivateTrigger(TriggerType::SPEND_GOLD, minion);
        });
    }
}

void Player::FreezeTavern()
{
    freezeTavern = !freezeTavern;
    tavern.fieldZone.ForEach(
        [this](MinionData& minion) { minion.value().SetFrozen(freezeTavern); });
    for (auto &slot : tavern.spellSlots) slot.SetFrozen(freezeTavern);
}

void Player::RearrangeMinion(std::size_t curIdx, std::size_t newIdx)
{
    if (curIdx == newIdx ||
        static_cast<int>(curIdx) >= recruitField.GetCount() ||
        static_cast<int>(newIdx) >= recruitField.GetCount())
    {
        return;
    }

    recruitField.Move(static_cast<int>(curIdx), static_cast<int>(newIdx));
}

void Player::CompleteRecruit()
{
    nextBoughtStatsArms.clear();
    ResolveRecruitEndDeaths();
    completeRecruitCallback();
}

void Player::ResolveRecruitEndDeaths()
{
    // Tomb Turning marks the selected minion instance, not its card ID.  At
    // recruit completion resolve its owned deathrattle while it is still on
    // the board, then return exactly that pool entry.
    for (int i = recruitField.GetCount() - 1; i >= 0; --i) {
        auto& minion = recruitField[static_cast<std::size_t>(i)];
        if (!minion.DiesAtRecruitEnd()) continue;
        if (minion.HasDeathrattle())
            minion.ActivateTask(PowerType::DEATHRATTLE, *this);
        const int poolIndex = minion.GetPoolIndex();
        recruitField.Remove(minion);
        returnMinionCallback(poolIndex);
    }
    // An undiscovered/unused Tomb Turning minion remains in hand, but its
    // one-turn marker must not leak into a later recruit phase.
    hand.ForEach([](std::optional<CardData>& data) {
        if (data.has_value() && std::holds_alternative<Minion>(data.value()))
            std::get<Minion>(data.value()).SetDiesAtRecruitEnd(false);
    });
}

void Player::ResolveDarkGiftEndTurnTriggers()
{
    recruitField.ForEachAlive([this](MinionData& data) {
        auto& minion = data.value();
        minion.AdvanceIncubation();
        if (minion.GetCardID() == "BG24_715" || minion.GetCardID() == "BG24_715_G")
            minion.AdvancePatientScout();
        if (minion.HasSteadyGrowth())
            minion.ApplySteadyGrowth();
        if (minion.AdvanceAffinity())
            (void)SimpleTasks::RandomCardToHandTask{
                minion.AffinityRace(), 0, 1}.Run(*this, minion);
        if (minion.HasPolarization())
            (void)SimpleTasks::RandomMagneticMechToTargetTask{}.Run(
                *this, minion, minion);
        if (minion.AdvanceReplication())
            AddMinionCopyToHand(minion);
        if (minion.HasEndTurnBattlecryTrigger())
            minion.ActivateTask(PowerType::POWER, *this);
    });
}

void Player::ResolveRelicsOfTheDeepStartTurn()
{
    if (season14.heroPowerDbfID != 85126)
        return;
    (void)SimpleTasks::RandomSpellcraftToHandTask{}.Run(*this);
}

void Player::ResolveMechGyverDeath()
{
    if (season14.heroPowerDbfID != 81572 || !season14.AdvanceMechGyverDeath())
        return;
    (void)SimpleTasks::RandomCardToHandTask{Race::MECHANICAL, 0, 1}.Run(*this);
}

void Player::AdvanceDarkGiftCounters(int kind)
{
    recruitField.ForEachAlive([kind](MinionData& data) {
        data.value().ApplyDarkGiftCounterStep(kind);
    });
}

void Player::ProcessDefeat()
{
    // Dead players may be selected as Battlegrounds ghosts and can receive
    // combat damage again. Defeat must be idempotent: processing the same
    // player twice corrupts the remaining-player count and placement ranks.
    if (playState != PlayState::PLAYING)
    {
        return;
    }
    processDefeatCallback(*this);
}
}  // namespace RosettaStone::Battlegrounds
