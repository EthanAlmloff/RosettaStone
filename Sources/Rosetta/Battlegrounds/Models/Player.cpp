// Copyright (c) 2017-2024 Chris Ohk

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Cards/CardDefs.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch21.hpp>
#include <Rosetta/Battlegrounds/CardSets/TavernSpellBehaviors.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/CardSets/EventCounterBehaviors.hpp>
#include <Rosetta/Battlegrounds/CardSets/TrinketBehaviors.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomCardToHandTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomMagneticMechToTargetTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomTavernSpellToHandTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ActivateRandomTavernSpellsTask.hpp>
#include <Rosetta/Battlegrounds/CardSets/DarkGiftBehaviors.hpp>
#include <Rosetta/Battlegrounds/CardSets/FodderBehaviors.hpp>
#include <Rosetta/Battlegrounds/CardSets/GiantSpellcraftBehaviors.hpp>
#include <Rosetta/Battlegrounds/CardSets/BuddyBehaviors.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomSpellcraftToHandTask.hpp>

#include <effolkronium/random.hpp>

#include <algorithm>
#include <array>
#include <iterator>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace RosettaStone::Battlegrounds
{
namespace
{
// The Rat King's active power is one of the rotating typed powers.  Pigeon
// Lord keys off the type currently offered by that power, not the hero's
// own historical tribe or the race of the buddy itself.
Race RatKingHeroPowerRace(std::int32_t dbfID) noexcept
{
    switch (dbfID)
    {
        case 59839: return Race::BEAST;
        case 59853: return Race::MECHANICAL;
        case 59852: return Race::MURLOC;
        case 59854: return Race::DEMON;
        case 60922: return Race::DRAGON;
        case 62277: return Race::PIRATE;
        case 64220: return Race::ELEMENTAL;
        case 71081: return Race::QUILBOAR;
        case 89813: return Race::NAGA;
        case 98888: return Race::UNDEAD;
        default: return Race::INVALID;
    }
}
}

void Player::OnCardAcquired(const CardData& card)
{
    if (!std::holds_alternative<Minion>(card) ||
        std::get<Minion>(card).GetRace() != Race::PIRATE)
        return;
    ++season14.piratesAcquiredThisGame;
    if (season14.piratesAcquiredThisGame % 4 == 0)
    {
        recruitField.ForEachAlive([this](MinionData& data) {
            const auto& id = data.value().GetCardID();
            if (id == "BG31_826") season14.IncreaseMaxGold(1);
            else if (id == "BG31_826_G") season14.IncreaseMaxGold(2);
        });
    }
}

namespace
{
using Random = effolkronium::random_thread_local;
void ApplySpellBoardEffect(Player&, const TavernSpellBehavior&, int, bool,
                           std::int32_t);

// These pool helpers are defined with the other supported-card predicates
// below, but are also used by modal legality/commit paths earlier in this
// translation unit.  Keep declarations here so every compiler sees the same
// fail-closed candidate filtering.
std::vector<Card> SupportedTierMinions(const Player& player);
std::vector<Card> SupportedMinionsForRace(Race race);
std::vector<Card> SupportedDeathrattleMinions();
std::vector<Card> SupportedBattlecryMinions();
std::vector<Card> SupportedEndTurnMinions();
struct SupportedMurlocHeroPair {
    std::int32_t heroDbfID;
    std::int32_t heroPowerDbfID;
};
// Exact Patch 36.4 active hero/power pair from the pinned manifest.  Keep
// this explicit: hero names/text are not a safe substitute for linked DBFs.
constexpr std::array<SupportedMurlocHeroPair, 1> SUPPORTED_MURLOC_HEROES = {{
    {84867, 90403}, // Murloc Holmes / Detective for Hire
}};
std::vector<Card> SupportedCombinedChooseOneMinions();
bool AddRandomMinionToHand(Player& player, std::vector<Card> candidates);
bool BeginMinionDiscover(Player& player, std::vector<Card> candidates,
                         std::int32_t sourceCardDbfID, bool lockHand = false);
bool BeginWindfallDiscover(Player& player, std::int32_t sourceCardDbfID,
                           std::int32_t attack, std::int32_t health,
                           std::int32_t remaining);
bool BeginBookOfMedivhDiscover(Player& player, std::int32_t sourceCardDbfID,
                               std::int32_t count = 1);

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
        case TavernSpellEffect::TARGET_STATS_NEXT_TURN:
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
        else if (id == "BG32_835" || id == "BG32_835_G")
        {
            attack += scale;
            health += scale;
        }
    });
    return {attack, health};
}

bool BeginBookOfMedivhDiscover(Player& player, std::int32_t sourceCardDbfID,
                               std::int32_t count)
{
    if (player.season14.pendingDecision != Season14Decision::NONE)
        return false;
    // A full hand does not consume the scheduled Discover.  Keep the
    // outstanding count in player-owned state so a later recruit-start (or
    // an explicit retry after a card leaves hand) can resume it instead of
    // silently burning the Book's trigger.
    if (player.hand.IsFull()) {
        player.season14.bookOfMedivhRemaining = std::max(
            player.season14.bookOfMedivhRemaining, std::max(1, count));
        return false;
    }
    std::vector<Card> candidates;
    for (const auto& card : Cards::GetAllCards())
    {
        if (!card.isBattlegroundsPoolSpell || card.normalDbfID != 0 ||
            card.GetCardType() != CardType::SPELL ||
            FindTavernSpellBehavior(card.id).effect == TavernSpellEffect::NONE)
            continue;
        candidates.push_back(card);
    }
    if (candidates.size() < 3) return false;
    Random::shuffle(candidates.begin(), candidates.end());
    std::vector<Season14Offering> offerings;
    for (std::size_t i = 0; i < 3; ++i)
        offerings.push_back({candidates[i].dbfID, 0, 0});
    player.season14.bookOfMedivhRemaining = std::max(1, count);
    player.season14.BeginOfferingDecision(
        Season14Decision::DISCOVER, 0, sourceCardDbfID, std::move(offerings));
    return true;
}
}

FieldZone& Player::GetField()
{
    return isInCombat ? battleField : recruitField;
}

void Player::IncrementStartCombatSpellImprovements()
{
    // Nalaa and Charging Czarina resolve from the same successful Tavern
    // spell boundary as Evoker.  Keep the card-specific payloads in this
    // small table so unsupported Spellcraft text cannot accidentally grant a
    // generic stat effect.
    struct TavernSpellTriggerSpec
    {
        std::string_view id;
        int attack;
        int health;
        bool onePerRace;
        bool divineShieldOnly;
    };
    static constexpr TavernSpellTriggerSpec specs[] = {
        {"BG28_551", 4, 3, true, false},
        {"BG28_741", 4, 0, false, true},
    };

    for (const auto& spec : specs)
    {
        int totalScale = 0;
        recruitField.ForEachAlive([&](const MinionData& data) {
            const auto id = data.value().GetCardID();
            if (id == spec.id || id == std::string(spec.id) + "_G")
                totalScale += data.value().IsGolden() ? 2 : 1;
        });
        if (totalScale == 0) continue;
        if (spec.divineShieldOnly)
        {
            const int attack = spec.attack * totalScale;
            recruitField.ForEachAlive([attack](MinionData& data) {
                auto& minion = data.value();
                if (minion.HasDivineShield())
                    minion.SetAttack(minion.GetAttack() + attack);
            });
            continue;
        }
        if (!spec.onePerRace) continue;
        // Each Nalaa source is an independent trigger.  Resolve them one at
        // a time so two copies may choose different members of a type.
        recruitField.ForEachAlive([&](MinionData& sourceData) {
            const auto sourceID = sourceData.value().GetCardID();
            if (sourceID != spec.id &&
                sourceID != std::string(spec.id) + "_G") return;
            const int scale = sourceData.value().IsGolden() ? 2 : 1;
            const int attack = spec.attack * scale;
            const int health = spec.health * scale;
            for (const auto race : RACES_IN_BATTLEGROUNDS)
            {
                std::vector<Minion*> candidates;
                recruitField.ForEachAlive([&](MinionData& data) {
                    if (data.value().HasRace(race))
                        candidates.push_back(&data.value());
                });
                if (candidates.empty()) continue;
                const auto target = candidates[Random::get<std::size_t>(0, candidates.size() - 1)];
                target->SetAttack(target->GetAttack() + attack);
                target->SetHealth(target->GetHealth() + health);
            }
        });
    }

    recruitField.ForEachAlive([](MinionData& data) {
        auto& minion = data.value();
        if (minion.GetCardID() == "BG32_822" ||
            minion.GetCardID() == "BG32_822_G")
            minion.IncrementStartCombatSpellImprovement();
    });
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
    int ashen = 0;
    recruitField.ForEachAlive([&](MinionData& data) { if (data.value().GetCardID() == "BG32_873") ashen = std::max(ashen, 1); else if (data.value().GetCardID() == "BG32_873_G") ashen = std::max(ashen, 2); });
    if (ashen > 0) {
        hero.health += event.healthLost;
        season14.AddTemporaryRefreshShopStats(ashen, ashen);
        tavern.fieldZone.ForEachAlive([ashen](MinionData& data) { data.value().SetAttack(data.value().GetAttack() + ashen); data.value().SetHealth(data.value().GetHealth() + ashen); });
    }

    // Soul Rewinder replaces recruit-phase damage and then grows for the
    // successful trigger.  Armor is already excluded by Hero::TakeDamage via
    // healthLost, so only actual hero Health loss is rewound.
    int rewindHealth = 0;
    recruitField.ForEachAlive([&rewindHealth](MinionData& data) {
        const auto& minion = data.value();
        if (minion.GetCardID() == "BG26_174") rewindHealth += 1;
        else if (minion.GetCardID() == "BG26_174_G") rewindHealth += 2;
    });
    if (rewindHealth != 0) {
        hero.health += event.healthLost;
        recruitField.ForEachAlive([rewindHealth](MinionData& data) {
            auto& minion = data.value();
            if (minion.GetCardID() == "BG26_174")
                minion.SetHealth(minion.GetHealth() + 1);
            else if (minion.GetCardID() == "BG26_174_G")
                minion.SetHealth(minion.GetHealth() + 2);
        });
    }

    // Unearthed Underling replaces recruit-phase hero damage and grows by the
    // prevented amount.  Golden copies use the pinned double-stat scaling;
    // armor-only and combat damage never reach this branch.
    bool hasUnderling = false;
    recruitField.ForEachAlive([&hasUnderling](MinionData& data) {
        const auto& id = data.value().GetCardID();
        if (id == "TB_BaconShop_HERO_25_Buddy" ||
            id == "TB_BaconShop_HERO_25_Buddy_G")
            hasUnderling = true;
    });
    if (hasUnderling) {
        hero.health += event.healthLost;
        recruitField.ForEachAlive([event](MinionData& data) {
            const auto& id = data.value().GetCardID();
            if (id != "TB_BaconShop_HERO_25_Buddy" &&
                id != "TB_BaconShop_HERO_25_Buddy_G")
                return;
            const int multiplier = id == "TB_BaconShop_HERO_25_Buddy_G" ? 2 : 1;
            data.value().ApplyPersistentMinionStats(
                multiplier * event.healthLost, multiplier * event.healthLost);
        });
    }

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
    ResolveHeroPowerUseBuddies();
    return true;
}

void Player::ResolveHeroPowerUseBuddies()
{
    // Karl's trigger is tied to the successful payment/use boundary, not to
    // damage. Each owned copy triggers independently; golden copies change
    // their own amount and therefore stack with normal/golden copies.
    int amount = 0;
    recruitField.ForEachAlive([&amount](const MinionData& data) {
        const auto& id = data.value().GetCardID();
        if (id == "TB_BaconShop_HERO_15_Buddy") amount += 2;
        else if (id == "TB_BaconShop_HERO_15_Buddy_G") amount += 4;
    });
    if (amount == 0) return;
    recruitField.ForEachAlive([amount](MinionData& data) {
        auto& minion = data.value();
        if (minion.HasDivineShield())
            minion.SetAttack(minion.GetAttack() + amount);
    });
}

void Player::ApplyFreshMinionModifiers(Minion& minion)
{
    // Warpwing's keyword is static in card data, but retaining it on freshly
    // copied/summoned entities makes the combat immunity check authoritative.
    if (minion.GetCardID() == "BG24_004" ||
        minion.GetCardID() == "BG24_004_G")
        minion.SetImmuneWhileAttacking(true);
    // Viper's Venomous is paired with immunity while it is attacking.  The
    // flag is installed on each combat copy here, never on the recruit card.
    if (minion.GetCardID() == "BG31_HERO_811t8" ||
        minion.GetCardID() == "BG31_HERO_811t8_G")
        minion.SetImmuneWhileAttacking(true);
    // Ultralisk's combat-start multiplier is carried by the combat copy;
    // unlike an ordinary stat buff it is consumed by Battle::Initialize.
    if (minion.GetCardID() == "BG31_HERO_811t10")
        minion.SetStartCombatStatMultipliers(2, 2);
    else if (minion.GetCardID() == "BG31_HERO_811t10_G")
        minion.SetStartCombatStatMultipliers(3, 3);
    // Sprout It Out! is a hidden-cost combat aura.  Restrict it to newly
    // summoned combat entities so recruit purchases/plays and hand cards do
    // not inherit combat-only stats or Taunt.
    if (isInCombat && season14.heroPowerDbfID == 67554)
    {
        minion.SetAttack(minion.GetAttack() + 1);
        minion.SetHealth(minion.GetHealth() + 2);
        minion.SetTaunt(true);
    }
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
    if (minion.GetCardID() == "BG25_008" ||
        minion.GetCardID() == "BG25_008_G")
        minion.ApplyEternalKnightDeathCount(eternalKnightsDiedThisGame);
    if (season14.GeneratedRewardGlobalAttack() != 0)
        minion.ApplyGlobalMinionAttack(season14.GeneratedRewardGlobalAttack());
}

void Player::ApplyInfestorPlayCardBuff()
{
    int total = 0;
    recruitField.ForEachAlive([&total](MinionData& data) {
        const auto id = data.value().GetCardID();
        if (id == "BG31_HERO_811t9") total += 1;
        else if (id == "BG31_HERO_811t9_G") total += 2;
    });
    if (total == 0) return;
    recruitField.ForEachAlive([total](MinionData& data) {
        data.value().SetAttack(data.value().GetAttack() + total);
        data.value().SetHealth(data.value().GetHealth() + total);
    });
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
    if (season14.HasGeneratedRewardAlterEgo() &&
        ((minion.GetTier() % 2 == 0) == season14.generatedRewardAlterEgoEven))
    {
        minion.SetAttack(minion.GetAttack() + 7);
        minion.SetHealth(minion.GetHealth() + 7);
    }
}

bool Player::ApplyGeneratedQuestReward(std::int32_t dbfID)
{
    if (dbfID == 110310) {
        // Secret Culprit's linked {0} is a random supported normal Tier 7
        // minion.  Resolve and copy it atomically while the reward is being
        // installed; never synthesize a card or admit a golden placeholder.
        if (hand.IsFull()) return false;
        std::vector<Card> candidates;
        for (const auto& candidate : Cards::GetTier7Minions())
            if (candidate.GetCardType() == CardType::MINION &&
                candidate.isBattlegroundsPoolMinion &&
                candidate.normalDbfID == 0 && candidate.hasBehavior)
                candidates.push_back(candidate);
        if (candidates.empty()) return false;
        const auto& selected = candidates[
            Random::get<std::size_t>(0, candidates.size() - 1)];
        hand.Add(CardData{Minion(selected)});
    }
    if (dbfID == 104673) {
        // Gilnean War Horn's {0} is the Battlecry minion selected by the
        // completed quest.  The compact simulator does not carry that quest
        // payload, so resolve it from the current public warband and fail
        // closed when no legal linked entity exists.
        if (hand.IsFull()) return false;
        std::vector<Card> candidates;
        recruitField.ForEachAlive([&candidates](MinionData& data) {
            const auto& minion = data.value();
            if (!minion.HasBattlecry()) return;
            const auto card = Cards::FindCardByID(minion.GetCardID());
            // The server-selected {0} is a normal, supported Battlecry
            // minion.  Never copy a golden/generated/unsupported placeholder
            // merely because the current warband happens to contain it.
            if (!card.id.empty() && card.GetCardType() == CardType::MINION &&
                card.isBattlegroundsPoolMinion && card.normalDbfID == 0 &&
                card.hasBehavior)
                candidates.push_back(card);
        });
        if (candidates.empty()) return false;
        const auto& selected = candidates[
            Random::get<std::size_t>(0, candidates.size() - 1)];
        hand.Add(CardData{Minion(selected)});
    }
    if (dbfID == 96148 && !season14.HasGeneratedRewardNineLives()) {
        // 9 Lives is an immediate reward: health is set before any later
        // start-of-turn observers and the eight Secret cards obey the normal
        // hand-cap rule (the simulator's authoritative Ice Block entity is
        // used rather than a synthetic placeholder).
        const auto iceBlock = Cards::FindCardByID("EX1_295");
        if (iceBlock.dbfID == 0) return false;
        hero.health = 1;
        for (int i = 0; i < 8 && !hand.IsFull(); ++i)
            hand.Add(CardData{Spell(iceBlock)});
    }
    if (dbfID == 91980) {
        const auto buddy = Cards::FindCardByDbfID(hero.card.relatedDbfID);
        if (buddy.dbfID == 0 || buddy.premiumDbfID == 0 ||
            buddy.GetCardType() != CardType::MINION || hand.IsFull())
            return false;
        const auto golden = Cards::FindCardByDbfID(buddy.premiumDbfID);
        if (golden.dbfID == 0 || golden.GetCardType() != CardType::MINION)
            return false;
        hand.Add(CardData{Minion(golden)});
    }
    if (dbfID == 90917 && season14.GeneratedRewardGhastlyCardDbfID() == 0 &&
        !season14.GhastlyCardDelivered())
    {
        // Ghastly Mask's linked {0} entity is absent from cards.json.  Build
        // the replacement from executable normal minions whose pinned
        // CardDef has an actual TURN_END trigger; never use arbitrary text or
        // metadata-only rows.
        const auto candidates = SupportedEndTurnMinions();
        if (candidates.empty()) return false;
        const auto& selected =
            candidates[Random::get<std::size_t>(0, candidates.size() - 1)];
        season14.SetGeneratedRewardGhastlyCardDbfID(selected.dbfID);
    }
    if (dbfID == 92551 &&
        season14.GeneratedRewardFriendsRace() == Race::INVALID)
    {
        // The card text stores a server-selected {0} tribe.  Pin that
        // selection from the lobby's authoritative ten-race pool and only
        // accept races for which the pinned card data exposes supported
        // normal Battlegrounds minions.
        std::vector<Race> races;
        for (const auto race : RACES_IN_BATTLEGROUNDS)
            if (!SupportedMinionsForRace(race).empty()) races.push_back(race);
        if (races.empty()) return false;
        season14.SetGeneratedRewardFriendsRace(
            races[Random::get<std::size_t>(0, races.size() - 1)]);
    }
    if (dbfID == 96151 &&
        season14.GeneratedRewardUnmurlocHeroDbfID() == 0)
    {
        std::vector<SupportedMurlocHeroPair> candidates;
        for (const auto pair : SUPPORTED_MURLOC_HEROES) {
            const auto heroCard = Cards::FindCardByDbfID(pair.heroDbfID);
            const auto powerCard = Cards::FindCardByDbfID(pair.heroPowerDbfID);
            if (heroCard.GetCardType() == CardType::HERO &&
                heroCard.isCurHero && heroCard.heroPowerDbfID == pair.heroPowerDbfID &&
                powerCard.GetCardType() == CardType::HERO_POWER)
                candidates.push_back(pair);
        }
        if (candidates.empty()) return false;
        const auto selected = candidates[
            Random::get<std::size_t>(0, candidates.size() - 1)];
        season14.SetGeneratedRewardUnmurloc(selected.heroDbfID);
    }
    if (!season14.ApplyGeneratedQuestReward(dbfID)) return false;
    if (dbfID == 96150) {
        // Purified Shard is an immediate win condition.  Mark the player
        // terminal at selection time so the lobby coordinator stops offering
        // recruit/combat decisions for this seat.
        season14.generatedRewardPurifiedShard = true;
        playState = PlayState::WON;
        rank = 1;
    }
    if (dbfID == 104842)
        season14.AddBloodGemBonus(1, 1);

    if (dbfID == 96151 && season14.HasGeneratedRewardUnmurloc()) {
        const auto targetHero = Cards::FindCardByDbfID(
            season14.GeneratedRewardUnmurlocHeroDbfID());
        const auto targetPower = Cards::FindCardByDbfID(
            targetHero.heroPowerDbfID);
        const bool supportedPair = std::any_of(
            SUPPORTED_MURLOC_HEROES.begin(), SUPPORTED_MURLOC_HEROES.end(),
            [&targetHero](const auto pair) {
                return pair.heroDbfID == targetHero.dbfID &&
                       pair.heroPowerDbfID == targetHero.heroPowerDbfID;
            });
        if (targetHero.dbfID == 0 || targetPower.dbfID == 0)
            return false;
        if (!supportedPair) return false;
        if (hero.card.dbfID == targetHero.dbfID)
            return true;
        const auto health = hero.health;
        hero.Initialize(targetHero);
        hero.health = health;
        // Detective for Hire is the pinned zero-cost Holmes power.  Calling
        // SetHeroPower resets usage/cooldown state and keeps replay behavior
        // equivalent to selecting that exact hero/power pair.
        season14.SetHeroPower(targetPower.dbfID, 0, true);
    }
    if (dbfID == 91992 && season14.pendingDecision == Season14Decision::NONE) {
        std::vector<Card> candidates;
        for (const auto& card : Cards::GetAllCards())
            if (card.isBattlegroundsPoolMinion && card.normalDbfID == 0 &&
                card.hasBehavior && card.GetTier() == currentTier)
                candidates.push_back(card);
        if (!candidates.empty()) {
            std::vector<Season14Offering> offerings;
            for (std::size_t i = 0; i < std::min<std::size_t>(3, candidates.size()); ++i)
                offerings.push_back({candidates[i].dbfID, 0});
            season14.BeginOfferingDecision(Season14Decision::DISCOVER, 0,
                                           dbfID, std::move(offerings));
        }
    }
    if (dbfID == 90917 && !hand.IsFull()) {
        const auto card = Cards::FindCardByDbfID(
            season14.GeneratedRewardGhastlyCardDbfID());
        if (card.dbfID != 0) {
            hand.Add(CardData{Minion(card)});
            season14.MarkGhastlyCardDelivered();
        }
    }
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
    // Essence of Zerus grants its card at the end of the recruit turn.
    // Hand capacity is authoritative, so a full hand simply receives no
    // generated card for this turn.
    if (season14.HasGeneratedRewardEssenceOfZerus() && !hand.IsFull()) {
        const auto card = Cards::FindCardByID("BGS_029");
        if (!card.id.empty()) hand.Add(CardData{Minion(card)});
    }
    if (season14.HasGeneratedRewardSturdyShard()) {
        std::int32_t taunts = 0;
        recruitField.ForEachAlive([&taunts](const MinionData& data) {
            if (data.value().HasTaunt()) ++taunts;
        });
        recruitField.ForEachAlive([taunts](MinionData& data) {
            if (!data.value().HasTaunt()) {
                data.value().SetAttack(data.value().GetAttack() + taunts);
                data.value().SetHealth(data.value().GetHealth() + 2 * taunts);
            }
        });
    }
    if (season14.HasGeneratedRewardDevilsInDetails()) {
        std::vector<std::uint64_t> targets;
        recruitField.ForEachAlive([&](MinionData& data) {
            targets.push_back(static_cast<std::uint64_t>(data.value().GetIndex()));
        });
        if (!targets.empty()) {
            std::vector<std::uint64_t> edges{targets.front()};
            if (targets.back() != targets.front()) edges.push_back(targets.back());
            for (const auto entityID : edges) {
                Minion* consumer = nullptr;
                recruitField.ForEachAlive([&](MinionData& data) {
                    if (static_cast<std::uint64_t>(data.value().GetIndex()) == entityID)
                        consumer = &data.value();
                });
                if (!consumer) continue;
                std::vector<std::uint64_t> offers;
                tavern.fieldZone.ForEachAlive([&](MinionData& data) {
                    offers.push_back(static_cast<std::uint64_t>(data.value().GetIndex()));
                });
                if (offers.empty()) continue;
                const auto selected = offers[Random::get<std::size_t>(0, offers.size() - 1)];
                Minion* consumed = nullptr;
                tavern.fieldZone.ForEachAlive([&](MinionData& data) {
                    if (static_cast<std::uint64_t>(data.value().GetIndex()) == selected)
                        consumed = &data.value();
                });
                if (!consumed) continue;
                consumer->SetAttack(consumer->GetAttack() + consumed->GetAttack());
                consumer->SetHealth(consumer->GetHealth() + consumed->GetHealth());
                (void)tavern.fieldZone.Remove(*consumed);
            }
        }
    }
    if (season14.HasGeneratedRewardMenagerieMayhem()) {
        std::int32_t raceCount = 0;
        for (const auto race : RACES_IN_BATTLEGROUNDS) {
            bool present = false;
            recruitField.ForEachAlive([&](const MinionData& data) {
                present = present || data.value().HasRace(race);
            });
            if (present) ++raceCount;
        }
        recruitField.ForEachAlive([raceCount](MinionData& data) {
            data.value().SetAttack(data.value().GetAttack() + raceCount);
        });
    }
    if (season14.HasGeneratedRewardBloodGoblet()) {
        std::vector<Minion*> candidates;
        recruitField.ForEachAlive([&candidates](MinionData& data) {
            candidates.push_back(&data.value());
        });
        if (!candidates.empty()) {
            auto* rightmost = candidates.back();
            // Battlegrounds starts at 30 health; armor is intentionally not
            // part of the missing-health calculation.
            rightmost->SetAttack(rightmost->GetAttack() +
                                 std::max(0, 30 - hero.health));
        }
    }
    if (season14.HasGeneratedRewardTheWall()) {
        std::int32_t nonTaunts = 0;
        recruitField.ForEachAlive([&nonTaunts](const MinionData& data) {
            if (!data.value().HasTaunt()) ++nonTaunts;
        });
        recruitField.ForEachAlive([nonTaunts](MinionData& data) {
            if (data.value().HasTaunt()) {
                data.value().SetAttack(data.value().GetAttack() + nonTaunts);
                data.value().SetHealth(data.value().GetHealth() + nonTaunts);
            }
        });
    }
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

void Player::ResolveTrinketEndTurn()
{
    for (auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0)
            continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect == TrinketEffect::END_TURN_FIXED_CARD) {
            if (behavior.value <= 0 || ++trinket.triggerProgress < behavior.value)
                continue;
            // The timer advances even when the hand is full.  A missed
            // generated card is not deferred into a later turn; this keeps
            // "every 3 turns" cadence exact and prevents a full hand from
            // permanently freezing the Trinket's counter.
            trinket.triggerProgress = 0;
            if (behavior.cardID.empty() || hand.IsFull()) continue;
            const auto card = Cards::FindCardByID(behavior.cardID);
            if (card.id.empty()) continue;
            hand.Add(CardData{Spell(card)});
            continue;
        }
        if (behavior.effect != TrinketEffect::END_TURN_RANDOM_TYPE_MINIONS)
            continue;
        std::set<Race> seen;
        recruitField.ForEachAlive([&](const MinionData& data) {
            for (const auto race : RACES_IN_BATTLEGROUNDS)
                if (data.value().HasRace(race))
                    seen.insert(race);
        });
        for (const auto race : RACES_IN_BATTLEGROUNDS)
        {
            if (!seen.contains(race) || hand.IsFull()) continue;
            (void)AddRandomMinionToHand(*this, SupportedMinionsForRace(race));
        }
    }
}

void Player::ResolveGeneratedQuestRewardStartCombat(FieldZone& combatField)
{
    if (season14.HasGeneratedRewardRighteousCharge() && !combatField.IsEmpty()) {
        // The combat scheduler starts at the leftmost eligible attacker.  Arm
        // the copied entity before that scheduler runs so it receives the
        // charge's Divine Shield and immediate first attack without mutating
        // the recruit-phase entity.
        combatField[0].SetGameTag(GameTag::DIVINE_SHIELD, 1);
    }
    if (season14.HasGeneratedRewardVolatileVenom()) {
        combatField.ForEachAlive([](MinionData& data) {
            data.value().SetAttack(data.value().GetAttack() + 7);
            data.value().SetHealth(data.value().GetHealth() + 7);
        });
    }
    if (season14.HasGeneratedRewardStaffOfOrigination())
    {
        combatField.ForEachAlive([](MinionData& data) {
            data.value().SetAttack(data.value().GetAttack() + 12);
            data.value().SetHealth(data.value().GetHealth() + 12);
        });
    }
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

void Player::ResolveGeneratedQuestRewardTinyHenchmen()
{
    if (!season14.HasGeneratedRewardTinyHenchmen()) return;
    std::vector<Minion*> candidates;
    recruitField.ForEachAlive([&candidates](MinionData& data) {
        if (data.value().GetTier() <= 3) candidates.push_back(&data.value());
    });
    Random::shuffle(candidates.begin(), candidates.end());
    const auto count = std::min<std::size_t>(3, candidates.size());
    for (std::size_t i = 0; i < count; ++i)
    {
        candidates[i]->SetAttack(candidates[i]->GetAttack() + 3);
        candidates[i]->SetHealth(candidates[i]->GetHealth() + 3);
    }
}

void Player::ResolveGeneratedQuestRewardDeath(Minion& deadMinion)
{
    if (!season14.HasGeneratedRewardRitualDagger()) return;
    // The entity has already been removed from the combat field when this
    // hook runs.  Mutate that removed instance so Reborn and any later
    // permanent reconciliation retain the reward; searching recruitField
    // cannot find a dead minion and silently over-credits nothing.
    deadMinion.SetAttack(deadMinion.GetAttack() + 4);
    deadMinion.SetHealth(deadMinion.GetHealth() + 4);
}

void Player::ResolveGeneratedQuestRewardCombatDeath(FieldZone& enemyField)
{
    if (season14.AdvanceGeneratedRewardAvengeRefresh())
        season14.AddFreeRefreshes(1);
    if (season14.HasGeneratedRewardBoomSquad() &&
        ++season14.generatedRewardBoomSquadDeaths >= 3) {
        season14.generatedRewardBoomSquadDeaths = 0;
        std::vector<Minion*> candidates;
        int highestHealth = -1;
        enemyField.ForEachAlive([&](MinionData& data) {
            auto& minion = data.value();
            if (minion.GetHealth() > highestHealth) {
                highestHealth = minion.GetHealth();
                candidates.clear();
                candidates.push_back(&minion);
            } else if (minion.GetHealth() == highestHealth) {
                candidates.push_back(&minion);
            }
        });
        if (!candidates.empty())
            candidates[Random::get<std::size_t>(0, candidates.size() - 1)]->TakeDamage(10);
    }
    if (season14.HasGeneratedRewardCycleEnergy() &&
        ++season14.generatedRewardCycleEnergyDeaths >= 3) {
        season14.generatedRewardCycleEnergyDeaths = 0;
        if (!hand.IsFull())
            (void)SimpleTasks::RandomTavernSpellToHandTask{1}.Run(*this);
    }
    if (season14.HasGeneratedRewardStableAmalgamation() &&
        ++season14.generatedRewardStableAmalgamationDeaths >= 7) {
        season14.generatedRewardStableAmalgamationDeaths = 0;
        const auto card = Cards::FindCardByID("BG28_Reward_518t");
        if (!card.id.empty() && !battleField.IsFull()) {
            Minion summoned(card);
            ApplyFreshMinionModifiers(summoned);
            (void)SummonCombatSnapshot(std::move(summoned));
        }
    }
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
    // Quaint Boutique and Jumbo Warehouse arm their four-gold grant when the
    // reward is selected; deliver deferred gold exactly once at recruit start.
    remainCoin += season14.TakeNextTurnGold();
    if (season14.HasGeneratedRewardMagicfin() &&
        season14.pendingDecision == Season14Decision::NONE && !hand.IsFull()) {
        const auto card = Cards::FindCardByID("BG33_890t");
        if (!card.id.empty()) {
            Minion apprentice(card);
            ApplyFreshMinionModifiers(apprentice);
            hand.Add(CardData{std::move(apprentice)});
            // Magicfin's Discover teaches the newly-created Apprentice.  The
            // special ApplyChoice path keeps the selected spell out of hand.
            if (!BeginTavernSpellDiscover(1, 0, 122825))
                hand.Remove(hand[hand.GetCount() - 1]);
        }
    }
    if (season14.HasGeneratedRewardUntoldRiches()) {
        // Untold Riches is a one-shot reward grant (unlike the explicitly
        // recurring Magicfin/Golden Forge start-turn effects).
        season14.generatedRewardUntoldRiches = false;
        season14.IncreaseMaxGold(5);
        if (!hand.IsFull()) {
            const auto card = Cards::FindCardByID("BG33_Reward_012t");
            if (!card.id.empty()) hand.Add(CardData{Spell(card)});
        }
    }
    if (season14.HasGeneratedRewardGoldenForge()) {
        std::vector<Minion*> highestTier;
        int highest = 0;
        tavern.fieldZone.ForEachAlive([&](MinionData& data) {
            auto& candidate = data.value();
            if (candidate.IsGolden() || !candidate.CanMakeGolden()) return;
            if (candidate.GetTier() > highest) {
                highest = candidate.GetTier();
                highestTier.clear();
            }
            if (candidate.GetTier() == highest) highestTier.push_back(&candidate);
        });
        if (!highestTier.empty())
            highestTier[Random::get<std::size_t>(0, highestTier.size() - 1)]->MakeGolden();
    }
    // Quaint Boutique/Jumbo Warehouse pay their four Gold at the next
    // recruit start (armed when the reward is installed), then open the
    // corresponding Trinket purchase modal.  Keep the reward armed until a
    // valid public offer can be built so a full/empty pool never loses it.
    const auto beginGeneratedTrinketOffer = [this](bool greater,
                                                    std::int32_t sourceDbfID,
                                                    bool& armed) {
        if (!armed || season14.pendingDecision != Season14Decision::NONE ||
            !season14.CanAddTrinket() || remainCoin < 4) return;
        std::vector<Card> candidates;
        for (const auto& candidate : Cards::GetAllCards())
            if (candidate.trinketType == (greater ? "GREATER_TRINKET" : "LESSER_TRINKET") &&
                candidate.normalDbfID == 0 && candidate.dbfID > 0 &&
                candidate.GetCardType() == CardType::BATTLEGROUND_TRINKET &&
                std::none_of(season14.trinkets.begin(), season14.trinkets.end(),
                    [&candidate](const Season14PersistentEffect& existing) {
                        return existing.dbfID == candidate.dbfID;
                    }))
                candidates.push_back(candidate);
        if (candidates.size() < 3) return;
        Random::shuffle(candidates.begin(), candidates.end());
        remainCoin -= 4;
        RecordGoldSpent(4);
        season14.BeginOfferingDecision(
            Season14Decision::TRINKET_SELECTION, 0, sourceDbfID,
            {{candidates[0].dbfID, 0}, {candidates[1].dbfID, 0},
             {candidates[2].dbfID, 0}});
        armed = false;
    };
    beginGeneratedTrinketOffer(false, 122013,
                               season14.generatedRewardQuaintBoutique);
    beginGeneratedTrinketOffer(true, 122014,
                               season14.generatedRewardJumboWarehouse);
    if (season14.HasGeneratedRewardCosmicReward() &&
        season14.pendingDecision == Season14Decision::NONE) {
        std::vector<Card> powers;
        for (const auto& candidate : Cards::GetAllCards())
            if (candidate.GetCardType() == CardType::HERO_POWER &&
                candidate.dbfID > 0 && candidate.dbfID != season14.heroPowerDbfID &&
                candidate.normalDbfID == 0 &&
                candidate.hasBehavior &&
                FindSeason14HeroPowerBehavior(candidate.dbfID) != nullptr &&
                !FindSeason14HeroPowerBehavior(candidate.dbfID)->passive)
                powers.push_back(candidate);
        if (powers.size() >= 3) {
            Random::shuffle(powers.begin(), powers.end());
            season14.BeginOfferingDecision(
                Season14Decision::DISCOVER, 0, 122924,
                {{powers[0].dbfID, 0}, {powers[1].dbfID, 0},
                 {powers[2].dbfID, 0}});
            season14.generatedRewardCosmicReward = false;
        }
    }
    if (season14.HasGeneratedRewardOpponentWarbandGuess() &&
        season14.pendingDecision == Season14Decision::NONE) {
        // No Place Like Holmes can only offer information observed from the
        // immediately preceding combat.  Preserve the reward until an
        // observation exists; never substitute hidden/future opponent state.
        std::vector<std::int32_t> observed;
        for (const auto dbfID : season14.lastOpponentCombatMinionDbfIDs) {
            const auto seenCard = Cards::FindCardByDbfID(dbfID);
            // A golden minion is still publicly observable and is a valid
            // Holmes guess.  Present the canonical (plain) identity in the
            // modal so golden and plain copies do not consume two offerings.
            const auto card = seenCard.normalDbfID != 0
                ? Cards::FindCardByDbfID(seenCard.normalDbfID) : seenCard;
            if (card.GetCardType() != CardType::MINION || !card.hasBehavior ||
                std::find(observed.begin(), observed.end(), card.dbfID) != observed.end())
                continue;
            observed.push_back(card.dbfID);
        }
        if (observed.size() >= 2) {
            Random::shuffle(observed.begin(), observed.end());
            observed.resize(std::min<std::size_t>(3, observed.size()));
            std::vector<Season14Offering> offerings;
            for (const auto dbfID : observed) offerings.push_back({dbfID, 0});
            season14.BeginOfferingDecision(
                Season14Decision::DISCOVER, 0, 106440, std::move(offerings));
        }
    }
    if (season14.HasGeneratedRewardRushingWinds() && !hand.IsFull()) {
        const auto card = Cards::FindCardByID("BG33_Reward_006t");
        if (!card.id.empty()) {
            Spell spell(card);
            spell.SetTemporary(true);
            hand.Add(CardData{std::move(spell)});
        }
    }
    if (season14.HasGeneratedRewardNorgannon()) {
        season14.generatedRewardNorgannon = false;
        UpgradeTavernForGeneratedReward();
    }
    if (season14.HasGeneratedRewardOpenAuditions() &&
        season14.pendingDecision == Season14Decision::NONE && !hand.IsFull()) {
        std::vector<Card> buddies;
        for (const auto& card : Cards::GetAllCards())
            if (card.GetCardType() == CardType::MINION &&
                card.id.ends_with("_Buddy") && card.normalDbfID == 0 &&
                card.isBattlegroundsPoolMinion && card.hasBehavior)
                buddies.push_back(card);
        if (buddies.size() >= 3) {
            Random::shuffle(buddies.begin(), buddies.end());
            season14.BeginOfferingDecision(
                Season14Decision::DISCOVER, 0, 110325,
                {{buddies[0].dbfID, 0}, {buddies[1].dbfID, 0},
                 {buddies[2].dbfID, 0}});
        }
    }
    if (season14.HasGeneratedRewardStartTurnRandomSpells()) {
        season14.BeginGeneratedRewardRandomSpells(5);
        (void)SimpleTasks::ActivateRandomTavernSpellsTask{5}.Run(*this);
    }
    season14.generatedRewardWisdomballUsedThisTurn = false;
    season14.generatedRewardDoubleHeadedUsedThisTurn = false;
    if (season14.HasGeneratedRewardTimelineAcceleration()) {
        const auto card = Cards::FindCardByID("BG27_Reward_504t");
        if (!card.id.empty()) {
            for (int i = 0; i < 2 && !hand.IsFull(); ++i) {
                Spell spell(card);
                spell.SetTemporary(true);
                hand.Add(CardData{std::move(spell)});
            }
        }
    }
    if (season14.HasGeneratedRewardStashOfTheScribe())
        (void)SimpleTasks::RandomTavernSpellToHandTask{3}.Run(*this);
    if (season14.HasGeneratedRewardSmeltingChamber()) {
        std::vector<Minion*> candidates;
        recruitField.ForEachAlive([&](MinionData& data) {
            if (data.value().GetTier() == season14.GeneratedRewardSmeltingTier() &&
                data.value().CanMakeGolden())
                candidates.push_back(&data.value());
        });
        if (!candidates.empty())
            candidates[Random::get<std::size_t>(0, candidates.size() - 1)]->MakeGolden();
        season14.AdvanceGeneratedRewardSmeltingTier();
    }
    // Shifter Zerus transforms once per turn while it remains in hand.  The
    // reward creates the canonical BGS_029 card at end turn; this lifecycle
    // pass performs its next-turn transformation from the executable normal
    // Battlegrounds pool rather than silently replacing the card at grant
    // time.
    std::vector<Card> zerusCandidates;
    for (const auto& card : Cards::GetAllCards())
        if (card.id != "BGS_029" && card.isBattlegroundsPoolMinion &&
            card.GetCardType() == CardType::MINION && card.normalDbfID == 0 &&
            card.hasBehavior)
            zerusCandidates.push_back(card);
    if (!zerusCandidates.empty()) {
        hand.ForEach([&zerusCandidates](std::optional<CardData>& data) {
            if (!data.has_value() || !std::holds_alternative<Minion>(data.value()) ||
                std::get<Minion>(data.value()).GetCardID() != "BGS_029")
                return;
            auto& zerus = std::get<Minion>(data.value());
            const auto& replacement = zerusCandidates[Random::get<std::size_t>(
                0, zerusCandidates.size() - 1)];
            (void)zerus.TransformTo(replacement);
        });
    }
    if (season14.HasGeneratedRewardEnhanceAMatic() && !hand.IsFull()) {
        static constexpr std::array<std::string_view, 4> parts = {
            "BG24_Reward_715t", "BG24_Reward_715t2",
            "BG24_Reward_715t3", "BG24_Reward_715t4"};
        const auto card = Cards::FindCardByID(
            parts[Random::get<std::size_t>(0, parts.size() - 1)]);
        if (!card.id.empty()) {
            Spell spell(card);
            // Enhanced Parts are the reward's per-turn Spellcraft-like
            // tokens; remove them at the next recruit start if unused.
            spell.SetTemporary(true);
            hand.Add(CardData{std::move(spell)});
        }
    }
    if (season14.HasGeneratedRewardGoldenHammer() && !hand.IsFull()) {
        const auto card = Cards::FindCardByID("BG24_Reward_719t");
        if (!card.id.empty()) {
            Spell spell(card);
            spell.SetTemporary(true);
            hand.Add(CardData{std::move(spell)});
        }
    }
    if (season14.HasGeneratedRewardEndlessBloodMoon()) AddBloodGems(2);
    if (season14.HasGeneratedRewardEtherealEvidence() &&
        season14.pendingDecision == Season14Decision::NONE)
    {
        // Ethereal Evidence replaces itself with one of two new public
        // rewards.  Build the offerings from the typed executable registry,
        // never from arbitrary card text or hidden cards.json rows.
        std::vector<std::int32_t> candidates;
        std::set<std::int32_t> seen;
        for (const auto& definition : SEASON14_GENERATED_QUEST_REWARDS)
        {
            if (!definition.executable || definition.dbfID == 97485) continue;
            const auto card = Cards::FindCardByDbfID(definition.dbfID);
            // The registry contains one typed row per public card, but some
            // generated families also have metadata rows sharing the same DBF
            // (for example Ghastly Mask/Friends).  An Ethereal offering is a
            // pair of distinct public rewards, never two copies of one DBF.
            if (card.GetCardType() == CardType::BATTLEGROUND_QUEST_REWARD &&
                seen.insert(card.dbfID).second)
                candidates.push_back(card.dbfID);
        }
        Random::shuffle(candidates.begin(), candidates.end());
        if (candidates.size() >= 2)
        {
            season14.BeginOfferingDecision(
                Season14Decision::DISCOVER, 0, 97485,
                {{candidates[0], 0}, {candidates[1], 0}});
        }
    }
    if (season14.HasGeneratedRewardKidnapSack() && !hand.IsFull()) {
        const auto card = Cards::FindCardByID("BG24_Reward_718t");
        if (!card.id.empty()) {
            Spell spell(card);
            spell.SetTemporary(true);
            hand.Add(CardData{std::move(spell)});
        }
    }
    if (season14.HasGeneratedRewardHiddenVault()) {
        // The improvement is committed only after the gold is delivered; a
        // replayed start-turn event therefore cannot duplicate a failed turn.
        remainCoin += season14.generatedRewardHiddenVaultGold;
        season14.generatedRewardHiddenVaultGold =
            std::min<std::int32_t>(10, season14.generatedRewardHiddenVaultGold + 1);
    }
    if (season14.HasGeneratedRewardAlterEgo()) {
        // Alter Ego swaps its parity at each recruit boundary.  The active
        // parity is consumed by PrepareTavern when fresh offers are created.
        season14.generatedRewardAlterEgoEven =
            !season14.generatedRewardAlterEgoEven;
    }
    if (season14.GeneratedRewardFriendsRace() != Race::INVALID) {
        const auto friends =
            SupportedMinionsForRace(season14.GeneratedRewardFriendsRace());
        // Each grant is an independent random draw from the pinned race
        // pool.  A full hand consumes no draw, matching normal generated-card
        // delivery semantics while remaining deterministic under replay.
        if (!friends.empty()) {
            static_cast<void>(AddRandomMinionToHand(*this, friends));
            static_cast<void>(AddRandomMinionToHand(*this, friends));
        }
    }
    if (season14.HasGeneratedRewardGhastlyMask() && !hand.IsFull() &&
        !season14.GhastlyCardDelivered() &&
        season14.GeneratedRewardGhastlyCardDbfID() != 0) {
        const auto card = Cards::FindCardByDbfID(
            season14.GeneratedRewardGhastlyCardDbfID());
        if (card.dbfID != 0) {
            hand.Add(CardData{Minion(card)});
            season14.MarkGhastlyCardDelivered();
        }
    }
    season14.generatedRewardRefreshesThisTurn = 0;
    season14.chromieRefreshesThisTurn = 0;
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

void Player::ResolveGeneratedQuestRewardAfterCombat()
{
    if (season14.HasGeneratedRewardDoppelgangersLocket() &&
        season14.pendingDecision == Season14Decision::NONE && !hand.IsFull()) {
        std::vector<Minion> candidates;
        const auto& lastOpponentCombatMinionDbfIDs =
            season14.lastOpponentCombatMinionDbfIDs;
        for (std::size_t i = 0;; ++i) {
            if (i >= lastOpponentCombatMinionDbfIDs.size()) break;
            auto snapshot = season14.LastOpponentCombatMinionSnapshot(i);
            if (!snapshot.has_value()) break;
            auto minion = std::move(*snapshot);
            const auto card = Cards::FindCardByID(std::string(minion.GetCardID()));
            if (card.GetCardType() != CardType::MINION ||
                !card.isBattlegroundsPoolMinion || !card.hasBehavior ||
                std::any_of(candidates.begin(), candidates.end(),
                            [&minion](const Minion& prior) {
                                return prior.GetDbfID() == minion.GetDbfID();
                            }))
                continue;
            // Locket is explicitly non-Golden but keeps enchantments.  The
            // snapshot carries those instance effects; converting its card
            // identity back to the normal entity preserves them.
            if (card.normalDbfID != 0)
                minion.TransformTo(Cards::FindCardByDbfID(card.normalDbfID));
            candidates.push_back(std::move(minion));
        }
        if (!candidates.empty()) {
            Random::shuffle(candidates.begin(), candidates.end());
            std::vector<Season14Offering> offerings;
            for (std::size_t i = 0; i < std::min<std::size_t>(3, candidates.size()); ++i)
                offerings.push_back({candidates[i].GetDbfID(), 0});
            season14.BeginOfferingDecision(Season14Decision::DISCOVER, 0,
                                           104677, std::move(offerings));
        }
    }
    const bool copyLastDead = season14.HasGeneratedRewardVictimsSpecter() ||
                              season14.HasGeneratedRewardTurbulentTombs();
    if (!copyLastDead || hand.IsFull()) return;
    auto snapshot = season14.CopyLastCombatDeadMinion();
    if (!snapshot.has_value()) return;
    // TakeCombatDeadMinions returns plain card definitions, so combat-only
    // buffs, golden state, and temporary enchantments cannot leak into hand.
    ApplyFreshMinionModifiers(*snapshot);
    hand.Add(CardData{std::move(*snapshot)});
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

void Player::DispatchMinionAttackGain(Minion& target, int amount)
{
    if (amount <= 0 || dispatchingMinionAttackGain) return;
    dispatchingMinionAttackGain = true;
    // Iterate the active zone only; combat copies must not notify recruit
    // listeners, and recruit gains must not leak into combat copies.
    GetField().ForEachAlive([&](MinionData& data) {
        data.value().ActivateTrigger(TriggerType::GAIN_ATTACK, target);
    });
    dispatchingMinionAttackGain = false;
    CheckAzsharaAmbition();
}

void Player::CheckAzsharaAmbition()
{
    if (season14.heroPowerDbfID != 79619 ||
        season14.AzsharaConquestStarted()) return;
    std::int32_t totalAttack = 0;
    recruitField.ForEachAlive([&](const MinionData& data) {
        totalAttack += std::max(0, data.value().GetAttack());
    });
    season14.MaybeBeginNagaConquest(totalAttack);
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
    const auto* batch9 =
        FindSeason14HeroPowerBehaviorBatch9(hero.card.heroPowerDbfID);
    const auto* batch10 =
        FindSeason14HeroPowerBehaviorBatch10(hero.card.heroPowerDbfID);
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
                                                              : (batch9 != nullptr
                                                                     ? batch9->cost
                                                                     : (batch10 != nullptr
                                                                            ? batch10->cost
                                                                            : 0))))));
    season14.SetHeroPower(hero.card.heroPowerDbfID, heroPowerCost,
                          hero.card.heroPowerDbfID != 0);
    if (hero.card.heroPowerDbfID == 119196)
        BeginWarpGateChoice();
    if (hero.card.heroPowerDbfID == 92961)
        BeginWhodunitQuestChoice();
    if (hero.card.heroPowerDbfID == 60450 &&
        season14.pendingDecision == Season14Decision::NONE) {
        std::vector<Card> powers;
        for (const auto& card : Cards::GetAllCards())
            if (card.GetCardType() == CardType::HERO_POWER &&
                card.dbfID > 0 && card.normalDbfID == 0 && card.hasBehavior &&
                card.dbfID != 60450)
                powers.push_back(card);
        if (powers.size() >= 3) {
            Random::shuffle(powers.begin(), powers.end());
            season14.BeginOfferingDecision(Season14Decision::DISCOVER, 0, 60450,
                {{powers[0].dbfID, 0}, {powers[1].dbfID, 0},
                 {powers[2].dbfID, 0}});
        }
    }

    // Menagerist's Amalgam is a one-time starting minion, not a combat
    // summon. Install it on the recruit board at hero selection so it
    // persists through combats and cannot duplicate each round.
    if (hero.card.heroPowerDbfID == 59201 && !recruitField.IsFull()) {
        const Card amalgamCard = Cards::FindCardByDbfID(59202);
        if (!amalgamCard.id.empty()) {
            Minion amalgam(amalgamCard);
            amalgam.SetAmalgamation();
            amalgam.getPlayerCallback = [this]() -> Player& { return *this; };
            if (getNextCardIndexCallback)
                amalgam.SetIndex(getNextCardIndexCallback());
            recruitField.Add(amalgam, recruitField.GetCount());
        }
    }

    // N'Zoth's Fish is a single start-game 2/2 combat companion;
    // its deathrattle payload is copied from each later friendly deathrattle.
    if (hero.card.heroPowerDbfID == 66484 && !recruitField.IsFull()) {
        const Card fishCard = Cards::FindCardByDbfID(67213);
        if (!fishCard.id.empty()) {
            Minion fish(fishCard);
            fish.SetAttack(2);
            fish.SetHealth(2);
            fish.getPlayerCallback = [this]() -> Player& { return *this; };
            if (getNextCardIndexCallback) fish.SetIndex(getNextCardIndexCallback());
            recruitField.Add(fish, recruitField.GetCount());
        }
    }

    // Lift Off's Battlecruiser is a real starting minion, not a hidden hero
    // power payload; retain its entity identity for refresh upgrade routing.
    if (hero.card.heroPowerDbfID == 118681 && !recruitField.IsFull()) {
        const Card cruiserCard = Cards::FindCardByDbfID(118684);
        if (!cruiserCard.id.empty()) {
            Minion cruiser(cruiserCard);
            cruiser.SetAttack(2);
            cruiser.SetHealth(2);
            cruiser.getPlayerCallback = [this]() -> Player& { return *this; };
            if (getNextCardIndexCallback) cruiser.SetIndex(getNextCardIndexCallback());
            season14.liftOffBattlecruiserEntityID = cruiser.GetIndex();
            recruitField.Add(cruiser, recruitField.GetCount());
        }
    }

    // Spawning Pool starts with one authoritative 2/2 Larva on the recruit
    // board. Keep its entity identity stable; morphing is deliberately
    // fail-closed until the Zerg candidate behaviors are registered.
    if (hero.card.heroPowerDbfID == 120362 && !recruitField.IsFull()) {
        const Card larvaCard = Cards::FindCardByDbfID(120359);
        if (!larvaCard.id.empty()) {
            Minion larva(larvaCard);
            larva.SetAttack(2);
            larva.SetHealth(2);
            larva.getPlayerCallback = [this]() -> Player& { return *this; };
            if (getNextCardIndexCallback) larva.SetIndex(getNextCardIndexCallback());
            season14.spawningPoolLarvaEntityID = larva.GetIndex();
            recruitField.Add(larva, recruitField.GetCount());
        }
    }

    if (hero.card.heroPowerDbfID == 104628) {
        std::vector<Card> tierSeven;
        for (const auto& card : Cards::GetAllCards())
            if (card.isBattlegroundsPoolMinion && card.GetCardType() == CardType::MINION &&
                card.normalDbfID == 0 && card.hasBehavior && card.GetTier() == 7)
                tierSeven.push_back(card);
        Random::shuffle(tierSeven.begin(), tierSeven.end());
        std::vector<Season14Offering> offerings;
        for (std::size_t i = 0; i < std::min<std::size_t>(3, tierSeven.size()); ++i)
            offerings.push_back({tierSeven[i].dbfID, 0});
        if (!offerings.empty())
            season14.BeginOfferingDecision(Season14Decision::DISCOVER, 0, 104628,
                                           std::move(offerings));
    }

    if (hero.card.heroPowerDbfID == 79720)
        season14.BeginOfferingDecision(
            Season14Decision::CHOICE, 0, 79720,
            {{79721, 0}, {79722, 0}, {79723, 0}, {79724, 0}});

    if (hero.card.heroPowerDbfID == 81570) {
        season14.expeditionFirstTurnSkipped = true;
        season14.expeditionSkipFirstRecruitTurn = true;
        BeginExpeditionDiscoveryForTier(6);
    }

    if (hero.card.heroPowerDbfID == 76520 &&
        !recruitField.IsFull())
    {
        const auto shredderCard = Cards::FindCardByID("BG21_HERO_030t");
        if (!shredderCard.id.empty())
        {
            Minion shredder(shredderCard);
            if (getNextCardIndexCallback)
                shredder.SetIndex(getNextCardIndexCallback());
            shredder.getPlayerCallback = [this]() -> Player& { return *this; };
            recruitField.Add(shredder, recruitField.GetCount());
        }
    }

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

void Player::MaybeBeginExpeditionDiscovery()
{
    DeliverExpeditionReward();
}

void Player::BeginExpeditionDiscoveryForTier(int tier)
{
    if (season14.heroPowerDbfID != 81570 || season14.pendingDecision != Season14Decision::NONE ||
        !season14.ExpeditionTierPending(tier)) return;
    std::vector<Card> candidates;
    for (const auto& card : Cards::GetAllCards())
        if (card.GetCardType() == CardType::MINION && card.isBattlegroundsPoolMinion &&
            card.normalDbfID == 0 && card.hasBehavior && card.GetTier() == tier)
            candidates.push_back(card);
    if (candidates.size() < 3) return;
    Random::shuffle(candidates.begin(), candidates.end());
    std::vector<Season14Offering> offerings;
    for (int i = 0; i < 3; ++i) offerings.push_back({candidates[i].dbfID, 0});
    season14.BeginOfferingDecision(Season14Decision::DISCOVER, 0, 81570,
                                   std::move(offerings));
}

void Player::DeliverExpeditionReward()
{
    if (season14.heroPowerDbfID != 81570 || hand.IsFull()) return;
    for (const int tier : {6, 4, 2}) {
        const auto dbfID = season14.ExpeditionReward(tier);
        if (dbfID == 0 || currentTier < tier) continue;
        const auto card = Cards::FindCardByDbfID(dbfID);
        if (card.GetCardType() != CardType::MINION) continue;
        Minion reward(card);
        ApplyFreshMinionModifiers(reward);
        hand.Add(CardData{std::move(reward)});
        season14.ClearExpeditionReward(tier);
    }
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

    // Valithria Dreamwalker buffs Dragons in Bob's Tavern, not Dragons in
    // hand/warband and not every fresh Minion instance. Apply the aura only
    // to every current offer. ApplyPersistentMinionStats makes this idempotent
    // for independently frozen cards while still updating an already-visible
    // shop when the Buddy is played mid-turn. The golden Buddy doubles the
    // printed +3/+3, and persistent stats carry correctly when purchased.
    int valithriaBonus = 0;
    recruitField.ForEachAlive([&valithriaBonus](const MinionData& data) {
        const auto& id = data.value().GetCardID();
        if (id == "TB_BaconShop_HERO_53_Buddy") valithriaBonus += 3;
        else if (id == "TB_BaconShop_HERO_53_Buddy_G") valithriaBonus += 6;
    });
    if (valithriaBonus != 0) {
        tavern.fieldZone.ForEachAlive(
            [valithriaBonus](MinionData& data) {
                auto& offer = data.value();
                if (!offer.HasRace(Race::DRAGON)) return;
                offer.ApplyPersistentMinionStats(valithriaBonus, valithriaBonus);
            });
    }

    // Totemic Tavern adds Totems to the active Tavern pool. Replace one
    // eligible offer through the authoritative pool callback; this consumes
    // a real pool entry and returns the displaced entry. If no eligible
    // Totem exists, leave the shop untouched rather than inventing a card.
    if (season14.HasGeneratedRewardTotemicTavern()) {
        std::vector<std::size_t> replaceable;
        tavern.fieldZone.ForEach([&replaceable](MinionData& data) {
            if (!data.value().IsFrozen())
                replaceable.push_back(static_cast<std::size_t>(data.value().GetZonePosition()));
        });
        if (!replaceable.empty() && replaceTavernMinionWithRaceCallback)
            (void)replaceTavernMinionWithRaceCallback(
                *this, tavern,
                replaceable[Random::get<std::size_t>(0, replaceable.size() - 1)],
                Race::TOTEM);
    }

    // Time Twister Chromie's Mana Per Minute replaces the normal Tavern
    // offer with Tavern spells whenever it is refreshed (including the
    // turn-start preparation).  Keep this in the authoritative preparation
    // path so both ordinary refreshes and lifecycle refreshes agree, and do
    // not leave stale minion/spell slots mixed together.
    if (season14.heroPowerDbfID == 126538)
    {
        // `clearTavernMinionsCallback` already returned every unfrozen
        // minion and removed every unfrozen spell.  Preserve frozen entries
        // exactly as the ordinary Tavern path does; this matters when a
        // Mana Per Minute turn ends with FreezeAndEndTurn.
        for (int i = tavern.fieldZone.GetCount() - 1; i >= 0; --i)
        {
            if (!tavern.fieldZone[static_cast<std::size_t>(i)].IsFrozen())
            {
                if (returnMinionCallback)
                    returnMinionCallback(tavern.fieldZone[static_cast<std::size_t>(i)].GetPoolIndex());
                tavern.fieldZone.Remove(tavern.fieldZone[static_cast<std::size_t>(i)]);
            }
        }

        tavern.spellSlots.erase(
            std::remove_if(tavern.spellSlots.begin(), tavern.spellSlots.end(),
                           [](const TavernSlot& slot) { return !slot.IsFrozen(); }),
            tavern.spellSlots.end());
        std::vector<Card> candidates;
        for (const auto& candidate : Cards::GetAllCards())
        {
            if (candidate.isBattlegroundsPoolSpell &&
                candidate.normalDbfID == 0 &&
                FindTavernSpellBehavior(candidate.id).effect !=
                    TavernSpellEffect::NONE)
                candidates.push_back(candidate);
        }
        if (candidates.empty())
            throw std::invalid_argument(
                "Mana Per Minute requires a non-empty supported Tavern spell pool");
        Random::shuffle(candidates.begin(), candidates.end());
        const auto capacity = season14.TavernOfferCount(MAX_FIELD_SIZE);
        const auto count = std::min<std::size_t>(
            capacity > tavern.SlotCount() ? capacity - tavern.SlotCount() : 0,
            candidates.size());
        for (std::size_t i = 0; i < count; ++i)
            tavern.spellSlots.emplace_back(Spell(candidates[i]));
        return;
    }

    const auto batch4 = season14.HeroPowerBatch4PassiveModifiers();
    if (season14.persistentShopAttack != 0 ||
        season14.persistentShopHealth != 0 ||
        !season14.persistentShopRaceStats.empty() ||
        batch4.globalMinionAttack != 0 || batch4.mechShopAttack != 0 ||
        batch4.mechShopHealth != 0 || season14.persistentTavernTierMax != 0 ||
        season14.temporaryRefreshShopAttack != 0 ||
        season14.temporaryRefreshShopHealth != 0 ||
        season14.HasGeneratedRewardAlterEgo())
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
                if (season14.HasGeneratedRewardAlterEgo() &&
                    ((minion.value().GetTier() % 2 == 0) ==
                     season14.generatedRewardAlterEgoEven)) {
                    minion.value().SetAttack(minion.value().GetAttack() + 7);
                    minion.value().SetHealth(minion.value().GetHealth() + 7);
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
    // Turn-start Tavern preparation is also a refresh for Enhancification.
    // Explicit RefreshTavern handles its own post-PrepareTavern hook while
    // refreshInProgress is set, so this branch cannot double-trigger it.
    if (season14.heroPowerDbfID == 96872 && !season14.refreshInProgress)
    {
        const auto hasKeyword = [](const Minion& minion, GameTag tag) {
            if (tag == GameTag::DIVINE_SHIELD) return minion.HasDivineShield();
            if (tag == GameTag::REBORN) return minion.HasReborn();
            if (tag == GameTag::WINDFURY) return minion.HasWindfury();
            if (tag == GameTag::VENOMOUS) return minion.HasVenomous();
            return minion.HasTaunt();
        };
        constexpr GameTag keywords[] = {GameTag::DIVINE_SHIELD,
            GameTag::REBORN, GameTag::WINDFURY, GameTag::VENOMOUS,
            GameTag::TAUNT};
        for (int pass = 0; pass < 2; ++pass) {
            std::vector<Minion*> targets;
            tavern.fieldZone.ForEachAlive([&](MinionData& data) {
                targets.push_back(&data.value());
            });
            if (targets.empty()) break;
            auto& target = *targets[Random::get<std::size_t>(
                0, targets.size() - 1)];
            std::vector<GameTag> eligible;
            for (const auto keyword : keywords)
                if (!hasKeyword(target, keyword)) eligible.push_back(keyword);
            if (eligible.empty()) continue;
            const auto keyword = eligible[Random::get<std::size_t>(
                0, eligible.size() - 1)];
            if (keyword == GameTag::TAUNT) target.SetTaunt(true);
            else if (keyword == GameTag::REBORN) target.SetReborn(true);
            else if (keyword == GameTag::VENOMOUS)
                target.SetGameTag(GameTag::POISONOUS, 1);
            else target.SetGameTag(keyword, 1);
        }
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

bool Player::AddRandomFriendlyMinionCopyToHand()
{
    if (hand.IsFull()) return false;
    std::vector<Minion*> candidates;
    recruitField.ForEachAlive([&candidates](MinionData& data) {
        candidates.push_back(&data.value());
    });
    if (candidates.empty()) return false;
    return AddMinionCopyToHand(*candidates[Random::get<std::size_t>(
        0, candidates.size() - 1)]);
}

bool Player::AddHighestLastOpponentMinionCopyToHand()
{
    if (hand.IsFull()) return false;
    std::vector<Card> candidates;
    int highestTier = 0;
    for (const auto dbfID : season14.lastOpponentCombatMinionDbfIDs)
    {
        auto card = Cards::FindCardByDbfID(dbfID);
        if (card.normalDbfID != 0)
            card = Cards::FindCardByDbfID(card.normalDbfID);
        if (card.GetCardType() != CardType::MINION || !card.hasBehavior ||
            !card.isBattlegroundsPoolMinion)
            continue;
        if (card.GetTier() > highestTier)
        {
            highestTier = card.GetTier();
            candidates.clear();
        }
        if (card.GetTier() == highestTier &&
            std::none_of(candidates.begin(), candidates.end(),
                         [&card](const Card& prior) {
                             return prior.dbfID == card.dbfID;
                         }))
            candidates.push_back(card);
    }
    if (candidates.empty()) return false;
    const auto index = Random::get<std::size_t>(0, candidates.size() - 1);
    Minion plain(candidates[index]);
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

bool Player::BeginFeelDevastationDiscover()
{
    if (season14.pendingDecision != Season14Decision::NONE || hand.IsFull())
        return false;
    std::vector<Card> candidates;
    for (const auto& card : Cards::GetTier5Minions())
        if (card.normalDbfID == 0 && card.hasBehavior) candidates.push_back(card);
    std::vector<Card> gifts;
    for (const auto& gift : Cards::GetAllCards())
        if (gift.isBattlegroundsDarkGift && FindDarkGiftBehavior(gift.id).effect != DarkGiftEffect::NONE)
            gifts.push_back(gift);
    if (candidates.empty() || gifts.empty()) return false;
    // Discover offerings are random, but only draw from the explicitly
    // supported Tier-5 minion and executable Dark Gift pools.
    Random::shuffle(candidates.begin(), candidates.end());
    Random::shuffle(gifts.begin(), gifts.end());
    std::vector<Season14Offering> offerings;
    for (std::size_t i = 0; i < std::min<std::size_t>(3, candidates.size()); ++i) {
        const Minion preview(candidates[i]);
        const auto gift = std::find_if(gifts.begin(), gifts.end(), [&preview](const Card& candidate) {
            return DarkGiftTargetIsLegal(preview, FindDarkGiftBehavior(candidate.id));
        });
        if (gift != gifts.end()) offerings.push_back({candidates[i].dbfID, 0, gift->dbfID});
    }
    if (offerings.empty()) return false;
    season14.BeginOfferingDecision(Season14Decision::DISCOVER, 0, 134010, std::move(offerings));
    return true;
}

void Player::RefreshSpellcraft()
{
    combinedChooseOneUses = 0;
    recruitField.ForEachAlive([&](MinionData& data) {
        if (data.value().GetCardID() == "BG31_327") combinedChooseOneUses = std::max(combinedChooseOneUses, 1);
        else if (data.value().GetCardID() == "BG31_327_G") combinedChooseOneUses = std::max(combinedChooseOneUses, 2);
    });
    malchezaarRefreshesRemaining = 0;
    recruitField.ForEachAlive([&](MinionData& data) {
        const auto& id = data.value().GetCardID();
        if (id == "BG26_524") malchezaarRefreshesRemaining = std::max(malchezaarRefreshesRemaining, 2);
        else if (id == "BG26_524_G") malchezaarRefreshesRemaining = std::max(malchezaarRefreshesRemaining, 4);
    });
    bool hasDarkcrest = false;
    recruitField.ForEachAlive([&](MinionData& data) {
        const auto& id = data.value().GetCardID();
        hasDarkcrest = hasDarkcrest || id == "BG31_920" || id == "BG31_920_G";
    });
    if (hasDarkcrest)
        darkcrestImprovement = std::min(5, darkcrestImprovement + 1);
    else
        darkcrestImprovement = 0;
    recruitField.ForEach([](MinionData& data) {
        data.value().ExpireTemporaryEffects();
        data.value().ResetSpellcraftUses();
        data.value().ResetZestyShakerUse();
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
        { "BG31_924", "BG31_924t", 1 },
        { "BG31_924_G", "BG31_924_Gt", 1 },
        { "BG31_920", "BG31_920t", 1 },
        { "BG31_920_G", "BG31_920_Gt", 1 },
        { "BG26_501", "BG26_501t", 1 },
        { "BG26_501_G", "BG26_501_Gt", 1 },
        { "BG24_Reward_719", "BG24_Reward_719t", 1 },
        { "BG25_044", "BG25_044t", 1 },
        { "BG25_044_G", "BG25_044t", 1 },
        { "BG29_879", "BG29_879t", 1 },
        { "BG29_879_G", "BG29_879t_G", 1 },
        { "BG34_Giant_035", "BG34_Giant_035t", 1 },
        { "BG34_Giant_035_G", "BG34_Giant_035t_G", 1 },
        { "BGS_200", "BG28_810", 1 },
        { "TB_BaconUps_256", "BG33_815", 1 },
        { "BG33_Reward_006", "BG33_Reward_006t", 1 },
        { "BG27_514", "BG27_514t", 1 },
        { "BG27_514_G", "BG27_514t_G", 1 },
        { "BG30_MagicItem_714", "BG30_MagicItem_714t", 1 },
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
        if (std::string_view(spec.minion) == "BG31_920" ||
            std::string_view(spec.minion) == "BG31_920_G")
            spell.SetDynamicTier(1 + darkcrestImprovement);
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
    // Demon Hunter Training makes only the first successful purchase of the
    // turn free once fourteen friendly attacks have occurred.  Determine the
    // entitlement before charging, but consume it only after the purchase
    // actually reaches hand (a full hand must not burn the entitlement).
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
    const auto purchasedDbfID = tavern.fieldZone[idx].GetDbfID();
    purchaseMinionCallback(*this, idx);

    if (hand.GetCount() > handCountBeforePurchase)
    {
        if (season14.heroPowerDbfID == 60218) {
            auto& bought = std::get<Minion>(hand[hand.GetCount() - 1]);
            if (bought.HasBattlecry() && !season14.battlecryRewardGiven &&
                ++season14.battlecryRewardBuys >= 5) {
                const Card brann = Cards::FindCardByID("BG_LOE_077");
                if (!brann.id.empty() && !hand.IsFull()) {
                    Minion generated(brann);
                    ApplyFreshMinionModifiers(generated);
                    hand.Add(CardData{std::move(generated)});
                    season14.battlecryRewardGiven = true;
                }
            }
        }
        if (battlecryDiscount) ++season14.battlecryBuysThisTurn;
        auto& purchased = std::get<Minion>(hand[hand.GetCount() - 1]);
        ApplyFreshMinionModifiers(purchased);
        // Shaman Prayer Beads counts only successful Battlecry purchases and
        // resolves each Trinket instance independently.  Keep the counter on
        // the persistent effect so two copies cannot share progress.
        if (purchased.HasBattlecry()) {
            for (auto& trinket : season14.trinkets) {
                if (!trinket.active || trinket.remainingUses == 0) continue;
                const auto behavior = FindTrinketBehavior(
                    Cards::FindCardByDbfID(trinket.dbfID).id);
                if (behavior.effect != TrinketEffect::AFTER_BUY_BATTLECRY_MINION)
                    continue;
                if (++trinket.triggerProgress < behavior.value)
                    continue;
                // The printed counter is a repeating cadence.  Reset before
                // attempting the hand grant so a full hand cannot leave the
                // Trinket permanently over threshold; the random pool is
                // restricted to Battlecry minions by the task itself.
                trinket.triggerProgress = 0;
                if (hand.IsFull()) continue;
                (void)SimpleTasks::RandomCardToHandTask{
                    Race::INVALID, 0, behavior.amount, false, true}.Run(*this);
            }
        }
        if (season14.HasGeneratedRewardCookedBook()) {
            purchased.SetAttack(purchased.GetAttack() +
                                season14.generatedRewardCookedBookBonus);
            purchased.SetHealth(purchased.GetHealth() +
                                season14.generatedRewardCookedBookBonus);
            ++season14.generatedRewardCookedBookBonus;
        }
        const auto attack = season14.OnBuyMinionBatch4();
        purchased.SetAttack(purchased.GetAttack() + attack);
        // Dranosh Saurfang's Buddy gains half of the purchased minion's
        // current stats.  Resolve this after every purchase-side stat aura
        // (including For the Horde!) has been applied, and mutate the Buddy,
        // never the purchased minion.  Golden Dranosh gains the full stats.
        recruitField.ForEachAlive([&purchased](MinionData& data) {
            auto& buddy = data.value();
            int multiplier = 0;
            if (buddy.GetCardID() == "BG20_HERO_102_Buddy") multiplier = 1;
            else if (buddy.GetCardID() == "BG20_HERO_102_Buddy_G") multiplier = 2;
            if (multiplier == 0) return;
            buddy.SetAttack(buddy.GetAttack() +
                            (purchased.GetAttack() * multiplier) / 2);
            buddy.SetHealth(buddy.GetHealth() +
                            (purchased.GetHealth() * multiplier) / 2);
        });
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
    if (season14.heroPowerDbfID == 119196 &&
        season14.warpGateSelectedDbfID != 0 &&
        season14.warpGateRewardDbfID == 0 && ++season14.warpGateBuyCount >= 14)
        season14.warpGateRewardDbfID = season14.warpGateSelectedDbfID;
    TryResolveWarpGateReward();
    if (hand.GetCount() > handCountBeforePurchase)
    {
        auto& purchased = std::get<Minion>(hand[hand.GetCount() - 1]);
        recruitField.ForEachAlive([&purchased](MinionData& data) {
            data.value().ActivateTrigger(TriggerType::BUY_MINION, purchased);
        });
        // Enhance-o Medico counts the purchased minion's Bonus Keywords at
        // the buy boundary. Resolve normal/golden copies independently so
        // multiple Medicos stack while each remains an event observer.
        int bonusKeywords = 0;
        bonusKeywords += purchased.HasTaunt() ? 1 : 0;
        bonusKeywords += purchased.HasDivineShield() ? 1 : 0;
        bonusKeywords += purchased.HasReborn() ? 1 : 0;
        bonusKeywords += purchased.HasWindfury() ? 1 : 0;
        bonusKeywords += purchased.HasVenomous() ? 1 : 0;
        bonusKeywords += purchased.HasStealth() ? 1 : 0;
        if (bonusKeywords > 0) {
            recruitField.ForEachAlive([bonusKeywords](MinionData& data) {
                auto& medico = data.value();
                const auto& id = medico.GetCardID();
                if (id == "BG24_HERO_204_Buddy") {
                    medico.SetAttack(medico.GetAttack() + 3 * bonusKeywords);
                    medico.SetHealth(medico.GetHealth() + 3 * bonusKeywords);
                } else if (id == "BG24_HERO_204_Buddy_G") {
                    medico.SetAttack(medico.GetAttack() + 6 * bonusKeywords);
                    medico.SetHealth(medico.GetHealth() + 6 * bonusKeywords);
                }
            });
        }
        if (season14.HasGeneratedRewardInvigoratingConch()) {
            std::vector<Minion*> targets;
            recruitField.ForEachAlive([&targets](MinionData& data) {
                targets.push_back(&data.value());
            });
            if (!targets.empty()) {
                auto& target = *targets[Random::get<std::size_t>(
                    0, targets.size() - 1)];
                target.SetAttack(target.GetAttack() + purchased.GetAttack());
                target.SetHealth(target.GetHealth() + purchased.GetHealth());
            }
        }
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
    if (season14.heroPowerDbfID == 104875 &&
        hand.GetCount() > handCountBeforePurchase)
    {
        const auto& purchased = std::get<Minion>(hand[hand.GetCount() - 1]);
        RecordSeason14HeroPowerBatch5GlaivePurchase(
            purchased.GetDbfID(), season14.heroPowerBatch5);
        if (Season14HeroPowerBatch5GlaiveReady(season14.heroPowerBatch5) &&
            !hand.IsFull())
        {
            const auto& ids = season14.heroPowerBatch5.glaivePurchaseDbfIDs;
            const auto sourceID = ids[Random::get<std::size_t>(0, ids.size() - 1)];
            const auto source = Cards::FindCardByDbfID(sourceID);
            if (source.GetCardType() == CardType::MINION)
            {
                Minion copy(source);
                if (getNextCardIndexCallback)
                    copy.SetIndex(getNextCardIndexCallback());
                hand.Add(CardData{copy});
                ConsumeSeason14HeroPowerBatch5Glaive(
                    season14.heroPowerBatch5);
            }
        }
    }
    // Double-Headed Reward listens to the first successful card purchase,
    // including minions (not merely Tavern spells).  The purchased entity
    // may have acquired dynamic state and other buy triggers above, so make
    // a fresh canonical copy only after those observers have run.  The
    // entitlement is consumed by the purchase even when the extra copy burns
    // because the hand became full.
    if (hand.GetCount() > handCountBeforePurchase &&
        season14.HasGeneratedRewardDoubleHeaded() &&
        !season14.generatedRewardDoubleHeadedUsedThisTurn) {
        season14.generatedRewardDoubleHeadedUsedThisTurn = true;
        if (!hand.IsFull()) {
            const auto card = Cards::FindCardByDbfID(purchasedDbfID);
            if (card.dbfID != 0) {
                Minion copy(card);
                ApplyFreshMinionModifiers(copy);
                hand.Add(CardData{std::move(copy)});
            }
        }
    }
    ResolveDoubleTimeCopies();
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

bool Player::StealAllTavernMinionsToHand()
{
    if (season14.heroPowerDbfID != 86292 || hand.IsFull() ||
        tavern.fieldZone.GetCount() == 0)
        return false;

    // Remove from the back so every original slot is visited exactly once.
    // Hand-zone acquisition follows the simulator's normal overflow contract:
    // steal until the hand is full, leaving untransferred offers in Bob's
    // Tavern.  Do not return pool ownership for cards actually stolen.
    bool stolen = false;
    for (int slot = tavern.fieldZone.GetCount() - 1;
         slot >= 0 && !hand.IsFull(); --slot)
    {
        auto& offer = tavern.fieldZone[static_cast<std::size_t>(slot)];
        if (offer.IsDestroyed()) continue;
        Minion minion = tavern.fieldZone.Remove(offer);
        minion.SetFrozen(false);
        hand.Add(CardData{std::move(minion)});
        stolen = true;
    }
    return stolen;
}

bool Player::ResolveRapidReanimationStartCombat()
{
    if (season14.heroPowerDbfID != 98728 ||
        season14.rapidReanimationTargetEntityID == 0)
        return false;
    int targetSlot = -1;
    for (int i = 0; i < recruitField.GetCount(); ++i)
        if (!recruitField[static_cast<std::size_t>(i)].IsDestroyed() &&
            static_cast<std::uint64_t>(recruitField[static_cast<std::size_t>(i)].GetIndex()) ==
                season14.rapidReanimationTargetEntityID) {
            targetSlot = i;
            break;
        }
    if (targetSlot < 0 || !season14.rapidReanimationSnapshot.has_value())
        return false;
    Minion snapshot = *season14.rapidReanimationSnapshot;
    season14.rapidReanimationTargetSlot = targetSlot;
    recruitField.Remove(recruitField[static_cast<std::size_t>(targetSlot)]);
    // Rapid Reanimation's destroy is a real destroy: resolve the selected
    // instance's Deathrattle before the exact snapshot is summoned back.
    if (snapshot.HasDeathrattle()) {
        ++season14.deathrattlesTriggered;
        snapshot.ActivateTask(PowerType::DEATHRATTLE, *this);
    }
    // Retain the exact snapshot until a slot is available.  A Deathrattle
    // may fill the just-opened slot, so resurrection is an observer-driven
    // operation rather than an unconditional Add.
    return TryResolveRapidReanimationIfSpace(recruitField);
}

bool Player::TryResolveRapidReanimationIfSpace(FieldZone& field)
{
    if (!season14.rapidReanimationArmed ||
        !season14.rapidReanimationSnapshot.has_value() ||
        field.IsFull())
        return false;
    Minion snapshot = std::move(*season14.rapidReanimationSnapshot);
    season14.rapidReanimationSnapshot.reset();
    const int slot = season14.rapidReanimationTargetSlot;
    season14.rapidReanimationArmed = false;
    season14.rapidReanimationTargetEntityID = 0;
    season14.rapidReanimationTargetSlot = -1;
    if (getNextCardIndexCallback) snapshot.SetIndex(getNextCardIndexCallback());
    snapshot.SetFrozen(false);
    field.Add(snapshot, std::clamp(slot, 0, field.GetCount()));
    return true;
}

bool Player::BeginFantasticTreasureOffer()
{
    if (season14.heroPowerDbfID != 113311 || season14.recruitTurnNumber != 5 ||
        season14.pendingDecision != Season14Decision::NONE)
        return false;
    std::vector<Card> candidates;
    for (const auto& card : Cards::GetAllCards())
        if (card.trinketType == "LESSER_TRINKET" && card.normalDbfID == 0 &&
            card.dbfID > 0 && card.GetCardType() == CardType::BATTLEGROUND_TRINKET)
            candidates.push_back(card);
    if (candidates.size() < 4) return false;
    Random::shuffle(candidates.begin(), candidates.end());
    std::vector<Season14Offering> offerings;
    for (std::size_t i = 0; i < 4; ++i)
        offerings.push_back({candidates[i].dbfID, 2});
    season14.BeginOfferingDecision(Season14Decision::TRINKET_SELECTION, 0,
                                   113311, std::move(offerings));
    return true;
}

bool Player::BeginWarpGateChoice()
{
    if (season14.heroPowerDbfID != 119196 || season14.warpGateSelectedDbfID != 0)
        return false;
    std::vector<Card> candidates;
    for (const auto& card : Cards::GetAllCards())
        // Warp Gate is a hero-generated source, so candidates need not be in
        // the ordinary Tavern pool. The explicit DBF allowlist is the tribe
        // boundary; unsupported card behavior remains fail-closed at reward
        // construction rather than silently substituting another tribe.
        if (IsWarpGateProtossDbfID(card.dbfID) &&
            card.GetCardType() == CardType::MINION && card.normalDbfID == 0 &&
            card.hasBehavior)
            candidates.push_back(card);
    if (candidates.size() < 2) return false;
    Random::shuffle(candidates.begin(), candidates.end());
    season14.BeginOfferingDecision(Season14Decision::CHOICE, 0, 119196,
        {{candidates[0].dbfID, 0}, {candidates[1].dbfID, 0}});
    return true;
}

bool Player::BeginWhodunitQuestChoice()
{
    if (season14.heroPowerDbfID != 92961 ||
        season14.pendingDecision != Season14Decision::NONE)
        return false;
    std::vector<std::int32_t> quests;
    for (const auto dbfID : WHODUNIT_QUEST_DBF_IDS) {
        const auto quest = Cards::FindCardByDbfID(dbfID);
        // RosettaStone's card schema stores legacy Battlegrounds Quest
        // tokens as SPELL cards with a QUEST mechanic; the pinned DBF pool is
        // the authoritative type guard here.
        if (quest.dbfID != 0 && quest.GetCardType() == CardType::SPELL)
            quests.push_back(dbfID);
    }
    if (quests.size() < 2) return false;
    Random::shuffle(quests.begin(), quests.end());
    season14.BeginOfferingDecision(Season14Decision::CHOICE, 0, 92961,
        {{quests[0], 0}, {quests[1], 0}});
    return true;
}

bool Player::BeginSpawningPoolMorphChoice()
{
    if (season14.heroPowerDbfID != 120362 ||
        !season14.spawningPoolUnlocked ||
        season14.spawningPoolLarvaEntityID == 0 ||
        season14.pendingDecision != Season14Decision::NONE)
        return false;
    bool larvaPresent = false;
    recruitField.ForEachAlive([&](MinionData& data) {
        larvaPresent = larvaPresent ||
            static_cast<std::uint64_t>(data.value().GetIndex()) ==
                season14.spawningPoolLarvaEntityID;
    });
    if (!larvaPresent) return false;
    std::vector<Card> candidates;
    for (const auto& id : SPAWNING_POOL_ZERG_DBF_IDS) {
        const auto card = Cards::FindCardByDbfID(id);
        // The pinned pool is tiered Zerg minions (T2 through T10).  Reject
        // malformed/generated entries with no tavern tier even if a behavior
        // definition happens to exist for them.
        if (card.dbfID != 0 &&
            card.GetTier() == SpawningPoolZergTier(card.dbfID) &&
            card.GetTier() <= SPAWNING_POOL_UNLOCK_TIER &&
            card.hasBehavior &&
            IsExecutableSpawningPoolZergDbfID(card.dbfID))
            candidates.push_back(card);
    }
    if (candidates.size() < 2) return false;
    Random::shuffle(candidates.begin(), candidates.end());
    std::vector<Season14Offering> offerings;
    for (std::size_t i = 0; i < std::min<std::size_t>(3, candidates.size()); ++i)
        offerings.push_back({candidates[i].dbfID, 0});
    season14.BeginOfferingDecision(Season14Decision::DISCOVER,
        season14.spawningPoolLarvaEntityID, 120359, std::move(offerings));
    return true;
}

bool Player::TryResolveWarpGateReward()
{
    if (season14.heroPowerDbfID != 119196 || season14.warpGateRewardDbfID == 0 ||
        hand.IsFull()) return false;
    const auto card = Cards::FindCardByDbfID(season14.warpGateRewardDbfID);
    if (card.dbfID == 0 || card.GetCardType() != CardType::MINION) return false;
    Minion reward(card);
    ApplyFreshMinionModifiers(reward);
    hand.Add(CardData{std::move(reward)});
    season14.warpGateRewardDbfID = 0;
    return true;
}

bool Player::ArmLockAndLoad(std::size_t idx)
{
    if (season14.heroPowerDbfID != 123150 || idx >= static_cast<std::size_t>(tavern.fieldZone.GetCount()) ||
        tavern.fieldZone[idx].IsDestroyed()) return false;
    season14.lockAndLoadProjectile = tavern.fieldZone.Remove(tavern.fieldZone[idx]);
    return true;
}

void Player::ResolveLockAndLoad()
{
    if (season14.heroPowerDbfID != 123150 || !season14.lockAndLoadProjectile || battleField.IsFull()) return;
    auto projectile = std::move(*season14.lockAndLoadProjectile);
    season14.lockAndLoadProjectile.reset();
    SummonCombatSnapshot(std::move(projectile));
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
    if (idx >= tavern.SlotCount()) return false;
    if (idx < static_cast<std::size_t>(tavern.fieldZone.GetCount()))
    {
        if (hand.IsFull()) return false;
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
    if (hand.IsFull()) return false;
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
    const bool splittingScrollCopy = season14.HasGeneratedRewardSplittingScroll() &&
                                     cost >= 3;
    tavern.spellSlots.erase(tavern.spellSlots.begin() + static_cast<std::ptrdiff_t>(idx - minionCount));
    hand.Add(CardData{spell});
    if (splittingScrollCopy && !hand.IsFull()) {
        // Splitting Scroll copies the purchased Tavern spell as a plain card.
        hand.Add(CardData{Spell(Cards::FindCardByDbfID(spell.GetDbfID()))});
    }
    lastBoughtTavernSpellID = spell.GetID();
    remainCoin -= cost;
    RecordGoldSpent(cost);
    if (season14.heroPowerDbfID == 119196 &&
        season14.warpGateSelectedDbfID != 0 &&
        season14.warpGateRewardDbfID == 0 && ++season14.warpGateBuyCount >= 14)
        season14.warpGateRewardDbfID = season14.warpGateSelectedDbfID;
    TryResolveWarpGateReward();
    // Celestial Archive watches successful zero-cost Tavern-spell purchases.
    // The copy is made only after payment/hand insertion succeeds, and each
    // golden Buddy contributes its printed two-copy fan-out independently.
    if (cost == 0 && !hand.IsFull()) {
        int archiveCopies = 0;
        recruitField.ForEachAlive([&archiveCopies](const MinionData& data) {
            const auto& id = data.value().GetCardID();
            if (id == "BG31_HERO_006_Buddy") ++archiveCopies;
            else if (id == "BG31_HERO_006_Buddy_G") archiveCopies += 2;
        });
        for (int copy = 0; copy < archiveCopies && !hand.IsFull(); ++copy)
            hand.Add(CardData{Spell(spell)});
    }
    recruitField.ForEachAlive([this](MinionData& data) {
        auto& minion = data.value();
        minion.ActivateTrigger(TriggerType::BUY_TAVERN_SPELL, minion);
    });
    lastBoughtTavernSpellID.clear();
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
        std::array<bool, RACES_IN_BATTLEGROUNDS.size()> mapUnknownHadRace{};
        if (season14.HasGeneratedRewardMapUnknown()) {
            for (std::size_t raceIdx = 0; raceIdx < RACES_IN_BATTLEGROUNDS.size(); ++raceIdx)
                recruitField.ForEachAlive([&](MinionData& data) {
                    mapUnknownHadRace[raceIdx] = mapUnknownHadRace[raceIdx] ||
                        data.value().HasRace(RACES_IN_BATTLEGROUNDS[raceIdx]);
                });
        }
        const bool clunkerJunker = handMinion.GetCardID() == "BG29_503" ||
                                   handMinion.GetCardID() == "BG29_503_G";
        if (clunkerJunker &&
            (targetIdx < 0 || targetIdx >= recruitField.GetCount() ||
             recruitField[static_cast<std::size_t>(targetIdx)].IsDestroyed() ||
             !recruitField[static_cast<std::size_t>(targetIdx)].HasRace(
                 Race::MECHANICAL)))
            return;
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
            TryDeliverChampionReward();
            TryDeliverHeroicInspirationReward();
            attachment.MagnetizeOnto(
                recruitField[static_cast<std::size_t>(targetIdx)]);
            ApplyMechagnomeInterpreterBonus(
                recruitField[static_cast<std::size_t>(targetIdx)]);
            if (attachment.GetCardID() == "BG31_HERO_802_Buddy" ||
                attachment.GetCardID() == "BG31_HERO_802_Buddy_G")
                recruitField[static_cast<std::size_t>(targetIdx)].MakeGolden();
            ApplyFirstMinionDivineShield(
                recruitField[static_cast<std::size_t>(targetIdx)]);
            season14.generatedRewardSinfallTier = attachment.GetTier();
            season14.generatedRewardSinfallSourceEntityID =
                static_cast<std::uint64_t>(
                    recruitField[static_cast<std::size_t>(targetIdx)].GetIndex());
            ApplyAfterPlayCardTrinkets(attachment.GetRace(), true);
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
        TryDeliverChampionReward();
        TryDeliverHeroicInspirationReward();
        auto minion = std::get<Minion>(card);
        if (minion.IsGolden()) ++season14.goldenMinionsPlayed;
        if (minion.GetRace() == Race::PIRATE)
            ++season14.piratesPlayedThisGame;
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
            // Valithria's aura is active as soon as the Buddy enters the
            // warband, so existing Dragon offers receive it immediately;
            // this also covers playing the Buddy after the shop was filled.
            if (minion.GetCardID() == "TB_BaconShop_HERO_53_Buddy" ||
                minion.GetCardID() == "TB_BaconShop_HERO_53_Buddy_G") {
                const int bonus = minion.GetCardID().ends_with("_G") ? 6 : 3;
                tavern.fieldZone.ForEachAlive([bonus](MinionData& data) {
                    if (data.value().HasRace(Race::DRAGON))
                        data.value().ApplyPersistentMinionStats(bonus, bonus);
                });
            }
            ApplyFirstMinionDivineShield(
                recruitField[static_cast<std::size_t>(minion.GetZonePosition())]);

            recruitField.ForEachAlive([&minion](MinionData& aliveMinion) {
                aliveMinion.value().ActivateTrigger(TriggerType::SUMMON,
                                                    minion);
            });
            ApplyMechagnomeInterpreterBonus(
                recruitField[static_cast<std::size_t>(minion.GetZonePosition())]);
            ApplySummonTrinkets(recruitField[static_cast<std::size_t>(minion.GetZonePosition())]);
            season14.generatedRewardSinfallTier = minion.GetTier();
            season14.generatedRewardSinfallSourceEntityID =
                static_cast<std::uint64_t>(minion.GetIndex());
            ApplyAfterPlayCardTrinkets(minion.GetRace());

            minion.ActivateTask(PowerType::POWER, *this);
            if (minion.GetCardID() == "TB_BaconShop_HERO_28_Buddy" ||
                minion.GetCardID() == "TB_BaconShop_HERO_28_Buddy_G")
                BeginClockworkAssistantDiscover(minion.IsGolden());
            if (minion.GetCardID() == "TB_BaconShop_HERO_55_Buddy" ||
                minion.GetCardID() == "TB_BaconShop_HERO_55_Buddy_G")
                ResolveSparkfinSoothsayer(minion.IsGolden());
            if (minion.GetCardID() == "TB_BaconShop_HERO_02_Buddy" ||
                minion.GetCardID() == "TB_BaconShop_HERO_02_Buddy_G") {
                const int replacements = tavern.fieldZone.GetCount();
                const int passes = minion.GetCardID().ends_with("_G") ? 2 : 1;
                for (int pass = 0; pass < passes && replacements > 0; ++pass) {
                    season14.ArmHigherTierRefresh(replacements);
                    RefreshTavern(true);
                }
            }
            // Baby N'Zoth's Battlecry is intentionally handled at the player
            // boundary: its normal form targets one friendly Deathrattle,
            // while the golden form makes every friendly Deathrattle minion
            // golden.  The generated buddy CardDef is empty because this
            // distinction cannot be represented by a fixed task target.
            if (minion.GetCardID() == "TB_BaconShop_HERO_93_Buddy_G") {
                recruitField.ForEachAlive([](MinionData& data) {
                    if (data.value().HasDeathrattle())
                        data.value().MakeGolden();
                });
            }
            int brannRepeats = 0;
            recruitField.ForEachAlive([&brannRepeats](MinionData& data) {
                if (data.value().GetCardID() == "BG_LOE_077") ++brannRepeats;
                else if (data.value().GetCardID() == "BG_LOE_077_G") brannRepeats += 2;
            });
            for (int i = 0; i < brannRepeats; ++i)
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
            if (minion.HasBattlecry() &&
                season14.HasGeneratedRewardBattlecryRepeat())
                minion.ActivateTask(PowerType::POWER, *this);
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
            ApplyMechagnomeInterpreterBonus(
                recruitField[static_cast<std::size_t>(minion.GetZonePosition())]);
            ApplySummonTrinkets(recruitField[static_cast<std::size_t>(minion.GetZonePosition())]);
            season14.generatedRewardSinfallTier = minion.GetTier();
            season14.generatedRewardSinfallSourceEntityID =
                static_cast<std::uint64_t>(minion.GetIndex());
            ApplyAfterPlayCardTrinkets(minion.GetRace());

            minion.ActivateTask(PowerType::POWER, *this, target);
            if (minion.GetCardID() == "TB_BaconShop_HERO_28_Buddy" ||
                minion.GetCardID() == "TB_BaconShop_HERO_28_Buddy_G")
                BeginClockworkAssistantDiscover(minion.IsGolden());
            if (minion.GetCardID() == "TB_BaconShop_HERO_55_Buddy" ||
                minion.GetCardID() == "TB_BaconShop_HERO_55_Buddy_G")
                ResolveSparkfinSoothsayer(minion.IsGolden());
            if (minion.GetCardID() == "TB_BaconShop_HERO_02_Buddy" ||
                minion.GetCardID() == "TB_BaconShop_HERO_02_Buddy_G") {
                const int replacements = tavern.fieldZone.GetCount();
                const int passes = minion.GetCardID().ends_with("_G") ? 2 : 1;
                for (int pass = 0; pass < passes && replacements > 0; ++pass) {
                    season14.ArmHigherTierRefresh(replacements);
                    RefreshTavern(true);
                }
            }
            if (minion.GetCardID() == "TB_BaconShop_HERO_93_Buddy" &&
                target.HasDeathrattle())
                target.MakeGolden();
            // Hackerfin's printed Battlecry scales its grant by each
            // distinct bonus keyword represented by the warband.  Resolve
            // this after the minion enters play so the source itself is part
            // of the keyword set, while excluding it from recipients.
            if (minion.GetCardID() == "BG31_148" ||
                minion.GetCardID() == "BG31_148_G")
            {
                bool keywords[6] = {};
                recruitField.ForEachAlive([&keywords](MinionData& data) {
                    const auto& candidate = data.value();
                    keywords[0] = keywords[0] || candidate.HasTaunt();
                    keywords[1] = keywords[1] || candidate.HasDivineShield();
                    keywords[2] = keywords[2] || candidate.HasReborn();
                    keywords[3] = keywords[3] || candidate.HasWindfury();
                    keywords[4] = keywords[4] || candidate.HasVenomous();
                    keywords[5] = keywords[5] || candidate.HasStealth();
                });
                int distinctKeywords = 0;
                for (const bool present : keywords)
                    if (present) ++distinctKeywords;
                const int multiplier = minion.IsGolden() ? 2 : 1;
                recruitField.ForEachAlive([distinctKeywords, multiplier,
                                           &minion](MinionData& data) {
                    if (&data.value() == &minion) return;
                    auto& recipient = data.value();
                    recipient.SetAttack(recipient.GetAttack() +
                                       multiplier * (1 + distinctKeywords));
                    recipient.SetHealth(recipient.GetHealth() +
                                       multiplier * 2 * (1 + distinctKeywords));
                });
            }
            int brannRepeats = 0;
            recruitField.ForEachAlive([&brannRepeats](MinionData& data) {
                if (data.value().GetCardID() == "BG_LOE_077") ++brannRepeats;
                else if (data.value().GetCardID() == "BG_LOE_077_G") brannRepeats += 2;
            });
            for (int i = 0; i < brannRepeats; ++i)
                minion.ActivateTask(PowerType::POWER, *this, target);
            if (minion.HasBattlecry() && season14.ConsumeGeneratedRewardConch())
            {
                minion.ActivateTask(PowerType::POWER, *this, target);
                minion.ActivateTask(PowerType::POWER, *this, target);
            }
            if (minion.HasBattlecry() &&
                season14.HasGeneratedRewardBattlecryRepeat())
                minion.ActivateTask(PowerType::POWER, *this, target);
            if (minion.GetRace() == Race::DRAGON &&
                ShouldDuplicateDragonBattlecry())
                // Targeted Battlecries receive the identical target again.
                minion.ActivateTask(PowerType::POWER, *this, target);
        }

        // Sklibb's printed trigger is a free *next refresh*, not an extra
        // Tavern offer.  Resolve it only after the minion has committed so a
        // failed/stale play cannot arm a refresh, and count golden copies as
        // two independent one-shot allowances.
        int sklibbRefreshes = 0;
        recruitField.ForEachAlive([&sklibbRefreshes](const MinionData& data) {
            const auto& id = data.value().GetCardID();
            if (id == "TB_BaconShop_HERO_59_Buddy") ++sklibbRefreshes;
            else if (id == "TB_BaconShop_HERO_59_Buddy_G") sklibbRefreshes += 2;
        });
        if (sklibbRefreshes > 0)
            season14.AddFreeRefreshes(sklibbRefreshes);

        // Ticket Collector is a Battlecry.  Keep the source entity in the
        // modal so replay validation cannot turn an arbitrary sale into a
        // Prize Discover.
        if (minion.GetCardID() == "TB_BaconShop_HERO_94_Buddy" ||
            minion.GetCardID() == "TB_BaconShop_HERO_94_Buddy_G")
            BeginTicketCollectorDiscover(minion.IsGolden());

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
            minion.GetCardID() == "BG36_332_G" ||
            minion.GetCardID() == "BG36_341" ||
            minion.GetCardID() == "BG36_341_G" ||
            minion.GetCardID() == "BG31_320" ||
            minion.GetCardID() == "BG31_320_G" ||
            minion.GetCardID() == "BG32_237" || minion.GetCardID() == "BG32_237_G")
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
                minion.GetCardID() == "BG32_237_G" ? "BG32_237_Gt" :
                minion.GetCardID() == "BG32_237" ? "BG32_237t" :
                minion.GetCardID() == "BG31_320_G" ? "BG31_320_Gt" :
                minion.GetCardID() == "BG31_320" ? "BG31_320t" :
                minion.GetCardID() == "BG27_084_G" ? "BG27_084_Gt" :
                minion.GetCardID() == "BG30_123_G" ? "BG30_123_Gt" :
                minion.GetCardID() == "BG36_330_G" ? "BG36_330_Gt" :
                minion.GetCardID() == "BG30_123" ? "BG30_123t" :
                minion.GetCardID() == "BG36_330" ? "BG36_330t" :
                minion.GetCardID() == "BG36_332" ? "BG36_332" :
                minion.GetCardID() == "BG36_341_G" ? "BG36_341_Gt" :
                minion.GetCardID() == "BG36_341" ? "BG36_341t" : "BG27_084t");
            const auto option1 = Cards::FindCardByID(
                minion.GetCardID() == "BG32_237_G" ? "BG32_237_Gt2" :
                minion.GetCardID() == "BG32_237" ? "BG32_237t2" :
                minion.GetCardID() == "BG31_320_G" ? "BG31_320_Gt2" :
                minion.GetCardID() == "BG31_320" ? "BG31_320t2" :
                minion.GetCardID() == "BG27_084_G" ? "BG27_084_Gt2" :
                minion.GetCardID() == "BG30_123_G" ? "BG30_123_Gt2" :
                minion.GetCardID() == "BG36_330_G" ? "BG36_330_Gt2" :
                minion.GetCardID() == "BG30_123" ? "BG30_123t2" :
                minion.GetCardID() == "BG36_330" ? "BG36_330t2" :
                minion.GetCardID() == "BG36_332" ? "BG36_332_G" :
                minion.GetCardID() == "BG36_341_G" ? "BG36_341_Gt2" :
                minion.GetCardID() == "BG36_341" ? "BG36_341t2" : "BG27_084t2");
            // A target-dependent modal is only exposed when both generated
            // options and at least one Beast target are available; otherwise
            // do not leave the recruit phase permanently locked.
            const bool targetless = minion.GetCardID() != "BG27_084" &&
                                    minion.GetCardID() != "BG27_084_G";
            if (targetless)
                targetMask = 0;
            if (combinedChooseOneUses > 0 && targetless) {
                minion.SetCombinedChooseOne(true);
                --combinedChooseOneUses;
            }
            if (minion.HasCombinedChooseOne() && targetless) {
                const bool golden = minion.GetCardID().ends_with("_G");
                if (minion.GetCardID().starts_with("BG31_320")) {
                    AddBloodGems(golden ? 4 : 2);
                } else if (minion.GetCardID().starts_with("BG30_123")) {
                    season14.AddBloodGemBonus(golden ? 2 : 1, golden ? 2 : 1);
                    AddBloodGems(golden ? 8 : 4);
                } else if (minion.GetCardID().starts_with("BG36_330")) {
                    season14.AddFreeRefreshes(golden ? 4 : 2);
                    AddBloodGems(golden ? 6 : 3);
                } else if (minion.GetCardID().starts_with("BG36_341")) {
                    recruitField.ForEachAlive([this, golden](MinionData& data) {
                        for (int i = 0; i < (golden ? 6 : 3); ++i)
                            ApplyBloodGemTo(data.value());
                    });
                    for (int cast = 0; cast < (golden ? 6 : 3); ++cast) {
                        std::vector<int> candidates;
                        recruitField.ForEachAlive([&candidates](MinionData& data) {
                            candidates.push_back(data.value().GetZonePosition());
                        });
                        Random::shuffle(candidates.begin(), candidates.end());
                        const int count = std::min<int>(3, candidates.size());
                        for (int i = 0; i < count; ++i)
                            ApplyBloodGemTo(recruitField[static_cast<std::size_t>(candidates[i])]);
                    }
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

        // Map of the Unknown snapshots the controlled races before the play.
        // Only a newly introduced race qualifies; then one friendly minion of
        // each controlled race receives the printed +2/+2 payload.
        if (season14.HasGeneratedRewardMapUnknown()) {
            bool introducedRace = false;
            for (std::size_t raceIdx = 0; raceIdx < RACES_IN_BATTLEGROUNDS.size(); ++raceIdx) {
                const auto race = RACES_IN_BATTLEGROUNDS[raceIdx];
                if (mapUnknownHadRace[raceIdx] || !minion.HasRace(race)) continue;
                introducedRace = true;
            }
            if (introducedRace) for (std::size_t raceIdx = 0; raceIdx < RACES_IN_BATTLEGROUNDS.size(); ++raceIdx) {
                const auto race = RACES_IN_BATTLEGROUNDS[raceIdx];
                bool present = false;
                recruitField.ForEachAlive([&](MinionData& data) { present = present || data.value().HasRace(race); });
                if (!present) continue;
                std::vector<Minion*> candidates;
                recruitField.ForEachAlive([&](MinionData& data) {
                    if (data.value().HasRace(race)) candidates.push_back(&data.value());
                });
                if (!candidates.empty()) {
                    auto& target = *candidates[Random::get<std::size_t>(
                        0, candidates.size() - 1)];
                    target.SetAttack(target.GetAttack() + 2);
                    target.SetHealth(target.GetHealth() + 2);
                }
            }
        }

        ApplyInfestorPlayCardBuff();

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
        ResolveDoubleTimeCopies();
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
    // Pending source identity is normalized as pendingSourceID == "BG36_342" || pendingSourceID == "BG36_342_G".
    // Golden source check: sourceCardDbfID == Cards::FindCardByID("BG36_342_G").dbfID.
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

bool Player::BeginTavernSpellDiscoverReplay(
    std::int32_t sourceSpellDbfID, std::uint64_t targetEntityID)
{
    if (season14.pendingDecision != Season14Decision::NONE || hand.IsFull() ||
        sourceSpellDbfID <= 0)
        return false;
    const auto source = Cards::FindCardByDbfID(sourceSpellDbfID);
    if (source.dbfID == 0) return false;
    const auto behavior = FindTavernSpellBehavior(source.id);
    std::vector<Card> candidates;
    switch (behavior.effect)
    {
    case TavernSpellEffect::DISCOVER_MINION:
        if (behavior.value == 1)
            AppendSupportedNormalMinions(Cards::GetTier1Minions(), candidates, Race::INVALID);
        else if (behavior.value == 7)
            AppendSupportedNormalMinions(Cards::GetTier7Minions(), candidates, Race::INVALID);
        else if (behavior.value == 8)
            candidates = SupportedDeathrattleMinions();
        else if (behavior.lockHand) {
            const auto tier = currentTier;
            if (tier == 1) AppendSupportedNormalMinions(Cards::GetTier1Minions(), candidates, Race::INVALID);
            else if (tier == 2) AppendSupportedNormalMinions(Cards::GetTier2Minions(), candidates, Race::INVALID);
            else if (tier == 3) AppendSupportedNormalMinions(Cards::GetTier3Minions(), candidates, Race::INVALID);
            else if (tier == 4) AppendSupportedNormalMinions(Cards::GetTier4Minions(), candidates, Race::INVALID);
            else if (tier == 5) AppendSupportedNormalMinions(Cards::GetTier5Minions(), candidates, Race::INVALID);
            else if (tier == 6) AppendSupportedNormalMinions(Cards::GetTier6Minions(), candidates, Race::INVALID);
            else if (tier == 7) AppendSupportedNormalMinions(Cards::GetTier7Minions(), candidates, Race::INVALID);
        } else
            candidates = SupportedMinionsForRace(MostCommonFriendlyRace(*this));
        return BeginMinionDiscover(*this, std::move(candidates), sourceSpellDbfID,
                                   behavior.lockHand);
    case TavernSpellEffect::DISCOVER_BATTLECRY_MINION:
        return BeginMinionDiscover(*this, SupportedBattlecryMinions(), sourceSpellDbfID, false);
    case TavernSpellEffect::DISCOVER_DIFFERENT_RACE:
    {
        Minion* target = nullptr;
        recruitField.ForEachAlive([&](MinionData& data) {
            if (static_cast<std::uint64_t>(data.value().GetIndex()) == targetEntityID)
                target = &data.value();
        });
        if (target == nullptr) return false;
        candidates = SupportedMinionsForRace(target->GetRace());
        candidates.erase(std::remove_if(candidates.begin(), candidates.end(),
            [target](const Card& card) { return card.id == target->GetCardID(); }),
            candidates.end());
        return BeginMinionDiscover(*this, std::move(candidates), sourceSpellDbfID, false);
    }
    case TavernSpellEffect::DISCOVER_HERO_POWER:
        for (const auto& card : Cards::GetHeroPowerMetadata())
            if (card.dbfID != 0 && card.hasBehavior) candidates.push_back(card);
        if (candidates.empty()) return false;
        Random::shuffle(candidates.begin(), candidates.end());
        candidates.resize(std::min<std::size_t>(3, candidates.size()));
        {
            std::vector<Season14Offering> offerings;
            for (const auto& card : candidates) offerings.push_back({card.dbfID, 0});
            BeginOfferingDecision(Season14Decision::DISCOVER, 0,
                                  sourceSpellDbfID, std::move(offerings));
            return true;
        }
    case TavernSpellEffect::DISCOVER_TIER_MINION_OR_SPELL:
        // This modal has a public two-step shape: first choose minion versus
        // Tavern spell, then choose one of the three generated offerings.
        // Replaying the source spell must reopen the typed branch choice,
        // rather than trying to reuse the stale offering list from the first
        // cast.  The final ApplyChoice path owns queue consumption.
        BeginSpellAllMinionChoice(
            Season14SpellModalKind::DISCOVER_TIER_MINION_OR_SPELL,
            sourceSpellDbfID, 0, 0, 0, 0, false);
        return true;
    case TavernSpellEffect::DISCOVER_CHOOSE_ONE_COMBINED:
        candidates = SupportedCombinedChooseOneMinions();
        if (candidates.empty()) return false;
        Random::shuffle(candidates.begin(), candidates.end());
        candidates.resize(std::min<std::size_t>(3, candidates.size()));
        {
            std::vector<Season14Offering> offerings;
            for (const auto& card : candidates) offerings.push_back({card.dbfID, 0});
            BeginOfferingDecision(Season14Decision::DISCOVER, 0,
                                  sourceSpellDbfID, std::move(offerings));
            return true;
        }
    case TavernSpellEffect::DISCOVER_UNDEAD_DIES_THIS_TURN:
        return BeginMinionDiscover(*this, SupportedMinionsForRace(Race::UNDEAD),
                                   sourceSpellDbfID, false);
    case TavernSpellEffect::DISCOVER_TIER_DARKMOON_PRIZE:
        for (const auto& card : Cards::GetAllCards())
            if (card.GetCardType() == CardType::SPELL &&
                card.normalDbfID == 0 && card.dbfID > 0 &&
                card.id.starts_with("BGS_Treasures_") &&
                card.darkmoonPrizeTurn == 3)
                candidates.push_back(card);
        if (candidates.size() < 3) return false;
        Random::shuffle(candidates.begin(), candidates.end());
        BeginOfferingDecision(Season14Decision::DISCOVER, 0,
                              sourceSpellDbfID,
                              {{candidates[0].dbfID, 0},
                               {candidates[1].dbfID, 0},
                               {candidates[2].dbfID, 0}});
        return true;
    default:
        return false;
    }
}

bool Player::ApplyChoice(std::size_t offeringIdx)
{
    // Quaint Boutique and Jumbo Warehouse are paid public Trinket modals.
    // Resolve them before the ordinary Discover path: Trinkets never enter
    // the hand, and stale/replayed options must not bypass the pool checks.
    if (season14.pendingDecision == Season14Decision::TRINKET_SELECTION) {
        const auto source = season14.pendingSourceCardDbfID;
        const bool greater = source == 122014;
        if ((!greater && source != 122013) ||
            offeringIdx >= season14.pendingOfferings.size() ||
            season14.pendingOfferings.size() != 3 || !season14.CanAddTrinket())
            return false;
        const auto selected = season14.pendingOfferings[offeringIdx].dbfID;
        const auto card = Cards::FindCardByDbfID(selected);
        const bool offered = std::any_of(
            season14.pendingOfferings.begin(), season14.pendingOfferings.end(),
            [selected](const Season14Offering& offering) {
                return offering.dbfID == selected;
            });
        if (card.dbfID == 0 || !offered ||
            card.GetCardType() != CardType::BATTLEGROUND_TRINKET ||
            card.normalDbfID != 0 ||
            card.trinketType != (greater ? "GREATER_TRINKET" : "LESSER_TRINKET") ||
            std::any_of(season14.trinkets.begin(), season14.trinkets.end(),
                [selected](const Season14PersistentEffect& existing) {
                    return existing.dbfID == selected;
                }))
            return false;
        if (!AcquireTrinket({selected, 1, true}) ||
            !season14.SelectDecision(offeringIdx))
            return false;
        return true;
    }
    if (season14.pendingSourceCardDbfID == 120359) {
        if (season14.pendingDecision != Season14Decision::DISCOVER ||
            offeringIdx >= season14.pendingOfferings.size() ||
            season14.pendingSourceEntityID == 0)
            return false;
        const auto chosen = season14.pendingOfferings[offeringIdx].dbfID;
        if (!IsExecutableSpawningPoolZergDbfID(chosen) ||
            !CardDefs::HasDefinition(Cards::FindCardByDbfID(chosen).id))
            return false;
        Minion* larva = nullptr;
        recruitField.ForEachAlive([&](MinionData& data) {
            if (static_cast<std::uint64_t>(data.value().GetIndex()) ==
                season14.pendingSourceEntityID)
                larva = &data.value();
        });
        if (larva == nullptr || larva->GetDbfID() != 120359 ||
            !season14.SelectDecision(offeringIdx))
            return false;
        const auto replacement = Cards::FindCardByDbfID(chosen);
        if (!larva->TransformTo(replacement)) return false;
        season14.spawningPoolLarvaEntityID = 0;
        return true;
    }
    if (season14.pendingSourceCardDbfID == 119196) {
        if (season14.pendingDecision != Season14Decision::CHOICE ||
            season14.pendingOfferings.size() != 2 || offeringIdx >= 2) return false;
        const auto chosen = season14.pendingOfferings[offeringIdx].dbfID;
        const auto card = Cards::FindCardByDbfID(chosen);
        if (card.GetCardType() != CardType::MINION ||
            !IsWarpGateProtossDbfID(card.dbfID) ||
            !season14.SelectDecision(offeringIdx)) return false;
        season14.warpGateSelectedDbfID = chosen;
        return true;
    }
    if (season14.pendingSourceCardDbfID == 90403)
    {
        if (season14.pendingDecision != Season14Decision::CHOICE ||
            offeringIdx >= season14.pendingOfferings.size() ||
            season14.pendingOfferings.size() != 2 ||
            season14.detectiveCorrectDbfID == 0)
            return false;
        const auto chosen = season14.pendingOfferings[offeringIdx].dbfID;
        const bool correct = chosen == season14.detectiveCorrectDbfID;
        if (!season14.SelectDecision(offeringIdx)) return false;
        season14.detectiveCorrectDbfID = 0;
        if (correct) AddTavernCoins(1);
        return true;
    }
    if (season14.pendingHeroPowerReplayDbfID != 0)
    {
        if (season14.pendingDecision != Season14Decision::DISCOVER ||
            offeringIdx >= season14.pendingOfferings.size() || hand.IsFull())
            return false;
        const auto card = Cards::FindCardByDbfID(
            season14.pendingOfferings[offeringIdx].dbfID);
        const auto source = season14.pendingHeroPowerReplayDbfID;
        const auto valid =
            card.dbfID != 0 && card.normalDbfID == 0 &&
            card.GetCardType() == CardType::MINION && card.hasBehavior &&
            card.isBattlegroundsPoolMinion &&
            ((source == 59891 && (card.GetTier() == 3 || card.GetTier() == 4)) ||
             (source == 63127 &&
              card.HasRace(static_cast<Race>(season14.pendingHeroPowerReplayRace))) ||
             (source == 97814 && card.HasRace(Race::UNDEAD)) ||
             (source == 62267 &&
              card.GetTier() == season14.pendingHeroPowerReplayTier) ||
             (source == 64481 &&
              card.GetTier() == season14.pendingHeroPowerReplayTier));
        if (!valid) return false;
        Minion generated{card};
        ApplyFreshMinionModifiers(generated);
        hand.Add(CardData{std::move(generated)});
        if (--season14.pendingHeroPowerReplayRemaining > 0)
        {
            std::vector<Card> candidates;
            for (const auto& candidate : Cards::GetAllCards())
            {
                if (candidate.dbfID == 0 || candidate.normalDbfID != 0 ||
                    candidate.GetCardType() != CardType::MINION ||
                    !candidate.hasBehavior || !candidate.isBattlegroundsPoolMinion)
                    continue;
                if (source == 59891 && candidate.GetTier() != 3 &&
                    candidate.GetTier() != 4)
                    continue;
                if (source == 63127 && !candidate.HasRace(static_cast<Race>(
                                              season14.pendingHeroPowerReplayRace)))
                    continue;
                if (source == 97814 && !candidate.HasRace(Race::UNDEAD))
                    continue;
                if ((source == 62267 || source == 64481) &&
                    candidate.GetTier() != season14.pendingHeroPowerReplayTier)
                    continue;
                candidates.push_back(candidate);
            }
            if (candidates.size() < 3) return false;
            Random::shuffle(candidates.begin(), candidates.end());
            season14.pendingOfferings.clear();
            for (std::size_t i = 0; i < 3; ++i)
                season14.pendingOfferings.push_back({candidates[i].dbfID, 0});
        }
        else
        {
            season14.pendingHeroPowerReplayDbfID = 0;
            season14.pendingHeroPowerReplayRemaining = 0;
            season14.pendingHeroPowerReplayTier = 0;
            season14.pendingHeroPowerReplayRace = 0;
            season14.pendingOfferings.clear();
            season14.pendingSourceCardDbfID = 0;
            season14.pendingDecision = Season14Decision::NONE;
        }
        return true;
    }
    if (season14.pendingUndeadDiscoverSourceEntityID != 0)
    {
        if (offeringIdx >= season14.pendingOfferings.size() || hand.IsFull()) return false;
        bool sourcePresent = false;
        recruitField.ForEachAlive([&](const MinionData& data) {
            if (static_cast<std::uint64_t>(data.value().GetIndex()) ==
                season14.pendingUndeadDiscoverSourceEntityID) sourcePresent = true;
        });
        if (!sourcePresent) return false;
        const auto card = Cards::FindCardByDbfID(season14.pendingOfferings[offeringIdx].dbfID);
        if (card.dbfID == 0 || card.normalDbfID != 0 ||
            card.GetCardType() != CardType::MINION || !card.HasRace(Race::UNDEAD)) return false;
        Minion generated{card};
        ApplyFreshMinionModifiers(generated);
        hand.Add(CardData{std::move(generated)});
        if (--season14.pendingUndeadDiscoverRemaining > 0)
        {
            std::vector<Card> candidates;
            for (const auto& candidate : Cards::GetAllCards())
                if (candidate.isBattlegroundsPoolMinion && candidate.normalDbfID == 0 &&
                    candidate.HasRace(Race::UNDEAD)) candidates.push_back(candidate);
            if (candidates.empty()) return false;
            Random::shuffle(candidates.begin(), candidates.end());
            season14.pendingOfferings.clear();
            for (std::size_t i = 0; i < std::min<std::size_t>(3, candidates.size()); ++i)
                season14.pendingOfferings.push_back({candidates[i].dbfID, 0});
        }
        else
        {
            season14.pendingUndeadDiscoverSourceEntityID = 0;
            season14.pendingOfferings.clear();
            season14.pendingDecision = Season14Decision::NONE;
        }
        return true;
    }
    if (season14.pendingGoldenizeSourceEntityID != 0)
    {
        if (offeringIdx >= season14.pendingGoldenizeTargets.size()) return false;
        Minion* source = nullptr;
        Minion* first = nullptr;
        Minion* second = nullptr;
        const auto secondID = season14.pendingGoldenizeTargets[offeringIdx];
        recruitField.ForEachAlive([&](MinionData& data) {
            auto& candidate = data.value();
            const auto id = static_cast<std::uint64_t>(candidate.GetIndex());
            if (id == season14.pendingGoldenizeSourceEntityID) source = &candidate;
            if (id == season14.pendingGoldenizeFirstTargetEntityID) first = &candidate;
            if (id == secondID) second = &candidate;
        });
        if (source == nullptr || first == nullptr || second == nullptr ||
            first == second || first->IsDestroyed() || second->IsDestroyed() ||
            first->GetTier() > 6 || second->GetTier() > 6 ||
            !first->CanMakeGolden() || !second->CanMakeGolden()) return false;
        // Validate both before mutating either target: stale choices cannot
        // produce a partial goldenization.
        if (!first->MakeGolden() || !second->MakeGolden()) return false;
        season14.pendingGoldenizeSourceEntityID = 0;
        season14.pendingGoldenizeFirstTargetEntityID = 0;
        season14.pendingGoldenizeTargets.clear();
        season14.pendingDecision = Season14Decision::NONE;
        return true;
    }
    if (season14.pendingDemonDiscoverSourceEntityID != 0)
    {
        if (offeringIdx >= season14.pendingOfferings.size() || hand.IsFull()) return false;
        bool sourcePresent = false;
        recruitField.ForEachAlive([&](const MinionData& data) {
            if (static_cast<std::uint64_t>(data.value().GetIndex()) ==
                season14.pendingDemonDiscoverSourceEntityID) sourcePresent = true;
        });
        if (!sourcePresent) return false;
        const auto offering = season14.pendingOfferings[offeringIdx];
        const auto demon = Cards::FindCardByDbfID(offering.dbfID);
        if (demon.dbfID == 0 || demon.normalDbfID != 0 ||
            demon.GetCardType() != CardType::MINION || !demon.HasRace(Race::DEMON)) return false;
        Minion generated{demon};
        ApplyFreshMinionModifiers(generated);
        hand.Add(CardData{std::move(generated)});
        hero.TakeDamage(*this, demon.GetTier());
        if (--season14.pendingDemonDiscoverRemaining > 0)
        {
            std::vector<Card> candidates;
            for (const auto& card : Cards::GetAllCards())
                if (card.isBattlegroundsPoolMinion && card.normalDbfID == 0 &&
                    card.HasRace(Race::DEMON)) candidates.push_back(card);
            if (candidates.empty()) return false;
            Random::shuffle(candidates.begin(), candidates.end());
            season14.pendingOfferings.clear();
            for (std::size_t i = 0; i < std::min<std::size_t>(3, candidates.size()); ++i)
                season14.pendingOfferings.push_back({candidates[i].dbfID, 0});
        }
        else
        {
            season14.pendingDemonDiscoverSourceEntityID = 0;
            season14.pendingOfferings.clear();
        }
        return true;
    }
    // Clunker Junker's second stage is a public, typed Discover: each chosen
    // normal Magnetic Mech is attached to the same friendly Mech selected by
    // the Battlecry.  Keep this state separate from ordinary Discover-to-hand
    // paths so an offering can never be accidentally materialized as a hand
    // card.  Golden Clunker Junker reopens the same public modal once after
    // each successful selection.
    if (season14.pendingMechMagnetizeSourceEntityID != 0)
    {
        if (season14.pendingDecision != Season14Decision::DISCOVER ||
            offeringIdx >= season14.pendingOfferings.size() ||
            season14.pendingMechMagnetizeRemaining <= 0)
            return false;

        Minion* source = nullptr;
        Minion* target = nullptr;
        recruitField.ForEachAlive([&](MinionData& data) {
            auto& candidate = data.value();
            const auto entity = static_cast<std::uint64_t>(candidate.GetIndex());
            if (entity == season14.pendingMechMagnetizeSourceEntityID &&
                candidate.GetDbfID() == season14.pendingSourceCardDbfID)
                source = &candidate;
            if (entity == season14.pendingMechMagnetizeTargetEntityID)
                target = &candidate;
        });
        if (source == nullptr || target == nullptr || target->IsDestroyed() ||
            !target->HasRace(Race::MECHANICAL) ||
            (source->GetDbfID() != Cards::FindCardByID("BG29_503").dbfID &&
             source->GetDbfID() != Cards::FindCardByID("BG29_503_G").dbfID))
            return false;

        const auto card = Cards::FindCardByDbfID(
            season14.pendingOfferings[offeringIdx].dbfID);
        if (card.dbfID == 0 || card.GetCardType() != CardType::MINION ||
            card.normalDbfID != 0 || !card.isBattlegroundsPoolMinion ||
            !card.hasBehavior || !card.HasRace(Race::MECHANICAL) ||
            !card.gameTags.contains(GameTag::MAGNETIC) ||
            card.gameTags.at(GameTag::MAGNETIC) == 0)
            return false;

        Minion attachment{card};
        if (!attachment.CanMagnetizeTo(*target)) return false;
        attachment.MagnetizeOnto(*target);

        const auto sourceEntityID = season14.pendingMechMagnetizeSourceEntityID;
        const auto sourceCardDbfID = season14.pendingSourceCardDbfID;
        const auto targetEntityID = season14.pendingMechMagnetizeTargetEntityID;
        --season14.pendingMechMagnetizeRemaining;
        if (!season14.SelectDecision(offeringIdx)) return false;
        if (season14.pendingMechMagnetizeRemaining > 0)
        {
            std::vector<Card> candidates;
            for (const auto& candidate : Cards::GetAllCards())
                if (candidate.isBattlegroundsPoolMinion &&
                    candidate.normalDbfID == 0 && candidate.hasBehavior &&
                    candidate.GetCardType() == CardType::MINION &&
                    candidate.HasRace(Race::MECHANICAL) &&
                    candidate.gameTags.contains(GameTag::MAGNETIC) &&
                    candidate.gameTags.at(GameTag::MAGNETIC) != 0)
                    candidates.push_back(candidate);
            if (candidates.empty()) return false;
            Random::shuffle(candidates.begin(), candidates.end());
            std::vector<Season14Offering> offerings;
            for (std::size_t i = 0;
                 i < std::min<std::size_t>(3, candidates.size()); ++i)
                offerings.push_back({candidates[i].dbfID, 0});
            season14.pendingMechMagnetizeSourceEntityID = sourceEntityID;
            season14.pendingMechMagnetizeTargetEntityID = targetEntityID;
            season14.BeginOfferingDecision(Season14Decision::DISCOVER,
                                           sourceEntityID, sourceCardDbfID,
                                           std::move(offerings));
        }
        else
        {
            season14.pendingMechMagnetizeSourceEntityID = 0;
            season14.pendingMechMagnetizeTargetEntityID = 0;
            season14.pendingMechMagnetizeRemaining = 0;
        }
        return true;
    }
    if (season14.pendingDecision != Season14Decision::CHOICE &&
        season14.pendingDecision != Season14Decision::DISCOVER)
    {
        return false;
    }
    const bool wasDiscover = season14.pendingDecision == Season14Decision::DISCOVER;
    const bool magicfinRelic = season14.pendingSourceCardDbfID == 122825;
    const bool expeditionPlans = season14.pendingSourceCardDbfID == 81570;
    if (wasDiscover && season14.pendingSourceCardDbfID == 110472)
    {
        if (hand.IsFull() || remainCoin < season14.EffectiveHeroPowerCost())
            return false;
        remainCoin -= season14.EffectiveHeroPowerCost();
        if (!season14.UseHeroPower()) return false;
        ResolveHeroPowerUseBuddies();
    }
    const bool galakrondGreed =
        season14.pendingTavernReplacementSlot >= 0 &&
        season14.pendingSourceCardDbfID == Cards::FindCardByID("TB_BaconShop_HP_011").dbfID;
    const bool dungarFlightpath = season14.pendingSourceCardDbfID == 75703;
    const bool ironforgeFlightpath = season14.pendingSourceCardDbfID == 75705;
    const bool powerOfStorm = season14.pendingSourceCardDbfID == 71909;
    const bool nagaConquest = season14.pendingSourceCardDbfID == 80007;
    const bool convictionImprovement = season14.pendingSourceCardDbfID == 73941;
    const bool embraceElements = season14.pendingSourceCardDbfID == 79720;
    const bool championChoice = season14.pendingSourceCardDbfID == 104628;
    if (offeringIdx >= season14.pendingOfferings.size() ||
        (!galakrondGreed && !dungarFlightpath && !convictionImprovement &&
         !championChoice && season14.pendingSourceCardDbfID != 97485 &&
         !embraceElements && !expeditionPlans && !magicfinRelic && hand.IsFull()))
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
    // Azeroth Model Globe is a constrained Tier 6 minion Discover.  Recheck
    // the complete modal at commit time so a stale/replayed choice cannot
    // turn its source into an arbitrary card (or a duplicated option).
    const bool azerothModelGlobe =
        season14.pendingSourceCardDbfID == 112001;
    if (azerothModelGlobe)
    {
        if (!wasDiscover || season14.pendingOfferings.size() != 3)
            return false;
        std::set<std::int32_t> globeOfferings;
        for (const auto& pending : season14.pendingOfferings)
        {
            const auto candidate = Cards::FindCardByDbfID(pending.dbfID);
            if (candidate.GetCardType() != CardType::MINION ||
                !candidate.isBattlegroundsPoolMinion ||
                candidate.normalDbfID != 0 || !candidate.hasBehavior ||
                candidate.GetTier() != 6 ||
                !globeOfferings.insert(candidate.dbfID).second)
                return false;
        }
    }
    if (season14.pendingSourceCardDbfID == 111609 ||
        season14.pendingSourceCardDbfID == 115230)
    {
        const auto sourceCardDbfID = season14.pendingSourceCardDbfID;
        const auto card = Cards::FindCardByDbfID(offering.dbfID);
        if (!wasDiscover || card.GetCardType() != CardType::BATTLEGROUND_SPELL ||
            FindTavernSpellBehavior(card.id).effect == TavernSpellEffect::NONE ||
            !season14.SelectDecision(offeringIdx))
            return false;
        hand.Add(CardData{Spell(card)});
        if (--season14.bookOfMedivhRemaining > 0)
            (void)BeginBookOfMedivhDiscover(*this,
                                             sourceCardDbfID, 1);
        else
            season14.bookOfMedivhRemaining = 0;
        return true;
    }
    if (magicfinRelic) {
        const auto card = Cards::FindCardByDbfID(offering.dbfID);
        if (!wasDiscover || card.GetCardType() != CardType::BATTLEGROUND_SPELL ||
            FindTavernSpellBehavior(card.id).effect == TavernSpellEffect::NONE ||
            !season14.SelectDecision(offeringIdx))
            return false;
        // The Apprentice is already in hand; the Discover teaches that exact
        // spell to it instead of creating a second hand card.
        Minion* apprentice = nullptr;
        hand.ForEach([&](std::optional<CardData>& data) {
            if (data.has_value() && std::holds_alternative<Minion>(data.value()) &&
                std::get<Minion>(data.value()).GetCardID() == "BG33_890t")
                apprentice = &std::get<Minion>(data.value());
        });
        if (apprentice == nullptr) return false;
        apprentice->SetTaughtTavernSpell(card.id);
        return true;
    }
    if (season14.pendingSourceCardDbfID == 104677) {
        if (season14.pendingDecision != Season14Decision::DISCOVER ||
            hand.IsFull() || offering.dbfID <= 0)
            return false;
        auto snapshot = season14.FindLastOpponentCombatMinionSnapshot(
            offering.dbfID);
        if (!snapshot.has_value() || !season14.SelectDecision(offeringIdx))
            return false;
        hand.Add(CardData{std::move(*snapshot)});
        return true;
    }
    // Prestidigitation puts the selected Secret directly on the battlefield;
    // it is not a hand Discover.  Keep this special case before the generic
    // card-to-hand path so the secret's trigger lifecycle is preserved.
    if (season14.pendingSourceCardDbfID == 58022) {
        const auto card = Cards::FindCardByDbfID(offering.dbfID);
        if (season14.pendingDecision != Season14Decision::DISCOVER ||
            card.GetCardType() != CardType::SPELL || season14.pendingOfferings.size() != 3 ||
            season14.pendingOfferings[offeringIdx].dbfID != card.dbfID ||
            season14.heroPowerBatch9.pendingPrestidigitationRemaining <= 0 ||
            season14.heroPowerBatch9.activeSecretCount >= 5) return false;
        if (!season14.SelectDecision(offeringIdx)) return false;
        if (season14.heroPowerBatch9.activeSecretCount >=
            season14.heroPowerBatch9.activeSecretDbfIDs.size()) return false;
        season14.heroPowerBatch9.activeSecretDbfIDs[
            season14.heroPowerBatch9.activeSecretCount++] = card.dbfID;
        if (--season14.heroPowerBatch9.pendingPrestidigitationRemaining > 0) {
            std::vector<Card> candidates;
            for (const auto& candidate : Cards::GetAllCards())
                if (candidate.GetCardType() == CardType::SPELL && candidate.dbfID > 0 &&
                    candidate.normalDbfID == 0)
                    candidates.push_back(candidate);
            if (candidates.size() < 3 ||
                season14.heroPowerBatch9.activeSecretCount >=
                    season14.heroPowerBatch9.activeSecretDbfIDs.size())
                return false;
            Random::shuffle(candidates.begin(), candidates.end());
            season14.BeginOfferingDecision(
                Season14Decision::DISCOVER, 0, 58022,
                {{candidates[0].dbfID, 0}, {candidates[1].dbfID, 0},
                 {candidates[2].dbfID, 0}});
        } else {
            season14.heroPowerBatch9.pendingPrestidigitationRemaining = 0;
        }
        return true;
    }
    if (season14.pendingSourceCardDbfID == 60450) {
        const auto card = Cards::FindCardByDbfID(offering.dbfID);
        if (season14.pendingDecision != Season14Decision::DISCOVER ||
            card.GetCardType() != CardType::HERO_POWER ||
            season14.pendingOfferings.size() != 3 || !card.hasBehavior)
            return false;
        if (!season14.SelectDecision(offeringIdx)) return false;
        // Adventure replaces the temporary discovery power with the selected
        // power while retaining the owning hero and all lobby state.
        const int cost = card.gameTags.contains(GameTag::COST)
                             ? card.gameTags.at(GameTag::COST)
                             : 0;
        season14.SetHeroPower(card.dbfID, cost, true);
        return true;
    }
    if (season14.pendingSourceCardDbfID == 122924) {
        const auto card = Cards::FindCardByDbfID(offering.dbfID);
        if (season14.pendingDecision != Season14Decision::DISCOVER ||
            season14.pendingOfferings.size() != 3 ||
            card.GetCardType() != CardType::HERO_POWER || !card.hasBehavior ||
            card.dbfID == season14.heroPowerDbfID)
            return false;
        if (!season14.SelectDecision(offeringIdx)) return false;
        const int cost = card.gameTags.contains(GameTag::COST)
                             ? card.gameTags.at(GameTag::COST)
                             : 0;
        season14.SetHeroPower(card.dbfID, cost, true);
        return true;
    }
    if (season14.pendingSourceCardDbfID == 106440) {
        // No Place Like Holmes is an information-set choice: only the
        // observed last-opponent cards captured in this modal are valid, and
        // resolving it consumes the reward without fabricating a hand card.
        if (season14.pendingDecision != Season14Decision::DISCOVER ||
            season14.pendingOfferings.size() < 2 ||
            offeringIdx >= season14.pendingOfferings.size())
            return false;
        const auto selected = season14.pendingOfferings[offeringIdx].dbfID;
        const bool wasObserved = std::any_of(
            season14.lastOpponentCombatMinionDbfIDs.begin(),
            season14.lastOpponentCombatMinionDbfIDs.end(),
            [selected](const auto observedDbfID) {
                const auto observed = Cards::FindCardByDbfID(observedDbfID);
                const auto canonical = observed.normalDbfID != 0
                    ? observed.normalDbfID : observed.dbfID;
                return canonical == selected;
            });
        if (!wasObserved)
            return false;
        if (!season14.SelectDecision(offeringIdx)) return false;
        season14.generatedRewardOpponentWarbandGuess = false;
        return true;
    }
    // Whodunit is a public quest selection, not a hand-card Discover. Keep
    // the selected DBF in the decision/replay stream; quest progression and
    // reward resolution are delegated to the lobby quest subsystem.
    if (season14.pendingSourceCardDbfID == 92961) {
        if (!IsWhodunitQuestDbfID(offering.dbfID) ||
            season14.pendingOfferings.size() != 2)
            return false;
        return season14.SelectDecision(offeringIdx);
    }
    if (championChoice) {
        const auto card = Cards::FindCardByDbfID(offering.dbfID);
        if (card.GetCardType() != CardType::MINION || card.GetTier() != 7 ||
            !card.isBattlegroundsPoolMinion || card.normalDbfID != 0 || !card.hasBehavior)
            return false;
        season14.SetChampionReward(card.dbfID);
        TryDeliverChampionReward();
        return season14.SelectDecision(offeringIdx);
    }


    if (embraceElements)
    {
        if (offering.dbfID < 79721 || offering.dbfID > 79724)
            return false;
        season14.embraceElementDbfID = offering.dbfID;
        return season14.SelectDecision(offeringIdx);
    }

    // Conviction improvements are typed options, not cards.  Resolve them
    // before Card lookup so the negative sentinel IDs can never enter a hand
    // or be mistaken for an executable generated entity.
    if (season14.pendingSourceCardDbfID == 73941)
        return season14.ApplyConvictionImprovement(offeringIdx);
    if (season14.pendingSourceCardDbfID == 77845 ||
        season14.pendingSourceCardDbfID == 77846) {
        const auto ticketSourceDbfID = season14.pendingSourceCardDbfID;
        const auto prize = Cards::FindCardByDbfID(offering.dbfID);
        if (!wasDiscover || season14.buddyTicketRemaining <= 0 ||
            season14.pendingOfferings.size() != 3 ||
            prize.GetCardType() != CardType::SPELL || prize.normalDbfID != 0 ||
            !prize.id.starts_with("BGS_Treasures_") ||
            prize.darkmoonPrizeTurn != std::min(currentTier + 1, TIER_UPPER_LIMIT) ||
            !season14.SelectDecision(offeringIdx))
            return false;
        // A full hand consumes the selected Prize without creating an
        // over-capacity card, matching normal Discover delivery semantics.
        if (!hand.IsFull())
            hand.Add(CardData{Spell(prize)});
        if (--season14.buddyTicketRemaining > 0) {
            if (hand.IsFull()) {
                season14.buddyTicketRemaining = 0;
                return true;
            }
            std::vector<Card> prizes;
            for (const auto& candidate : Cards::GetAllCards())
                if (candidate.GetCardType() == CardType::SPELL &&
                    candidate.normalDbfID == 0 &&
                    candidate.id.starts_with("BGS_Treasures_") &&
                    candidate.darkmoonPrizeTurn ==
                        std::min(currentTier + 1, TIER_UPPER_LIMIT))
                    prizes.push_back(candidate);
            if (prizes.size() < 3) return false;
            Random::shuffle(prizes.begin(), prizes.end());
            season14.BeginOfferingDecision(
                Season14Decision::DISCOVER, 0,
                ticketSourceDbfID,
                {{prizes[0].dbfID, 0}, {prizes[1].dbfID, 0},
                 {prizes[2].dbfID, 0}});
        }
        return true;
    }
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

    const auto pendingSourceID =
        Cards::FindCardByDbfID(season14.pendingSourceCardDbfID).id;
    if (pendingSourceID == "BG35_MagicItem_812t")
    {
        // Recheck the constrained prize pool at commit time. This keeps a
        // stale/replayed offering from awarding an arbitrary spell.
        const bool selectedOffering = std::any_of(
            season14.pendingOfferings.begin(), season14.pendingOfferings.end(),
            [&offering](const Season14Offering& pending) {
                return pending.dbfID == offering.dbfID;
            });
        const bool distinctOfferings =
            season14.pendingOfferings.size() == 3 &&
            season14.pendingOfferings[0].dbfID !=
                season14.pendingOfferings[1].dbfID &&
            season14.pendingOfferings[0].dbfID !=
                season14.pendingOfferings[2].dbfID &&
            season14.pendingOfferings[1].dbfID !=
                season14.pendingOfferings[2].dbfID &&
            std::all_of(
                season14.pendingOfferings.begin(),
                season14.pendingOfferings.end(), [](const Season14Offering& pending) {
                    const auto prize = Cards::FindCardByDbfID(pending.dbfID);
                    return prize.GetCardType() == CardType::SPELL &&
                           prize.normalDbfID == 0 && prize.dbfID > 0 &&
                           prize.id.starts_with("BGS_Treasures_") &&
                           prize.darkmoonPrizeTurn == 3;
                });
        if (season14.pendingDecision != Season14Decision::DISCOVER ||
            !distinctOfferings || !selectedOffering ||
            card.GetCardType() != CardType::SPELL || card.normalDbfID != 0 ||
            !card.id.starts_with("BGS_Treasures_") ||
            card.darkmoonPrizeTurn != 3 ||
            hand.IsFull())
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
        // Galakrond replacement is allowed to proceed even when the hand is
        // full; ordinary discover paths retain the (!galakrondGreed && hand.IsFull()) guard.
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
    const bool darkGiftDiscover =
        season14.pendingSourceCardDbfID == 132581 ||
        season14.pendingSourceCardDbfID == 134010;
    const auto darkGift = darkGiftDiscover
        ? FindDarkGiftBehavior(Cards::FindCardByDbfID(offering.darkGiftDbfID).id)
        : DarkGiftBehavior{};
    if (darkGiftDiscover &&
        (offering.darkGiftDbfID == 0 || darkGift.effect == DarkGiftEffect::NONE))
        return false;
    if (darkGiftDiscover &&
        (card.GetCardType() != CardType::MINION || card.normalDbfID != 0 ||
         card.GetTier() != 5 || !card.hasBehavior ||
         !DarkGiftTargetIsLegal(Minion(card), darkGift)))
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

    const bool windfallDiscover =
        season14.pendingDecision == Season14Decision::DISCOVER &&
        (pendingSourceID == "BG31_817" || pendingSourceID == "BG31_817_G");
    if (windfallDiscover) {
        const bool offered = card.GetCardType() == CardType::MINION &&
            card.normalDbfID == 0 && card.hasBehavior &&
            card.HasRace(Race::ELEMENTAL);
        if (!offered || hand.IsFull()) return false;
    }
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

    if (season14.pendingSourceCardDbfID == 97485)
    {
        // The source reward is consumed by the selected replacement.  Keep
        // this branch ahead of the ordinary quest-reward path so a stale
        // replay cannot award an arbitrary card merely because it has the
        // quest-reward type.
        if (season14.pendingDecision != Season14Decision::DISCOVER ||
            season14.pendingOfferings.size() != 2 ||
            season14.pendingOfferings[0].dbfID ==
                season14.pendingOfferings[1].dbfID ||
            (offering.dbfID != season14.pendingOfferings[0].dbfID &&
             offering.dbfID != season14.pendingOfferings[1].dbfID) ||
            card.GetCardType() != CardType::BATTLEGROUND_QUEST_REWARD ||
            !IsExecutableSeason14GeneratedQuestReward(card.dbfID))
            return false;
        const bool selected = season14.SelectDecision(offeringIdx);
        if (!selected) return false;
        season14.generatedRewardEtherealEvidence = false;
        season14.generatedQuestRewards.erase(
            std::remove(season14.generatedQuestRewards.begin(),
                        season14.generatedQuestRewards.end(), 97485),
            season14.generatedQuestRewards.end());
        season14.generatedQuestRewards.push_back(card.dbfID);
        return season14.ApplyGeneratedQuestReward(card.dbfID);
    }

    if (card.GetCardType() == CardType::BATTLEGROUND_QUEST_REWARD)
    {
        if (!IsSeason14GeneratedQuestReward(card.dbfID)) return false;
        if (card.dbfID == 104673) {
            // War Horn's linked minion is selected by the server when the
            // reward is granted.  Recheck the same legal pool before
            // consuming the public reward choice; otherwise a stale/replay
            // state could commit the reward without either its copy or aura.
            if (hand.IsFull()) return false;
            bool hasLegalBattlecry = false;
            recruitField.ForEachAlive([&hasLegalBattlecry](MinionData& data) {
                if (hasLegalBattlecry || !data.value().HasBattlecry()) return;
                const auto candidate = Cards::FindCardByID(data.value().GetCardID());
                hasLegalBattlecry =
                    candidate.GetCardType() == CardType::MINION &&
                    candidate.isBattlegroundsPoolMinion &&
                    candidate.normalDbfID == 0 && candidate.hasBehavior;
            });
            if (!hasLegalBattlecry) return false;
        }
        if (card.dbfID == 110310) {
            if (hand.IsFull()) return false;
            const bool hasTierSeven = std::any_of(
                Cards::GetTier7Minions().begin(), Cards::GetTier7Minions().end(),
                [](const Card& candidate) {
                    return candidate.GetCardType() == CardType::MINION &&
                           candidate.isBattlegroundsPoolMinion &&
                           candidate.normalDbfID == 0 && candidate.hasBehavior;
                });
            if (!hasTierSeven) return false;
        }
        if (card.dbfID == 91980 &&
            (hand.IsFull() || hero.card.relatedDbfID == 0 ||
             Cards::FindCardByDbfID(hero.card.relatedDbfID).premiumDbfID == 0))
            return false;
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

    if (expeditionPlans) {
        const auto card = Cards::FindCardByDbfID(offering.dbfID);
        const int tier = card.GetTier();
        if (card.GetCardType() != CardType::MINION ||
            (tier != 6 && tier != 4 && tier != 2)) return false;
        season14.SetExpeditionReward(tier, card.dbfID);
        season14.MarkExpeditionTierDiscovered(tier);
        const bool selected = season14.SelectDecision(offeringIdx);
        if (!selected) return false;
        if (tier == 6) BeginExpeditionDiscoveryForTier(4);
        else if (tier == 4) BeginExpeditionDiscoveryForTier(2);
        return true;
    }

    if (card.GetCardType() == CardType::MINION)
    {
        Minion minion(card);
        ApplyFreshMinionModifiers(minion);
        if (windfallDiscover) {
            minion.SetAttack(season14.windfallAttack);
            minion.SetHealth(season14.windfallHealth);
        }
        if (darkGiftDiscover && !ApplyDarkGift(*this, minion, darkGift))
            return false;
        hand.Add(CardData{ std::move(minion) });
        if (season14.pendingHandLock)
            std::get<Minion>(hand[hand.GetCount() - 1]).SetHandLocked(true);
    }
    else if (card.GetCardType() == CardType::HERO_POWER)
    {
        // Unmasked identity path is equivalent to SetHeroPower(card.dbfID, 0, true).
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
    const auto windfallAttack = season14.windfallAttack;
    const auto windfallHealth = season14.windfallHealth;
    const auto windfallRemaining = season14.windfallRemaining;
    const bool selected = season14.SelectDecision(offeringIdx);
    if (selected && wasDiscover)
        AddGeneratedDiscoverCopy(card);
    if (selected && wasDiscover)
        ResolveDiscoverTriggers();
    if (selected && wasDiscover &&
        (sourceCardDbfID == 77603 || sourceCardDbfID == 77507) &&
        season14.clockworkDiscoverRemaining > 0) {
        --season14.clockworkDiscoverRemaining;
        // A full hand, an exhausted next-tier pool, or any other failed
        // modal start consumes the chained golden Discover.  Do not leave a
        // stale counter that could be mistaken for a later Clockwork play.
        if (!BeginClockworkAssistantDiscover(false))
            season14.clockworkDiscoverRemaining = 0;
    }
    if (selected && sourceCardDbfID == 89294)
        season14.reclaimedSoulsDeaths.clear();
    if (selected && season14.tavernSpellDiscoverRemaining > 0)
    {
        --season14.tavernSpellDiscoverRemaining;
        BeginTavernSpellDiscover(1, sourceEntityID, sourceCardDbfID);
    }
    // Cathedral/Sushi repeats are queued while the original Discover modal
    // is open.  Re-open the same typed Discover family only after its first
    // choice commits; this keeps the policy-facing modal asynchronous and
    // prevents a repeat from being lost when SelectDecision clears source
    // metadata.  The target is an entity id, never a stale slot index.
    if (selected && wasDiscover && season14.discoverReplayRemaining > 0)
    {
        const auto replaySource = season14.discoverReplaySourceSpellDbfID;
        const auto replayTarget = season14.discoverReplayTargetEntityID;
        // A golden Activate may have opened its next Tavern-spell Discover
        // first.  Leave this queue untouched until that modal commits; two
        // public offerings must never be collapsed into one decision.
        if (season14.pendingDecision == Season14Decision::NONE)
        {
            --season14.discoverReplayRemaining;
            const bool reopened = BeginTavernSpellDiscoverReplay(replaySource, replayTarget);
            if (!reopened || season14.discoverReplayRemaining == 0)
            {
                season14.discoverReplaySourceSpellDbfID = 0;
                season14.discoverReplayTargetEntityID = 0;
                if (!reopened) season14.discoverReplayRemaining = 0;
            }
        }
    }
    if (selected && windfallDiscover) {
        season14.windfallRemaining = 0;
        if (windfallRemaining > 0 &&
            !BeginWindfallDiscover(*this, sourceCardDbfID, windfallAttack,
                                   windfallHealth, windfallRemaining)) {
            season14.windfallAttack = 0;
            season14.windfallHealth = 0;
        } else if (windfallRemaining == 0) {
            season14.windfallAttack = 0;
            season14.windfallHealth = 0;
        }
    }
    season14.pendingHandLock = false;
    return selected;
}

bool Player::ResolveFlightpathCompletion()
{
    // Westfall selects cards with GetCost() == 1; the wire metadata check
    // below is equivalent without constructing temporary CardData.
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
        else if (behavior.effect == TrinketEffect::TAVERN_SPELL_NO_TYPE_STATS)
        {
            // Wizard's Pipe affects only friendly minions with no tribe; it
            // is a board buff, not a shop/hand aura.
            recruitField.ForEachAlive([&behavior](MinionData& data) {
                auto& minion = data.value();
                bool hasType = false;
                for (const auto race : RACES_IN_BATTLEGROUNDS)
                    hasType = hasType || minion.HasRace(race);
                if (!hasType)
                {
                    minion.SetAttack(minion.GetAttack() + behavior.attack);
                    minion.SetHealth(minion.GetHealth() + behavior.health);
                }
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

void Player::ApplyAfterPlayCardTrinkets(Race playedRace, bool magnetic)
{
    // Both ordinary and magnetic play paths converge here; the no-argument
    // ApplyAfterPlayCardTrinkets(); form is reserved for successful spells.
    for (const auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect == TrinketEffect::AFTER_MAGNETIC_MECH_REPAIR)
        {
            if (!magnetic || playedRace != Race::MECHANICAL) continue;
            std::vector<Minion*> candidates;
            recruitField.ForEachAlive([&](MinionData& data) {
                if (data.value().HasRace(Race::MECHANICAL))
                    candidates.push_back(&data.value());
            });
            if (!candidates.empty())
            {
                auto& target = *candidates[Random::get<std::size_t>(
                    0, candidates.size() - 1)];
                target.SetAttack(target.GetAttack() + behavior.attack);
                target.SetHealth(target.GetHealth() + behavior.health);
            }
            continue;
        }
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
    if (season14.HasGeneratedRewardSinfallMedallion() &&
        season14.generatedRewardSinfallTier > 0)
    {
        std::vector<Minion*> candidates;
        recruitField.ForEachAlive([&](MinionData& data) {
            if (data.value().GetTier() == season14.generatedRewardSinfallTier &&
                static_cast<std::uint64_t>(data.value().GetIndex()) !=
                    season14.generatedRewardSinfallSourceEntityID)
                candidates.push_back(&data.value());
        });
        Random::shuffle(candidates.begin(), candidates.end());
        std::int32_t applied = 0;
        for (auto* target : candidates) {
            if (applied >= 2) break;
            target->SetAttack(target->GetAttack() + 2);
            target->SetHealth(target->GetHealth() + 2);
            ++applied;
        }
        season14.generatedRewardSinfallTier = 0;
        season14.generatedRewardSinfallSourceEntityID = 0;
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
        if (behavior.effect == TrinketEffect::SHIP_IN_A_BOTTLE)
        {
            // Ship in a Bottle resolves its random Pirate once, then uses
            // that same card for the hand copy and the combat summon.  This
            // keeps the two generated entities linked for deterministic
            // replay instead of drawing twice from the pool.
            std::vector<Card> pirates;
            for (const auto& candidate : Cards::GetAllCards())
                if (candidate.GetCardType() == CardType::MINION &&
                    candidate.isBattlegroundsPoolMinion &&
                    candidate.normalDbfID == 0 && candidate.hasBehavior &&
                    candidate.HasRace(Race::PIRATE))
                    pirates.push_back(candidate);
            if (pirates.empty()) continue;
            const auto& selected = pirates[
                Random::get<std::size_t>(0, pirates.size() - 1)];
            if (!hand.IsFull())
                hand.Add(CardData{Minion(selected)});
            if (battleField.IsFull()) continue;

            Minion summoned(selected);
            summoned.getPlayerCallback = [this]() -> Player& { return *this; };
            if (getNextCardIndexCallback)
                summoned.SetIndex(getNextCardIndexCallback());
            ApplyFreshMinionModifiers(summoned);
            // Insert at the left edge: Battle::FindAttacker starts from the
            // current attack cursor, so this fresh Pirate is the first legal
            // attacker and therefore receives the card's immediate attack.
            battleField.Add(summoned, 0);
            Minion& added = battleField[0];
            battleField.ForEachAlive([&added](MinionData& data) {
                data.value().ActivateTrigger(TriggerType::SUMMON, added);
            });
            ApplySummonTrinkets(added);
            continue;
        }
        if (behavior.effect == TrinketEffect::START_COMBAT_GOLDEN_FISH)
        {
            // Fishy Sticker's companion is combat-only and is omitted when
            // the copied board has no free slot.  Use the canonical golden
            // token so Battle's deathrattle-copy path remains authoritative.
            if (battleField.IsFull()) continue;
            const Card fish = Cards::FindCardByID("TB_BaconUps_307");
            if (fish.id.empty()) continue;
            Minion summoned(fish);
            summoned.getPlayerCallback = [this]() -> Player& { return *this; };
            if (getNextCardIndexCallback) summoned.SetIndex(getNextCardIndexCallback());
            battleField.Add(summoned, battleField.GetCount());
            Minion& added = battleField[battleField.GetCount() - 1];
            battleField.ForEachAlive([&added](MinionData& data) {
                data.value().ActivateTrigger(TriggerType::SUMMON, added);
            });
            ApplySummonTrinkets(added);
            continue;
        }
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
        if (behavior.effect == TrinketEffect::START_COMBAT_NAGA_DOUBLE_STATS)
        {
            std::vector<Minion*> nagas;
            battleField.ForEachAlive([&nagas](MinionData& data) {
                if (data.value().HasRace(Race::NAGA)) nagas.push_back(&data.value());
            });
            if (!nagas.empty())
            {
                nagas.front()->SetAttack(nagas.front()->GetAttack() * 2);
                nagas.front()->SetHealth(nagas.front()->GetHealth() * 2);
                if (nagas.size() > 1)
                {
                    nagas.back()->SetAttack(nagas.back()->GetAttack() * 2);
                    nagas.back()->SetHealth(nagas.back()->GetHealth() * 2);
                }
            }
            continue;
        }
        if (behavior.effect != TrinketEffect::START_COMBAT_MINION_STATS) continue;
        battleField.ForEachAlive([&behavior](MinionData& data) {
            data.value().SetAttack(data.value().GetAttack() + behavior.attack);
            data.value().SetHealth(data.value().GetHealth() + behavior.health);
        });
    }
}

void Player::OnCardDiscarded()
{
    for (const auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect != TrinketEffect::CONDUCTOR_DISCARD_BLOOD_GEM)
            continue;
        recruitField.ForEachAlive([this](MinionData& data) {
            ApplyBloodGemTo(data.value());
        });
    }
}

void Player::ResolveSpellCountTrinkets()
{
    const auto gold = season14.TakeSpellCountGold();
    if (gold > 0)
        remainCoin += gold;

    // A full hand must not consume Archaic Scroll's reward.  Retry the
    // pending reward on the next successful spell/event that reaches this
    // delivery point (or any later caller after hand space is available).
    while (season14.PendingSpellCountNagaRewards() > 0 && !hand.IsFull())
    {
        const auto before = hand.GetCount();
        (void)SimpleTasks::RandomCardToHandTask{Race::NAGA, 0, 1}.Run(*this);
        if (hand.GetCount() == before)
            break;
        season14.ConsumeSpellCountNagaReward();
    }
}

void Player::ApplySummonTrinkets(Minion& summoned)
{
    if (summoned.IsDestroyed()) return;
    if (isInCombat && season14.HasGeneratedRewardTumblingDisaster()) {
        const auto bonus = season14.GeneratedRewardTumblingBonus();
        summoned.SetAttack(summoned.GetAttack() + bonus);
        summoned.SetHealth(summoned.GetHealth() + bonus);
    }
    RecordAncestralAutomatonSummon(summoned);
    if (isInCombat) {
        battleField.ForEachAlive([&](MinionData& data) {
            auto& kodo = data.value();
            if ((kodo.GetCardID() == "BG34_322" || kodo.GetCardID() == "BG34_322_G") && kodo.ConsumeKodoSummonUse()) {
                const int scale = kodo.GetCardID() == "BG34_322_G" ? 2 : 1;
                summoned.SetAttack(summoned.GetAttack() + kodo.GetAttack() * scale);
                summoned.SetHealth(summoned.GetHealth() + kodo.GetMaxHealth() * scale);
            }
        });
    }
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
    CheckAzsharaAmbition();
}

void Player::RecordAncestralAutomatonSummon(Minion& summoned)
{
    const auto& id = summoned.GetCardID();
    if (id != "BG_TTN_401" && id != "BG_TTN_401_G") return;

    // The card's aura counts *other* copies summoned by this player before
    // this entity.  It is deliberately a lifetime counter rather than a
    // board scan: dead, hand, and combat copies still count, while an
    // opponent's summons cannot affect this player's copies.
    const bool golden = id == "BG_TTN_401_G";
    const int attackPerOther = golden ? 6 : 3;
    const int healthPerOther = golden ? 4 : 2;
    summoned.SetAttack(summoned.GetAttack() +
                       ancestralAutomatonsSummonedThisGame * attackPerOther);
    summoned.SetHealth(summoned.GetHealth() +
                       ancestralAutomatonsSummonedThisGame * healthPerOther);
    ++ancestralAutomatonsSummonedThisGame;
}

void Player::ApplyMechagnomeInterpreterBonus(Minion& target)
{
    if (target.IsDestroyed() || !target.HasRace(Race::MECHANICAL)) return;
    // Each living Interpreter is an independent aura; golden copies provide
    // the exact doubled payload.  This helper is called only after a
    // successful play/magnetization, so rejected actions cannot trigger it.
    recruitField.ForEachAlive([&target](MinionData& data) {
        const auto& id = data.value().GetCardID();
        if (id != "BG31_177" && id != "BG31_177_G") return;
        const int multiplier = id == "BG31_177_G" ? 2 : 1;
        target.SetAttack(target.GetAttack() + 3 * multiplier);
        target.SetHealth(target.GetHealth() + 1 * multiplier);
    });
}

void Player::ResolveStartTurnTrinkets()
{
    for (const auto& trinket : season14.trinkets) {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect == TrinketEffect::TAVERN_SPELL_GROWING_STATS) {
            season14.AddTavernSpellAttackBonus(behavior.attack);
            season14.AddTavernSpellHealthBonus(behavior.health);
        }
    }
    // Per-turn cadence state is reset before any start-turn grants.  Keep it
    // on each persistent effect so duplicate Cathedral copies stack exactly.
    for (auto& trinket : season14.trinkets) {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto startBehavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (startBehavior.effect == TrinketEffect::FIRST_SPELL_REPEAT ||
            startBehavior.effect == TrinketEffect::SPELLCRAFT_REPEAT)
            trinket.triggerProgress = 0;
    }
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

bool Player::ApplySpellChoice(std::size_t offeringIdx, std::size_t targetIdx)
{
    if (season14.spellModal.kind != Season14SpellModalKind::TARGET_STATS ||
        !season14.pendingTaughtSpell.pending)
        return false;
    const bool targetShop = season14.spellModal.targetShop;
    auto& zone = targetShop ? tavern.fieldZone : recruitField;
    if (targetIdx >= static_cast<std::size_t>(zone.GetCount()) ||
        (season14.spellModal.legalTargetMask & (std::uint32_t{1} << targetIdx)) == 0)
        return false;
    auto& target = zone[targetIdx];
    if (target.IsDestroyed() || target.GetCardID().empty() ||
        (season14.spellModal.legalTargetEntityIDs[targetIdx] != 0 &&
         static_cast<std::uint64_t>(target.GetIndex()) !=
             season14.spellModal.legalTargetEntityIDs[targetIdx]))
        return false;
    const auto card = Cards::FindCardByDbfID(season14.spellModal.sourceCardDbfID);
    const auto effect = FindTavernSpellBehavior(card.id);
    if (!TavernSpellRequiresTarget(effect.effect)) return false;
    ApplySpellBoardEffect(*this, effect, static_cast<int>(targetIdx), false, card.dbfID);
    season14.OnTavernSpellResolved(true, card.dbfID, !targetShop);
    ResolveSpellCountTrinkets();
    const bool resumeGeneratedRewardSpells =
        season14.HasGeneratedRewardStartTurnRandomSpells() &&
        season14.HasPendingGeneratedRewardRandomSpells();
    season14.spellModal = {};
    season14.pendingTaughtSpell = {};
    season14.pendingDecision = Season14Decision::NONE;
    if (resumeGeneratedRewardSpells)
        (void)SimpleTasks::ActivateRandomTavernSpellsTask{
            season14.GeneratedRewardRandomSpellsRemaining()}.Run(*this);
    return true;
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
    if (season14.spellModal.kind ==
        Season14SpellModalKind::BLOOD_GEM_CHOOSE_ONE)
    {
        const int extraResolutions = season14.spellModal.extraResolutionCount;
        const auto source = Cards::FindCardByDbfID(
            season14.spellModal.sourceCardDbfID);
        if (source.id != "BG31_893") return false;
        int attack = offeringIdx == 0 ? 1 : 0;
        int health = offeringIdx == 1 ? 1 : 0;
        const auto sourceDbfID = source.dbfID;
        if (!season14.SelectSpellTargetChoice(offeringIdx, attack, health))
            return false;
        for (int repeat = 0; repeat <= extraResolutions; ++repeat)
            season14.AddBloodGemBonus(attack, health);
        for (int repeat = 0; repeat <= extraResolutions; ++repeat) {
            season14.OnTavernSpellResolved(true, sourceDbfID, false);
            ResolveSpellCountTrinkets();
            ApplyTavernSpellTrinkets();
            AdvanceDarkGiftCounters(3);
        }
        return true;
    }
    if (season14.spellModal.kind == Season14SpellModalKind::ALL_MINION_STATS)
    {
        const int extraResolutions = season14.spellModal.extraResolutionCount;
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
        for (int repeat = 0; repeat < extraResolutions; ++repeat)
        {
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
        }
        // Successful casts enter the shared hook: season14.OnTavernSpellResolved(true).
        for (int repeat = 0; repeat <= extraResolutions; ++repeat) {
            season14.OnTavernSpellResolved(true, sourceDbfID, false);
            ResolveSpellCountTrinkets();
            IncrementStartCombatSpellImprovements();
            ApplyTavernSpellTrinkets();
            AdvanceDarkGiftCounters(3);
        }
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
    const int extraResolutions = season14.spellModal.extraResolutionCount;
    if (modalKind == Season14SpellModalKind::TARGET_OR_ALL_STATS &&
        offeringIdx == 1)
    {
        const int extraResolutions = season14.spellModal.extraResolutionCount;
        int attack = 0, health = 0;
        const auto sourceDbfID = season14.spellModal.sourceCardDbfID;
        if (!season14.SelectSpellTargetChoice(offeringIdx, attack, health)) return false;
        recruitField.ForEachAlive([&](MinionData& data) {
            auto& minion = data.value();
            minion.SetAttack(minion.GetAttack() + attack);
            minion.SetHealth(minion.GetHealth() + health);
        });
        for (int repeat = 0; repeat < extraResolutions; ++repeat)
            recruitField.ForEachAlive([&](MinionData& data) {
                auto& minion = data.value();
                minion.SetAttack(minion.GetAttack() + attack);
                minion.SetHealth(minion.GetHealth() + health);
            });
        for (int repeat = 0; repeat <= extraResolutions; ++repeat) {
            season14.OnTavernSpellResolved(true, sourceDbfID, true);
            ResolveSpellCountTrinkets();
            IncrementStartCombatSpellImprovements();
            ApplyTavernSpellTrinkets();
            AdvanceDarkGiftCounters(3);
        }
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
    for (int repeat = 0; repeat < extraResolutions; ++repeat)
    {
        target.SetAttack(target.GetAttack() + attack);
        target.SetHealth(target.GetHealth() + health);
        if (modalKind == Season14SpellModalKind::TARGET_OR_ALL_STATS &&
            offeringIdx == 0)
        {
            target.SetAttack(target.GetAttack() + 6);
            target.SetHealth(target.GetHealth() + 6);
        }
    }
    for (int repeat = 0; repeat <= extraResolutions; ++repeat)
        recruitField.ForEachAlive([&target](MinionData& data) {
            data.value().ActivateTrigger(TriggerType::AFTER_CAST_SPELL, target);
        });
    for (int repeat = 0; repeat <= extraResolutions; ++repeat) {
        season14.OnTavernSpellResolved(true, sourceDbfID, true);
        ResolveSpellCountTrinkets();
        IncrementStartCombatSpellImprovements();
        ApplyTavernSpellTrinkets();
        AdvanceDarkGiftCounters(3);
    }
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
                            season14.chooseOne.sourceCardDbfID == Cards::FindCardByID("BG36_330_G").dbfID ||
                            season14.chooseOne.sourceCardDbfID == Cards::FindCardByID("BG36_341_G").dbfID ||
                            season14.chooseOne.sourceCardDbfID == Cards::FindCardByID("BG31_320_G").dbfID;
        if (season14.chooseOne.sourceCardDbfID == Cards::FindCardByID("BG30_123").dbfID ||
            season14.chooseOne.sourceCardDbfID == Cards::FindCardByID("BG30_123_G").dbfID)
        {
            if (offeringIdx == 0)
                season14.AddBloodGemBonus(golden ? 2 : 1, golden ? 2 : 1);
            else
                AddBloodGems(golden ? 8 : 4);
        }
        else if (season14.chooseOne.sourceCardDbfID == Cards::FindCardByID("BG31_320").dbfID ||
                 season14.chooseOne.sourceCardDbfID == Cards::FindCardByID("BG31_320_G").dbfID)
        {
            if (offeringIdx == 0) AddBloodGems(golden ? 4 : 2);
            else gemDays += golden ? 2 : 1;
        }
        else if (season14.chooseOne.sourceCardDbfID == Cards::FindCardByID("BG32_237").dbfID ||
                 season14.chooseOne.sourceCardDbfID == Cards::FindCardByID("BG32_237_G").dbfID)
        {
            if (offeringIdx == 0) season14.AddTavernSpellAttackBonus(golden ? 2 : 1);
            else season14.AddTavernSpellHealthBonus(golden ? 2 : 1);
        }
        else if (season14.chooseOne.sourceCardDbfID == Cards::FindCardByID("BG36_330").dbfID ||
                 season14.chooseOne.sourceCardDbfID == Cards::FindCardByID("BG36_330_G").dbfID)
        {
            if (offeringIdx == 0)
                season14.AddFreeRefreshes(golden ? 4 : 2);
            else
                AddBloodGems(golden ? 6 : 3);
        }
        else if (season14.chooseOne.sourceCardDbfID == Cards::FindCardByID("BG36_341").dbfID ||
                 season14.chooseOne.sourceCardDbfID == Cards::FindCardByID("BG36_341_G").dbfID)
        {
            const int amount = golden ? 6 : 3;
            if (offeringIdx == 0) {
                recruitField.ForEachAlive([this, amount](MinionData& data) {
                    for (int i = 0; i < amount; ++i) ApplyBloodGemTo(data.value());
                });
            } else {
                for (int cast = 0; cast < amount; ++cast) {
                    std::vector<int> candidates;
                    recruitField.ForEachAlive([&candidates](MinionData& data) {
                        candidates.push_back(data.value().GetZonePosition());
                    });
                    Random::shuffle(candidates.begin(), candidates.end());
                    const int count = std::min<int>(3, candidates.size());
                    for (int i = 0; i < count; ++i)
                        ApplyBloodGemTo(recruitField[static_cast<std::size_t>(candidates[i])]);
                }
            }
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
        const auto sourceID = season14.chooseOne.sourceEntityID;
        int shakerGems = 0;
        recruitField.ForEachAlive([&](MinionData& data) { if (data.value().GetCardID() == "BG31_323") shakerGems = std::max(shakerGems, 1); else if (data.value().GetCardID() == "BG31_323_G") shakerGems = std::max(shakerGems, 2); });
        if (shakerGems > 0) recruitField.ForEachAlive([&](MinionData& data) {
            auto& other = data.value();
            if (static_cast<std::uint64_t>(other.GetIndex()) != sourceID && other.HasRace(Race::QUILBOAR))
                for (int i = 0; i < shakerGems; ++i) ApplyBloodGemTo(other);
        });
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
    const auto sourceID = season14.chooseOne.sourceEntityID;
    int shakerGems = 0;
    recruitField.ForEachAlive([&](MinionData& data) { if (data.value().GetCardID() == "BG31_323") shakerGems = std::max(shakerGems, 1); else if (data.value().GetCardID() == "BG31_323_G") shakerGems = std::max(shakerGems, 2); });
    if (shakerGems > 0) recruitField.ForEachAlive([&](MinionData& data) {
        auto& other = data.value();
        if (other.GetIndex() != static_cast<int>(sourceID) && other.HasRace(Race::QUILBOAR))
            for (int i = 0; i < shakerGems; ++i) ApplyBloodGemTo(other);
    });
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

bool Player::ResolveSneedShredderDeathrattle(bool golden)
{
    const int wanted = golden ? 2 : 1;
    const int available = static_cast<int>(MAX_FIELD_SIZE) - battleField.GetCount();
    // A golden Shredder requests two rolls, but a nearly-full combat board
    // still resolves every roll that has room to summon.  Reject only when
    // no slot is available; requiring both slots would incorrectly turn a
    // valid one-summon resolution into a failed deathrattle.
    if (available <= 0) return false;
    // Sneed's Replicator says "random minion from a Tavern Tier lower".
    // Select from the canonical supported Battlegrounds pool rather than
    // consuming a hand card (the old implementation accidentally did the
    // latter and granted Divine Shield).  Keep each roll independent, as a
    // golden Shredder resolves its deathrattle twice.
    std::vector<Card> candidates;
    for (const auto& card : Cards::GetAllCards())
    {
        if (card.GetCardType() != CardType::MINION ||
            !card.isBattlegroundsPoolMinion || card.normalDbfID != 0 ||
            !card.hasBehavior || card.GetTier() < 1 ||
            card.GetTier() >= currentTier)
            continue;
        candidates.push_back(card);
    }
    if (candidates.empty()) return false;
    int summoned = 0;
    for (int n = 0; n < wanted && !battleField.IsFull(); ++n)
    {
        const auto& card = candidates[
            Random::get<std::size_t>(0, candidates.size() - 1)];
        Minion minion(card);
        ApplyFreshMinionModifiers(minion);
        if (!SummonCombatSnapshot(std::move(minion))) break;
        ++summoned;
    }
    return summoned != 0;
}

bool Player::ResolveDoubleTimeCopies()
{
    const bool pilferedLamps = season14.HasGeneratedRewardPilferedLamps();
    if (season14.heroPowerDbfID != 126533 && !pilferedLamps) return false;
    // Double Time's pair completion grants a Tavern Coin and therefore keeps
    // the historical full-hand guard.  Pilfered Lamps is a different rule:
    // two copies simply become one Golden minion, so a full hand must not
    // prevent a pair that lives on the board from resolving.
    if (!pilferedLamps && hand.IsFull()) return false;

    auto family = [](const Minion& minion) {
        const auto card = Cards::FindCardByDbfID(minion.GetDbfID());
        return card.normalDbfID == 0 ? card.dbfID : card.normalDbfID;
    };
    std::vector<int> handMatches;
    std::vector<int> boardMatches;
    for (int i = 0; i < hand.GetCount(); ++i)
    {
        if (std::holds_alternative<Minion>(hand[i]))
        {
            const auto& minion = std::get<Minion>(hand[i]);
            if (!minion.IsGolden()) handMatches.push_back(i);
        }
    }
    for (int i = 0; i < recruitField.GetCount(); ++i)
    {
        const auto& minion = recruitField[i];
        if (!minion.IsDestroyed() && !minion.IsGolden())
            boardMatches.push_back(i);
    }

    // Resolve at most one family per acquisition event.  This keeps the
    // operation idempotent when observers call it more than once.
    std::vector<int> candidates;
    for (const int index : handMatches)
    {
        const auto& minion = std::get<Minion>(hand[index]);
        const int key = family(minion);
        const auto count = static_cast<int>(std::count_if(
            handMatches.begin(), handMatches.end(), [&](int other) {
                return family(std::get<Minion>(hand[other])) == key;
            })) + static_cast<int>(std::count_if(
                boardMatches.begin(), boardMatches.end(), [&](int other) {
                    return family(recruitField[other]) == key;
                }));
        if (count >= 2) candidates.push_back(key);
    }
    for (const int index : boardMatches)
    {
        const int key = family(recruitField[index]);
        const auto count = static_cast<int>(std::count_if(
            handMatches.begin(), handMatches.end(), [&](int other) {
                return family(std::get<Minion>(hand[other])) == key;
            })) + static_cast<int>(std::count_if(
                boardMatches.begin(), boardMatches.end(), [&](int other) {
                    return family(recruitField[other]) == key;
                }));
        if (count >= 2) candidates.push_back(key);
    }
    if (candidates.empty()) return false;
    const int key = candidates.front();
    if (!handMatches.empty() && family(std::get<Minion>(hand[handMatches.front()])) == key)
    {
        auto& first = std::get<Minion>(hand[handMatches.front()]);
        if (!first.MakeGolden()) return false;
        for (int i = hand.GetCount() - 1; i >= 0; --i)
        {
            if (i == handMatches.front() || !std::holds_alternative<Minion>(hand[i])) continue;
            if (family(std::get<Minion>(hand[i])) == key)
            {
                hand.Remove(hand[i]);
                if (!pilferedLamps) AddTavernCoins(1);
                return true;
            }
        }
        for (int i = recruitField.GetCount() - 1; i >= 0; --i)
        {
            if (!recruitField[i].IsDestroyed() && !recruitField[i].IsGolden() && family(recruitField[i]) == key)
            {
                recruitField.Remove(recruitField[i]);
                if (!pilferedLamps) AddTavernCoins(1);
                return true;
            }
        }
    }
    for (int i = recruitField.GetCount() - 1; i >= 0; --i)
    {
        if (recruitField[i].IsDestroyed() || recruitField[i].IsGolden() || family(recruitField[i]) != key) continue;
        auto& first = recruitField[i];
        if (!first.MakeGolden()) return false;
        for (int j = i - 1; j >= 0; --j)
        {
            if (!recruitField[j].IsDestroyed() && !recruitField[j].IsGolden() && family(recruitField[j]) == key)
            {
                recruitField.Remove(recruitField[j]);
                if (!pilferedLamps) AddTavernCoins(1);
                return true;
            }
        }
    }
    return false;
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
    if (behavior.effect == TrinketEffect::ACQUIRE_FIXED_CARD ||
        behavior.effect == TrinketEffect::ACQUIRE_FIXED_GLOWSCALE ||
        behavior.effect == TrinketEffect::ACQUIRE_FIXED_LIONFISH) {
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
    if (behavior.effect == TrinketEffect::TAVERN_SPELL_STATS ||
        behavior.effect == TrinketEffect::TAVERN_SPELL_GROWING_STATS) {
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
    // Acquisition-time grants are resolved exactly once below, alongside
    // the other executable Trinket effects.  Keeping a single dispatch here
    // is important: Compass/Pendant and fixed-card effects may also repeat
    // at recruit start, but their initial grant must not be doubled.
    if (behavior.effect == TrinketEffect::BOOK_OF_MEDIVH_DISCOVER)
        (void)BeginBookOfMedivhDiscover(*this, card.dbfID, behavior.value);
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
    else if (behavior.effect == TrinketEffect::ACQUIRE_RANDOM_FRIENDLY_COPY)
        (void)AddRandomFriendlyMinionCopyToHand();
    else if ((behavior.effect == TrinketEffect::ACQUIRE_FIXED_CARD ||
              behavior.effect == TrinketEffect::ACQUIRE_FIXED_GLOWSCALE ||
              behavior.effect == TrinketEffect::ACQUIRE_FIXED_LIONFISH ||
              behavior.effect == TrinketEffect::CONDUCTOR_DISCARD_BLOOD_GEM) &&
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
    else if (behavior.effect == TrinketEffect::BRONZEBEARD_PORTRAIT &&
             hand.GetCount() < MAX_HAND_SIZE)
    {
        // Bronzebeard Portrait grants Brann carrying both Murloc and Dragon
        // tribes, plus one random Battlecry minion.  The linked `_045t`
        // entity is the authoritative dual-tribe variant in the card data.
        const auto brann = Cards::FindCardByID("TB_BaconUps_045t");
        if (brann.dbfID != 0)
            hand.Add(CardData{Minion(brann)});
        if (!hand.IsFull())
            (void)SimpleTasks::RandomCardToHandTask{
                Race::INVALID, 0, 1, false, true}.Run(*this);
    }
    return true;
}

int Player::GrantTrinketStartTurnCards()
{
    int added = 0;
    for (auto& trinket : season14.trinkets) {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect == TrinketEffect::BOOK_OF_MEDIVH_DISCOVER)
        {
            const auto count = season14.bookOfMedivhRemaining > 0
                                   ? season14.bookOfMedivhRemaining
                                   : behavior.value;
            (void)BeginBookOfMedivhDiscover(*this, trinket.dbfID, count);
            continue;
        }
        if (behavior.effect == TrinketEffect::AZEROTH_MODEL_GLOBE)
        {
            if (++trinket.triggerProgress < behavior.value) continue;
            std::vector<Card> candidates;
            for (const auto& candidate : Cards::GetTier6Minions())
                if (!candidate.id.empty() && candidate.isBattlegroundsPoolMinion &&
                    candidate.hasBehavior &&
                    candidate.normalDbfID == 0 &&
                    candidate.GetCardType() == CardType::MINION)
                    candidates.push_back(candidate);
            if (hand.IsFull() || candidates.empty()) continue;
            Random::shuffle(candidates.begin(), candidates.end());
            const auto count = std::min<std::size_t>(3, candidates.size());
            std::vector<Season14Offering> offerings;
            offerings.reserve(count);
            for (std::size_t i = 0; i < count; ++i)
                offerings.push_back({candidates[i].dbfID, 0});
            remainCoin += behavior.amount;
            season14.BeginOfferingDecision(Season14Decision::DISCOVER, 0,
                                           trinket.dbfID, std::move(offerings));
            trinket.triggerProgress = 0;
            continue;
        }
        if (behavior.effect != TrinketEffect::START_TURN_RANDOM_MINIONS &&
            !(behavior.effect == TrinketEffect::ACQUIRE_RANDOM_MINIONS &&
              behavior.repeatAtStartTurn) &&
            !(behavior.effect == TrinketEffect::ACQUIRE_RANDOM_FRIENDLY_COPY &&
              behavior.repeatAtStartTurn) &&
            !(behavior.effect == TrinketEffect::ACQUIRE_LAST_OPPONENT_COPY &&
              behavior.repeatAtStartTurn) &&
            !(behavior.effect == TrinketEffect::ACQUIRE_FIXED_CARD &&
              behavior.repeatAtStartTurn)) continue;
        if (behavior.effect == TrinketEffect::ACQUIRE_FIXED_CARD) {
            if (behavior.cardID.empty() || hand.IsFull()) continue;
            const Card generated = Cards::FindCardByID(behavior.cardID);
            if (generated.dbfID == 0) continue;
            const auto before = hand.GetCount();
            const int startTurnAmount = behavior.startTurnAmount > 0
                                            ? behavior.startTurnAmount
                                            : behavior.amount;
            for (int i = 0; i < startTurnAmount && !hand.IsFull(); ++i)
            {
                if (generated.GetCardType() == CardType::SPELL)
                    hand.Add(CardData{Spell(generated)});
                else if (generated.GetCardType() == CardType::MINION)
                    hand.Add(CardData{Minion(generated)});
            }
            added += hand.GetCount() - before;
            continue;
        }
        if (behavior.effect == TrinketEffect::ACQUIRE_RANDOM_FRIENDLY_COPY) {
            const auto before = hand.GetCount();
            (void)AddRandomFriendlyMinionCopyToHand();
            added += hand.GetCount() - before;
            continue;
        }
        if (behavior.effect == TrinketEffect::ACQUIRE_LAST_OPPONENT_COPY) {
            const auto before = hand.GetCount();
            (void)AddHighestLastOpponentMinionCopyToHand();
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

std::vector<Card> SupportedEndTurnMinions()
{
    std::vector<Card> result;
    const auto append = [&result](const auto& cards) {
        for (const auto& card : cards) {
            if (card.id.empty() || !card.hasBehavior ||
                card.normalDbfID != 0 ||
                card.GetCardType() != CardType::MINION)
                continue;
            auto def = CardDefs::FindCardDefByID(card.id);
            const auto& trigger = def.power.GetTrigger();
            if (trigger.has_value() &&
                trigger->GetTriggerType() == TriggerType::TURN_END)
                result.push_back(card);
        }
    };
    append(Cards::GetTier1Minions()); append(Cards::GetTier2Minions());
    append(Cards::GetTier3Minions()); append(Cards::GetTier4Minions());
    append(Cards::GetTier5Minions()); append(Cards::GetTier6Minions());
    append(Cards::GetTier7Minions());
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
    for (const auto id : {"BG27_084", "BG30_123", "BG36_330", "BG36_341"}) {
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
                         std::int32_t sourceCardDbfID, bool lockHand)
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

bool BeginWindfallDiscover(Player& player, std::int32_t sourceCardDbfID,
                           std::int32_t attack, std::int32_t health,
                           std::int32_t remaining)
{
    if (remaining <= 0 || player.hand.IsFull() ||
        player.season14.pendingDecision != Season14Decision::NONE)
        return false;
    std::vector<Card> candidates;
    for (const auto& card : Cards::GetAllCards())
        if (card.isBattlegroundsPoolMinion && card.hasBehavior &&
            card.normalDbfID == 0 && card.GetCardType() == CardType::MINION &&
            card.HasRace(Race::ELEMENTAL))
            candidates.push_back(card);
    if (candidates.empty()) return false;
    Random::shuffle(candidates.begin(), candidates.end());
    const auto count = std::min<std::size_t>(3, candidates.size());
    std::vector<Season14Offering> offerings;
    offerings.reserve(count);
    for (std::size_t i = 0; i < count; ++i)
        offerings.push_back({candidates[i].dbfID, 0});
    player.season14.windfallAttack = attack;
    player.season14.windfallHealth = health;
    player.season14.windfallRemaining = remaining - 1;
    player.season14.BeginOfferingDecision(Season14Decision::DISCOVER, 0,
                                          sourceCardDbfID, std::move(offerings));
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
    const auto armDiscoverReplay = [&]() {
        if (sourceCardDbfID <= 0) return;
        if (effect.effect != TavernSpellEffect::DISCOVER_MINION &&
            effect.effect != TavernSpellEffect::DISCOVER_BATTLECRY_MINION &&
            effect.effect != TavernSpellEffect::DISCOVER_DIFFERENT_RACE &&
            effect.effect != TavernSpellEffect::DISCOVER_HERO_POWER &&
            effect.effect != TavernSpellEffect::DISCOVER_CHOOSE_ONE_COMBINED &&
            effect.effect != TavernSpellEffect::DISCOVER_UNDEAD_DIES_THIS_TURN)
            return;
        std::int32_t extra = 0;
        for (auto& trinket : player.season14.trinkets) {
            if (!trinket.active || trinket.remainingUses == 0) continue;
            const auto behavior = FindTrinketBehavior(
                Cards::FindCardByDbfID(trinket.dbfID).id);
            if (behavior.effect == TrinketEffect::FIRST_SPELL_REPEAT &&
                trinket.triggerProgress == 0)
            {
                ++trinket.triggerProgress;
                ++extra;
            }
            if (temporary && behavior.effect == TrinketEffect::SPELLCRAFT_REPEAT &&
                trinket.triggerProgress < behavior.value)
            {
                ++trinket.triggerProgress;
                ++extra;
            }
        }
        if (extra > 0) {
            player.season14.discoverReplayRemaining = extra;
            player.season14.discoverReplaySourceSpellDbfID = sourceCardDbfID;
            player.season14.discoverReplayTargetEntityID =
                targetIdx >= 0 && targetIdx < player.recruitField.GetCount()
                    ? static_cast<std::uint64_t>(player.recruitField[
                          static_cast<std::size_t>(targetIdx)].GetIndex()) : 0;
        }
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

            // Groundshaker's linked enchantment is a temporary +2 Attack
            // payload on every other friendly minion.  Resolve it through the
            // shared typed lifecycle so it expires at the next recruit start.
            if (target.GetCardID() == "BG20_106")
            {
                player.recruitField.ForEachAlive(
                    [&target](MinionData& data) {
                        Minion& observer = data.value();
                        if (&observer != &target)
                            observer.ApplyTemporaryEnchantment(
                                Minion::TemporaryEnchantment::Stats, 2, 0);
                    });
            }

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

            // Tough Tusk Sticker grants a temporary shield to every minion
            // receiving a Blood Gem, without changing the gem's stats.
            if (sourceCardDbfID > 0)
            {
                for (const auto& trinket : player.season14.trinkets)
                {
                    if (!trinket.active || trinket.remainingUses == 0) continue;
                    if (FindTrinketBehavior(
                            Cards::FindCardByDbfID(trinket.dbfID).id).effect ==
                        TrinketEffect::BLOOD_GEM_DIVINE_SHIELD)
                        target.ApplyTemporaryKeyword(GameTag::DIVINE_SHIELD);
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
                minion.ApplyTemporaryEnchantment(
                    Minion::TemporaryEnchantment::Stats, effect.attack,
                    effect.health);
            else
            {
                minion.SetAttack(minion.GetAttack() + effect.attack);
                minion.SetHealth(minion.GetHealth() + effect.health);
                // Spellcraft tokens such as Ophidian Staff grant Reborn as
                // part of the recruit-phase enchantment. The old branch
                // only attached the keyword for temporary casts.
                if (effect.effect == TavernSpellEffect::TARGET_STATS_AND_REBORN &&
                    (effect.race == Race::INVALID || minion.HasRace(effect.race)))
                    minion.SetReborn(true);
            }
            return;
        }
        case TavernSpellEffect::SHOP_STATS_TO_RANDOM_FRIENDLY:
        {
            if (targetIdx < 0 ||
                targetIdx >= player.tavern.fieldZone.GetCount() ||
                player.recruitField.GetCount() == 0)
                return;
            const Minion& source = player.tavern.fieldZone[
                static_cast<std::size_t>(targetIdx)];
            if (source.GetCardID().empty()) return;
            std::vector<Minion*> recipients;
            player.recruitField.ForEachAlive([&recipients](MinionData& data) {
                recipients.push_back(&data.value());
            });
            if (recipients.empty()) return;
            Random::shuffle(recipients.begin(), recipients.end());
            Minion& target = *recipients.front();
            const int multiplier = effect.value > 0 ? effect.value : 1;
            target.ApplyTemporaryEnchantment(
                Minion::TemporaryEnchantment::Stats,
                multiplier * source.GetAttack(), multiplier * source.GetHealth());
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
                minion.ApplyTemporaryEnchantment(
                    Minion::TemporaryEnchantment::StatsAndTaunt, effect.attack,
                    effect.health);
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
                minion.ApplyTemporaryEnchantment(
                    Minion::TemporaryEnchantment::Stats, effect.attack,
                    effect.health);
                if (minion.HasRace(effect.race))
                    minion.ApplyTemporaryEnchantment(
                        effect.effect == TavernSpellEffect::TARGET_STATS_AND_WINDFURY
                            ? Minion::TemporaryEnchantment::StatsAndWindfury
                            : Minion::TemporaryEnchantment::StatsAndReborn,
                        0, 0);
            }
            else
            {
                minion.SetAttack(minion.GetAttack() + effect.attack);
                minion.SetHealth(minion.GetHealth() + effect.health);
            }
            return;
        }
        case TavernSpellEffect::TARGET_DIVINE_SHIELD_AND_WINDFURY:
        {
            Minion& minion = player.recruitField[static_cast<std::size_t>(targetIdx)];
            // Rushing Winds is a Spellcraft token.  Its keyword grants expire
            // with the recruit turn, while an ordinary permanent cast of the
            // same typed effect must retain both keywords.  Applying the
            // temporary keyword bookkeeping here also preserves pre-existing
            // Divine Shield/Windfury instead of clearing it on expiry.
            if (temporary) {
                minion.ApplyTemporaryKeyword(GameTag::DIVINE_SHIELD);
                minion.ApplyTemporaryKeyword(GameTag::WINDFURY);
            } else {
                minion.SetGameTag(GameTag::DIVINE_SHIELD, 1);
                minion.SetGameTag(GameTag::WINDFURY, 1);
            }
            return;
        }
        case TavernSpellEffect::TARGET_STATS_AND_STEALTH:
        {
            Minion& minion = player.recruitField[static_cast<std::size_t>(targetIdx)];
            if (temporary)
                minion.ApplyTemporaryEnchantment(
                    Minion::TemporaryEnchantment::StatsAndStealth,
                    effect.attack, effect.health);
            else
            {
                minion.SetAttack(minion.GetAttack() + effect.attack);
                minion.SetHealth(minion.GetHealth() + effect.health);
                minion.SetGameTag(GameTag::STEALTH, 1);
            }
            return;
        }
        case TavernSpellEffect::TARGET_DIVINE_SHIELD_TEMP:
        {
            Minion& minion = player.recruitField[static_cast<std::size_t>(targetIdx)];
            if (temporary)
                minion.ApplyTemporaryEnchantment(
                    Minion::TemporaryEnchantment::DivineShield);
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
        case TavernSpellEffect::TARGET_GOLDEN_TEMPORARY:
        {
            static_cast<void>(player.recruitField[
                static_cast<std::size_t>(targetIdx)]
                .MakeGoldenUntilNextTurn());
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
        case TavernSpellEffect::RANDOM_NAGA_MINION_TO_HAND:
        {
            std::vector<Card> candidates = SupportedMinionsForRace(Race::NAGA);
            if (effect.value > 0)
                candidates.erase(std::remove_if(candidates.begin(), candidates.end(),
                    [&](const Card& card) { return card.GetTier() != effect.value; }),
                    candidates.end());
            static_cast<void>(AddRandomMinionToHand(player, candidates));
            if (effect.randomCount > 1)
                static_cast<void>(AddRandomMinionToHand(player, candidates));
            return;
        }
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
            if (BeginMinionDiscover(player, std::move(candidates), sourceCardDbfID, effect.lockHand))
                armDiscoverReplay();
            return;
        }
        case TavernSpellEffect::DISCOVER_BATTLECRY_MINION:
            if (BeginMinionDiscover(player, SupportedBattlecryMinions(), sourceCardDbfID))
                armDiscoverReplay();
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
            if (BeginMinionDiscover(player, std::move(candidates), sourceCardDbfID))
                armDiscoverReplay();
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
        case TavernSpellEffect::TARGET_SHOP_COPY:
        case TavernSpellEffect::TARGET_SHOP_COPY_TIER:
        {
            if (targetIdx < 0 || targetIdx >= player.tavern.fieldZone.GetCount()) return;
            const auto& shop = player.tavern.fieldZone[static_cast<std::size_t>(targetIdx)];
            if (shop.IsDestroyed() || shop.GetCardID().empty() ||
                player.hand.GetCount() + effect.value > MAX_HAND_SIZE) return;
            if (effect.effect == TavernSpellEffect::TARGET_SHOP_COPY_TIER &&
                shop.GetTier() > 3) return;
            for (int copy = 0; copy < effect.value; ++copy) {
                Minion generated{Cards::FindCardByID(shop.GetCardID())};
                generated.SetAttack(shop.GetAttack());
                generated.SetHealth(shop.GetHealth());
                player.ApplyFreshMinionModifiers(generated);
                player.hand.Add(CardData{std::move(generated)});
            }
            return;
        }
        case TavernSpellEffect::TARGET_SHOP_MOVE_NON_GOLDEN:
        {
            if (targetIdx < 0 || targetIdx >= player.tavern.fieldZone.GetCount() ||
                player.hand.IsFull()) return;
            auto& source = player.tavern.fieldZone[static_cast<std::size_t>(targetIdx)];
            if (source.IsDestroyed() || source.IsGolden()) return;
            auto moved = player.tavern.fieldZone.Remove(source);
            player.hand.Add(CardData{std::move(moved)});
            return;
        }
        case TavernSpellEffect::TARGET_DOUBLE_STATS_HAND_LOCK:
        {
            if (targetIdx < 0 || targetIdx >= player.recruitField.GetCount() ||
                player.hand.IsFull()) return;
            auto& target = player.recruitField[static_cast<std::size_t>(targetIdx)];
            target.SetAttack(target.GetAttack() * 2);
            target.SetHealth(target.GetHealth() * 2);
            auto moved = player.recruitField.Remove(target);
            moved.SetHandLocked(true);
            player.hand.Add(CardData{std::move(moved)});
            return;
        }
        case TavernSpellEffect::TARGET_TRIGGER_DEATHRATTLE:
            if (targetIdx >= 0 && targetIdx < player.recruitField.GetCount())
                player.recruitField[static_cast<std::size_t>(targetIdx)].ActivateTask(
                    PowerType::DEATHRATTLE, player);
            return;
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
            armDiscoverReplay();
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
        case TavernSpellEffect::STEAL_RANDOM_SHOP_RACE:
        {
            std::vector<int> candidates;
            player.tavern.fieldZone.ForEach([&](const MinionData& minion) {
                if (!minion.value().IsDestroyed() &&
                    minion.value().HasRace(effect.race))
                    candidates.push_back(minion.value().GetZonePosition());
            });
            Random::shuffle(candidates.begin(), candidates.end());
            if (!candidates.empty() && !player.hand.IsFull()) {
                Minion& source = player.tavern.fieldZone[static_cast<std::size_t>(candidates.front())];
                Minion stolen = player.tavern.fieldZone.Remove(source);
                player.hand.Add(CardData{std::move(stolen)});
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
        case TavernSpellEffect::DESTROY_UNDEAD_RANDOM_TO_HAND:
        {
            if (targetIdx < 0 || targetIdx >= player.recruitField.GetCount()) return;
            auto& target = player.recruitField[static_cast<std::size_t>(targetIdx)];
            if (!target.HasRace(Race::UNDEAD) || player.hand.IsFull()) return;
            player.recruitField.Remove(target);
            auto candidates = SupportedMinionsForRace(Race::UNDEAD);
            if (candidates.empty()) return;
            for (int i = 0; i < effect.randomCount && !player.hand.IsFull(); ++i) {
                Random::shuffle(candidates.begin(), candidates.end());
                player.hand.Add(CardData{Minion(candidates.front())});
            }
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
        case TavernSpellEffect::GAIN_GOLD:
            player.remainCoin += effect.gold;
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
        case TavernSpellEffect::TARGET_STATS_NEXT_TURN:
        {
            if (targetIdx < 0 || targetIdx >= player.recruitField.GetCount())
                return;
            auto& target = player.recruitField[static_cast<std::size_t>(targetIdx)];
            target.ApplyTemporaryEnchantment(Minion::TemporaryEnchantment::Stats,
                                              effect.attack, effect.health);
            return;
        }
        case TavernSpellEffect::TAVERN_SPELL_STATS_PERMANENT:
            player.season14.AddTavernSpellAttackBonus(effect.attack);
            player.season14.AddTavernSpellHealthBonus(effect.health);
            return;
        case TavernSpellEffect::RANDOM_STAT_TAVERN_SPELL:
        {
            if (player.hand.IsFull()) return;
            std::vector<Card> candidates;
            for (const auto& candidate : Cards::GetAllCards())
            {
                const auto behavior = FindTavernSpellBehavior(candidate.id);
                if (candidate.isBattlegroundsPoolSpell &&
                    candidate.normalDbfID == 0 && candidate.hasBehavior &&
                    behavior.effect != TavernSpellEffect::NONE &&
                    (TavernSpellReceivesHealthBonus(behavior.effect) ||
                     TavernSpellReceivesAttackBonus(behavior.effect)))
                    candidates.push_back(candidate);
            }
            if (candidates.empty()) return;
            Random::shuffle(candidates.begin(), candidates.end());
            const auto amount = std::min<int>(effect.randomCount,
                                              MAX_HAND_SIZE - player.hand.GetCount());
            for (int i = 0; i < amount; ++i)
                player.hand.Add(CardData{Spell(candidates[static_cast<std::size_t>(i)])});
            return;
        }
        case TavernSpellEffect::TARGET_RANDOM_RACE_KEYWORD:
        {
            if (targetIdx < 0 || targetIdx >= player.recruitField.GetCount())
                return;
            auto& target = player.recruitField[static_cast<std::size_t>(targetIdx)];
            if (effect.race != Race::INVALID && !target.HasRace(effect.race))
                return;
            constexpr GameTag keywords[] = {GameTag::DIVINE_SHIELD,
                GameTag::REBORN, GameTag::WINDFURY, GameTag::VENOMOUS,
                GameTag::TAUNT};
            std::vector<GameTag> eligible;
            for (const auto keyword : keywords)
            {
                const bool present = keyword == GameTag::DIVINE_SHIELD
                    ? target.HasDivineShield()
                    : keyword == GameTag::REBORN ? target.HasReborn()
                    : keyword == GameTag::WINDFURY ? target.HasWindfury()
                    : keyword == GameTag::VENOMOUS ? target.HasVenomous()
                    : target.HasTaunt();
                if (!present) eligible.push_back(keyword);
            }
            if (eligible.empty()) return;
            const auto keyword = eligible[Random::get<std::size_t>(
                0, eligible.size() - 1)];
            if (keyword == GameTag::TAUNT) target.SetTaunt(true);
            else if (keyword == GameTag::REBORN) target.SetReborn(true);
            else if (keyword == GameTag::VENOMOUS)
                target.SetGameTag(GameTag::POISONOUS, 1);
            else target.SetGameTag(keyword, 1);
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
    if (targetIdx < 0 && TavernSpellRequiresTarget(behavior.effect) &&
        season14.pendingTaughtSpell.pending)
    {
        const bool targetShop = TavernSpellTargetsShop(behavior.effect);
        season14.BeginSpellTargetChoice(card.dbfID, -1, 0, behavior.attack,
                                        behavior.health, behavior.attack,
                                        behavior.health, {},
                                        Season14SpellModalKind::TARGET_STATS);
        season14.spellModal.targetShop = targetShop;
        season14.spellModal.legalTargetMask = 0;
        std::size_t slot = 0;
        if (targetShop)
        {
            tavern.fieldZone.ForEach([&](MinionData& data) {
                const auto& target = data.value();
                if (slot < 7 && !target.IsDestroyed() &&
                    !target.GetCardID().empty() &&
                    TavernSpellTargetIsLegal(behavior.effect, target.GetTier(),
                                             target.IsGolden()))
                {
                    season14.spellModal.legalTargetMask |=
                        std::uint32_t{1} << slot;
                    season14.spellModal.legalTargetEntityIDs[slot] =
                        static_cast<std::uint64_t>(target.GetIndex());
                }
                ++slot;
            });
        }
        else
        {
            GetField().ForEachAlive([&](MinionData& data) {
                if (slot < 7) {
                    season14.spellModal.legalTargetMask |=
                        std::uint32_t{1} << slot;
                    season14.spellModal.legalTargetEntityIDs[slot] =
                        static_cast<std::uint64_t>(data.value().GetIndex());
                }
                ++slot;
            });
        }
        return season14.spellModal.legalTargetMask != 0;
    }
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
    if (spell.GetID() == "BG30_MagicItem_714t")
        return targetIdx >= 0 && ValidFriendlyBoardTarget(*this, targetIdx);
    const Spell playedSpell = spell;
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
    const bool shopTarget = behavior.effect ==
                            TavernSpellEffect::SHOP_STATS_TO_RANDOM_FRIENDLY ||
                            behavior.effect == TavernSpellEffect::TARGET_SHOP_COPY ||
                            behavior.effect == TavernSpellEffect::TARGET_SHOP_COPY_TIER ||
                            behavior.effect == TavernSpellEffect::TARGET_SHOP_MOVE_NON_GOLDEN;
    if (behavior.gold < 0 ||
        TavernSpellRequiresTarget(behavior.effect) != (targetIdx >= 0) ||
        (shopTarget && (targetIdx < 0 || targetIdx >= tavern.fieldZone.GetCount() ||
                        tavern.fieldZone[static_cast<std::size_t>(targetIdx)].GetCardID().empty())))
    {
        return false;
    }
    if (targetIdx >= 0 && shopTarget &&
        (targetIdx >= tavern.fieldZone.GetCount() ||
         tavern.fieldZone[static_cast<std::size_t>(targetIdx)].IsDestroyed() ||
         tavern.fieldZone[static_cast<std::size_t>(targetIdx)].GetCardID().empty()))
        return false;
    if (behavior.effect == TavernSpellEffect::SHOP_STATS_TO_RANDOM_FRIENDLY &&
        recruitField.GetCount() == 0)
        return false;
    if (targetIdx >= 0 && behavior.effect == TavernSpellEffect::TARGET_SHOP_COPY_TIER &&
        tavern.fieldZone[static_cast<std::size_t>(targetIdx)].GetTier() > 3)
    {
        // Duplicating Lens is capped at Tavern Tier 3. Reject before paying
        // the spell so a Tier 4+ target cannot consume it and do nothing.
        return false;
    }
    if (targetIdx >= 0 && behavior.effect == TavernSpellEffect::TARGET_SHOP_MOVE_NON_GOLDEN &&
        (tavern.fieldZone[static_cast<std::size_t>(targetIdx)].IsGolden() ||
         hand.IsFull()))
        return false;
    if (targetIdx >= 0 && !shopTarget && !ValidFriendlyBoardTarget(*this, targetIdx))
    {
        return false;
    }
    if (targetIdx >= 0 && !shopTarget)
    {
        const Minion& target = recruitField[static_cast<std::size_t>(targetIdx)];
        if (behavior.effect == TavernSpellEffect::TARGET_CONSUME_SHOP_STATS &&
            !target.HasRace(behavior.race))
        {
            return false;
        }
        if (behavior.effect == TavernSpellEffect::TARGET_STATS_AND_REBORN &&
            behavior.race != Race::INVALID && !target.HasRace(behavior.race))
        {
            return false;
        }
        if (behavior.effect == TavernSpellEffect::TARGET_RANDOM_RACE_KEYWORD &&
            behavior.race != Race::INVALID && !target.HasRace(behavior.race))
        {
            return false;
        }
        if (behavior.effect == TavernSpellEffect::TARGET_DOUBLE_STATS_HAND_LOCK &&
            hand.IsFull())
            return false;
        if (behavior.effect == TavernSpellEffect::TARGET_TRIGGER_DEATHRATTLE &&
            !target.HasDeathrattle())
            return false;
        if (behavior.effect == TavernSpellEffect::DESTROY_UNDEAD_RANDOM_TO_HAND &&
            (!target.HasRace(Race::UNDEAD) || hand.IsFull()))
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
        if (behavior.effect == TavernSpellEffect::TARGET_GOLDEN_TEMPORARY)
        {
            if (!target.CanMakeGolden()) return false;
            // Gold-Gun is the dual-tribe Spellcraft from Greta Gold-Gun;
            // Greta itself is explicitly excluded by the card text.
            if (spell.GetID() == "BG25_044t" &&
                (target.GetCardID() == "BG25_044" ||
                 target.GetCardID() == "BG25_044_G" ||
                 (!target.HasRace(Race::PIRATE) &&
                  !target.HasRace(Race::NAGA))))
                return false;
        }
    }
    if (behavior.effect == TavernSpellEffect::RANDOM_SHOP_GOLDEN &&
        !HasRandomGoldenShopTarget(*this))
    {
        return false;
    }
    if ((behavior.effect == TavernSpellEffect::RANDOM_MINION_TO_HAND ||
         behavior.effect == TavernSpellEffect::RANDOM_NAGA_MINION_TO_HAND ||
         behavior.effect == TavernSpellEffect::RANDOM_COMMON_RACE_MINION_TO_HAND ||
         (behavior.effect == TavernSpellEffect::STEAL_RANDOM_SHOP_MINION ||
          behavior.effect == TavernSpellEffect::STEAL_RANDOM_SHOP_RACE)) &&
        hand.IsFull())
    {
        return false;
    }
    if (behavior.effect == TavernSpellEffect::RANDOM_STAT_TAVERN_SPELL)
    {
        if (hand.GetCount() >= MAX_HAND_SIZE) return false;
        const bool available = std::any_of(
            Cards::GetAllCards().begin(), Cards::GetAllCards().end(),
            [](const Card& candidate) {
                const auto effect = FindTavernSpellBehavior(candidate.id).effect;
                return candidate.isBattlegroundsPoolSpell &&
                       candidate.normalDbfID == 0 && candidate.hasBehavior &&
                       effect != TavernSpellEffect::NONE &&
                       (TavernSpellReceivesHealthBonus(effect) ||
                        TavernSpellReceivesAttackBonus(effect));
            });
        if (!available) return false;
    }
    if (behavior.effect == TavernSpellEffect::RANDOM_MINION_TO_HAND &&
        !HasSupportedTier1Minion())
    {
        return false;
    if (behavior.effect == TavernSpellEffect::RANDOM_NAGA_MINION_TO_HAND &&
        SupportedMinionsForRace(Race::NAGA).empty())
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
    if (behavior.effect == TavernSpellEffect::STEAL_RANDOM_SHOP_RACE)
    {
        bool found = false;
        tavern.fieldZone.ForEach([&](const MinionData& minion) {
            if (!minion.value().IsDestroyed() && minion.value().HasRace(behavior.race))
                found = true;
        });
        if (!found) return false;
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
    if (spell.GetID() == "BG30_MagicItem_714t")
    {
        auto& target = recruitField[static_cast<std::size_t>(targetIdx)];
        target.ApplyTemporaryStats(30, 30);
        hand.Remove(card);
        return true;
    }
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
    const int cost = season14.generatedRewardTemporalTamperingReentry
                         ? 0
                         : (season14.trinketFreeSpellUses > 0 ? 0 : costBeforeFree);
    season14.ConsumeTavernSpellDiscount();
    if (season14.trinketFreeSpellUses > 0)
        --season14.trinketFreeSpellUses;
    const int sourceSpellDbfID = spell.GetDbfID();
    const bool spellcraftSpell = spell.IsTemporary();
    bool temporarySpell = spellcraftSpell;
    TavernSpellBehavior effect = FindTavernSpellBehavior(spell.GetID());
    if (spell.GetID() == "BG31_920t" || spell.GetID() == "BG31_920_Gt")
        effect.value = spell.GetDynamicTier();
    // Thaumaturgy scales by one stat point per three successful Tavern
    // spells cast this game.  Keep the scaling at resolution so generated
    // and refreshed Spellcraft copies observe the same counter.
    if (spell.GetID() == "BG31_924t" || spell.GetID() == "BG31_924_Gt")
    {
        const int improvement = season14.SuccessfulSpellCount() / 3;
        effect.attack += improvement;
        effect.health += improvement;
    }
    if (spell.GetID() == "BG26_501t" || spell.GetID() == "BG26_501_Gt")
    {
        const int multiplier = spell.GetID() == "BG26_501_Gt" ? 2 : 1;
        effect.attack = multiplier * currentTier;
        effect.health = multiplier * currentTier;
    }
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
        effect.health += season14.tavernLightingHealth;
    }
    if (TavernSpellReceivesAttackBonus(effect.effect))
    {
        effect.attack += season14.tavernSpellAttackBonus;
        effect.attack += season14.tavernLightingAttack;
        effect.attack += season14.TemporaryTavernSpellStats().first;
    }
    if (TavernSpellReceivesHealthBonus(effect.effect))
        effect.health += season14.TemporaryTavernSpellStats().second;
    const Spell playedSpell = spell;
    hand.Remove(card);
    TryResolveWarpGateReward();
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
    // Barov's Apprentice watches the generated Battlegrounds Gold Coin
    // (TB_BaconShop_HP_008a), not ordinary Hearthstone Coins.  Resolve this
    // after successful payment/removal so failed plays cannot mint gold.
    if (playedSpell.GetID() == "TB_BaconShop_HP_008a")
    {
        int barovBonus = 0;
        recruitField.ForEachAlive([&barovBonus](const MinionData& data) {
            const auto& id = data.value().GetCardID();
            if (id == "TB_BaconShop_HERO_72_Buddy")
                barovBonus = std::max(barovBonus, 1);
            else if (id == "TB_BaconShop_HERO_72_Buddy_G")
                barovBonus = std::max(barovBonus, 2);
        });
        remainCoin += barovBonus;
    }
    season14.Emit(Season14Event::SPELL_CAST);
    // Emit is reached only after payment/removal and before any modal branch;
    // this ensures Infestor observes discover/choice spells exactly once.
    ApplyInfestorPlayCardBuff();
    // Modal spells resolve asynchronously.  Arm repeat-Trinket state on the
    // modal itself so the eventual choice uses the same stable target and
    // branch rather than silently bypassing Cathedral/Sushi Roll.
    const auto armModalRepeat = [&]() {
        int extra = 0;
        for (auto& trinket : season14.trinkets) {
            if (!trinket.active || trinket.remainingUses == 0) continue;
            const auto behavior = FindTrinketBehavior(
                Cards::FindCardByDbfID(trinket.dbfID).id);
            if (behavior.effect == TrinketEffect::FIRST_SPELL_REPEAT &&
                trinket.triggerProgress == 0) {
                trinket.triggerProgress = 1;
                ++extra;
            }
            if (spellcraftSpell &&
                behavior.effect == TrinketEffect::SPELLCRAFT_REPEAT &&
                trinket.triggerProgress < behavior.value) {
                ++trinket.triggerProgress;
                ++extra;
            }
        }
        season14.spellModal.extraResolutionCount =
            static_cast<std::uint8_t>(std::min(extra, 255));
    };
    const auto armDiscoverReplay = [&]() {
        if (effect.effect != TavernSpellEffect::DISCOVER_TIER_MINION_OR_SPELL &&
            effect.effect != TavernSpellEffect::DISCOVER_CHOOSE_ONE_COMBINED &&
            effect.effect != TavernSpellEffect::DISCOVER_UNDEAD_DIES_THIS_TURN &&
            effect.effect != TavernSpellEffect::DISCOVER_TIER_DARKMOON_PRIZE)
            return;
        std::int32_t extra = 0;
        for (auto& trinket : season14.trinkets) {
            if (!trinket.active || trinket.remainingUses == 0) continue;
            const auto behavior = FindTrinketBehavior(
                Cards::FindCardByDbfID(trinket.dbfID).id);
            if (behavior.effect == TrinketEffect::FIRST_SPELL_REPEAT &&
                trinket.triggerProgress == 0) {
                ++trinket.triggerProgress;
                ++extra;
            }
            if (spellcraftSpell && behavior.effect == TrinketEffect::SPELLCRAFT_REPEAT &&
                trinket.triggerProgress < behavior.value) {
                ++trinket.triggerProgress;
                ++extra;
            }
        }
        if (extra > 0) {
            season14.discoverReplayRemaining = extra;
            season14.discoverReplaySourceSpellDbfID = sourceSpellDbfID;
            season14.discoverReplayTargetEntityID =
                targetIdx >= 0 && targetIdx < recruitField.GetCount()
                    ? static_cast<std::uint64_t>(recruitField[
                          static_cast<std::size_t>(targetIdx)].GetIndex()) : 0;
        }
    };
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
    if (effect.effect == TavernSpellEffect::BLOOD_GEM_CHOOSE_ONE)
    {
        // Gem Day is a player-owned two-way modal.  Keep the source DBF in
        // the dedicated spell state so a replay cannot apply a branch to a
        // different token after the hand has changed.
        season14.BeginSpellAllMinionChoice(
            Season14SpellModalKind::BLOOD_GEM_CHOOSE_ONE,
            sourceSpellDbfID, 1, 0, 0, 1, false);
        armModalRepeat();
        return true;
    }
    if (effect.effect == TavernSpellEffect::REFRESH_BATTLECRY_ONE_COST)
    {
        // Chillmere Mosaic refreshes the minion Tavern, retaining only
        // normal Battlecry minions whose printed cost is exactly one.  The
        // pool callback remains authoritative for pool accounting; filtering
        // happens on the newly offered entities before they become visible.
        clearTavernMinionsCallback(*this);
        PrepareTavern();
        for (int i = tavern.fieldZone.GetCount() - 1; i >= 0; --i)
        {
            auto& minion = tavern.fieldZone[static_cast<std::size_t>(i)];
            const auto card = Cards::FindCardByID(minion.GetCardID());
            const bool battlecry = card.dbfID != 0 &&
                CardDefs::FindCardDefByID(card.id).HasBattlecry();
            // Refresh preserves independently frozen cards.  Newly offered
            // non-Battlecry minions are removed, but a frozen pre-existing
            // entity remains visible exactly as it would after a normal
            // Tavern refresh.
            if (!minion.IsFrozen() && !battlecry)
            {
                const auto poolIndex = minion.GetPoolIndex();
                auto removed = tavern.fieldZone.Remove(minion);
                if (poolIndex >= 0) returnMinionCallback(poolIndex);
                (void)removed;
            }
        }
        // The token's "They cost (1)" is a purchase-cost override, not a
        // printed-card-cost filter. It expires at the next recruit start.
        season14.SetTemporaryMinionPurchaseCost(1);
        season14.OnRefreshTavern(true);
        ApplyTavernSpellTrinkets();
        AdvanceDarkGiftCounters(3);
        return true;
    }
    if (effect.effect == TavernSpellEffect::DISCOVER_TIER_DARKMOON_PRIZE)
    {
        if (hand.IsFull()) return false;
        std::vector<Card> prizes;
        for (const auto& candidate : Cards::GetAllCards())
            if (candidate.GetCardType() == CardType::SPELL &&
                candidate.normalDbfID == 0 && candidate.dbfID > 0 &&
                candidate.id.starts_with("BGS_Treasures_") &&
                candidate.darkmoonPrizeTurn == 3)
                prizes.push_back(candidate);
        if (prizes.size() < 3) return false;
        Random::shuffle(prizes.begin(), prizes.end());
        season14.BeginOfferingDecision(
            Season14Decision::DISCOVER, 0, sourceSpellDbfID,
            {{prizes[0].dbfID, 0}, {prizes[1].dbfID, 0},
             {prizes[2].dbfID, 0}});
        armDiscoverReplay();
        return true;
    }
    if (effect.effect == TavernSpellEffect::DISCOVER_TIER_MINION_OR_SPELL)
    {
        season14.BeginSpellAllMinionChoice(
            Season14SpellModalKind::DISCOVER_TIER_MINION_OR_SPELL,
            sourceSpellDbfID, 0, 0, 0, 0, false);
        // Cathedral/Sushi repeat the spell, not merely the currently visible
        // offering.  Keep the repeat queued until the nested offering is
        // committed, then BeginTavernSpellDiscoverReplay reopens this exact
        // branch modal with the same source DBF.
        armDiscoverReplay();
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
        armDiscoverReplay();
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
        armDiscoverReplay();
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
        armModalRepeat();
        return true;
    }
    if (effect.effect == TavernSpellEffect::ALL_MINION_CHOOSE_ONE_STATS)
    {
        season14.BeginSpellAllMinionChoice(
            Season14SpellModalKind::ALL_MINION_STATS, sourceSpellDbfID,
            2 + auraAttack, 2 + auraHealth,
            2 + auraAttack, 2 + auraHealth, true);
        armModalRepeat();
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
        armModalRepeat();
        return true;
    }
    int castRepeats = 1;
    // Replica Cathedral's repeat is consumed at the first spell resolution
    // of each recruit turn.  Applying the extra pass here keeps all spell
    // observers and target validation on the normal resolution path.
    for (auto& trinket : season14.trinkets) {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect == TrinketEffect::FIRST_SPELL_REPEAT &&
            trinket.triggerProgress == 0) {
            castRepeats += 1;
            trinket.triggerProgress = 1;
        }
    }
    // Spitescale Sushi Roll repeats only the first two Spellcraft cards each
    // recruit turn.  Increment the instance counter only after a successful
    // spell has passed all legality/payment checks.
    if (spellcraftSpell)
    {
        for (auto& trinket : season14.trinkets) {
            if (!trinket.active || trinket.remainingUses == 0) continue;
            const auto behavior = FindTrinketBehavior(
                Cards::FindCardByDbfID(trinket.dbfID).id);
            if (behavior.effect != TrinketEffect::SPELLCRAFT_REPEAT ||
                trinket.triggerProgress >= behavior.value)
                continue;
            ++trinket.triggerProgress;
            ++castRepeats;
        }
    }
    // Hot-Air Surveyor adds the printed +6/+6 payload to each Blood Gem
    // played from hand.  It is a stat bonus, not an extra cast: an additional
    // cast would incorrectly trigger per-cast counters and only add +1/+1.
    if (!temporarySpell && spell.GetID() == "BG20_GEM" && targetIdx >= 0)
    {
        recruitField.ForEachAlive([&effect](MinionData& data) {
            const auto& id = data.value().GetCardID();
            if (id == "BG30_121") effect.attack += 6, effect.health += 6;
            else if (id == "BG30_121_G") effect.attack += 12, effect.health += 12;
        });
    }
    if (targetIdx >= 0 &&
        (spell.GetTargetingType() == TargetingType::FRIENDLY_MINIONS ||
         spell.GetTargetingType() == TargetingType::FRIENDLY_CHARACTERS))
    {
        recruitField.ForEachAlive([&castRepeats](MinionData& data) {
            if (data.value().GetCardID() == "BG35_883")
                castRepeats = std::max(castRepeats, 2);
            else if (data.value().GetCardID() == "BG35_883_G")
                castRepeats = std::max(castRepeats, 3);
        });
    }
    for (int repeat = 0; repeat < castRepeats; ++repeat)
        ApplySpellBoardEffect(*this, effect, targetIdx, temporarySpell,
                              sourceSpellDbfID);
    if (spellcraftSpell)
    {
        for (const auto& trinket : season14.trinkets)
        {
            if (!trinket.active || trinket.remainingUses == 0) continue;
            const auto behavior = FindTrinketBehavior(
                Cards::FindCardByDbfID(trinket.dbfID).id);
            if (behavior.effect != TrinketEffect::ACQUIRE_FIXED_GLOWSCALE)
                continue;
            recruitField.ForEachAlive([&behavior](MinionData& data) {
                auto& minion = data.value();
                if (minion.GetGameTag(GameTag::DIVINE_SHIELD) != 0)
                {
                    minion.SetAttack(minion.GetAttack() + 3);
                    minion.SetHealth(minion.GetHealth() + 3);
                }
            });
        }
    }
    // Temporal Tampering re-enters the normal spell resolver once.  The
    // guard prevents an infinite chain and the replay path has zero cost.
    const auto sourceCard = Cards::FindCardByID(spell.GetID());
    const bool isTavernSpell = sourceCard.isBattlegroundsPoolSpell &&
                               sourceCard.normalDbfID == 0 &&
                               effect.effect != TavernSpellEffect::NONE;
    if (season14.HasGeneratedRewardTemporalTampering() && isTavernSpell &&
        !season14.generatedRewardTemporalTamperingReentry && !hand.IsFull()) {
        const auto replayIndex = static_cast<std::size_t>(hand.GetCount());
        hand.Add(CardData{playedSpell});
        season14.generatedRewardTemporalTamperingReentry = true;
        // Re-enter the normal Tavern-spell resolver, including its target
        // validation and all ordinary spell observers.  PlayCard only routes
        // untargeted spells and would silently skip a targeted replay.
        const bool replayResolved = PlaySpell(replayIndex, targetIdx);
        // A target can cease to be legal while resolving the first cast (for
        // example, the first spell can destroy it).  The generated replay is
        // an attempted cast, not a free card grant: do not strand an
        // uncastable copy in hand when the resolver rejects that target.
        if (!replayResolved && replayIndex < static_cast<std::size_t>(hand.GetCount()))
            hand.Remove(hand[static_cast<int>(replayIndex)]);
        season14.generatedRewardTemporalTamperingReentry = false;
    }
    if (spell.IsTemporary() && targetIdx >= 0 && targetIdx < recruitField.GetCount()) {
        auto& target = recruitField[static_cast<std::size_t>(targetIdx)];
        if ((target.GetCardID() == "BG26_505" || target.GetCardID() == "BG26_505_G") &&
            target.ConsumeZestyShakerUse()) {
            const int copies = target.GetCardID() == "BG26_505_G" ? 2 : 1;
            for (int copy = 0; copy < copies && !hand.IsFull(); ++copy)
                hand.Add(CardData{playedSpell});
        }
    }
    // Daggerspine Thrasher chooses one temporary keyword after every
    // successfully resolved spell.  Keep this dispatch at the common spell
    // resolution boundary so generated and hand spells share the lifecycle.
    recruitField.ForEachAlive([](MinionData& data) {
        Minion& minion = data.value();
        if (minion.GetCardID() != "BG27_024" &&
            minion.GetCardID() != "BG27_024_G") return;
        switch (Random::get<int>(0, 2)) {
            case 0: minion.ApplyTemporaryEnchantment(Minion::TemporaryEnchantment::DivineShield); break;
            case 1: minion.ApplyTemporaryEnchantment(Minion::TemporaryEnchantment::StatsAndWindfury, 0, 0); break;
            default: minion.ApplyTemporaryEnchantment(Minion::TemporaryEnchantment::Venomous); break;
        }
    });
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
    // Reliquary Attendant copies the first Tavern spell cast each recruit
    // turn.  The golden form contributes two copies, but the allowance is
    // still one trigger per turn.  Track consumption in Season14 state so
    // replayed/generated spells cannot reset it and hand-full resolution
    // remains fail-closed.
    if (isTavernSpell) {
        if (season14.buddyReliquaryCopiesTurn != season14.recruitTurnNumber) {
            season14.buddyReliquaryCopiesTurn = season14.recruitTurnNumber;
            season14.buddyReliquaryCopiesUsed = 0;
        }
        int reliquaryCopies = 0;
        recruitField.ForEachAlive([&reliquaryCopies](const MinionData& data) {
            const auto& id = data.value().GetCardID();
            if (id == "BG28_HERO_800_Buddy") ++reliquaryCopies;
            else if (id == "BG28_HERO_800_Buddy_G") reliquaryCopies += 2;
        });
        if (reliquaryCopies > season14.buddyReliquaryCopiesUsed) {
            season14.buddyReliquaryCopiesUsed = reliquaryCopies;
            for (int copy = 0; copy < reliquaryCopies && !hand.IsFull(); ++copy)
                hand.Add(CardData{playedSpell});
        }
    }
    // Arcane Knowledge and other one-shot Tavern-spell discounts are
    // consumed only after a supported spell has actually resolved.  The
    // legality check above ensures unaffordable/unsupported attempts leave
    // the discount untouched.
    season14.OnTavernSpellResolved(true, sourceSpellDbfID, targetIdx >= 0);
    ResolveSpellCountTrinkets();
    IncrementStartCombatSpellImprovements();
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
    if (targetIdx >= 0 && targetIdx < recruitField.GetCount())
    {
        for (const auto& trinket : season14.trinkets)
        {
            if (!trinket.active || trinket.remainingUses == 0) continue;
            const auto behavior = FindTrinketBehavior(
                Cards::FindCardByDbfID(trinket.dbfID).id);
            if (behavior.effect ==
                TrinketEffect::TAVERN_SPELL_IMPROVE_AFTER_MINION_CAST)
                season14.AddTemporaryTavernSpellStats(behavior.attack,
                                                       behavior.health);
        }
    }
    AdvanceDarkGiftCounters(3);
    return true;
}

bool Player::ApplySeason14HeroPowerBatch3ResolvedTargets(
    const Season14HeroPowerBatch3Activation& activation,
    const std::array<std::uint64_t, 32>& entityIDs,
    std::uint8_t entityCount)
{
    if (entityCount == 0 || entityCount > entityIDs.size()) return false;
    bool applied = false;
    for (std::size_t i = 0; i < entityCount; ++i)
    {
        for (int boardIdx = 0; boardIdx < recruitField.GetCount(); ++boardIdx)
        {
            auto& target = recruitField[static_cast<std::size_t>(boardIdx)];
            if (target.IsDestroyed() ||
                static_cast<std::uint64_t>(target.GetIndex()) != entityIDs[i])
                continue;
            target.SetAttack(target.GetAttack() + activation.attack);
            target.SetHealth(target.GetHealth() + activation.health);
            applied = true;
            break;
        }
    }
    return applied;
}

bool Player::ApplyArcaneAlteration(std::size_t slot, std::uint64_t entityID,
                                   std::int32_t replacementDbfID)
{
    if (slot >= static_cast<std::size_t>(tavern.fieldZone.GetCount()) ||
        replacementDbfID <= 0)
        return false;
    auto& old = tavern.fieldZone[slot];
    if (old.IsDestroyed() || static_cast<std::uint64_t>(old.GetIndex()) != entityID)
        return false;
    const bool frozen = old.IsFrozen();
    auto replacement = Cards::FindCardByDbfID(replacementDbfID);
    if (replacement.dbfID == 0 || replacement.GetCardType() != CardType::MINION ||
        !replacement.isBattlegroundsPoolMinion || replacement.normalDbfID != 0 ||
        !replacement.hasBehavior || replacement.GetTier() != old.GetTier())
        return false;
    tavern.fieldZone.Remove(old);
    Minion minion(replacement);
    ApplyFreshTavernMinionModifiers(minion);
    minion.SetFrozen(frozen);
    tavern.fieldZone.Add(minion, slot);
    return true;
}

bool Player::ApplySwapShopMinion(std::size_t boardSlot, std::uint64_t boardEntityID,
                                 std::size_t tavernSlot, std::uint64_t tavernEntityID)
{
    if (boardSlot >= static_cast<std::size_t>(recruitField.GetCount()) ||
        tavernSlot >= static_cast<std::size_t>(tavern.fieldZone.GetCount()) ||
        boardEntityID == 0 || tavernEntityID == 0)
        return false;
    auto& boardRef = recruitField[boardSlot];
    auto& tavernRef = tavern.fieldZone[tavernSlot];
    const auto boardID = static_cast<std::uint64_t>(boardRef.GetIndex());
    const auto tavernID = static_cast<std::uint64_t>(tavernRef.GetIndex());
    const bool forward = boardID == boardEntityID && tavernID == tavernEntityID;
    if (boardRef.IsDestroyed() || tavernRef.IsDestroyed() || !forward ||
        boardRef.IsGolden())
        return false;
    const bool tavernFrozen = tavernRef.IsFrozen();
    auto boardMinion = recruitField.Remove(boardRef);
    auto tavernMinion = tavern.fieldZone.Remove(tavernRef);
    boardMinion.SetFrozen(false);
    tavernMinion.SetFrozen(tavernFrozen);
    recruitField.Add(tavernMinion, boardSlot);
    tavern.fieldZone.Add(boardMinion, tavernSlot);
    return true;
}

bool Player::ApplySeason14HeroPowerBatch3Activation(
    const Season14HeroPowerBatch3Activation& activation)
{
    std::vector<Minion*> candidates;
    recruitField.ForEachAlive([&candidates](MinionData& minion) {
        candidates.push_back(&minion.value());
    });
    if (season14.heroPowerDbfID == 59808) {
        candidates.erase(std::remove_if(candidates.begin(), candidates.end(),
            [](const Minion* minion) { return !minion->HasRace(Race::DEMON); }),
            candidates.end());
    }
    if (season14.heroPowerDbfID == 59863) {
        const Race types[] = {Race::BEAST, Race::DEMON, Race::DRAGON,
            Race::ELEMENTAL, Race::MECHANICAL, Race::MURLOC, Race::NAGA,
            Race::PIRATE, Race::QUILBOAR, Race::UNDEAD};
        for (const auto type : types) {
            std::vector<Minion*> eligible;
            for (auto* minion : candidates)
                if (minion->HasRace(type)) eligible.push_back(minion);
            if (!eligible.empty()) {
                auto* target = eligible[Random::get<std::size_t>(
                    0, eligible.size() - 1)];
                target->SetAttack(target->GetAttack() + activation.attack);
                target->SetHealth(target->GetHealth() + activation.health);
            }
        }
        return true;
    }
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

int Player::ResolveHunterOfOldBuddy()
{
    if (hand.IsFull() || season14.lastOpponentBuddyDbfID == 0)
        return 0;
    auto buddy = Cards::FindCardByDbfID(season14.lastOpponentBuddyDbfID);
    if (buddy.GetCardType() != CardType::MINION || buddy.dbfID == 0)
        return 0;
    if (buddy.normalDbfID != 0)
        buddy = Cards::FindCardByDbfID(buddy.normalDbfID);
    int copies = 0;
    recruitField.ForEachAlive([&copies](const MinionData& data) {
        const auto& id = data.value().GetCardID();
        if (id == "TB_BaconShop_HERO_50_Buddy") ++copies;
        else if (id == "TB_BaconShop_HERO_50_Buddy_G") copies += 2;
    });
    int added = 0;
    for (; added < copies && !hand.IsFull(); ++added) {
        Minion generated(buddy);
        ApplyFreshMinionModifiers(generated);
        hand.Add(CardData{std::move(generated)});
    }
    return added;
}

int Player::ResolveLilKTMinions()
{
    if (hand.IsFull() || !getLowestHealthOpponentPlayerCallback)
        return 0;
    Player& opponent = getLowestHealthOpponentPlayerCallback(*this);
    if (&opponent == this) return 0;
    std::vector<Minion> candidates;
    opponent.recruitField.ForEachAlive([&candidates](const MinionData& data) {
        const auto& minion = data.value();
        if (!minion.IsDestroyed() && minion.GetDbfID() > 0)
            candidates.push_back(minion);
    });
    if (candidates.empty()) return 0;
    int copies = 0;
    recruitField.ForEachAlive([&copies](const MinionData& data) {
        const auto& id = data.value().GetCardID();
        if (id == "TB_BaconShop_HERO_70_Buddy") ++copies;
        else if (id == "TB_BaconShop_HERO_70_Buddy_G") copies += 2;
    });
    int added = 0;
    for (; added < copies && !hand.IsFull() && !candidates.empty(); ++added) {
        const auto& source = candidates[Random::get<std::size_t>(
            0, candidates.size() - 1)];
        (void)AddMinionCopyToHand(source);
    }
    return added;
}

bool Player::BeginTicketCollectorDiscover(bool golden)
{
    if (season14.pendingDecision != Season14Decision::NONE)
        return false;
    std::vector<Card> prizes;
    const auto nextPrizeTier = std::min(currentTier + 1, TIER_UPPER_LIMIT);
    for (const auto& candidate : Cards::GetAllCards()) {
        if (candidate.GetCardType() != CardType::SPELL ||
            candidate.normalDbfID != 0 ||
            !candidate.id.starts_with("BGS_Treasures_") ||
            candidate.darkmoonPrizeTurn != nextPrizeTier)
            continue;
        prizes.push_back(candidate);
    }
    if (prizes.size() < 3)
        return false;
    Random::shuffle(prizes.begin(), prizes.end());
    season14.buddyTicketRemaining = golden ? 2 : 1;
    season14.BeginOfferingDecision(
        Season14Decision::DISCOVER, 0,
        golden ? 77846 : 77845,
        {{prizes[0].dbfID, 0}, {prizes[1].dbfID, 0},
         {prizes[2].dbfID, 0}});
    return true;
}

int Player::ResolveMaxwellBuddyCopies(int soldCopies)
{
    if (hand.IsFull() || hero.card.relatedDbfID == 0) return 0;
    const int copies = std::max(0, soldCopies);
    if (copies == 0) return 0;
    auto buddy = Cards::FindCardByDbfID(hero.card.relatedDbfID);
    if (buddy.GetCardType() != CardType::MINION || buddy.dbfID == 0) return 0;
    int added = 0;
    for (; added < copies && !hand.IsFull(); ++added) {
        Minion generated(buddy);
        ApplyFreshMinionModifiers(generated);
        hand.Add(CardData{std::move(generated)});
    }
    return added;
}

bool Player::BeginClockworkAssistantDiscover(bool golden)
{
    if (hand.IsFull() || season14.pendingDecision != Season14Decision::NONE)
        return false;
    std::vector<Card> candidates;
    const int tier = currentTier + 1;
    if (tier > 6) return false;
    for (const auto& card : Cards::GetAllCards())
        if (card.GetCardType() == CardType::MINION &&
            card.isBattlegroundsPoolMinion && card.hasBehavior &&
            card.normalDbfID == 0 && card.GetTier() == tier)
            candidates.push_back(card);
    if (candidates.size() < 3) return false;
    Random::shuffle(candidates.begin(), candidates.end());
    season14.clockworkDiscoverRemaining = golden ? 1 : 0;
    season14.BeginOfferingDecision(
        Season14Decision::DISCOVER, 0,
        golden ? 77603 : 77507,
        {{candidates[0].dbfID, 0}, {candidates[1].dbfID, 0},
         {candidates[2].dbfID, 0}});
    return true;
}

bool Player::ResolveSparkfinSoothsayer(bool golden)
{
    bool transformed = false;
    for (int slot = 0; slot < tavern.fieldZone.GetCount(); ++slot)
    {
        auto& offer = tavern.fieldZone[static_cast<std::size_t>(slot)];
        if (offer.IsDestroyed() || offer.GetTier() < 1) continue;
        const bool wasFrozen = offer.IsFrozen();
        const int targetTier = std::min(6, offer.GetTier() + (golden ? 1 : 0));
        std::vector<Card> candidates;
        for (const auto& card : Cards::GetAllCards())
            if (card.GetCardType() == CardType::MINION &&
                card.isBattlegroundsPoolMinion && card.hasBehavior &&
                card.normalDbfID == 0 && card.HasRace(Race::MURLOC) &&
                card.GetTier() == targetTier)
                candidates.push_back(card);
        if (candidates.empty()) continue;
        Random::shuffle(candidates.begin(), candidates.end());
        auto replaced = tavern.fieldZone.Remove(offer);
        if (replaced.GetPoolIndex() >= 0)
            returnMinionCallback(replaced.GetPoolIndex());
        Minion murloc(candidates.front());
        ApplyFreshTavernMinionModifiers(murloc);
        // Transforming a frozen offer does not thaw it.  Preserve the
        // entity's slot and freeze state while returning the old minion to
        // the pool; subsequent offers are transformed independently.
        murloc.SetFrozen(wasFrozen);
        tavern.fieldZone.Add(murloc, slot);
        transformed = true;
    }
    return transformed;
}

void Player::ResolveLoyalHenchmanKill(const Minion& killed)
{
    int copies = 0;
    // During combat GetField() is the combat snapshot, so a Henchman that
    // died earlier cannot retroactively trigger on a later kill.
    GetField().ForEachAlive([&copies](const MinionData& data) {
        const auto& id = data.value().GetCardID();
        if (id == "TB_BaconShop_HERO_45_Buddy") ++copies;
        else if (id == "TB_BaconShop_HERO_45_Buddy_G") copies += 2;
    });
    if (copies == 0 || !season14.AdvanceLoyalHenchmanKill()) return;
    for (int i = 0; i < copies && !hand.IsFull(); ++i)
        (void)AddMinionCopyToHand(killed);
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
    // Maxwell and Sharkbait are sale-triggered Buddies in the authoritative
    // 36.4 catalogue.  Resolve them from the copied sold entity, after it has
    // left the warband, so a failed/invalid sale cannot fire either effect.
    if (soldID == "TB_BaconShop_HERO_40_Buddy" ||
        soldID == "TB_BaconShop_HERO_40_Buddy_G")
        ResolveMaxwellBuddyCopies(soldID.ends_with("_G") ? 2 : 1);
    if (soldID == "TB_BaconShop_HERO_68_Buddy" ||
        soldID == "TB_BaconShop_HERO_68_Buddy_G") {
        // Refresh the currently equipped power in place.  Preserve its
        // identity/cost/discounts, but clear every per-use gate consulted by
        // CanUseHeroPower (including the two-use Bloodbound/Arcane paths).
        season14.heroPowerAvailable = true;
        season14.heroPowerUsed = false;
        season14.luckyRollCooldown = 0;
        season14.heroPowerBatch3State = 0;
        season14.heroPowerBatch2.bloodboundUsesThisTurn = 0;
    }
    // The sold entity is no longer in recruitField, so it cannot be reached
    // by the observer loop below.  Resolve its self-scoped SELL_MINION
    // trigger explicitly after removal; this is the lifecycle used by
    // Twisted Wrathguard while preserving observers on surviving minions.
    minion.ActivateTrigger(TriggerType::SELL_MINION, minion);
    recruitField.ForEachAlive([&minion](MinionData& data) {
        data.value().ActivateTrigger(TriggerType::SELL_MINION,
                                      minion);
    });
    // Asher the Haberdasher buffs itself after each successful sale.  Apply
    // this after the observer dispatch: the sold minion is gone, and only
    // surviving Asher instances may receive the self-scoped gain.
    recruitField.ForEachAlive([](MinionData& data) {
        const auto& id = data.value().GetCardID();
        if (id == "TB_BaconShop_HERO_36_Buddy")
            data.value().ApplyPersistentMinionStats(1, 1);
        else if (id == "TB_BaconShop_HERO_36_Buddy_G")
            data.value().ApplyPersistentMinionStats(2, 2);
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
    if (season14.HasGeneratedRewardAnimaBribe()) {
        std::vector<Minion*> candidates;
        tavern.fieldZone.ForEachAlive([&](MinionData& data) {
            candidates.push_back(&data.value());
        });
        if (!candidates.empty()) {
            auto* target = candidates[Random::get<std::size_t>(
                0, candidates.size() - 1)];
            target->SetAttack(target->GetAttack() + minion.GetAttack());
            target->SetHealth(target->GetHealth() + minion.GetHealth());
        }
    }
    // Threshold sell Trinkets resolve from the authoritative sale event,
    // after the sold entity has left the warband and normal sell counters
    // have advanced. Progress is per Trinket instance and survives refreshes.
    for (auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect != TrinketEffect::AFTER_SELL_RANDOM_MINION ||
            behavior.value <= 0) continue;
        if (++trinket.triggerProgress >= behavior.value)
        {
            trinket.triggerProgress = 0;
            (void)SimpleTasks::RandomCardToHandTask{
                behavior.race, behavior.tier, 1,
                behavior.magneticOnly, behavior.battlecryOnly}.Run(*this);
        }
    }
    if (season14.heroPowerDbfID == 60448 &&
        season14.murlocRewardsRemaining > 0)
    {
        // A full hand does not consume the five-sale progress or the reward;
        // the pending reward is retried by the next successful sale.
        season14.murlocRewardSells = std::min<std::int32_t>(
            5, season14.murlocRewardSells + 1);
        if (season14.murlocRewardSells >= 5 && !hand.IsFull()) {
            std::vector<Card> murlocs;
            for (const auto& card : Cards::GetAllCards())
                if (card.GetCardType() == CardType::MINION &&
                    card.normalDbfID == 0 && card.isBattlegroundsPoolMinion &&
                    card.hasBehavior && card.HasRace(Race::MURLOC))
                    murlocs.push_back(card);
            if (!murlocs.empty()) {
                const auto& card = murlocs[Random::get<std::size_t>(
                    0, murlocs.size() - 1)];
                Minion generated(card);
                ApplyFreshMinionModifiers(generated);
                hand.Add(CardData{std::move(generated)});
                season14.murlocRewardSells = 0;
                --season14.murlocRewardsRemaining;
            }
        }
    }
    if (season14.heroPowerDbfID == 59860)
    {
        for (int i = 0; i < 3; ++i)
        {
            std::vector<Minion*> offers;
            tavern.fieldZone.ForEachAlive([&](MinionData& data) { offers.push_back(&data.value()); });
            if (offers.empty()) break;
            Minion& target = *offers[Random::get<std::size_t>(0, offers.size() - 1)];
            target.SetAttack(target.GetAttack() + 1);
            target.SetHealth(target.GetHealth() + 1);
        }
    }
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

void Player::UpgradeTavernForGeneratedReward()
{
    if (currentTier >= TIER_UPPER_LIMIT) return;
    upgradeTavernCallback(*this);
    (void)season14.OnUpgradeTavern();
}

void Player::RefreshTavern(bool freeRefresh)
{
    bool pigeonFreeRefresh = false;
    const auto heroRace = RatKingHeroPowerRace(season14.heroPowerDbfID);
    if (heroRace != Race::INVALID)
    {
        bool matchingOffer = false;
        tavern.fieldZone.ForEachAlive([&matchingOffer, heroRace](const MinionData& data) {
            matchingOffer = matchingOffer || data.value().HasRace(heroRace);
        });
        if (!matchingOffer)
        {
            recruitField.ForEachAlive([&pigeonFreeRefresh](const MinionData& data) {
                const auto& id = data.value().GetCardID();
                pigeonFreeRefresh = pigeonFreeRefresh ||
                    id == "TB_BaconShop_HERO_12_Buddy" ||
                    id == "TB_BaconShop_HERO_12_Buddy_G";
            });
        }
    }
    const bool payHealth = !freeRefresh && !season14.HasFreeRefresh() &&
                           malchezaarRefreshesRemaining > 0;
    if (payHealth && hero.health <= 1) return;
    const bool allowanceRefresh = !freeRefresh && season14.HasFreeRefresh();
    const int cost = freeRefresh || allowanceRefresh || payHealth || pigeonFreeRefresh
                         ? 0
                         : season14.RefreshCost(NUM_COIN_REFRESH_TAVERN);
    if (remainCoin < cost)
    {
        return;
    }

    clearTavernMinionsCallback(*this);
    season14.refreshInProgress = true;
    season14.refreshExtraShopSlots = 0;
    for (const auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect == TrinketEffect::REFRESH_EXTRA_SHOP_SLOTS)
            season14.refreshExtraShopSlots += behavior.value;
    }
    if (payHealth) {
        hero.health -= 1;
        --malchezaarRefreshesRemaining;
    } else {
        remainCoin -= cost;
        RecordGoldSpent(cost);
    }

    if (allowanceRefresh)
    {
        season14.ConsumeFreeRefresh();
    }

    PrepareTavern();
    // Snow Elemental adds fresh frozen Elemental offers after every completed
    // refresh.  Sklibb is a free-refresh Buddy and must not alter shop size.
    // Keep the next-tier cap authoritative; golden Snow contributes two.
    int extraSnowOffers = 0;
    recruitField.ForEachAlive([&](const MinionData& data) {
        const auto& id = data.value().GetCardID();
        if (id == "TB_BaconShop_HERO_78_Buddy")
            ++extraSnowOffers;
        else if (id == "TB_BaconShop_HERO_78_Buddy_G")
            extraSnowOffers += 2;
    });
    if (extraSnowOffers > 0 && !tavern.fieldZone.IsFull()) {
        const int offerTier = std::min(currentTier + 1, TIER_UPPER_LIMIT);
        std::vector<Card> candidates;
        for (const auto& candidate : Cards::GetAllCards()) {
            if (candidate.normalDbfID != 0 ||
                candidate.GetCardType() != CardType::MINION ||
                !candidate.isBattlegroundsPoolMinion ||
                !candidate.hasBehavior || candidate.GetTier() != offerTier)
                continue;
            if (!candidate.HasRace(Race::ELEMENTAL))
                continue;
            candidates.push_back(candidate);
        }
        Random::shuffle(candidates.begin(), candidates.end());
        const auto count = std::min<std::size_t>(
            static_cast<std::size_t>(extraSnowOffers), candidates.size());
        for (std::size_t i = 0; i < count && !tavern.fieldZone.IsFull(); ++i) {
            Minion offer(candidates[i]);
            ApplyFreshTavernMinionModifiers(offer);
            offer.SetFrozen(true);
            tavern.fieldZone.Add(std::move(offer));
        }
    }
    // Shining Sailor adds fresh Pirate offers after every completed refresh.
    // Resolve from the post-PrepareTavern pool so the offer is a legal,
    // executable minion and never leaks a non-pool/unsupported card.
    int extraPirateOffers = 0;
    recruitField.ForEachAlive([&extraPirateOffers](const MinionData& data) {
        const auto& id = data.value().GetCardID();
        if (id == "BG26_HERO_101_Buddy") ++extraPirateOffers;
        else if (id == "BG26_HERO_101_Buddy_G") extraPirateOffers += 2;
    });
    if (extraPirateOffers > 0 && addRandomRaceTavernMinionCallback) {
        // Draw through MinionPool so each extra offer consumes a real pool
        // entity (and returns to the pool on later refresh/sale).  The pool
        // callback also enforces the actual Tavern capacity and fail-closes
        // when no same-tier Pirate is available.
        for (int i = 0; i < extraPirateOffers && !tavern.fieldZone.IsFull(); ++i)
            if (!addRandomRaceTavernMinionCallback(*this, currentTier,
                                                   Race::PIRATE))
                break;
    }
    // Gift of the Golden Kobold counts successful player refreshes.  Resolve
    // against the current Tavern only after the refresh has filled it, and
    // retain progress if every offer is already golden/empty.
    season14.RecordGeneratedRewardGoldenKoboldRefresh();
    if (season14.GeneratedRewardGoldenKoboldRefreshes() >= 5) {
        std::vector<Minion*> highestTier;
        int highest = 0;
        tavern.fieldZone.ForEachAlive([&highestTier, &highest](MinionData& data) {
            auto& candidate = data.value();
            if (candidate.IsGolden() || !candidate.CanMakeGolden()) return;
            if (candidate.GetTier() > highest) {
                highest = candidate.GetTier();
                highestTier.clear();
            }
            if (candidate.GetTier() == highest)
                highestTier.push_back(&candidate);
        });
        // The reward chooses uniformly among tied highest-tier eligible
        // offers; preserving field order would make the leftmost offer win
        // every time and would diverge from the simulator's RNG semantics.
        if (!highestTier.empty()) {
            Minion* highest = highestTier[Random::get<std::size_t>(
                0, highestTier.size() - 1)];
            highest->MakeGolden();
            (void)season14.ConsumeGeneratedRewardGoldenKoboldTrigger();
        }
    }
    if (season14.HasGeneratedRewardWisdomball() &&
        !season14.generatedRewardWisdomballUsedThisTurn &&
        Random::get<int>(0, 3) == 0) {
        season14.AddFreeRefreshes(1);
        season14.generatedRewardWisdomballUsedThisTurn = true;
    }
    if (season14.HasGeneratedRewardTealTiger()) {
        ++season14.generatedRewardRefreshesThisTurn;
        const auto amount = season14.generatedRewardRefreshesThisTurn;
        tavern.fieldZone.ForEachAlive([amount](MinionData& data) {
            data.value().SetAttack(data.value().GetAttack() + amount);
            data.value().SetHealth(data.value().GetHealth() + amount);
        });
    }
    if (season14.heroPowerDbfID == 96872) {
        const auto hasKeyword = [](const Minion& minion, GameTag tag) {
            if (tag == GameTag::DIVINE_SHIELD) return minion.HasDivineShield();
            if (tag == GameTag::REBORN) return minion.HasReborn();
            if (tag == GameTag::WINDFURY) return minion.HasWindfury();
            if (tag == GameTag::VENOMOUS) return minion.HasVenomous();
            return minion.HasTaunt();
        };
        const GameTag keywords[] = {GameTag::DIVINE_SHIELD, GameTag::REBORN,
            GameTag::WINDFURY, GameTag::VENOMOUS, GameTag::TAUNT};
        const auto applyPersistentKeyword = [](Minion& minion, GameTag tag) {
            if (tag == GameTag::TAUNT) minion.SetTaunt(true);
            else if (tag == GameTag::REBORN) minion.SetReborn(true);
            else if (tag == GameTag::VENOMOUS) minion.SetGameTag(GameTag::POISONOUS, 1);
            else minion.SetGameTag(tag, 1);
        };
        for (int pass = 0; pass < 2; ++pass) {
            std::vector<Minion*> minions;
            tavern.fieldZone.ForEachAlive([&](MinionData& data) { minions.push_back(&data.value()); });
            if (minions.empty()) break;
            auto* target = minions[Random::get<std::size_t>(0, minions.size() - 1)];
            std::vector<GameTag> eligible;
            for (const auto keyword : keywords)
                if (!hasKeyword(*target, keyword)) eligible.push_back(keyword);
            if (eligible.empty()) break;
            // Enhancification modifies the Tavern entity itself.  The old
            // ApplyTemporaryKeyword path is intentionally not used here:
            // these Bonus Keywords must survive purchase/hand/board
            // transitions, so do not mark them as turn-limited Spellcraft state.
            applyPersistentKeyword(*target,
                eligible[Random::get<std::size_t>(0, eligible.size() - 1)]);
        }
    }
    // Count only a completed, player-initiated refresh.  Initial tavern
    // preparation and failed/full-hand operations do not advance the power.
    season14.RecordRefreshBatch5();
    ++season14.chromieRefreshesThisTurn;
    season14.refreshInProgress = false;
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
                // ApplyFreshMinionModifiers(generated) is intentionally
                // represented by the Tavern-specific modifier hook below.
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
    // Twice as Nice copies the highest-Tier offer after every successful
    // Tavern refresh and freezes both source and copy.  Resolve this after
    // all refresh auras so the copy carries the authoritative current stats.
    if (season14.heroPowerDbfID == 80539 && !tavern.fieldZone.IsFull())
    {
        int bestTier = -1;
        std::vector<int> bestSlots;
        for (int i = 0; i < tavern.fieldZone.GetCount(); ++i)
        {
            const auto& offer = tavern.fieldZone[static_cast<std::size_t>(i)];
            if (!offer.IsDestroyed() && offer.GetTier() > bestTier)
            {
                bestTier = offer.GetTier();
                bestSlots.clear();
                bestSlots.push_back(i);
            }
            else if (!offer.IsDestroyed() && offer.GetTier() == bestTier)
            {
                bestSlots.push_back(i);
            }
        }
        // The live effect chooses randomly among tied highest-tier offers.
        // Use RosettaStone's shared RNG stream so seeded replays remain
        // deterministic without imposing a slot-order bias.
        if (!bestSlots.empty())
        {
            Random::shuffle(bestSlots.begin(), bestSlots.end());
            const auto bestSlot = bestSlots.front();
            auto& source = tavern.fieldZone[static_cast<std::size_t>(bestSlot)];
            Minion copy(Cards::FindCardByID(source.GetCardID()));
            if (copy.GetDbfID() != 0)
            {
                copy.SetAttack(source.GetAttack());
                copy.SetHealth(source.GetHealth());
                copy.SetFrozen(true);
                if (getNextCardIndexCallback)
                    copy.SetIndex(getNextCardIndexCallback());
                source.SetFrozen(true);
                tavern.fieldZone.Add(copy);
            }
        }
    }
    // Chromie is a refresh-scoped Tavern aura.  Apply the accumulated count
    // after all other refresh producers so every newly offered minion sees
    // the same authoritative bonus; golden Chromie contributes twice per
    // successful refresh.
    int chromieMultiplier = 0;
    recruitField.ForEachAlive([&chromieMultiplier](const MinionData& data) {
        const auto& id = data.value().GetCardID();
        if (id == "TB_BaconShop_HERO_57_Buddy") chromieMultiplier = std::max(chromieMultiplier, 1);
        else if (id == "TB_BaconShop_HERO_57_Buddy_G") chromieMultiplier = std::max(chromieMultiplier, 2);
    });
    if (chromieMultiplier > 0 && season14.chromieRefreshesThisTurn > 0)
    {
        const int amount = chromieMultiplier * season14.chromieRefreshesThisTurn;
        tavern.fieldZone.ForEachAlive([amount](MinionData& data) {
            data.value().SetAttack(data.value().GetAttack() + amount);
            data.value().SetHealth(data.value().GetHealth() + amount);
        });
    }
}

void Player::TryDeliverChampionReward()
{
    if (season14.heroPowerDbfID != 104628 || season14.GoldSpentThisGame() < 60 ||
        !season14.ChampionRewardPending() || hand.IsFull()) return;
    const auto reward = Cards::FindCardByDbfID(season14.championRewardDbfID);
    if (reward.dbfID == 0 || reward.GetCardType() != CardType::MINION) return;
    Minion generated(reward);
    ApplyFreshMinionModifiers(generated);
    hand.Add(CardData{std::move(generated)});
    season14.championRewardDbfID = 0;
}

void Player::TryDeliverHeroicInspirationReward()
{
    if (season14.heroPowerDbfID != 129164 ||
        !season14.HeroicInspirationRewardPending() || hand.IsFull()) return;
    const auto reward = Cards::FindCardByDbfID(59604); // TB_BaconShop_Triples_01
    if (reward.dbfID == 0 || reward.GetCardType() != CardType::SPELL) return;
    hand.Add(CardData{Spell(reward)});
    season14.ClearHeroicInspirationReward();
}

void Player::RecordGoldSpent(std::int32_t amount)
{
    if (amount <= 0) return;
    const int malorneBefore = season14.GoldSpentThisGame() / 3;
    // Keep the central gold threshold state on the generic counted-event
    // primitive.  This preserves the per-turn remainder while allowing all
    // spend-gold descriptors to share exact threshold semantics.
    EventCounterState goldCounter{season14.goldSpentThisTurn % 5};
    const int thresholds = AdvanceEventCounter(goldCounter, amount, 5, true);
    season14.goldSpentThisTurn += std::max(0, amount);
    season14.goldSpentThisGame += std::max(0, amount);
    TryDeliverChampionReward();
    // Booty Bay Brew is an event-level trigger: spending a bundle of gold
    // (buy, refresh, upgrade, or spell) triggers once, not once per coin.
    // Select up to two distinct Pirates from the live recruit board, then
    // apply a persistent buff so refreshes and combat copies retain it.
    for (const auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect != TrinketEffect::SPEND_GOLD_PIRATE_STATS)
            continue;
        std::vector<Minion*> pirates;
        recruitField.ForEachAlive([&pirates](MinionData& data) {
            if (data.value().HasRace(Race::PIRATE))
                pirates.push_back(&data.value());
        });
        Random::shuffle(pirates.begin(), pirates.end());
        const auto count = std::min<std::size_t>(2, pirates.size());
        for (std::size_t i = 0; i < count; ++i)
            pirates[i]->ApplyPersistentMinionStats(behavior.attack,
                                                   behavior.health);
    }
    // Exact-threshold observers receive every spend; the legacy SPEND_GOLD
    // event below remains a five-gold crossing for existing cards.
    recruitField.ForEachAlive([this](MinionData& data) {
        auto& minion = data.value();
        minion.ActivateTrigger(TriggerType::SPEND_GOLD_EXACT, minion);
    });
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
    }
}

void Player::RecordTreasureParrotDamage(Minion& source, int amount)
{
    if (amount <= 0 || source.TreasureParrotRewarded() ||
        (source.GetCardID() != "BG36_763" && source.GetCardID() != "BG36_763_G"))
        return;
    source.AddDamageDealt(amount);
    const int count = source.GetCardID() == "BG36_763_G" ? 2 : 1;
    if (source.DamageDealt() < 35 || hand.GetCount() + count > MAX_HAND_SIZE) return;
    const Card reward = Cards::FindCardByID("BG28_830");
    if (reward.id.empty()) return;
    for (int i = 0; i < count && !hand.IsFull(); ++i)
        hand.Add(CardData{Spell(reward)});
    source.SetTreasureParrotRewarded(true);
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
    // Akali, Rock Rhino copies the left-most hand card at recruit end. The
    // normal Buddy fires every other turn; golden copies fire every turn.
    int akaliCopies = 0;
    recruitField.ForEachAlive([&akaliCopies](MinionData& data) {
        const auto& id = data.value().GetCardID();
        if (id == "BG26_HERO_104_Buddy") {
            // Count normal copies separately below using the shared cadence.
            ++akaliCopies;
        } else if (id == "BG26_HERO_104_Buddy_G") {
            akaliCopies += 2;
        }
    });
    if (akaliCopies > 0) {
        ++akaliBuddyTurns;
        int copies = 0;
        recruitField.ForEachAlive([&copies, this](MinionData& data) {
            const auto& id = data.value().GetCardID();
            if (id == "BG26_HERO_104_Buddy_G") ++copies;
            else if (id == "BG26_HERO_104_Buddy" && akaliBuddyTurns % 2 == 0)
                ++copies;
        });
        if (copies > 0 && hand.GetCount() > 0 && !hand.IsFull()) {
            const auto& leftmost = hand[0];
            if (std::holds_alternative<Minion>(leftmost)) {
                for (int copy = 0; copy < copies && !hand.IsFull(); ++copy)
                    AddMinionCopyToHand(std::get<Minion>(leftmost));
            }
        }
    }
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

void Player::ResolveSulfurasEndTurn()
{
    const auto buff = Season14HeroPowerBatch3SulfurasEndTurnBuff(
        season14.heroPowerDbfID);
    if (buff.attack == 0 && buff.health == 0) return;
    std::vector<Minion*> edges;
    recruitField.ForEachAlive([&edges](MinionData& data) {
        edges.push_back(&data.value());
    });
    if (edges.empty()) return;
    edges.front()->SetAttack(edges.front()->GetAttack() + buff.attack);
    edges.front()->SetHealth(edges.front()->GetHealth() + buff.health);
    if (edges.size() > 1) {
        edges.back()->SetAttack(edges.back()->GetAttack() + buff.attack);
        edges.back()->SetHealth(edges.back()->GetHealth() + buff.health);
    }
}

void Player::ResolveCthunEndTurn()
{
    if (season14.heroPowerDbfID != 66246 ||
        !season14.HasCthunEndTurnTargets())
        return;
    const auto applications = std::max<std::uint8_t>(1,
                                                       season14.cthunEndTurnApplications);
    for (std::uint8_t pass = 0; pass < applications; ++pass)
    {
        std::vector<Minion*> eligible;
        recruitField.ForEachAlive([&](MinionData& data) {
            eligible.push_back(&data.value());
        });
        for (std::uint8_t repeat = 0;
             repeat <= season14.cthunRepeatCount && !eligible.empty(); ++repeat)
        {
            const auto selected = Random::get<std::size_t>(0, eligible.size() - 1);
            eligible[selected]->SetAttack(eligible[selected]->GetAttack() + 1);
            eligible[selected]->SetHealth(eligible[selected]->GetHealth() + 1);
        }
    }
    season14.cthunEndTurnTargetCount = 0;
    season14.cthunEndTurnApplications = 0;
    season14.cthunEndTurnPending = false;
}

void Player::AdvanceCthunUpgrade() noexcept
{
    if (season14.heroPowerDbfID == 66246 && season14.cthunRepeatCount < 31)
        ++season14.cthunRepeatCount;
}

void Player::ResolveTierMinionStartCombat()
{
    if (season14.heroPowerDbfID != 66197) return;
    std::int32_t tier = 0;
    if (!season14.TakeTierMinionStartCombat(tier)) return;
    std::vector<Card> candidates;
    for (const auto& card : Cards::GetAllCards())
        if (card.isBattlegroundsPoolMinion && card.GetCardType() == CardType::MINION &&
            card.normalDbfID == 0 && card.hasBehavior && card.GetTier() == tier)
            candidates.push_back(card);
    if (candidates.empty()) return;
    const auto card = candidates[Random::get<std::size_t>(0, candidates.size() - 1)];
    if (!battleField.IsFull()) {
        Minion summoned(card);
        ApplyFreshMinionModifiers(summoned);
        summoned.getPlayerCallback = [this]() -> Player& { return *this; };
        if (getNextCardIndexCallback) summoned.SetIndex(getNextCardIndexCallback());
        battleField.Add(summoned, battleField.GetCount());
        Minion& added = battleField[battleField.GetCount() - 1];
        battleField.ForEachAlive([&added](MinionData& data) {
            data.value().ActivateTrigger(TriggerType::SUMMON, added);
        });
        ApplySummonTrinkets(added);
    }
    if (!hand.IsFull()) {
        Minion copy(card);
        ApplyFreshMinionModifiers(copy);
        hand.Add(CardData{std::move(copy)});
    }
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
    // Spellcraft cadence is equivalent to SuccessfulSpellCount() % 3 == 0;
    // the typed counter below advances exactly once per successful spell.
    recruitField.ForEachAlive([kind](MinionData& data) {
        data.value().ApplyDarkGiftCounterStep(kind);
    });
    if (kind == 3) {
        std::vector<Minion*> felboars;
        recruitField.ForEachAlive([&](MinionData& data) {
            if ((data.value().GetCardID() == "BG28_633" || data.value().GetCardID() == "BG28_633_G") &&
                data.value().AdvanceFelboarSpellCounter()) felboars.push_back(&data.value());
        });
        std::vector<Minion*> candidates;
        for (auto* felboar : felboars) {
            candidates.clear();
            tavern.fieldZone.ForEachAlive([&](MinionData& data) { candidates.push_back(&data.value()); });
            if (candidates.empty()) break;
            auto* consumed = candidates[Random::get<std::size_t>(0, candidates.size() - 1)];
            const int attack = consumed->GetAttack(), health = consumed->GetHealth();
            const int poolIndex = consumed->GetPoolIndex();
            tavern.fieldZone.Remove(*consumed);
            if (returnMinionCallback) returnMinionCallback(poolIndex);
            const int multiplier = felboar->GetCardID() == "BG28_633_G" ? 2 : 1;
            felboar->SetAttack(felboar->GetAttack() + attack * multiplier);
            felboar->SetHealth(felboar->GetHealth() + health * multiplier);
        }
    }
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
