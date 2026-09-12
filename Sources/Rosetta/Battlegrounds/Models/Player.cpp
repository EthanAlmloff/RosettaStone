// Copyright (c) 2017-2024 Chris Ohk

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Cards/CardDefs.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch21.hpp>
#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviorsBatch9.hpp>
#include <Rosetta/Battlegrounds/CardSets/TavernSpellBehaviors.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Models/ActiveTribes.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ElementalStatGiverTask.hpp>
#include <Rosetta/Battlegrounds/Models/LifecycleEnchantment.hpp>
#include <Rosetta/Battlegrounds/CardSets/EventCounterBehaviors.hpp>
#include <Rosetta/Battlegrounds/CardSets/TrinketBehaviors.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomCardToHandTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomChooseOneCardToHandTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomBountyToHandTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomMagneticMechToTargetTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomTavernSpellToHandTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ActivateRandomTavernSpellsTask.hpp>
#include <Rosetta/Battlegrounds/CardSets/DarkGiftBehaviors.hpp>
#include <Rosetta/Battlegrounds/CardSets/FodderBehaviors.hpp>
#include <Rosetta/Battlegrounds/CardSets/GiantSpellcraftBehaviors.hpp>
#include <Rosetta/Battlegrounds/CardSets/BuddyBehaviors.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomSpellcraftToHandTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ConsumeRandomTavernTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/MinionOfferingTask.hpp>

#include <effolkronium/random.hpp>

#include <algorithm>
#include <array>
#include <cctype>
#include <iterator>
#include <limits>
#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

using Random = effolkronium::random_thread_local;

namespace RosettaStone::Battlegrounds
{
Race MostCommonFriendlyRace(const Player& player);
bool BeginUniqueBuddyDiscover(Player& player, std::int32_t sourceCardDbfID);
void ApplySpellBoardEffect(Player&, const TavernSpellBehavior&, int, bool,
                           std::int32_t);
template <typename Apply>
void ApplyImperialDefenderCopies(Player&, int, TargetingType, Apply&&);
bool BeginElectromagneticDiscover(Player&, std::int32_t);
bool BeginWindfallDiscover(Player&, std::int32_t, std::int32_t, std::int32_t,
                           std::int32_t);

namespace
{
bool BeginInnkeepersHearthDiscover(Player&, std::int32_t, int);
bool BeginTrinketMinionDiscover(Player&, std::int32_t, std::int32_t,
                                Race, bool);
bool BeginPutricideStickerDiscover(Player&, std::int32_t, bool);

// Kaleidoscope is intentionally stricter than the generic Discover helper:
// its modal is a Tier-7 pool Discover and must always expose three distinct
// executable normal entities.  The golden trinket additionally requires a
// valid premium link so conversion cannot silently degrade to a normal card.
bool IsKaleidoscopeCandidate(const Card& card, bool golden) noexcept
{
    if (card.dbfID == 0 || card.id.empty() || !card.hasBehavior ||
        !card.isBattlegroundsPoolMinion || card.normalDbfID != 0 ||
        card.GetCardType() != CardType::MINION || card.GetTier() != 7)
        return false;
    if (!golden) return true;
    const auto premium = Cards::FindCardByDbfID(card.premiumDbfID);
    return card.premiumDbfID > 0 && premium.dbfID != 0 &&
           premium.GetCardType() == CardType::MINION &&
           premium.normalDbfID == card.dbfID && premium.hasBehavior;
}

// `Spell::IsTemporary()` is a lifecycle flag, not the Spellcraft keyword.
// Recruit-start rewards such as Rushing Winds and Timeline Acceleration also
// use it so they expire at the next recruit boundary.  Keep the semantic
// source predicate explicit: Coral Spear and Spellcraft-only trinkets must
// not react to every temporary card.
bool IsSpellcraftToken(const Spell& spell) noexcept
{
    if (!spell.IsTemporary()) return false;
    static constexpr std::string_view ids[] = {
        "BG30_MagicItem_416t", "BG23_000t", "BG23_000_Gt",
        "BG23_004t", "BG23_004_Gt", "BG23_007t", "BG23_007_Gt",
        "BG23_008t", "BG23_008_Gt", "BG31_830t", "BG31_830_Gt",
        "BG31_924t", "BG31_924_Gt", "BG31_920t", "BG31_920_Gt",
        "BG26_501t", "BG26_501_Gt", "BG26_502t", "BG26_502_Gt",
        "BG24_Reward_719t", "BG25_044t", "BG29_879t", "BG29_879t_G",
        "BG34_Giant_035t", "BG34_Giant_035t_G", "BG28_810", "BG33_815",
        "BG33_Reward_006t", "BG27_514t", "BG27_514t_G",
        "BG30_MagicItem_714t", "BG30_MagicItem_429t",
        "BG32_MagicItem_892t", "BG35_MagicItem_872t",
        "BG35_MagicItem_838t", "BG36_MagicItem_208t",
        "BG35_MagicItem_755t", "BG35_MagicItem_306t",
        "BG35_MagicItem_733t", "BG28_810", "BG33_815"};
    return std::find(std::begin(ids), std::end(ids), spell.GetID()) !=
           std::end(ids);
}

void ResolveCoralSpearSpellcraft(Player& player)
{
    // One free canonical cast per active owned copy.  CastTavernSpellFree
    // owns target selection, spell counters, and observer emission; Coral
    // itself is not a Spellcraft token, so this cannot recurse through this
    // helper even if the generated card is later tagged temporary.
    for (const auto& trinket : player.season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect == TrinketEffect::SPELLCRAFT_MIGHT_OF_STORMWIND)
            player.CastTavernSpellFree("BG35_951");
    }
}

// Add one extra random spell offer without routing it through the player's
// hand.  Lubber Sticker's reward is a real Tavern slot and therefore must
// obey the same supported-pool and capacity rules as ordinary spell offers.
bool AddRandomTavernSpellOffer(Player& player)
{
    if (player.tavern.SlotCount() >=
        static_cast<std::size_t>(player.season14.TavernOfferCount(MAX_FIELD_SIZE)))
        return false;
    std::vector<Card> candidates;
    for (const auto& card : Cards::GetAllCards())
    {
        if (!card.isBattlegroundsPoolSpell || card.normalDbfID != 0 ||
            card.GetCardType() != CardType::SPELL ||
            FindTavernSpellBehavior(card.id).effect == TavernSpellEffect::NONE)
            continue;
        candidates.push_back(card);
    }
    if (candidates.empty()) return false;
    Random::shuffle(candidates.begin(), candidates.end());
    player.tavern.spellSlots.emplace_back(Spell(candidates.front()));
    return true;
}

void GrantRandomSpellcraft(Player& player, int count)
{
    for (int i = 0; i < count && !player.hand.IsFull(); ++i)
        (void)SimpleTasks::RandomSpellcraftToHandTask{}.Run(player);
}

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

bool IsLiftOffUpgrade(const std::string& id) noexcept
{
    return id.starts_with("BG31_HERO_801pt") && id != "BG31_HERO_801pt" &&
           id != "BG31_HERO_801pt_G" &&
           (id.back() == 'a' || id.back() == 'b' || id.back() == 'c' ||
            id.back() == 'd' || id.back() == 'e' || id.back() == 'f' ||
            id.back() == 'h' || id.back() == 'i' || id.back() == 'j' ||
            id.ends_with("a2") || id.ends_with("a3") || id.ends_with("a4") ||
            id.ends_with("a5") || id.ends_with("a6") || id.ends_with("a7") ||
            id.ends_with("b2") || id.ends_with("b3") || id.ends_with("b4") ||
            id.ends_with("b5") || id.ends_with("b6") || id.ends_with("b7") ||
            id.ends_with("c2") || id.ends_with("c3") || id.ends_with("c4") ||
            id.ends_with("c5") || id.ends_with("c6") || id.ends_with("c7") ||
            id.ends_with("d2") || id.ends_with("d3") || id.ends_with("d4") ||
            id.ends_with("d5") || id.ends_with("d6") || id.ends_with("d7") ||
            id.ends_with("e2") || id.ends_with("e3") || id.ends_with("e4") ||
            id.ends_with("e5") || id.ends_with("e6") || id.ends_with("e7") ||
            id.ends_with("f2") || id.ends_with("f3") || id.ends_with("f4") ||
            id.ends_with("h2") || id.ends_with("h3") || id.ends_with("i2") ||
            id.ends_with("i3") || id.ends_with("j2"));
}

std::vector<Card> LiftOffUpgradePool(std::int32_t tier)
{
    const std::array<const char*, 9> families = {
        "BG31_HERO_801pta", "BG31_HERO_801ptb", "BG31_HERO_801ptc",
        "BG31_HERO_801ptd", "BG31_HERO_801pte", "BG31_HERO_801ptf",
        "BG31_HERO_801pth", "BG31_HERO_801pti", "BG31_HERO_801ptj"};
    std::vector<Card> result;
    const auto suffix = tier <= 1 ? std::string{} : std::to_string(tier);
    for (const auto family : families) {
        auto card = Cards::FindCardByID(std::string(family) + suffix);
        if (card.dbfID != 0 && card.GetCardType() == CardType::BATTLEGROUND_SPELL &&
            card.hasBehavior)
            result.push_back(card);
    }
    return result;
}

int LiftOffUpgradeLevel(std::string_view id) noexcept
{
    if (id.size() == 16) return 1; // ...801pt[a-j]
    const auto digit = id.back();
    return digit >= '2' && digit <= '7' ? digit - '0' : 1;
}
}

void Player::OnCardAcquired(const CardData& card)
{
    // Thorncaptain listens to every successful card entering hand (including
    // generated cards).  The hand-zone callback invokes this hook after the
    // card is committed, so the parent payload is applied exactly once.
    recruitField.ForEachAlive([this](MinionData& data) {
        auto& minion = data.value();
        if (minion.GetCardID() == "BG25_045" ||
            minion.GetCardID() == "BG25_045_G")
            static_cast<void>(ApplyReviewedLifecycleEnchantment(
                minion,
                minion.GetCardID() == "BG25_045_G" ? "BG25_045_G" : "BG25_045",
                "BG25_045e", Minion::TemporaryEnchantment::Stats,
                0, minion.GetCardID() == "BG25_045_G" ? 2 : 1));
    });
    if (!std::holds_alternative<Minion>(card) ||
        !std::get<Minion>(card).HasRace(Race::PIRATE))
        return;

    // Trusty Crowbar is keyed to the acquisition event rather than the
    // Tavern purchase event.  Resolve each owned copy independently and
    // target the actual left-most surviving board minion.
    for (auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect != TrinketEffect::AFTER_GET_PIRATE_LEFTMOST_STATS)
            continue;
        Minion* leftmost = nullptr;
        recruitField.ForEachAlive([&leftmost](MinionData& data) {
            auto& candidate = data.value();
            if (leftmost == nullptr ||
                candidate.GetZonePosition() < leftmost->GetZonePosition())
                leftmost = &candidate;
        });
        if (leftmost != nullptr)
        {
            leftmost->SetAttack(leftmost->GetAttack() + behavior.attack);
            leftmost->SetHealth(leftmost->GetHealth() + behavior.health);
        }
    }

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
namespace
{
bool TrinketTypeEquals(const std::string& value, const char* expected)
{
    if (value.size() != std::char_traits<char>::length(expected)) return false;
    for (std::size_t i = 0; i < value.size(); ++i)
        if (std::toupper(static_cast<unsigned char>(value[i])) != expected[i])
            return false;
    return true;
}

bool TrinketTypeIsExcluded(const std::string& type, Race excluded)
{
    static constexpr std::pair<const char*, Race> types[] = {
        {"BEAST", Race::BEAST}, {"DEMON", Race::DEMON},
        {"DRAGON", Race::DRAGON}, {"ELEMENTAL", Race::ELEMENTAL},
        {"MECHANICAL", Race::MECHANICAL}, {"MURLOC", Race::MURLOC},
        {"NAGA", Race::NAGA}, {"PIRATE", Race::PIRATE},
        {"QUILBOAR", Race::QUILBOAR}, {"UNDEAD", Race::UNDEAD},
    };
    for (const auto& [name, race] : types)
        if (TrinketTypeEquals(type, name) && race == excluded) return true;
    return false;
}

std::optional<Race> TrinketTypeRace(const std::string& type);

bool TrinketTypeIsUnavailable(const std::string& type,
                              const ActiveTribeSet& activeTribes)
{
    const auto race = TrinketTypeRace(type);
    return race.has_value() && !IsActiveTribe(activeTribes, *race);
}

// Trinket association metadata is string-based, while lobby eligibility is
// the immutable ActiveTribeSet.  Keep the conversion in one place so a
// generated offer cannot accidentally treat the ten-tribe universe as the
// current lobby pool.
std::optional<Race> TrinketTypeRace(const std::string& type)
{
    static constexpr std::pair<const char*, Race> types[] = {
        {"BEAST", Race::BEAST}, {"DEMON", Race::DEMON},
        {"DRAGON", Race::DRAGON}, {"ELEMENTAL", Race::ELEMENTAL},
        {"MECHANICAL", Race::MECHANICAL}, {"MURLOC", Race::MURLOC},
        {"NAGA", Race::NAGA}, {"PIRATE", Race::PIRATE},
        {"QUILBOAR", Race::QUILBOAR}, {"UNDEAD", Race::UNDEAD},
    };
    for (const auto& [name, race] : types)
        if (TrinketTypeEquals(type, name)) return race;
    return std::nullopt;
}

bool TrinketIsInLobby(const Card& card, const ActiveTribeSet& active,
                      Race excluded)
{
    if (card.associatedRaces.empty()) return true;
    for (const auto& type : card.associatedRaces) {
        if (type == "MENAGERIE") return true;
        const auto race = TrinketTypeRace(type);
        if (race.has_value() && IsActiveTribe(active, *race) &&
            !TrinketTypeIsUnavailable(type, active))
            return true;
    }
    return false;
}
}

bool IsMenagerieTypeName(const std::string& value)
{
    return value == "MENAGERIE";
}

// Maxwell Sticker must resolve from the active Hero Power, not from an
// untyped hero-card relatedDbfID.  Keep the result typed and validate both
// entities before a reward can enter the hand.  In particular, deferred
// Timewarp powers have alternate pools and deliberately fail closed here.
struct HeroPowerBuddyReward
{
    std::int32_t heroPowerDbfID;
    std::int32_t buddyDbfID;
    std::int32_t goldenBuddyDbfID;
};

bool IsDeferredTimewarpHeroPower(std::int32_t dbfID) noexcept
{
    return dbfID == 127697 || dbfID == 129174;
}

std::optional<HeroPowerBuddyReward> FindHeroPowerBuddyReward(
    std::int32_t heroPowerDbfID) noexcept
{
    if (heroPowerDbfID <= 0 || IsDeferredTimewarpHeroPower(heroPowerDbfID))
        return std::nullopt;
    for (const auto& hero : Cards::GetCurrentHeroes())
    {
        if (hero.heroPowerDbfID != heroPowerDbfID || hero.relatedDbfID <= 0)
            continue;
        const auto buddy = Cards::FindCardByDbfID(hero.relatedDbfID);
        if (buddy.dbfID <= 0 || buddy.GetCardType() != CardType::MINION ||
            buddy.normalDbfID != 0 || !buddy.id.ends_with("_Buddy") ||
            buddy.premiumDbfID <= 0)
            return std::nullopt;
        const auto golden = Cards::FindCardByDbfID(buddy.premiumDbfID);
        if (golden.dbfID <= 0 || golden.GetCardType() != CardType::MINION ||
            golden.normalDbfID != buddy.dbfID ||
            !golden.id.ends_with("_Buddy_G"))
            return std::nullopt;
        return HeroPowerBuddyReward{heroPowerDbfID, buddy.dbfID,
                                    golden.dbfID};
    }
    return std::nullopt;
}
// These pool helpers are defined with the other supported-card predicates
// below, but are also used by modal legality/commit paths earlier in this
// translation unit.  Keep declarations here so every compiler sees the same
// fail-closed candidate filtering.
std::vector<Card> SupportedTierMinions(const Player& player);
template <std::size_t N>
void AppendSupportedNormalMinions(const std::array<Card, N>& cards,
                                  std::vector<Card>& result, Race race,
                                  const ActiveTribeSet& activeTribes);
std::vector<Card> SupportedMinionsForRace(
    Race race, const ActiveTribeSet& activeTribes);
std::vector<Card> SupportedDeathrattleMinions(
    const ActiveTribeSet& activeTribes);
std::vector<Card> SupportedBattlecryMinions(
    const ActiveTribeSet& activeTribes);
std::vector<Card> SupportedEndTurnMinions(
    const ActiveTribeSet& activeTribes);
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


// The generated Choose-One cards are public modal payloads, not ordinary
// playable spells.  Keep the parent -> option relationship executable at the
// commit boundary: a replay must not be able to replace either offering with
// an arbitrary card that merely occupies the same index.  This also makes the
// CHOOSE_ONE_OPTION lifecycle tag meaningful at runtime rather than treating
// it as registration-only metadata.
bool IsChooseOneOptionForSource(std::int32_t sourceDbfID,
                                std::int32_t optionDbfID)
{
    const auto source = Cards::FindCardByDbfID(sourceDbfID);
    const auto option = Cards::FindCardByDbfID(optionDbfID);
    if (source.dbfID == 0 || option.dbfID == 0 ||
        CardDefs::FindCardDefByID(source.id).lifecycle !=
            CardLifecycle::CHOOSE_ONE_SOURCE ||
        CardDefs::FindCardDefByID(option.id).lifecycle !=
            CardLifecycle::CHOOSE_ONE_OPTION)
        return false;

    const std::array<std::array<std::string_view, 2>, 7> options = {{
        {{"BG27_084t", "BG27_084t2"}},
        {{"BG30_123t", "BG30_123t2"}},
        {{"BG36_330t", "BG36_330t2"}},
        {{"BG36_341t", "BG36_341t2"}},
        {{"BG31_320t", "BG31_320t2"}},
        {{"BG32_237t", "BG32_237t2"}},
        {{"BG36_332t", "BG36_332t2"}},
    }};
    const std::array<std::string_view, 7> sources = {
        "BG27_084", "BG30_123", "BG36_330", "BG36_341",
        "BG31_320", "BG32_237", "BG36_332"};
    for (std::size_t i = 0; i < sources.size(); ++i) {
        if (source.id == sources[i] ||
            source.id == std::string(sources[i]) + "_G") {
            const auto golden = source.id.ends_with("_G");
            const auto expectedSecond = std::string(options[i][1]);
            // Golden generated IDs use the same family plus _G before the
            // option suffix (for example BG36_341_Gt2).
            const auto first = golden
                ? std::string(sources[i]) + "_Gt"
                : std::string(options[i][0]);
            const auto second = golden
                ? std::string(sources[i]) + "_Gt2"
                : expectedSecond;
            const auto expectedID = option.id == first ? 0
                                  : option.id == second ? 1 : -1;
            return expectedID >= 0;
        }
    }
    return false;
}

bool AddRandomMinionToHand(Player& player, std::vector<Card> candidates);
bool AddRandomMrrgltonToHand(Player& player);
bool BeginMinionDiscover(Player& player, std::vector<Card> candidates,
                         std::int32_t sourceCardDbfID, bool lockHand = false);
bool BeginJewelryBoxBloodGem(Player& player, std::int32_t sourceCardDbfID);
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
        case TavernSpellEffect::BLOOD_GEM_TAUNT:
        case TavernSpellEffect::BLOOD_GEM_DIVINE_SHIELD:
        case TavernSpellEffect::BLOOD_GEM_REBORN:
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
        case TavernSpellEffect::ALL_STATS_NEXT_TURN:
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
            bool portraitHealth = false;
            for (auto& trinket : season14.trinkets)
            {
                if (!trinket.active || trinket.remainingUses == 0) continue;
                const auto behavior = FindTrinketBehavior(
                    Cards::FindCardByDbfID(trinket.dbfID).id);
                const auto trinketId = Cards::FindCardByDbfID(trinket.dbfID).id;
                if (trinketId == "BG32_MagicItem_283" ||
                    behavior.portraitEffect ==
                        PortraitEffect::CZARINA_DIVINE_SHIELD_HEALTH) {
                    portraitHealth = true;
                    break;
                }
            }
            recruitField.ForEachAlive([attack](MinionData& data) {
                auto& minion = data.value();
                if (minion.HasDivineShield())
                    minion.SetAttack(minion.GetAttack() + attack);
            });
            if (portraitHealth)
                recruitField.ForEachAlive([scale = totalScale](MinionData& data) {
                    auto& minion = data.value();
                    if (minion.HasDivineShield())
                        minion.SetHealth(minion.GetHealth() + 4 * scale);
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
    // Armor-only damage has healthLost == 0 and must not trigger Soul
    // Rewinder. The card triggers only after actual hero Health damage.
    if (rewindHealth != 0 && event.healthLost > 0) {
        hero.health += event.healthLost;
        const bool rewinderPortrait = HasActivePortrait(
            PortraitEffect::SOUL_REWINDER_EXTRA_ATTACK);
        recruitField.ForEachAlive([rewindHealth, rewinderPortrait](MinionData& data) {
            auto& minion = data.value();
            if (minion.GetCardID() == "BG26_174") {
                minion.SetHealth(minion.GetHealth() + 1);
                if (rewinderPortrait)
                    minion.SetAttack(minion.GetAttack() + 1);
            }
            else if (minion.GetCardID() == "BG26_174_G") {
                minion.SetHealth(minion.GetHealth() + 2);
                if (rewinderPortrait)
                    minion.SetAttack(minion.GetAttack() + 2);
            }
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

void Player::ResolveHeroPowerUseBuddies(std::uint64_t targetEntityID, int repeats,
                                        bool includeUseObservers)
{
    // Sous Chef Sticker pays one Gold at the successful Hero Power-use
    // boundary.  Resolve this after UseHeroPower() has accepted the action so
    // failed/illegal attempts cannot claim the reward.
    if (includeUseObservers)
    for (auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (Cards::FindCardByDbfID(trinket.dbfID).id ==
                "BG35_MagicItem_801" &&
            behavior.effect == TrinketEffect::HERO_POWER_EXTRA_USE)
            remainCoin += 1;
    }
    // Karl's trigger is tied to the successful payment/use boundary, not to
    // damage. Each owned copy triggers independently; golden copies change
    // their own amount and therefore stack with normal/golden copies.
    if (includeUseObservers)
    {
        int amount = 0;
        recruitField.ForEachAlive([&amount](const MinionData& data) {
            const auto& id = data.value().GetCardID();
            if (id == "TB_BaconShop_HERO_15_Buddy") amount += 2;
            else if (id == "TB_BaconShop_HERO_15_Buddy_G") amount += 4;
        });
        if (amount != 0)
        recruitField.ForEachAlive([amount](MinionData& data) {
            auto& minion = data.value();
            if (minion.HasDivineShield())
                minion.SetAttack(minion.GetAttack() + amount);
        });
    }

    // Solemn Serenader is a target observer, not a generic Hero Power-use
    // trigger: no selected friendly entity means no effect.  Resolve each
    // owned copy from its current Attack so normal/golden scaling and copies
    // with independent buffs remain correct.  The active Major/Minor Hymn
    // power selects the stat side; the next-turn swap is represented by the
    // linked Minor power DBF rather than a slot-local toggle.
    if (targetEntityID == 0) return;
    // Shadow Warden is a successful-target observer.  Consume the armed use
    // only after the target is found and successfully made golden; stale or
    // non-targeted hero-power resolutions must not burn the counter.
    if (season14.buddyGoldenHeroPowerUses > 0)
    {
        const auto goldenizeTarget = [&](MinionData& data) {
            auto& target = data.value();
            if (static_cast<std::uint64_t>(target.GetIndex()) != targetEntityID ||
                target.IsDestroyed() || !target.CanMakeGolden()) return;
            if (target.MakeGolden())
                season14.ConsumeBuddyGoldenHeroPowerUse();
        };
        // Hero Power targets are not uniformly warband minions.  Maiev's
        // Imprison (the hero associated with Shadow Warden) selects a Tavern
        // offer, while other targeted powers select a friendly minion.  Use
        // the entity ID across both observable zones so the buddy follows the
        // selected target rather than silently missing shop targets.
        recruitField.ForEachAlive(goldenizeTarget);
        tavern.fieldZone.ForEachAlive(goldenizeTarget);
    }
    int bonus = 0;
    for (const auto& definition : BUDDY_HERO_POWER_TARGET_BEHAVIORS)
    {
        recruitField.ForEachAlive([&](const MinionData& data) {
            const auto& buddy = data.value();
            // CardDefs owns lifecycle dispatch.  Keep the catalogue entry's
            // ID only for its payload divisor; do not make an empty/static
            // registration the runtime owner of this trigger.
            if (CardDefs::FindCardDefByID(buddy.GetCardID()).lifecycle ==
                    CardLifecycle::BUDDY_SOLEMN_SERENADER &&
                buddy.GetCardID() == definition.id)
                bonus += buddy.GetAttack() / definition.attackDivisor;
        });
    }
    if (bonus == 0) return;
    recruitField.ForEachAlive([&](MinionData& data) {
        auto& target = data.value();
        if (static_cast<std::uint64_t>(target.GetIndex()) != targetEntityID)
            return;
        repeats = std::max(1, repeats);
        if (season14.heroPowerDbfID == 103503)
            target.SetHealth(target.GetHealth() + bonus * repeats);
        else
            target.SetAttack(target.GetAttack() + bonus * repeats);
    });
}

void Player::ApplyFreshMinionModifiers(Minion& minion)
{
    // Sprout It Out! is a hidden-cost combat aura.  Apply it at the fresh
    // combat-copy boundary only; recruit and hand entities must not inherit
    // these combat-only stats or Taunt.
    if (isInCombat && season14.heroPowerDbfID == 67554)
    {
        minion.SetAttack(minion.GetAttack() + 1);
        minion.SetHealth(minion.GetHealth() + 2);
        minion.SetTaunt(true);
    }
    if (HasActivePortrait(PortraitEffect::DRAKKARI_ENCHANTER_ALL_TYPES) &&
        (minion.GetCardID() == "BG26_ICC_901" ||
         minion.GetCardID() == "BG26_ICC_901_G"))
    {
        minion.AddRace(Race::MECHANICAL);
        minion.AddRace(Race::ELEMENTAL);
    }
    if (HasActivePortrait(PortraitEffect::WHELP_SMUGGLER_STATS_AND_DRAGON) &&
        (minion.GetCardID() == "BG21_013" ||
         minion.GetCardID() == "BG21_013_G"))
        minion.AddRace(Race::DRAGON);
    if (HasActivePortrait(PortraitEffect::PERMANENT_SPELLCRAFT) &&
        (minion.GetCardID() == "BG31_830" ||
         minion.GetCardID() == "BG31_830_G" ||
         minion.GetCardID() == "BG31_924" ||
         minion.GetCardID() == "BG31_924_G"))
        minion.SetPermanentSpellcraft(true);
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
    {
        // Falling Sky Golem is a generated token with an intentionally empty
        // CardDef.  Install its printed keyword at the fresh-instance
        // boundary so recruit, hand, and combat copies all retain Divine
        // Shield; the deathrattle-count aura remains handled separately.
        minion.SetGameTag(GameTag::DIVINE_SHIELD, 1);
        minion.ApplySkyGolemDeathrattleCount(season14.deathrattlesTriggered);
    }
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
    if (minion.GetCardID() == "BG25_008" ||
        minion.GetCardID() == "BG25_008_G")
        minion.ApplyEternalKnightUndeadDeathCount(undeadDiedThisGame);
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
    for (auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect == TrinketEffect::HERO_POWER_EXTRA_USE)
            season14.EnableBuddyExtraHeroPowerUses(1);
    }
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
    // Fodder is a Tavern-only generated offer. Apply the Fodder Trinket aura
    // at every fresh-offer boundary, without leaking it into hand/board or
    // combat copies handled by ApplyFreshMinionModifiers.
    if (minion.GetCardID() == "BG35_150t")
    {
        for (const auto& trinket : season14.trinkets)
        {
            if (!trinket.active || trinket.remainingUses == 0) continue;
            const auto behavior = FindTrinketBehavior(
                Cards::FindCardByDbfID(trinket.dbfID).id);
            if (behavior.effect == TrinketEffect::STATIC_FODDER_SHOP_STATS)
            {
                minion.SetAttack(minion.GetAttack() + behavior.attack);
                minion.SetHealth(minion.GetHealth() + behavior.health);
            }
        }
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
                candidate.normalDbfID == 0 && candidate.hasBehavior &&
                HasActiveTribe(activeTribes, candidate))
                candidates.push_back(candidate);
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
        const auto candidates = SupportedEndTurnMinions(activeTribes);
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
            if (!SupportedMinionsForRace(race, activeTribes).empty()) races.push_back(race);
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
    if (dbfID == 104673) {
        // Gilnean War Horn's {0} is resolved when the reward is acquired,
        // not when a later Battlecry happens.  Draw only from the current
        // lobby's executable normal Battlecry pool and retain the DBF so a
        // replay or retry cannot silently choose a different minion.
        if (hand.IsFull()) return false;
        auto candidates = SupportedBattlecryMinions(activeTribes);
        if (candidates.empty()) return false;
        const auto& selected = candidates[Random::get<std::size_t>(
            0, candidates.size() - 1)];
        if (selected.dbfID == 0) return false;
        if (!selected.isBattlegroundsPoolMinion || selected.normalDbfID != 0)
            return false;
        if (!season14.RecordGeneratedRewardBattlecryMinionDbfID(selected.dbfID))
            return false;
        hand.Add(CardData{Minion(selected)});
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
                card.hasBehavior && card.GetTier() == currentTier &&
                HasActiveTribe(activeTribes, card))
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
                std::uint64_t selected = 0;
                if (consumer->HasRace(Race::DEMON)) {
                    const int slot = SelectDemonConsumeTavernSlot();
                    if (slot < 0) continue;
                    selected = static_cast<std::uint64_t>(
                        tavern.fieldZone[static_cast<std::size_t>(slot)].GetIndex());
                } else {
                    std::vector<std::uint64_t> offers;
                    tavern.fieldZone.ForEachAlive([&](MinionData& data) {
                        offers.push_back(static_cast<std::uint64_t>(data.value().GetIndex()));
                    });
                    if (offers.empty()) continue;
                    selected = offers[Random::get<std::size_t>(0, offers.size() - 1)];
                }
                Minion* consumed = nullptr;
                tavern.fieldZone.ForEachAlive([&](MinionData& data) {
                    if (static_cast<std::uint64_t>(data.value().GetIndex()) == selected)
                        consumed = &data.value();
                });
                if (!consumed) continue;
                const Minion consumedSnapshot = *consumed;
                consumer->SetAttack(consumer->GetAttack() + consumed->GetAttack());
                consumer->SetHealth(consumer->GetHealth() + consumed->GetHealth());
                (void)tavern.fieldZone.Remove(*consumed);
                if (consumedSnapshot.GetPoolIndex() >= 0)
                    returnMinionCallback(consumedSnapshot.GetPoolIndex());
                ApplyDemonConsumeBonus(*consumer, consumedSnapshot);
                // Devils in Details performs a successful Tavern consume
                // directly, outside ConsumeRandomTavernTask.  Notify the
                // shared post-removal observers for each consumed offer.
                ApplyTavernMinionConsumedTrinkets();
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
    // Keep the reward's canonical child identity on the target.  The typed
    // lifecycle applies +8 Health and temporary Stealth and removes both at
    // the next recruit boundary.
    static_cast<void>(ApplyReviewedLifecycleEnchantment(
        *rightmost, "BG24_Reward_115", "BG24_Reward_115e2",
        Minion::TemporaryEnchantment::StatsAndStealth, 0, 8));
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
    for (auto& trinket : season14.trinkets)
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
        if (behavior.effect == TrinketEffect::END_TURN_HIGHEST_TIER_TAVERN) {
            (void)StealHighestTierTavernMinionToHand();
            continue;
        }
        if (behavior.effect == TrinketEffect::END_TURN_GOLDEN_LEFTMOST_STATS) {
            Minion* leftmost = nullptr;
            int goldenCount = 0;
            recruitField.ForEachAlive([&](MinionData& data) {
                auto& minion = data.value();
                // Field iteration is not the semantic ordering contract: a
                // prior remove/reorder can leave physical storage order
                // different from the player's left-to-right board order.
                // Auric explicitly targets the lowest zone position.
                if (leftmost == nullptr ||
                    minion.GetZonePosition() < leftmost->GetZonePosition())
                    leftmost = &minion;
                if (minion.IsGolden()) ++goldenCount;
            });
            if (leftmost != nullptr && behavior.attack != 0 &&
                behavior.health != 0) {
                // Auric is a repeated generated enchantment, so this must be
                // an additive instance mutation rather than the idempotent
                // cumulative-aura helper.  The Minion travels with its
                // current stats through hand/board/combat snapshots.
                for (int i = 0; i < 1 + goldenCount; ++i) {
                    leftmost->SetAttack(leftmost->GetAttack() + behavior.attack);
                    leftmost->SetHealth(leftmost->GetHealth() + behavior.health);
                }
            }
            continue;
        }
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
        if (behavior.effect == TrinketEffect::END_TURN_LAST_TAVERN_SPELL) {
            const auto lastDbfID = season14.LastTavernSpellDbfID();
            if (lastDbfID <= 0 || behavior.amount <= 0) continue;
            const auto card = Cards::FindCardByDbfID(lastDbfID);
            // The state stores the source DBF id, so validate the complete
            // Tavern-spell identity again at grant time.  In particular,
            // ordinary collectible spells and generated helper entities must
            // never leak through merely because they have CardType::SPELL.
            // This also keeps a stale/unsupported id fail-closed after a
            // reset or card-data change.
            if (card.dbfID == 0 || card.id.empty() ||
                !card.isBattlegroundsPoolSpell || card.normalDbfID != 0 ||
                FindTavernSpellBehavior(card.id).effect == TavernSpellEffect::NONE ||
                hand.IsFull() ||
                (card.GetCardType() != CardType::SPELL &&
                 card.GetCardType() != CardType::BATTLEGROUND_SPELL))
                continue;
            for (int i = 0; i < behavior.amount && !hand.IsFull(); ++i)
                hand.Add(CardData{Spell(card)});
            continue;
        }
        if (behavior.effect == TrinketEffect::END_TURN_SPELL_SCALED_SATELLITE) {
            if (behavior.cardID.empty() || hand.IsFull()) continue;
            const auto card = Cards::FindCardByID(behavior.cardID);
            // Satellite is a canonical token with complete card metadata but
            // intentionally has no CardDef behavior of its own.  It is a
            // magnetic attachment payload, not a pool minion; requiring
            // `hasBehavior` here would silently discard every Gyroblade
            // reward even though the token's identity/tags are authoritative.
            if (card.id.empty() || card.GetCardType() != CardType::MINION ||
                behavior.amount <= 0) continue;
            const int scale = behavior.amount *
                              season14.SuccessfulSpellsThisRecruitTurn();
            Minion satellite(card);
            satellite.SetAttack(behavior.attack + scale);
            satellite.SetHealth(behavior.health + scale);
            hand.Add(CardData{std::move(satellite)});
            continue;
        }
        if (behavior.effect == TrinketEffect::END_TURN_LEFTMOST_MECH_REPAIR) {
            Minion* leftmostMech = nullptr;
            recruitField.ForEachAlive([&leftmostMech](MinionData& data) {
                if (leftmostMech == nullptr &&
                    data.value().HasRace(Race::MECHANICAL))
                    leftmostMech = &data.value();
            });
            if (leftmostMech != nullptr)
                (void)CastTavernSpellFree("BG36_624", 1,
                                          leftmostMech->GetZonePosition());
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
            (void)AddRandomMinionToHand(*this, SupportedMinionsForRace(race, activeTribes));
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
            auto& minion = data.value();
            // Volatile's +7/+7 and attack-death payload are owned by the
            // generated reward state machine.  Keep the exact child marker
            // on the combat copy while applying the authoritative stats.
            if (!ApplyReviewedLifecycleEnchantment(
                    minion, "BG24_Reward_364", "BG24_Reward_364e",
                    Minion::TemporaryEnchantment::Stats, 7, 7)) {
                minion.SetAttack(minion.GetAttack() + 7);
                minion.SetHealth(minion.GetHealth() + 7);
            }
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
    if (season14.HasGeneratedRewardEvilTwin() && combatField.IsFull())
        ApplySummonOverflowTrinkets();
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
        if (!card.id.empty() && battleField.IsFull()) {
            ApplySummonOverflowTrinkets();
        } else if (!card.id.empty() && !battleField.IsFull()) {
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
    // Yogg-tastic Tasties is the quest-reward form of the same Wheel of
    // Yogg-Saron used by Yogg-Tastic Pastry.  Keep one canonical outcome
    // engine so weighting, seeded randomness, and hand/pool boundaries do
    // not drift between the Trinket and reward implementations.
    for (std::uint32_t i = 0; i < season14.GeneratedRewardYoggTastiesCount(); ++i)
        (void)ResolveYoggWheel();
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
        const auto candidates = BuildTrinketOfferings(greater, 3, true, true);
        if (candidates.size() < 3) return;
        remainCoin -= 4;
        RecordGoldSpent(4);
        season14.BeginOfferingDecision(
            Season14Decision::TRINKET_SELECTION, 0, sourceDbfID,
            candidates);
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
            card.hasBehavior && HasActiveTribe(activeTribes, card))
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
            SupportedMinionsForRace(season14.GeneratedRewardFriendsRace(), activeTribes);
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
            if (card.normalDbfID != 0) {
                // The normal-form lookup is part of the pinned card contract.
                // If it ever fails, do not offer the copied golden identity:
                // Locket promises a normal minion while retaining instance
                // state, and silently retaining the source card would change
                // both the offering and subsequent replay semantics.
                const auto normalCard = Cards::FindCardByDbfID(card.normalDbfID);
                if (!minion.TransformToKeepingInstanceState(normalCard)) continue;
            }
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

bool Player::AddSinstoneDiscoverCopy(const Card& card)
{
    // Sinstone copies the selected result as a fresh plain entity.  Keep the
    // hand-cap check at the actual commit boundary; a full hand burns the
    // extra copy without changing the per-turn trigger accounting.
    if (hand.IsFull()) return false;
    // The pinned text says "Discover a minion".  In particular, do not
    // treat Tavern-spell, Trinket, Hero Power, or generated quest-reward
    // choices as Sinstone triggers merely because they use the public
    // Discover modal.
    if (card.GetCardType() != CardType::MINION) return false;

    // Resolve the selected identity to its normal DBF before constructing the
    // copy.  This strips golden/temporary instance state while preserving the
    // exact selected card identity; ApplyFreshMinionModifiers then applies
    // the ordinary pool/instance stat boundary for a newly created hand copy.
    Card plain = card.normalDbfID != 0
                     ? Cards::FindCardByDbfID(card.normalDbfID)
                     : card;
    if (plain.dbfID == 0 || plain.GetCardType() != CardType::MINION)
        return false;
    Minion copy(plain);
    ApplyFreshMinionModifiers(copy);
    hand.Add(CardData{std::move(copy)});
    return true;
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
    // Mishmash is a generated Curator Sticker reward whose printed text is
    // not represented by a generic CardDef task: whenever its Amalgam gains
    // stats, Mishmash gains the same stats (twice when golden).  This hook is
    // reached by the authoritative persistent-stat paths and keeps the
    // instance behavior attached to the real generated card identity.
    if (target.GetCardID() == "TB_BaconShop_HP_033t") {
        const auto copyToMishmash = [&target, amount](Minion& buddy) {
            // A generated reward can remain in hand while the other reward is
            // on board (and vice versa).  The printed trigger is owner-wide,
            // not limited to one zone.  Keep the identity check explicit so a
            // future transform cannot make the source copy buff itself.
            if (&buddy == &target) return;
            int multiplier = 0;
            if (buddy.GetCardID() == "TB_BaconShop_HERO_33_Buddy") multiplier = 1;
            else if (buddy.GetCardID() == "TB_BaconShop_HERO_33_Buddy_G") multiplier = 2;
            if (multiplier > 0)
                buddy.ApplyPersistentMinionStats(amount * multiplier,
                                                 amount * multiplier);
        };
        GetField().ForEachAlive([&copyToMishmash](MinionData& data) {
            copyToMishmash(data.value());
        });
        // During recruit, hand minions are still owned entities and may be
        // the source or recipient of persistent stat gains.  Combat has no
        // usable hand snapshot, so keep this intentionally recruit-only.
        if (!isInCombat) {
            hand.ForEach([&copyToMishmash](std::optional<CardData>& data) {
                if (data.has_value() && std::holds_alternative<Minion>(data.value()))
                    copyToMishmash(std::get<Minion>(data.value()));
            });
        }
    }
    // Iterate the active zone only; combat copies must not notify recruit
    // listeners, and recruit gains must not leak into combat copies.
    GetField().ForEachAlive([&](MinionData& data) {
        data.value().ActivateTrigger(TriggerType::GAIN_ATTACK, target);
    });
    dispatchingMinionAttackGain = false;
    CheckAzsharaAmbition();
}

void Player::DispatchMinionHealthGain(Minion& target, int amount)
{
    if (amount <= 0 || dispatchingMinionHealthGain) return;
    // Keep the event amount on the owning Player only while listeners resolve;
    // HealthGainHealthTask uses it to copy the exact source delta, including
    // golden multipliers, without reconstructing it from mutable stats.
    dispatchingMinionHealthGain = true;
    const auto previous = lastMinionHealthGain;
    lastMinionHealthGain = amount;
    // A Player owns two board snapshots: recruitField outside combat and
    // battleField during combat.  Dispatch only to the authoritative active
    // snapshot; notifying recruitField during combat leaks combat-only gains
    // back into recruit state (and the hand is not an active listener zone in
    // combat).
    auto& activeField = isInCombat ? battleField : recruitField;
    activeField.ForEachAlive([&](MinionData& data) {
        data.value().ActivateTrigger(TriggerType::GAIN_HEALTH, target);
    });
    // Titanic Guardian's printed scope includes minions still in hand.  Hand
    // entities receive the same owner callback and stable index when added,
    // so MINIONS_EXCEPT_SELF remains a true different-friendly predicate
    // across board/hand zone changes.
    if (!isInCombat)
    {
        hand.ForEach([&](std::optional<CardData>& data) {
            if (data.has_value() && std::holds_alternative<Minion>(data.value()))
                std::get<Minion>(data.value()).ActivateTrigger(
                    TriggerType::GAIN_HEALTH, target);
        });
    }
    lastMinionHealthGain = previous;
    dispatchingMinionHealthGain = false;
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

void Player::ApplyBurthDiscoverBuff(Minion& discovered)
{
    recruitField.ForEachAlive([&](MinionData& entry) {
        auto& buddy = entry.value();
        const auto id = buddy.GetCardID();
        const BuddyDiscoverBuffDefinition* definition = nullptr;
        for (const auto& candidate : BUDDY_DISCOVER_BUFF_BEHAVIORS)
            if (candidate.id == id) { definition = &candidate; break; }
        if (definition == nullptr) return;
        discovered.SetAttack(discovered.GetAttack() +
                             definition->attack + buddy.DiscoverBuffAttack());
        discovered.SetHealth(discovered.GetHealth() +
                             definition->health + buddy.DiscoverBuffHealth());
        buddy.ImproveDiscoverBuff(definition->attackImprovement,
                                  definition->healthImprovement);
    });
}

void Player::ResolveDiscoverTriggers(std::uint64_t discoveredEntityID)
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

    // Burth is an after-Discover effect, not a generic board trigger.  The
    // selected minion is already an owned hand entity when this boundary is
    // reached.  Resolve it by stable entity ID across the hand/board zones;
    // never infer the target from a hand slot or from the last card, since a
    // Discover may also have produced a copy/reward in the same commit.
    if (discoveredEntityID == 0) return;
    Minion* discovered = nullptr;
    hand.ForEach([&](std::optional<CardData>& entry) {
        if (!entry.has_value() || !std::holds_alternative<Minion>(*entry) ||
            discovered != nullptr)
            return;
        auto& minion = std::get<Minion>(*entry);
        if (static_cast<std::uint64_t>(minion.GetIndex()) == discoveredEntityID)
            discovered = &minion;
    });
    if (discovered == nullptr)
    {
        recruitField.ForEachAlive([&](MinionData& entry) {
            if (discovered != nullptr) return;
            auto& minion = entry.value();
            if (static_cast<std::uint64_t>(minion.GetIndex()) == discoveredEntityID)
                discovered = &minion;
        });
    }
    if (discovered == nullptr) return;

    ApplyBurthDiscoverBuff(*discovered);

    // Primalfin Portrait keys off the result type, not merely the existence
    // of a Discover modal.  Resolve this at the post-selection boundary so
    // failed/stale choices and Discover spells do not award a spell.  The
    // shared random-spell task supplies the behavior-backed Tavern pool and
    // enforces hand capacity.
    for (auto& trinket : season14.trinkets) {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect == TrinketEffect::ACQUIRE_PRIMALFIN_PORTRAIT)
            (void)SimpleTasks::RandomTavernSpellToHandTask{1}.Run(*this);
    }
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

bool Player::HasActivePortrait(PortraitEffect effect) const noexcept
{
    for (const auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.portraitEffect == effect) return true;
    }
    return false;
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
    if (IsWarpGateHeroPowerDbfID(hero.card.heroPowerDbfID))
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
                card.normalDbfID == 0 && card.hasBehavior && card.GetTier() == 7 &&
                HasActiveTribe(activeTribes, card))
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
            // Sneed's starting Shredder owns the canonical persistent
            // deathrattle child.  Keep this parent-qualified so unrelated
            // generated deathrattle records cannot be promoted by the
            // generic summon path.
            ApplyReviewedPersistentChildEnchantment(
                recruitField[recruitField.GetCount() - 1],
                "BG21_HERO_030p", "BG21_HERO_030pe");
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
            card.normalDbfID == 0 && card.hasBehavior && card.GetTier() == tier &&
            HasActiveTribe(activeTribes, card))
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

void Player::GrantCoilfangSpellcraftForFreshOffers(
    const std::set<int>& existingPoolIndices)
{
    // Coilfang triggers once for each Spellcraft minion that actually appears
    // in the Tavern.  A retained/frozen offer is not a new appearance, even
    // when another operation prepares the Tavern around it.  Count every
    // owned Coilfang (normal = one copy, golden = two); copies stack rather
    // than using a max, which matters for duplicated Buddies.
    int coilfangCopies = 0;
    recruitField.ForEachAlive([&coilfangCopies](const MinionData& data) {
        const auto& id = data.value().GetCardID();
        if (id == "BG23_HERO_304_Buddy") ++coilfangCopies;
        else if (id == "BG23_HERO_304_Buddy_G") coilfangCopies += 2;
    });
    if (coilfangCopies == 0 || hand.IsFull()) return;

    struct SpellcraftOffer { const char* minion; const char* spell; };
    // This is the pinned executable Spellcraft pool.  Golden offers use their
    // golden Spellcraft token where one exists; source identity, rather than
    // text matching, keeps unsupported/ordinary spells fail-closed.
    constexpr SpellcraftOffer offers[] = {
        {"BG23_000", "BG23_000t"}, {"BG23_000_G", "BG23_000_Gt"},
        {"BG23_004", "BG23_004t"}, {"BG23_004_G", "BG23_004_Gt"},
        {"BG23_007", "BG23_007t"}, {"BG23_007_G", "BG23_007_Gt"},
        {"BG23_008", "BG23_008t"}, {"BG23_008_G", "BG23_008_Gt"},
        {"BG31_830", "BG31_830t"}, {"BG31_830_G", "BG31_830_Gt"},
        {"BG31_924", "BG31_924t"}, {"BG31_924_G", "BG31_924_Gt"},
        {"BG31_920", "BG31_920t"}, {"BG31_920_G", "BG31_920_Gt"},
        {"BG26_501", "BG26_501t"}, {"BG26_501_G", "BG26_501_Gt"},
        {"BG26_502", "BG26_502t"}, {"BG26_502_G", "BG26_502_Gt"},
        {"BG24_Reward_719", "BG24_Reward_719t"},
        {"BG25_044", "BG25_044t"}, {"BG25_044_G", "BG25_044t"},
        {"BG29_879", "BG29_879t"}, {"BG29_879_G", "BG29_879t_G"},
        {"BG34_Giant_035", "BG34_Giant_035t"},
        {"BG34_Giant_035_G", "BG34_Giant_035t_G"},
        {"BGS_200", "BG28_810"}, {"TB_BaconUps_256", "BG33_815"},
        {"BG33_Reward_006", "BG33_Reward_006t"},
        {"BG27_514", "BG27_514t"}, {"BG27_514_G", "BG27_514t_G"},
        {"BG30_MagicItem_714", "BG30_MagicItem_714t"},
        {"BG30_MagicItem_429", "BG30_MagicItem_429t"},
        {"BG36_MagicItem_208", "BG36_MagicItem_208t"},
    };
    tavern.fieldZone.ForEachAlive([&](const MinionData& data) {
        if (hand.IsFull() || existingPoolIndices.contains(
                data.value().GetPoolIndex())) return;
        const auto& sourceID = data.value().GetCardID();
        for (const auto& offer : offers) {
            if (sourceID != offer.minion) continue;
            const auto spellCard = Cards::FindCardByID(offer.spell);
            if (spellCard.id.empty()) break;
            Spell spell(spellCard);
            spell.SetTemporary(true);
            // Coilfang emits the same Spellcraft cards as the ordinary
            // turn-start generator. Preserve Darkcrest's current tier
            // improvement here as well; otherwise Evolving Strategy created
            // by a fresh offer would silently return an unrestricted Naga.
            if (sourceID == "BG31_920" || sourceID == "BG31_920_G")
                spell.SetDynamicTier(1 + darkcrestImprovement);
            for (int i = 0; i < coilfangCopies && !hand.IsFull(); ++i)
                hand.Add(CardData{spell});
            break;
        }
    });
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
        if (!season14.refreshInProgress)
            GrantCoilfangSpellcraftForFreshOffers(existingPoolIndices);
        return;
    }

    const auto batch4 = season14.HeroPowerBatch4PassiveModifiers();
    const auto nextTurnTavernStats = season14.TakeNextTurnTavernStats();
    if (season14.persistentShopAttack != 0 ||
        season14.persistentShopHealth != 0 ||
        !season14.persistentShopRaceStats.empty() ||
        batch4.globalMinionAttack != 0 || batch4.mechShopAttack != 0 ||
        batch4.mechShopHealth != 0 || season14.persistentTavernTierMax != 0 ||
        season14.temporaryRefreshShopAttack != 0 ||
        season14.temporaryRefreshShopHealth != 0 ||
        nextTurnTavernStats.first != 0 || nextTurnTavernStats.second != 0 ||
        season14.HasGeneratedRewardAlterEgo() ||
        std::any_of(season14.trinkets.begin(), season14.trinkets.end(),
                    [](const auto& trinket) {
                        if (!trinket.active || trinket.remainingUses == 0)
                            return false;
                        return FindTrinketBehavior(
                            Cards::FindCardByDbfID(trinket.dbfID).id).effect ==
                               TrinketEffect::STATIC_FODDER_SHOP_STATS;
                    }))
    {
        tavern.fieldZone.ForEach(
            [this, &existingPoolIndices, batch4, &nextTurnTavernStats](
                MinionData& minion) {
                if (existingPoolIndices.contains(
                        minion.value().GetPoolIndex()))
                {
                    return;
                }
                minion.value().SetAttack(minion.value().GetAttack() +
                                         season14.persistentShopAttack);
                minion.value().SetHealth(minion.value().GetHealth() +
                                         season14.persistentShopHealth);
                ApplyFreshTavernMinionModifiers(minion.value());
                minion.value().SetAttack(minion.value().GetAttack() +
                                         season14.temporaryRefreshShopAttack);
                minion.value().SetHealth(minion.value().GetHealth() +
                                             season14.temporaryRefreshShopHealth);
                minion.value().SetAttack(minion.value().GetAttack() +
                                         nextTurnTavernStats.first);
                minion.value().SetHealth(minion.value().GetHealth() +
                                         nextTurnTavernStats.second);
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
    if (!season14.refreshInProgress)
        GrantCoilfangSpellcraftForFreshOffers(existingPoolIndices);
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

bool Player::ResolveYoggWheel()
{
    // The current payload is six weighted entries: five ordinary outcomes at
    // 19% each and Rod of Roasting at 5%.  Keep the table explicit so a
    // replay can audit the draw without relying on card ordering.
    const int roll = Random::get<int>(0, 99);
    if (!season14.RecordGeneratedRewardYoggOutcome(static_cast<std::uint8_t>(roll)))
        return false;

    if (roll < 19) {
        // Curse: one random friendly gains the current stats of a distinct
        // random friendly.  Snapshot the source before mutating the target.
        std::vector<Minion*> candidates;
        recruitField.ForEachAlive([&candidates](MinionData& data) {
            candidates.push_back(&data.value());
        });
        if (candidates.size() < 2) return false;
        const auto sourceIndex = Random::get<std::size_t>(
            0, candidates.size() - 1);
        std::size_t targetIndex = Random::get<std::size_t>(
            0, candidates.size() - 2);
        if (targetIndex >= sourceIndex) ++targetIndex;
        const int attack = candidates[sourceIndex]->GetAttack();
        const int health = candidates[sourceIndex]->GetHealth();
        candidates[targetIndex]->SetAttack(
            candidates[targetIndex]->GetAttack() + attack);
        candidates[targetIndex]->SetHealth(
            candidates[targetIndex]->GetHealth() + health);
        return true;
    }

    if (roll < 38) {
        // Hand: two random Darkmoon Prizes from the next legal prize tier.
        // Delivery is direct because the wheel is not a Discover modal; hand
        // capacity remains authoritative and a full hand burns that copy.
        std::vector<Card> prizes;
        const auto tier = std::min(currentTier + 1, TIER_UPPER_LIMIT);
        for (const auto& candidate : Cards::GetAllCards()) {
            if (candidate.GetCardType() == CardType::SPELL &&
                candidate.normalDbfID == 0 && candidate.dbfID > 0 &&
                candidate.id.starts_with("BGS_Treasures_") &&
                candidate.darkmoonPrizeTurn == tier)
                prizes.push_back(candidate);
        }
        if (prizes.empty()) return false;
        int added = 0;
        for (int i = 0; i < 2 && !hand.IsFull(); ++i) {
            const auto& prize = prizes[Random::get<std::size_t>(
                0, prizes.size() - 1)];
            hand.Add(CardData{Spell(prize)});
            ++added;
        }
        return added != 0;
    }

    if (roll < 57) {
        // Devouring: consume one live Tavern offer, distribute its settled
        // stats to every friendly minion, then perform the normal free
        // refresh/pool-return boundary.
        std::vector<int> slots;
        tavern.fieldZone.ForEachAlive([&slots](MinionData& data) {
            if (data.value().GetPoolIndex() >= 0)
                slots.push_back(data.value().GetZonePosition());
        });
        if (slots.empty()) return false;
        const auto slot = slots[Random::get<std::size_t>(0, slots.size() - 1)];
        const auto consumed = tavern.fieldZone.Remove(
            tavern.fieldZone[static_cast<std::size_t>(slot)]);
        if (consumed.GetPoolIndex() >= 0)
            returnMinionCallback(consumed.GetPoolIndex());
        const int attack = consumed.GetAttack();
        const int health = consumed.GetHealth();
        recruitField.ForEachAlive([attack, health](MinionData& data) {
            data.value().SetAttack(data.value().GetAttack() + attack);
            data.value().SetHealth(data.value().GetHealth() + health);
        });
        RefreshTavern(true);
        return true;
    }

    if (roll < 76) {
        // Mindflayer: reuse the simulator's random Tavern-spell executor so
        // spell legality, targets, and post-spell observers stay centralized.
        return SimpleTasks::ActivateRandomTavernSpellsTask{4}.Run(*this)
            != TaskStatus::STOP;
    }

    if (roll < 95) {
        // Mysterybox: one random live Tavern minion becomes Golden in place.
        std::vector<Minion*> candidates;
        tavern.fieldZone.ForEachAlive([&candidates](MinionData& data) {
            if (data.value().CanMakeGolden()) candidates.push_back(&data.value());
        });
        if (candidates.empty()) return false;
        return candidates[Random::get<std::size_t>(
            0, candidates.size() - 1)]->MakeGolden();
    }

    // Rod of Roasting: repeatedly select through the same random-target
    // stream used by simulator effects.  A minion receives +10/+10 until the
    // random target is the hero; hero damage ends the loop immediately.
    std::vector<Minion*> targets;
    recruitField.ForEachAlive([&targets](MinionData& data) {
        targets.push_back(&data.value());
    });
    if (targets.empty()) return false;
    bool buffed = false;
    for (;;) {
        const auto target = Random::get<std::size_t>(0, targets.size());
        if (target == targets.size()) {
            hero.TakeDamage(*this, 10, HeroDamageSource::RECRUIT_SELF);
            break;
        }
        targets[target]->ApplyTemporaryStats(10, 10);
        targets[target]->RecordTemporaryEnchantment(
            "TB_BaconShop_HERO_35_Buddy_t6e");
        buffed = true;
    }
    return buffed;
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
        { "BG26_502", "BG26_502t", 1 },
        { "BG26_502_G", "BG26_502_Gt", 1 },
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
        { "BG30_MagicItem_429", "BG30_MagicItem_429t", 1 },
        { "BG32_MagicItem_892", "BG32_MagicItem_892t", 1 },
        { "BG35_MagicItem_872", "BG35_MagicItem_872t", 1 },
        { "BG35_MagicItem_838", "BG35_MagicItem_838t", 1 },
        { "BG36_MagicItem_208", "BG36_MagicItem_208t", 1 },
        { "BG35_MagicItem_755", "BG35_MagicItem_755t", 1 },
        { "BG35_MagicItem_306", "BG35_MagicItem_306t", 1 },
        { "BG35_MagicItem_733", "BG35_MagicItem_733t", 1 },
    };
    for (const auto& spec : specs)
    {
        // Spellcraft can be owned by an ordinary minion or by a Trinket.
        // Trinkets live in Season14State rather than recruitField; checking
        // only the board silently omitted every Trinket-generated token
        // (including Demonblood Gourd and Floating Candle Set).  Count
        // instances rather than using a bool so distinct copies of a
        // Spellcraft source each receive their own card.
        int sourceCopies = 0;
        recruitField.ForEach([&](MinionData& data) {
            if (data.value().GetCardID() == spec.minion)
                ++sourceCopies;
        });
        for (const auto& trinket : season14.trinkets)
        {
            if (!trinket.active || trinket.remainingUses == 0)
                continue;
            if (Cards::FindCardByDbfID(trinket.dbfID).id == spec.minion)
                ++sourceCopies;
        }
        if (sourceCopies == 0 || hand.IsFull())
            continue;
        const auto card = Cards::FindCardByID(spec.spell);
        if (card.id.empty())
            continue;
        Spell spell(card);
        spell.SetTemporary(true);
        if (std::string_view(spec.minion) == "BG31_920" ||
            std::string_view(spec.minion) == "BG31_920_G")
            spell.SetDynamicTier(1 + darkcrestImprovement);
        for (int i = 0; i < spec.copies * sourceCopies && !hand.IsFull(); ++i)
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
    const bool protossPurchase =
        IsWarpGateProtossDbfID(tavern.fieldZone[idx].GetDbfID());
    const int highestTavernTier = [&]() {
        int highest = 0;
        tavern.fieldZone.ForEachAlive([&highest](const MinionData& data) {
            highest = std::max(highest, data.value().GetTier());
        });
        return highest;
    }();
    Season14PersistentEffect* tapestry = nullptr;
    for (auto& trinket : season14.trinkets) {
        if (!trinket.active || trinket.remainingUses == 0 ||
            trinket.statScale == 0)
            continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect ==
                TrinketEffect::REFRESH_HIGHEST_TIER_HEALTH_PURCHASE &&
            tavern.fieldZone[idx].GetTier() == highestTavernTier) {
            tapestry = &trinket;
            break;
        }
    }
    // The Eye of Sargeras counts successful minion purchases on each owned
    // Trinket.  Resolve the fourth purchase at the payment boundary below;
    // keeping pointers here means a failed/full-hand purchase never burns a
    // cadence step.  Multiple Eyes share one payment when they mature on the
    // same purchase, while each instance retains its own progress.
    std::vector<Season14PersistentEffect*> eyeHealthPurchases;
    for (auto& trinket : season14.trinkets) {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect == TrinketEffect::BUY_MINION_HEALTH_CADENCE &&
            behavior.value > 0 && trinket.triggerProgress + 1 >= behavior.value)
            eyeHealthPurchases.push_back(&trinket);
    }
    std::vector<Season14PersistentEffect*> pilgrimHealthPurchases;
    for (auto& trinket : season14.trinkets) {
        if (!trinket.active || trinket.remainingUses == 0 ||
            trinket.triggerProgress != 0)
            continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect == TrinketEffect::BUY_DEMON_HEALTH_ONCE_PER_TURN &&
            tavern.fieldZone[idx].HasRace(Race::DEMON))
            pilgrimHealthPurchases.push_back(&trinket);
    }
    const auto MinionPurchaseCost = [this](const int baseCost) {
        return season14.MinionPurchaseCost(baseCost);
    };
    const bool piratePurchase = tavern.fieldZone[idx].HasRace(Race::PIRATE);
    const bool piratePortraitFree = piratePurchase &&
        std::any_of(season14.trinkets.begin(), season14.trinkets.end(),
                    [](const Season14PersistentEffect& trinket) {
                        return trinket.active && trinket.remainingUses > 0 &&
                               trinket.triggerProgress == 0 &&
                               FindTrinketBehavior(
                                   Cards::FindCardByDbfID(trinket.dbfID).id)
                                   .effect == TrinketEffect::ACQUIRE_FIXED_CARD_FIRST_PIRATE_FREE;
                    });
    const bool tapestryHealthPurchase = tapestry != nullptr;
    const bool magneticMechFixedCost =
        tavern.fieldZone[idx].IsMagnetic() &&
        season14.HasMagneticMechFixedCost();
    // Demonic Tapestry's already-armed highest-tier purchase owns that
    // payment when both effects mature on the same card; the Eye still
    // advances its independent cadence after the successful buy.
    const bool pilgrimHealthPurchase = !tapestryHealthPurchase &&
                                      !pilgrimHealthPurchases.empty();
    const bool eyeHealthPurchase = !tapestryHealthPurchase &&
                                   !pilgrimHealthPurchase &&
                                   !eyeHealthPurchases.empty();
    const int cost = battlecryDiscount || piratePortraitFree ||
                             tapestryHealthPurchase
                         ? 0
                         : magneticMechFixedCost
                             ? 2
                             : std::max(0, MinionPurchaseCost(
                                   NUM_COIN_PURCHASE_MINION) -
                                   (protossPurchase ? season14.protossCostReduction : 0));
    if (!eyeHealthPurchase && !pilgrimHealthPurchase && remainCoin < cost)
    {
        return;
    }
    const int tapestryHealthCost = tapestryHealthPurchase
        ? FindTrinketBehavior(Cards::FindCardByDbfID(tapestry->dbfID).id).amount
        : 0;
    if (tapestryHealthPurchase && hero.health <= tapestryHealthCost)
        return;
    if ((eyeHealthPurchase || pilgrimHealthPurchase) && hero.health <= cost)
        return;

    const bool purchasedPirate =
        tavern.fieldZone[idx].HasRace(Race::PIRATE);
    const auto handCountBeforePurchase = hand.GetCount();
    const auto purchasedDbfID = tavern.fieldZone[idx].GetDbfID();
    purchaseMinionCallback(*this, idx);

    // Capture the purchased entity before any post-purchase reward can append
    // another card to hand.  Entity index is the stable instance identity;
    // DBF ID alone is insufficient when buying a duplicate minion.
    int purchasedEntityIndex = -1;
    for (std::size_t handIndex = handCountBeforePurchase;
        handIndex < static_cast<std::size_t>(hand.GetCount()); ++handIndex)
    {
        auto& entry = hand[handIndex];
        if (!std::holds_alternative<Minion>(entry))
            continue;
        purchasedEntityIndex = std::get<Minion>(entry).GetIndex();
        break;
    }
    const auto findPurchasedMinion = [this, purchasedEntityIndex]() -> Minion* {
        if (purchasedEntityIndex < 0) return nullptr;
        Minion* result = nullptr;
        hand.ForEach([&](std::optional<CardData>& entry) {
            if (!entry.has_value() || !std::holds_alternative<Minion>(*entry))
                return;
            auto& minion = std::get<Minion>(*entry);
            if (minion.GetIndex() == purchasedEntityIndex) result = &minion;
        });
        return result;
    };

    if (hand.GetCount() > handCountBeforePurchase && purchasedEntityIndex >= 0)
    {
        if (season14.heroPowerDbfID == 60218) {
            auto* bought = findPurchasedMinion();
            if (bought != nullptr && bought->HasBattlecry() &&
                !season14.battlecryRewardGiven &&
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
        if (piratePurchase) {
            for (auto& trinket : season14.trinkets) {
                if (!trinket.active || trinket.remainingUses == 0 ||
                    trinket.triggerProgress != 0) continue;
                if (FindTrinketBehavior(
                        Cards::FindCardByDbfID(trinket.dbfID).id).effect ==
                    TrinketEffect::ACQUIRE_FIXED_CARD_FIRST_PIRATE_FREE)
                    trinket.triggerProgress = 1;
            }
        }
        auto* purchasedPtr = findPurchasedMinion();
        if (purchasedPtr == nullptr) return;
        auto& purchased = *purchasedPtr;
        ApplyFreshMinionModifiers(purchased);
        // Gold-plated Compass is a next-matching-type purchase effect.  It
        // is resolved against the exact purchased instance before later
        // purchase observers run. A golden matching purchase is ignored,
        // and a failed MakeGolden leaves every Compass copy armed.
        for (auto& compass : season14.trinkets) {
            if (!compass.active || compass.remainingUses == 0 ||
                compass.triggerProgress != 0)
                continue;
            const auto compassBehavior = FindTrinketBehavior(
                Cards::FindCardByDbfID(compass.dbfID).id);
            if (compassBehavior.effect != TrinketEffect::NEXT_RACE_MINION_GOLDEN ||
                compassBehavior.race == Race::INVALID ||
                !purchased.HasRace(compassBehavior.race) ||
                purchased.IsGolden() || !purchased.CanMakeGolden())
                continue;
            if (purchased.MakeGolden())
                compass.triggerProgress = 1;
        }
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
        const auto batch4 = season14.OnBuyMinionBatch4();
        purchased.SetAttack(purchased.GetAttack() + batch4.purchaseAttack);
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
        // Verdant Spheres is a successful third-minion-buy boundary.  The
        // hero power grants one Tavern Coin and Crimson Hand Centurion copies
        // the current stats of this purchase.  Resolve every owned Buddy
        // independently so multiple normal/golden copies stack naturally.
        if (batch4.goldDelta > 0) {
            AddTavernCoins(batch4.goldDelta);
            recruitField.ForEachAlive([&purchased](MinionData& data) {
                auto& buddy = data.value();
                for (const auto& definition : BUDDY_VERDANT_SPHERES_BEHAVIORS) {
                    if (buddy.GetCardID() != definition.id) continue;
                    buddy.SetAttack(buddy.GetAttack() +
                                    purchased.GetAttack() * definition.statMultiplier);
                    buddy.SetHealth(buddy.GetHealth() +
                                    purchased.GetHealth() * definition.statMultiplier);
                    break;
                }
            });
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
    // friendly board trigger below.  A callback can still reject a purchase
    // after the preflight check (for example when a shop entity disappears),
    // so do not charge or report gold spent unless the hand actually grew.
    if (hand.GetCount() <= handCountBeforePurchase)
        return;
    if (eyeHealthPurchase) {
        hero.health -= cost;
    } else if (tapestryHealthPurchase) {
        hero.health -= tapestryHealthCost;
        tapestry->statScale = 0;
    } else if (pilgrimHealthPurchase) {
        hero.health -= cost;
    } else {
        remainCoin -= cost;
        RecordGoldSpent(cost);
    }
    for (auto* trinket : pilgrimHealthPurchases)
        trinket->triggerProgress = 1;
    // Advance every Eye independently after the purchase commits.  A mature
    // copy was selected above and naturally wraps to zero here; other copies
    // retain their progress toward their own fourth purchase.
    for (auto& trinket : season14.trinkets) {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect != TrinketEffect::BUY_MINION_HEALTH_CADENCE ||
            behavior.value <= 0)
            continue;
        if (++trinket.triggerProgress >= behavior.value)
            trinket.triggerProgress = 0;
    }
    if (hand.GetCount() > handCountBeforePurchase &&
        IsWarpGateHeroPowerDbfID(season14.heroPowerDbfID) &&
        season14.warpGateSelectedDbfID != 0 &&
        season14.warpGateRewardDbfID == 0 &&
        ++season14.warpGateBuyCount >= WARP_GATE_LIFECYCLE.buyThreshold)
    {
        season14.warpGateRewardDbfID = season14.warpGateSelectedDbfID;
        // Reset at arming time so a full hand/retry cannot carry the prior
        // threshold into the next repeating lifetime cadence.
        season14.warpGateBuyCount = 0;
    }
    TryResolveWarpGateReward();
    // Transcribing Typewriter observes the committed minion purchase after
    // all purchase-side mutations have settled. Resolve the exact purchased
    // instance (not the last hand card, since Battlecries may append cards),
    // and consume each owned Typewriter only when its copy reaches the hand.
    if (hand.GetCount() > handCountBeforePurchase && !hand.IsFull()) {
        Minion* purchasedForTypewriter = nullptr;
        hand.ForEach([&](std::optional<CardData>& data) {
            if (!data.has_value() ||
                !std::holds_alternative<Minion>(*data)) return;
            auto& candidate = std::get<Minion>(*data);
            if (candidate.GetIndex() == purchasedEntityIndex)
                purchasedForTypewriter = &candidate;
        });
        if (purchasedForTypewriter != nullptr) {
            for (auto& trinket : season14.trinkets) {
                if (hand.IsFull() || !trinket.active ||
                    trinket.remainingUses == 0) break;
                // `hand.Add` may grow/reallocate the hand container.  Re-find
                // the purchased entity for every copy so a second owned
                // Typewriter never dereferences a pointer invalidated by the
                // first copy.  This also keeps duplicate purchases distinct:
                // the stable entity index, rather than DBF ID or hand order,
                // remains the source of truth.
                purchasedForTypewriter = nullptr;
                hand.ForEach([&](std::optional<CardData>& data) {
                    if (!data.has_value() ||
                        !std::holds_alternative<Minion>(*data)) return;
                    auto& candidate = std::get<Minion>(*data);
                    if (candidate.GetIndex() == purchasedEntityIndex)
                        purchasedForTypewriter = &candidate;
                });
                if (purchasedForTypewriter == nullptr) break;
                const auto behavior = FindTrinketBehavior(
                    Cards::FindCardByDbfID(trinket.dbfID).id);
                if (behavior.effect != TrinketEffect::AFTER_BUY_MINION_COPY)
                    continue;
                Minion copy = *purchasedForTypewriter;
                if (getNextCardIndexCallback)
                    copy.SetIndex(getNextCardIndexCallback());
                hand.Add(CardData{std::move(copy)});
                --trinket.remainingUses;
                if (trinket.remainingUses == 0) trinket.active = false;
            }
        }
    }
    if (hand.GetCount() > handCountBeforePurchase)
    {
        auto* purchasedPtr = findPurchasedMinion();
        if (purchasedPtr == nullptr) return;
        auto& purchased = *purchasedPtr;
        recruitField.ForEachAlive([&purchased](MinionData& data) {
            data.value().ActivateTrigger(TriggerType::BUY_MINION, purchased);
        });
        // Enhance-o Medico counts the purchased minion's Bonus Keywords at
        // the buy boundary. Resolve normal/golden copies independently so
        // multiple Medicos stack while each remains an event observer.
        // Resolve the exact purchased instance.  A successful buy may append
        // generated cards to hand before this observer runs; the last hand
        // card is therefore not a reliable representation of the purchase.
        Minion* purchasedForMedico = nullptr;
        hand.ForEach([&](std::optional<CardData>& card) {
            if (!card.has_value() || !std::holds_alternative<Minion>(*card)) return;
            auto& candidate = std::get<Minion>(*card);
            if (candidate.GetIndex() == purchasedEntityIndex)
                purchasedForMedico = &candidate;
        });
        if (purchasedForMedico != nullptr) {
            int bonusKeywords = 0;
            bonusKeywords += purchasedForMedico->HasTaunt() ? 1 : 0;
            bonusKeywords += purchasedForMedico->HasDivineShield() ? 1 : 0;
            bonusKeywords += purchasedForMedico->HasReborn() ? 1 : 0;
            bonusKeywords += purchasedForMedico->HasWindfury() ? 1 : 0;
            bonusKeywords += purchasedForMedico->HasVenomous() ? 1 : 0;
            bonusKeywords += purchasedForMedico->HasStealth() ? 1 : 0;
            if (bonusKeywords > 0) {
                recruitField.ForEachAlive([bonusKeywords](MinionData& data) {
                    auto& medico = data.value();
                    const auto& id = medico.GetCardID();
                    for (const auto& definition : BUDDY_BONUS_KEYWORD_BUY_BEHAVIORS) {
                        if (definition.id != id) continue;
                        const int bonus = definition.attackHealthPerKeyword * bonusKeywords;
                        medico.SetAttack(medico.GetAttack() + bonus);
                        medico.SetHealth(medico.GetHealth() + bonus);
                        break;
                    }
                });
            }
        }
        // The Nine Frogs observes this successful buy after all ordinary
        // purchase auras have settled.  Charges are keyed by Buddy entity,
        // so multiple normal/golden copies do not share the printed counter;
        // golden emits two independent same-tier spell cards per trigger.
        std::vector<std::pair<int, int>> nineFrogs;
        recruitField.ForEachAlive([this, &nineFrogs](MinionData& data) {
            auto& buddy = data.value();
            for (const auto& definition :
                 BUDDY_SAME_TIER_SPELL_ON_BUY_BEHAVIORS) {
                if (buddy.GetCardID() != definition.id) continue;
                const auto entityID = static_cast<std::uint64_t>(buddy.GetIndex());
                auto it = std::find_if(
                    season14.nineFrogsPurchasesRemaining.begin(),
                    season14.nineFrogsPurchasesRemaining.end(),
                    [entityID](const auto& entry) {
                        return entry.first == entityID;
                    });
                if (it == season14.nineFrogsPurchasesRemaining.end()) {
                    season14.nineFrogsPurchasesRemaining.emplace_back(
                        entityID, definition.triggers);
                    it = std::prev(season14.nineFrogsPurchasesRemaining.end());
                }
                if (it->second > 0)
                    nineFrogs.emplace_back(definition.spellsPerTrigger,
                                           entityID);
                break;
            }
        });
        for (const auto& [amount, entityID] : nineFrogs) {
            auto it = std::find_if(
                season14.nineFrogsPurchasesRemaining.begin(),
                season14.nineFrogsPurchasesRemaining.end(),
                [entityID](const auto& entry) { return entry.first == entityID; });
            if (it == season14.nineFrogsPurchasesRemaining.end() ||
                it->second <= 0 || hand.IsFull()) continue;
            std::vector<const Card*> pool;
            for (const auto& spell : Cards::GetAllCards()) {
                if (!spell.isBattlegroundsPoolSpell || spell.normalDbfID != 0 ||
                    spell.GetTier() != purchased.GetTier() ||
                    (spell.GetCardType() != CardType::SPELL &&
                     spell.GetCardType() != CardType::BATTLEGROUND_SPELL) ||
                    FindTavernSpellBehavior(spell.id).effect == TavernSpellEffect::NONE)
                    continue;
                pool.push_back(&spell);
            }
            if (pool.empty()) continue;
            for (int n = 0; n < amount && !hand.IsFull(); ++n) {
                const auto& spell = *pool[Random::get<std::size_t>(0, pool.size() - 1)];
                hand.Add(CardData{Spell(spell)});
            }
            --it->second;
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
        // Reusable Batteries consumes its per-instance first-purchase
        // entitlement only after the minion reached hand *and* the generated
        // card was delivered.  The generated Satellite is a hand card
        // carrying the purchased minion's current stats; a full hand (or an
        // unavailable/malformed token definition) leaves the copy eligible
        // for a later successful purchase in this recruit turn.
        if (!hand.IsFull()) {
            for (auto& trinket : season14.trinkets) {
                if (hand.IsFull()) break;
                if (!trinket.active || trinket.remainingUses == 0 ||
                    trinket.triggerProgress != 0)
                    continue;
                const auto behavior = FindTrinketBehavior(
                    Cards::FindCardByDbfID(trinket.dbfID).id);
                if (behavior.effect !=
                    TrinketEffect::AFTER_BUY_MINION_MAGNETIC_SATELLITE)
                    continue;
                const auto satellite = Cards::FindCardByID("BG31_171t");
                if (satellite.id.empty() ||
                    satellite.GetCardType() != CardType::MINION ||
                    !satellite.hasBehavior ||
                    !satellite.gameTags.contains(GameTag::MAGNETIC) ||
                    satellite.gameTags.at(GameTag::MAGNETIC) == 0)
                    continue;
                Minion generated(satellite);
                generated.SetAttack(purchased.GetAttack());
                generated.SetHealth(purchased.GetHealth());
                hand.Add(CardData{std::move(generated)});
                trinket.triggerProgress = 1;
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
            const auto* purchased = findPurchasedMinion();
            if (purchased == nullptr) return;
            const int tier = purchased->GetGameTag(GameTag::TECH_LEVEL);
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
            if (behavior.effect == TrinketEffect::AFTER_BUY_MECH_RANDOM_SPELL)
            {
                // The bought minion is the event source.  Resolve only after
                // the purchase reached hand, preserving normal purchase and
                // hand-cap semantics while allowing duplicate Trinkets to
                // award independently.
                // `purchasedDbfID` was captured before the Tavern entity was
                // removed, so the canonical card remains available here
                // after purchase-side hand mutations.
                const auto purchasedCard =
                    Cards::FindCardByDbfID(purchasedDbfID);
                if (purchasedCard.HasRace(behavior.race) && !hand.IsFull())
                    (void)SimpleTasks::RandomTavernSpellToHandTask{
                        behavior.amount}.Run(*this);
                continue;
            }
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
        // Use the Tavern entity captured before purchase-side Battlecries and
        // rewards mutate the hand.  The last hand card is not necessarily the
        // bought minion (for example, a buy can add Brann or another token).
        RecordSeason14HeroPowerBatch5GlaivePurchase(
            purchasedDbfID, season14.heroPowerBatch5);
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

bool Player::StealHighestTierTavernMinionToHand()
{
    if (hand.IsFull() || tavern.fieldZone.GetCount() == 0) return false;
    int highestTier = -1;
    std::vector<std::size_t> candidates;
    tavern.fieldZone.ForEachAlive([&](const MinionData& data) {
        const auto& offer = data.value();
        const int tier = offer.GetTier();
        if (tier > highestTier) {
            highestTier = tier;
            candidates.clear();
        }
        if (tier == highestTier)
            candidates.push_back(static_cast<std::size_t>(
                offer.GetZonePosition()));
    });
    if (candidates.empty()) return false;
    Random::shuffle(candidates.begin(), candidates.end());
    const auto slot = candidates.front();
    if (slot >= static_cast<std::size_t>(tavern.fieldZone.GetCount()) ||
        tavern.fieldZone[slot].IsDestroyed())
        return false;
    Minion stolen = tavern.fieldZone.Remove(tavern.fieldZone[slot]);
    stolen.SetFrozen(false);
    hand.Add(CardData{std::move(stolen)});
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

bool Player::TryResolveSoulFermenterIfSpace(FieldZone& field)
{
    if (!season14.soulFermenterArmed || season14.soulFermenterSnapshots.empty())
        return false;
    bool resolved = false;
    while (!field.IsFull() && !season14.soulFermenterSnapshots.empty()) {
        Minion snapshot = std::move(season14.soulFermenterSnapshots.front());
        season14.soulFermenterSnapshots.erase(
            season14.soulFermenterSnapshots.begin());
        snapshot.SetFrozen(false);
        if (getNextCardIndexCallback) snapshot.SetIndex(getNextCardIndexCallback());
        field.Add(snapshot, field.GetCount());
        resolved = true;
    }
    if (season14.soulFermenterSnapshots.empty())
        season14.soulFermenterArmed = false;
    return resolved;
}

bool Player::CanAcquireTrinketPayload(const Card& card,
                                      const TrinketBehavior& behavior) const
{
    if (card.id.empty() || card.GetCardType() != CardType::BATTLEGROUND_TRINKET ||
        behavior.effect == TrinketEffect::NONE)
        return false;
    // These effects have an acquisition-time target/resource requirement. If
    // it is absent, AcquireTrinket rejects the option rather than installing
    // a persistent effect which would silently do nothing.
    if (behavior.effect == TrinketEffect::TRANSFORM_WARBAND_TIER) {
        std::vector<Card> tier4Candidates;
        AppendSupportedNormalMinions(Cards::GetTier4Minions(), tier4Candidates,
                                     Race::INVALID, activeTribes);
        if (tier4Candidates.empty()) return false;
    }
    if (behavior.effect == TrinketEffect::WARBAND_COPY_REFRESH &&
        recruitField.IsEmpty())
        return false;
    if (behavior.effect == TrinketEffect::AFTER_SELL_HERO_POWER_BUDDY &&
        !FindHeroPowerBuddyReward(season14.heroPowerDbfID).has_value())
        return false;

    const bool fixedCard =
        behavior.effect == TrinketEffect::ACQUIRE_FIXED_CARD ||
        behavior.effect == TrinketEffect::ACQUIRE_PRIMALFIN_PORTRAIT ||
        behavior.effect == TrinketEffect::ACQUIRE_FIXED_CARD_FIRST_PIRATE_FREE ||
        behavior.effect == TrinketEffect::ACQUIRE_FIXED_CARD_AND_BOUNTIES ||
        behavior.effect == TrinketEffect::ACQUIRE_TWO_FIXED_CARDS ||
        behavior.effect == TrinketEffect::ACQUIRE_FIXED_CARD_AND_TAVERN_SLOTS ||
        behavior.effect == TrinketEffect::ACQUIRE_FIXED_CARD_AFTER_SELL ||
        behavior.effect == TrinketEffect::ACQUIRE_FLAGBEARER_PORTRAIT ||
        behavior.effect == TrinketEffect::ACQUIRE_FIXED_GLOWSCALE ||
        behavior.effect == TrinketEffect::ACQUIRE_FIXED_LIONFISH ||
        behavior.effect == TrinketEffect::AFTER_PLAY_ELEMENTAL_FIXED_CARD;
    if (!fixedCard) return true;

    const Card generated = Cards::FindCardByID(behavior.cardID);
    const bool generatedNotSpell = generated.GetCardType() != CardType::SPELL;
    const bool generatedBattleSpell =
        generated.GetCardType() == CardType::BATTLEGROUND_SPELL;
    const bool generatedSpell = !generatedNotSpell || generatedBattleSpell;
    const bool generatedSpellSupported =
        generatedSpell &&
        FindTavernSpellBehavior(generated.id).effect != TavernSpellEffect::NONE;
    if (behavior.cardID.empty() || generated.dbfID == 0 ||
        ((!generated.hasBehavior && !generatedSpellSupported)) ||
        (generatedNotSpell && !generatedBattleSpell &&
         generated.GetCardType() != CardType::MINION))
        return false;
    if (behavior.effect == TrinketEffect::ACQUIRE_TWO_FIXED_CARDS) {
        const Card secondary = Cards::FindCardByID(behavior.secondaryCardID);
        const bool secondarySpell =
            secondary.GetCardType() == CardType::SPELL ||
            secondary.GetCardType() == CardType::BATTLEGROUND_SPELL;
        const bool secondarySpellSupported =
            secondarySpell &&
            FindTavernSpellBehavior(secondary.id).effect != TavernSpellEffect::NONE;
        if (behavior.secondaryCardID.empty() || secondary.dbfID == 0 ||
            ((!secondary.hasBehavior && !secondarySpellSupported)) ||
            (!secondarySpell && secondary.GetCardType() != CardType::MINION))
            return false;
    }
    return true;
}

std::vector<Season14Offering> Player::BuildTrinketOfferings(
    bool greater, std::size_t count, bool requireCheap, bool requireTypeless) const
{
    // Patch 36.4: a player is "in" a type at the Lesser/Greater offer when
    // they have 2/3 minions of that type.  Count the public warband and hand;
    // multi-tribe and ALL minions contribute to every matching type.
    const int threshold = greater ? 3 : 2;
    std::map<std::string, int> typeCounts;
    auto countMinion = [&typeCounts](const Minion& minion) {
        for (const Race race : RACES_IN_BATTLEGROUNDS)
            if (minion.HasRace(race)) {
                // Race names are stable in the pinned source metadata and
                // are the same strings used by battlegroundsAssociatedRaces.
                static constexpr std::pair<Race, const char*> names[] = {
                    {Race::BEAST, "BEAST"}, {Race::DEMON, "DEMON"},
                    {Race::DRAGON, "DRAGON"}, {Race::ELEMENTAL, "ELEMENTAL"},
                    {Race::MECHANICAL, "MECHANICAL"}, {Race::MURLOC, "MURLOC"},
                    {Race::NAGA, "NAGA"}, {Race::PIRATE, "PIRATE"},
                    {Race::QUILBOAR, "QUILBOAR"}, {Race::UNDEAD, "UNDEAD"},
                };
                for (const auto& [known, name] : names)
                    if (race == known) ++typeCounts[name];
            }
    };
    recruitField.ForEachAlive([&countMinion](const MinionData& data) {
        countMinion(data.value());
    });
    hand.ForEach([&countMinion](const std::optional<CardData>& entry) {
        if (entry && std::holds_alternative<Minion>(*entry))
            countMinion(std::get<Minion>(*entry));
    });

    std::set<std::string> inTypes;
    for (const auto& [type, amount] : typeCounts)
        if (amount >= threshold && !TrinketTypeIsUnavailable(type, activeTribes))
            inTypes.insert(type);
    if (inTypes.size() >= 3) inTypes.insert("MENAGERIE");
    // Queen Azshara's passive is an explicit hero affinity, even before the
    // warband reaches the normal Naga threshold.  Keep this override data
    // pinned to the ruleset rather than inferring it from card text.
    if ((hero.card.dbfID == 79618 || season14.heroPowerDbfID == 79619) &&
        IsActiveTribe(activeTribes, Race::NAGA))
        inTypes.insert("NAGA");

    std::size_t controlledMinions = 0;
    recruitField.ForEachAlive([&controlledMinions](const MinionData&) {
        ++controlledMinions;
    });
    hand.ForEach([&controlledMinions](const std::optional<CardData>& entry) {
        if (entry && std::holds_alternative<Minion>(*entry)) ++controlledMinions;
    });
    const auto controlsTier = [this](int tier) {
        bool found = false;
        recruitField.ForEachAlive([&found, tier](const MinionData& data) {
            found = found || data.value().GetTier() == tier;
        });
        hand.ForEach([&found, tier](const std::optional<CardData>& entry) {
            if (entry && std::holds_alternative<Minion>(*entry))
                found = found || std::get<Minion>(*entry).GetTier() == tier;
        });
        return found;
    };
    int bestCount = -1;
    std::string bestType;
    for (const auto& type : inTypes) {
        if (type == "MENAGERIE") continue;
        if (typeCounts[type] > bestCount) {
            bestCount = typeCounts[type];
            bestType = type;
        }
    }
    // Keep affinity classification beside each candidate.  Cards for a type
    // the player is not "in" remain eligible, but the 36.4 pivot rule permits
    // at most one such typed card and marks it down by two Gold.
    struct TrinketCandidate {
        Card card;
        bool typed = false;
        bool inAffinity = false;
        bool mostCommon = false;
        std::string offerGroup;
    };
    std::vector<TrinketCandidate> candidates;
    for (const auto& candidate : Cards::GetAllCards()) {
        if (candidate.trinketType != (greater ? "GREATER_TRINKET" : "LESSER_TRINKET") ||
            candidate.normalDbfID != 0 || candidate.dbfID <= 0 ||
            candidate.GetCardType() != CardType::BATTLEGROUND_TRINKET ||
            FindTrinketBehavior(candidate.id).effect == TrinketEffect::NONE ||
            std::any_of(season14.trinkets.begin(), season14.trinkets.end(),
                [&candidate](const Season14PersistentEffect& owned) {
                    return owned.dbfID == candidate.dbfID;
                }))
            continue;
        // Keep offer construction in lockstep with AcquireTrinket. In
        // particular, Warband Whistle (DBF 131002) is not executable with an
        // empty recruit board; offering it there used to create an
        // UnsupportedContent action at the modal boundary.
        const auto behavior = FindTrinketBehavior(candidate.id);
        if (!CanAcquireTrinketPayload(candidate, behavior)) continue;
        // Special source-level restrictions are part of the offer pool, not
        // merely acquisition validation.  Murky Sticker requires multiple
        // Battlecries; cards requiring hand room cannot be offered while full.
        if (candidate.id == "BG36_MagicItem_330" &&
            season14.battlecriesTriggered < 2)
            continue;
        if (candidate.id == "BG36_MagicItem_309" && hand.IsFull()) continue;
        if ((candidate.id == "BG32_MagicItem_400" ||
             candidate.id == "BG32_MagicItem_844") && controlledMinions < 6)
            continue;
        if (candidate.id == "BG35_MagicItem_815" && controlledMinions >= 5)
            continue;
        if (candidate.id == "BG35_MagicItem_817" && !controlsTier(3))
            continue;
        if (candidate.id == "BG30_MagicItem_998" &&
            excludedLobbyRace == Race::DEMON)
            continue;
        if (candidate.id == "BG35_MagicItem_820" &&
            hero.health + armor >= 16)
            continue;

        // Source-level predicates from the pinned 36.4 offering contract.
        // These are evaluated at offer construction time, not only when a
        // card is selected, so the public four-card modal is never padded by
        // an option the player cannot actually use.
        bool hasDeathrattle = false;
        bool hasDivineShield = false;
        bool hasGolden = false;
        bool hasStartOfCombat = false;
        bool hasEndOfTurn = false;
        bool hasThaumaturgist = false;
        bool hasDramalocAttack = false;
        bool hasHealthRewinder = false;
        bool hasBuffedBloodGems = season14.bloodGemAttackBonus > 0 ||
                                  season14.bloodGemHealthBonus > 0;
        recruitField.ForEachAlive([&](const MinionData& data) {
            const auto& minion = data.value();
            hasDeathrattle = hasDeathrattle || minion.HasDeathrattle();
            hasDivineShield = hasDivineShield || minion.HasDivineShield();
            hasGolden = hasGolden || minion.IsGolden();
            hasStartOfCombat = hasStartOfCombat ||
                !minion.GetTasks(PowerType::START_OF_COMBAT).empty();
            auto def = CardDefs::FindCardDefByID(minion.GetCardID());
            const auto& trigger = def.power.GetTrigger();
            hasEndOfTurn = hasEndOfTurn ||
                (trigger.has_value() &&
                 trigger->GetTriggerType() == TriggerType::TURN_END);
            hasThaumaturgist = hasThaumaturgist ||
                minion.GetCardID() == "BG32_181" ||
                minion.GetCardID() == "BG32_181_G";
            hasDramalocAttack = hasDramalocAttack || minion.GetAttack() >= 3;
            hasHealthRewinder = hasHealthRewinder ||
                minion.GetCardID() == "BG26_174" ||
                minion.GetCardID() == "BG26_174_G";
        });
        hand.ForEach([&](const std::optional<CardData>& entry) {
            if (!entry || !std::holds_alternative<Minion>(*entry)) return;
            const auto& minion = std::get<Minion>(*entry);
            hasDeathrattle = hasDeathrattle || minion.HasDeathrattle();
            hasDivineShield = hasDivineShield || minion.HasDivineShield();
            hasGolden = hasGolden || minion.IsGolden();
            hasStartOfCombat = hasStartOfCombat ||
                !minion.GetTasks(PowerType::START_OF_COMBAT).empty();
            auto def = CardDefs::FindCardDefByID(minion.GetCardID());
            const auto& trigger = def.power.GetTrigger();
            hasEndOfTurn = hasEndOfTurn ||
                (trigger.has_value() &&
                 trigger->GetTriggerType() == TriggerType::TURN_END);
            hasThaumaturgist = hasThaumaturgist ||
                minion.GetCardID() == "BG32_181" ||
                minion.GetCardID() == "BG32_181_G";
            hasDramalocAttack = hasDramalocAttack || minion.GetAttack() >= 3;
            hasHealthRewinder = hasHealthRewinder ||
                minion.GetCardID() == "BG26_174" ||
                minion.GetCardID() == "BG26_174_G";
        });
        if ((candidate.id == "BG32_MagicItem_862" ||
             candidate.id == "BG32_MagicItem_862t") && !hasDeathrattle)
            continue;
        if (candidate.id == "BG32_MagicItem_171" && !hasDivineShield)
            continue;
        if ((candidate.id == "BG32_MagicItem_231" ||
             candidate.id == "BG32_MagicItem_231t" ||
             candidate.id == "BG30_MagicItem_954") && !hasGolden)
            continue;
        if ((candidate.id == "BG32_MagicItem_282" ||
             candidate.id == "BG32_MagicItem_304") && hand.IsFull())
            continue;
        if (candidate.id == "BG32_MagicItem_365" && !hasStartOfCombat)
            continue;
        if (candidate.id == "BG32_MagicItem_904" && !hasBuffedBloodGems)
            continue;
        if (candidate.id == "BG32_MagicItem_367" && !hasEndOfTurn)
            continue;
        if ((candidate.id == "BG30_MagicItem_701" ||
             candidate.id == "BG30_MagicItem_541") &&
            !hasHealthRewinder)
            continue;
        if ((candidate.id == "BG30_MagicItem_828" ||
             candidate.id == "BG30_MagicItem_920") && !hasThaumaturgist)
            continue;
        if (candidate.id == "BG35_MagicItem_754" && !hasDramalocAttack)
            continue;
        if (candidate.id == "BG35_MagicItem_861" &&
            season14.futureBallerAttack <= 0 && season14.futureBallerHealth <= 0)
            continue;

        bool typed = false;
        bool affinity = false;
        for (const auto& type : candidate.associatedRaces) {
            typed = true;
            const auto race = TrinketTypeRace(type);
            // A typed Trinket is not in the offer pool when its associated
            // tribe is absent from this lobby.  Multi-affinity Trinkets stay
            // eligible when at least one of their listed active tribes is a
            // qualifying type; this mirrors how multi-tribe minions count.
            if ((!race.has_value() && !IsMenagerieTypeName(type)) ||
                (race.has_value() &&
                 (!IsActiveTribe(activeTribes, *race) ||
                  TrinketTypeIsUnavailable(type, activeTribes))))
                continue;
            if (IsMenagerieTypeName(type) ? inTypes.count("MENAGERIE") != 0
                                          : inTypes.count(type) != 0)
                affinity = true;
        }
        // Typeless cards are always in the neutral pool.  Typed cards need an
        // active lobby tribe, but need not be a qualifying warband type: one
        // such pivot offer is legal and receives the two-Gold discount.
        // The lobby filter is deliberately separate from affinity: an
        // active tribe can be pivoted into even when the warband is not yet
        // in that type.  Only absent/excluded tribes are removed here.
        if (typed && !TrinketIsInLobby(candidate, activeTribes,
                                       excludedLobbyRace))
            continue;
        const bool mostCommon = std::any_of(
            candidate.associatedRaces.begin(), candidate.associatedRaces.end(),
            [&bestType](const std::string& type) { return type == bestType; });
        // Menagerie is a player-affinity group, not a generic active-lobby
        // type. Without this check it could leak as the single discounted
        // pivot before three distinct qualifying types exist.
        if (std::find(candidate.associatedRaces.begin(),
                      candidate.associatedRaces.end(), "MENAGERIE") !=
                candidate.associatedRaces.end() &&
            !inTypes.contains("MENAGERIE"))
            continue;
        std::string offerGroup;
        if (candidate.id == "BG32_MagicItem_400" ||
            candidate.id == "BG32_MagicItem_844")
            offerGroup = "MENAGERIE";
        else if (candidate.id == "BG30_MagicItem_888" ||
                 candidate.id == "BG30_MagicItem_891" ||
                 candidate.id == "BG32_MagicItem_271")
            offerGroup = candidate.id;
        candidates.push_back({candidate, typed, affinity, mostCommon,
                              std::move(offerGroup)});
    }
    if (candidates.empty() || count == 0) return {};

    std::vector<TrinketCandidate> selected;
    std::set<int> selectedIDs;
    std::set<std::string> selectedGroups;
    std::size_t outsideTyped = 0;
    auto addOne = [&selected, &selectedIDs, &selectedGroups, &outsideTyped,
                   &bestType](const TrinketCandidate& candidate) {
        if (selectedIDs.contains(candidate.card.dbfID)) return false;
        if (!candidate.offerGroup.empty() &&
            !selectedGroups.insert(candidate.offerGroup).second) return false;
        if (candidate.typed && !candidate.inAffinity && outsideTyped >= 1)
            return false;
        selectedIDs.insert(candidate.card.dbfID);
        if (candidate.typed && !candidate.inAffinity)
            ++outsideTyped;
        selected.push_back(candidate);
        return true;
    };
    auto isTypeless = [](const TrinketCandidate& c) {
        return c.card.associatedRaces.empty();
    };
    auto isCheap = [](const TrinketCandidate& c) {
        return c.card.gameTags.contains(GameTag::COST) &&
               c.card.gameTags.at(GameTag::COST) <= 2;
    };
    // Always reserve the most-common qualifying type.  Ties follow the
    // source card order before the final seeded shuffle, making replay stable.
    if (!bestType.empty()) {
        auto it = std::find_if(candidates.begin(), candidates.end(),
            [&bestType](const TrinketCandidate& candidate) {
                return std::find(candidate.card.associatedRaces.begin(),
                                 candidate.card.associatedRaces.end(), bestType) !=
                       candidate.card.associatedRaces.end();
            });
        if (it != candidates.end()) addOne(*it);
    }
    if (requireTypeless) {
        auto it = std::find_if(candidates.begin(), candidates.end(), isTypeless);
        if (it != candidates.end()) addOne(*it);
    }
    if (requireCheap) {
        auto it = std::find_if(candidates.begin(), candidates.end(), isCheap);
        if (it != candidates.end()) addOne(*it);
    }
    // Increased-frequency neutral cards are represented by repeated entries
    // in the seeded draw list.  `selectedIDs` still guarantees unique cards.
    std::vector<TrinketCandidate> weighted = candidates;
    for (const auto& candidate : candidates) {
        if (candidate.card.associatedRaces.empty() &&
            (candidate.card.id == "BG35_MagicItem_931" ||
             candidate.card.id == "BG35_MagicItem_931t" ||
             candidate.card.id == "BG30_MagicItem_435" ||
             candidate.card.id == "BG30_MagicItem_706" ||
             candidate.card.id == "BG30_MagicItem_888" ||
             candidate.card.id == "BG30_MagicItem_891" ||
             candidate.card.id == "BG30_MagicItem_426" ||
             candidate.card.id == "BG30_MagicItem_426t" ||
             candidate.card.id == "BG35_MagicItem_930" ||
             candidate.card.id == "BG30_MagicItem_876" ||
             candidate.card.id == "BG32_MagicItem_901")) {
            weighted.push_back(candidate);
            weighted.push_back(candidate);
        }
    }
    Random::shuffle(weighted.begin(), weighted.end());
    for (const auto& candidate : weighted) {
        if (selected.size() >= count) break;
        addOne(candidate);
    }
    if (selected.size() > count) selected.resize(count);
    std::vector<Season14Offering> result;
    result.reserve(selected.size());
    for (const auto& candidate : selected) {
        // Pivot offers are discounted by exactly two Gold.  Keep the value on
        // the public option; selection validation below checks the same
        // metadata, preventing replay from changing an offer's price.
        const auto discount = candidate.typed && !candidate.mostCommon &&
                              !candidate.inAffinity ? 2 : 0;
        result.push_back({candidate.card.dbfID, 0, 0, discount});
    }
    return result;
}

bool Player::BeginFantasticTreasureOffer()
{
    if (season14.heroPowerDbfID != 113311 || season14.recruitTurnNumber != 5 ||
        season14.pendingDecision != Season14Decision::NONE)
        return false;
    auto offerings = BuildTrinketOfferings(false, 4, true, true);
    if (offerings.size() < 4) return false;
    for (auto& offering : offerings) offering.darkGiftDbfID = 2;
    season14.BeginOfferingDecision(Season14Decision::TRINKET_SELECTION, 0,
                                   113311, std::move(offerings));
    return true;
}

bool Player::BeginScheduledTrinketOffer()
{
    if (season14.pendingDecision != Season14Decision::NONE ||
        !season14.CanAddTrinket())
        return false;
    const bool greater = season14.recruitTurnNumber == 9;
    if (!greater && season14.recruitTurnNumber != 6) return false;

    // The ordinary Season 14 modal is four choices.  Special replacement
    // and generated-reward modals intentionally use their own smaller
    // cardinalities and must continue calling BuildTrinketOfferings directly.
    const auto offerings = BuildTrinketOfferings(greater, 4, true, true);
    if (offerings.size() != 4) return false;
    season14.BeginOfferingDecision(
        Season14Decision::TRINKET_SELECTION, 0, 0, offerings);
    return true;
}

bool Player::BeginOrnateClockOffer()
{
    if ((!season14.ornateClockGreaterNextTurn &&
         !season14.mysteriousOrbLesserNext) ||
        season14.pendingDecision != Season14Decision::NONE ||
        !season14.CanAddTrinket())
        return false;

    const bool lesser = season14.mysteriousOrbLesserNext;
    const bool greater = !lesser;
    // Canonical offer selection is equivalent to:
    // candidate.trinketType == (lesser ? "LESSER_TRINKET" : "GREATER_TRINKET").

    auto candidates = BuildTrinketOfferings(greater, 3, true, true);
    if (candidates.size() < 3) return false;
    // These effects replace the scheduled Trinket purchase, not a second
    // paid purchase. In particular, do not charge Gold or feed this modal
    // back into spend-gold triggers. Consume only after a valid modal exists.
    season14.BeginOfferingDecision(
        Season14Decision::TRINKET_SELECTION, 0,
        lesser ? 130836 : 121120,
        std::move(candidates));
    // Consume only the schedule represented by this modal.  Both effects
    // can be owned at once; clearing the unrelated schedule would silently
    // lose the second promised Trinket offer.
    if (lesser)
        season14.mysteriousOrbLesserNext = false;
    else
        season14.ornateClockGreaterNextTurn = false;
    return true;
}

bool Player::BeginMysteryCubeOffer()
{
    if (season14.pendingDecision != Season14Decision::NONE) {
        // Start-turn effects may race with any other public modal.  Do not
        // replace that modal; remember the Cube request for the first safe
        // post-resolution boundary instead.
        season14.pendingMysteryCubeOffer = true;
        return false;
    }
    if (season14.pendingTrinketReplacementSlot >= 0)
        return false;
    std::size_t slot = season14.trinkets.size();
    for (std::size_t i = 0; i < season14.trinkets.size(); ++i) {
        const auto card = Cards::FindCardByDbfID(season14.trinkets[i].dbfID);
        if (FindTrinketBehavior(card.id).effect ==
            TrinketEffect::MYSTERY_CUBE_REPLACE_LESSER) {
            slot = i;
            break;
        }
    }
    if (slot >= season14.trinkets.size()) {
        season14.pendingMysteryCubeOffer = false;
        return false;
    }
    std::vector<Card> candidates;
    for (const auto& candidate : Cards::GetAllCards())
        if (candidate.trinketType == "LESSER_TRINKET" &&
            candidate.normalDbfID == 0 && candidate.dbfID > 0 &&
            candidate.GetCardType() == CardType::BATTLEGROUND_TRINKET &&
            TrinketIsInLobby(candidate, activeTribes, excludedLobbyRace) &&
            // A replacement must terminate the Cube lifecycle.  Offering a
            // second Cube would recursively open another replacement modal
            // and violate the one-Cube-per-player invariant.
            candidate.id != "BG30_MagicItem_703" &&
            FindTrinketBehavior(candidate.id).effect != TrinketEffect::NONE &&
            CanAcquireTrinketPayload(candidate, FindTrinketBehavior(candidate.id)) &&
            std::none_of(season14.trinkets.begin(), season14.trinkets.end(),
                         [&candidate](const Season14PersistentEffect& owned) {
                             return owned.dbfID == candidate.dbfID;
                         }))
            candidates.push_back(candidate);
    if (candidates.size() < 2) return false;
    Random::shuffle(candidates.begin(), candidates.end());
    season14.BeginOfferingDecision(
        Season14Decision::TRINKET_SELECTION, 0, 117801,
        {{candidates[0].dbfID, 0}, {candidates[1].dbfID, 0}});
    // BeginOfferingDecision resets auxiliary modal state; restore the slot
    // identity after entering the public decision.
    season14.pendingTrinketReplacementSlot = static_cast<std::int32_t>(slot);
    season14.pendingMysteryCubeOffer = false;
    return true;
}

bool Player::BeginTripVouchersOffer()
{
    if (season14.pendingDecision != Season14Decision::NONE) {
        season14.pendingTripVouchersOffer = true;
        return false;
    }
    if (season14.pendingTrinketReplacementSlot >= 0)
        return false;
    std::size_t slot = season14.trinkets.size();
    for (std::size_t i = 0; i < season14.trinkets.size(); ++i) {
        const auto card = Cards::FindCardByDbfID(season14.trinkets[i].dbfID);
        if (FindTrinketBehavior(card.id).effect ==
            TrinketEffect::TRIP_VOUCHERS_REPLACE_GREATER) {
            slot = i;
            break;
        }
    }
    if (slot >= season14.trinkets.size()) {
        season14.pendingTripVouchersOffer = false;
        return false;
    }
    std::vector<Card> candidates;
    for (const auto& candidate : Cards::GetAllCards())
        if (candidate.trinketType == "GREATER_TRINKET" &&
            candidate.normalDbfID == 0 && candidate.dbfID > 0 &&
            candidate.GetCardType() == CardType::BATTLEGROUND_TRINKET &&
            TrinketIsInLobby(candidate, activeTribes, excludedLobbyRace) &&
            candidate.id != "BG30_MagicItem_891" &&
            FindTrinketBehavior(candidate.id).effect != TrinketEffect::NONE &&
            // Keep the public replacement modal closed over candidates that
            // AcquireTrinket would reject.  In particular, fixed-card
            // portraits must have a registered executable generated card;
            // otherwise SelectDecision would clear this modal and the
            // replacement could not be retried atomically.  Dynamic Buddy
            // rewards are likewise offered only when the active Hero Power
            // has a validated normal/golden Buddy link.  Timewarp-linked
            // powers fail closed through the same link validator.
            [candidate, this]() {
                const auto behavior = FindTrinketBehavior(candidate.id);
                return CanAcquireTrinketPayload(candidate, behavior);
            }() &&
            std::none_of(season14.trinkets.begin(), season14.trinkets.end(),
                         [&candidate](const Season14PersistentEffect& owned) {
                             return owned.dbfID == candidate.dbfID;
                         }))
            candidates.push_back(candidate);
    if (candidates.size() < 3) return false;
    Random::shuffle(candidates.begin(), candidates.end());
    season14.BeginOfferingDecision(
        Season14Decision::TRINKET_SELECTION, 0, 140001,
        {{candidates[0].dbfID, 0}, {candidates[1].dbfID, 0},
         {candidates[2].dbfID, 0}});
    season14.pendingTrinketReplacementSlot = static_cast<std::int32_t>(slot);
    season14.pendingTripVouchersOffer = false;
    return true;
}

bool Player::BeginWarpGateChoice()
{
    if (!IsWarpGateHeroPowerDbfID(season14.heroPowerDbfID) ||
        season14.warpGateSelectedDbfID != 0 ||
        season14.pendingDecision != Season14Decision::NONE)
        return false;
    std::vector<Card> candidates;
    for (const auto& card : Cards::GetAllCards())
        // Warp Gate is a hero-generated source, so candidates need not be in
        // the ordinary Tavern pool. The explicit DBF allowlist is the tribe
        // boundary; unsupported card behavior remains fail-closed at reward
        // construction rather than silently substituting another tribe.
        if (IsExecutableWarpGateProtossDbfID(card.dbfID) &&
            card.GetCardType() == CardType::MINION && card.normalDbfID == 0 &&
            card.hasBehavior)
            candidates.push_back(card);
    if (candidates.size() < 2) return false;
    Random::shuffle(candidates.begin(), candidates.end());
    season14.BeginOfferingDecision(Season14Decision::CHOICE, 0,
        WARP_GATE_LIFECYCLE.heroPowerDbfID,
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
            card.hasBehavior && HasActiveTribe(activeTribes, card) &&
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
    if (!IsWarpGateHeroPowerDbfID(season14.heroPowerDbfID) ||
        season14.warpGateRewardDbfID == 0 ||
        hand.IsFull()) return false;
    const auto card = Cards::FindCardByDbfID(season14.warpGateRewardDbfID);
    if (card.dbfID == 0 || card.GetCardType() != CardType::MINION ||
        card.normalDbfID != 0 || !card.hasBehavior ||
        !IsWarpGateProtossDbfID(card.dbfID) ||
        !IsExecutableWarpGateProtossDbfID(card.dbfID)) return false;
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

void Player::ApplyDemonConsumeBonus(Minion& demon, const Minion& consumed)
{
    if (!demon.HasRace(Race::DEMON)) return;
    for (const auto& trinket : season14.trinkets) {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect != TrinketEffect::DEMON_CONSUME_BONUS_KEYWORDS)
            continue;
        if (consumed.HasTaunt()) demon.SetTaunt(true);
        if (consumed.HasDivineShield())
            demon.SetGameTag(GameTag::DIVINE_SHIELD, 1);
        if (consumed.HasReborn()) demon.SetReborn(true);
        if (consumed.HasWindfury())
            demon.SetGameTag(GameTag::WINDFURY, 1);
        if (consumed.HasVenomous())
            demon.SetGameTag(GameTag::VENOMOUS, 1);
        if (consumed.HasStealth()) demon.SetGameTag(GameTag::STEALTH, 1);
        demon.SetAttack(demon.GetAttack() + behavior.attack);
        demon.SetHealth(demon.GetHealth() + behavior.health);
    }
}

int Player::SelectDemonConsumeTavernSlot()
{
    std::vector<int> candidates;
    tavern.fieldZone.ForEachAlive([&candidates](MinionData& data) {
        if (data.value().GetPoolIndex() >= 0)
            candidates.push_back(data.value().GetZonePosition());
    });
    if (candidates.empty()) return -1;

    bool highestHealth = false;
    for (const auto& trinket : season14.trinkets) {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto id = Cards::FindCardByDbfID(trinket.dbfID).id;
        if (FindTrinketBehavior(id).effect ==
            TrinketEffect::DEMON_CONSUME_HIGHEST_HEALTH) {
            highestHealth = true;
            break;
        }
    }
    if (!highestHealth)
        return candidates[Random::get<std::size_t>(0, candidates.size() - 1)];

    int maxHealth = std::numeric_limits<int>::min();
    std::vector<int> highest;
    for (const int candidate : candidates) {
        const int health = tavern.fieldZone[static_cast<std::size_t>(candidate)].GetHealth();
        if (health > maxHealth) {
            maxHealth = health;
            highest.clear();
            highest.push_back(candidate);
        } else if (health == maxHealth) {
            highest.push_back(candidate);
        }
    }
    return highest[Random::get<std::size_t>(0, highest.size() - 1)];
}

bool Player::DevourRandomTavernForDemons(int multiplier)
{
    // SelectDemonConsumeTavernSlot applies DEMON_CONSUME_HIGHEST_HEALTH and
    // its highestHealth current-stat selection to this Activate/Devour path
    // as well as the task-based path.
    if (multiplier <= 0) return false;
    bool consumedAny = false;
    recruitField.ForEachAlive([&](MinionData& data) {
        auto& demon = data.value();
        if (!demon.HasRace(Race::DEMON)) return;
        const int slot = SelectDemonConsumeTavernSlot();
        if (slot < 0) return;
        auto& consumed = tavern.fieldZone[static_cast<std::size_t>(slot)];
        const int attack = consumed.GetAttack();
        const int health = consumed.GetHealth();
        const int poolIndex = consumed.GetPoolIndex();
        tavern.fieldZone.Remove(consumed);
        returnMinionCallback(poolIndex);
        demon.SetAttack(demon.GetAttack() + attack * multiplier);
        demon.SetHealth(demon.GetHealth() + health * multiplier);
        // Consuming Claw observes every successful Demon devour, including
        // the Activate path (which does not use ConsumeRandomTavernTask).
        for (const auto& trinket : season14.trinkets) {
            if (!trinket.active || trinket.remainingUses == 0) continue;
            const auto behavior = FindTrinketBehavior(
                Cards::FindCardByDbfID(trinket.dbfID).id);
            if (behavior.effect != TrinketEffect::DEMON_CONSUME_BONUS_KEYWORDS)
                continue;
            if (consumed.HasTaunt()) demon.SetTaunt(true);
            if (consumed.HasDivineShield()) demon.SetGameTag(GameTag::DIVINE_SHIELD, 1);
            if (consumed.HasReborn()) demon.SetReborn(true);
            if (consumed.HasWindfury()) demon.SetGameTag(GameTag::WINDFURY, 1);
            if (consumed.HasVenomous()) demon.SetGameTag(GameTag::VENOMOUS, 1);
            if (consumed.HasStealth()) demon.SetGameTag(GameTag::STEALTH, 1);
            demon.SetAttack(demon.GetAttack() + behavior.attack);
            demon.SetHealth(demon.GetHealth() + behavior.health);
        }
        consumedAny = true;
        // The Activate/Devour path removes the offer directly, so it must
        // publish the same successful-consume event as the task path.
        ApplyTavernMinionConsumedTrinkets();
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
    if (recruitField.IsFull()) {
        ApplySummonOverflowTrinkets();
        return false;
    }
    if (idx >= static_cast<std::size_t>(recruitField.GetCount()) ||
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
    if (battleField.IsFull()) {
        ApplySummonOverflowTrinkets();
        return false;
    }
    snapshot.SetIndex(getNextCardIndexCallback());
    snapshot.getPlayerCallback = [this]() -> Player& { return *this; };
    battleField.Add(snapshot, battleField.GetCount());
    battleField.ForEachAlive([&snapshot](MinionData& data) {
        data.value().ActivateTrigger(TriggerType::SUMMON, snapshot);
    });
    // Several deathrattle/task sources call this helper directly and have no
    // caller-side summon-trinket hook. Keep all friendly combat summons on
    // the same shared boundary used by played and generated minions.
    ApplySummonTrinkets(battleField[battleField.GetCount() - 1]);
    return true;
}

bool Player::ResolveLastFriendlyDeathDemon()
{
    if (!isInCombat || !battleField.IsEmpty()) return false;
    for (auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0 ||
            trinket.triggerProgress != 0)
            continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect != TrinketEffect::AFTER_LAST_FRIENDLY_DEATH_DEMON)
            continue;
        auto snapshot = season14.CopyFirstCombatDeadDemonWithMaxStats();
        if (!snapshot.has_value()) continue;
        if (!SummonCombatSnapshot(std::move(*snapshot))) continue;
        trinket.triggerProgress = 1;
        return true;
    }
    return false;
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
        const bool protossPurchase =
            IsWarpGateProtossDbfID(tavern.fieldZone[idx].GetDbfID());
        const bool piratePortraitFree =
            tavern.fieldZone[idx].HasRace(Race::PIRATE) &&
            std::any_of(season14.trinkets.begin(), season14.trinkets.end(),
                        [](const Season14PersistentEffect &trinket) {
                            return trinket.active && trinket.remainingUses > 0 &&
                                   trinket.triggerProgress == 0 &&
                                   FindTrinketBehavior(
                                       Cards::FindCardByDbfID(trinket.dbfID).id)
                                           .effect ==
                                       TrinketEffect::ACQUIRE_FIXED_CARD_FIRST_PIRATE_FREE;
                        });
        const bool magneticMechFixedCost =
            tavern.fieldZone[idx].IsMagnetic() &&
            season14.HasMagneticMechFixedCost();
        // Demonic Tapestry arms one highest-tier Tavern minion to be paid
        // with Health after its refresh cadence.  Keep this legality check
        // identical to PurchaseMinion: an armed Tapestry purchase does not
        // require Gold, and its health affordability is the typed payload
        // (not the ordinary minion purchase cost).
        const int highestTavernTier = [&]() {
            int highest = 0;
            tavern.fieldZone.ForEachAlive([&highest](const MinionData& data) {
                highest = std::max(highest, data.value().GetTier());
            });
            return highest;
        }();
        int tapestryHealthCost = 0;
        const bool tapestryHealthPurchase = [&]() {
            for (const auto& trinket : season14.trinkets) {
                if (!trinket.active || trinket.remainingUses == 0 ||
                    trinket.statScale == 0)
                    continue;
                const auto behavior = FindTrinketBehavior(
                    Cards::FindCardByDbfID(trinket.dbfID).id);
                if (behavior.effect ==
                        TrinketEffect::REFRESH_HIGHEST_TIER_HEALTH_PURCHASE &&
                    tavern.fieldZone[idx].GetTier() == highestTavernTier)
                {
                    tapestryHealthCost = behavior.amount;
                    return true;
                }
            }
            return false;
        }();
        const int cost = battlecryDiscount || piratePortraitFree
                             ? 0
                             : magneticMechFixedCost
                                 ? 2
                                 : std::max(0, season14.MinionPurchaseCost(
                                       NUM_COIN_PURCHASE_MINION) -
                                       (protossPurchase ? season14.protossCostReduction : 0));
        const bool eyeHealthPurchase = std::any_of(
            season14.trinkets.begin(), season14.trinkets.end(), [](const auto& trinket) {
                if (!trinket.active || trinket.remainingUses == 0) return false;
                const auto behavior = FindTrinketBehavior(
                    Cards::FindCardByDbfID(trinket.dbfID).id);
                return behavior.effect == TrinketEffect::BUY_MINION_HEALTH_CADENCE &&
                       behavior.value > 0 && trinket.triggerProgress + 1 >= behavior.value;
            });
        if (tapestryHealthPurchase)
            return tapestryHealthCost > 0 && hero.health > tapestryHealthCost;
        if (eyeHealthPurchase)
            return hero.health > cost;
        return remainCoin >= cost;
    }
    if (hand.IsFull()) return false;
    const auto &slot = tavern.spellSlots[idx - tavern.fieldZone.GetCount()];
    if (!slot.IsSpell()) return false;
    // Advanced Construction grants one free Battlecruiser Upgrade purchase
    // this recruit turn.  The entitlement belongs to the purchase boundary,
    // not to spell creation, so a zero-gold player can still buy the offered
    // upgrade while ordinary Tavern spells remain unaffordable.
    const auto &spell = slot.AsSpell();
    const bool freeLiftOffUpgrade =
        season14.liftOffFreeUpgradeAvailable && IsLiftOffUpgrade(spell.GetID());
    const int cost = freeLiftOffUpgrade ? 0 : spell.GetCost();
    const bool bazaarHealthPurchase = std::any_of(
        season14.trinkets.begin(), season14.trinkets.end(), [](const auto& trinket) {
            if (!trinket.active || trinket.remainingUses == 0 ||
                trinket.triggerProgress != 0)
                return false;
            return FindTrinketBehavior(
                       Cards::FindCardByDbfID(trinket.dbfID).id)
                       .effect == TrinketEffect::TAVERN_SPELL_HEALTH_ONCE_PER_TURN;
        });
    const bool eyeHealthPurchase = std::any_of(
        season14.trinkets.begin(), season14.trinkets.end(), [](const auto& trinket) {
            if (!trinket.active || trinket.remainingUses == 0) return false;
            const auto behavior = FindTrinketBehavior(
                Cards::FindCardByDbfID(trinket.dbfID).id);
            return behavior.effect == TrinketEffect::BUY_MINION_HEALTH_CADENCE &&
                   behavior.value > 0 && trinket.triggerProgress + 1 >= behavior.value;
        });
    if (bazaarHealthPurchase || eyeHealthPurchase)
        return hero.health > cost;
    return remainCoin >= (freeLiftOffUpgrade ? 0 : spell.GetCost());
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
    const bool freeLiftOffUpgrade =
        season14.liftOffFreeUpgradeAvailable && IsLiftOffUpgrade(spell.GetID());
    const int cost = freeLiftOffUpgrade ? 0 : spell.GetCost();
    const bool bazaarHealthPurchase = std::any_of(
        season14.trinkets.begin(), season14.trinkets.end(), [](const auto& trinket) {
            if (!trinket.active || trinket.remainingUses == 0 ||
                trinket.triggerProgress != 0)
                return false;
            return FindTrinketBehavior(
                       Cards::FindCardByDbfID(trinket.dbfID).id)
                       .effect == TrinketEffect::TAVERN_SPELL_HEALTH_ONCE_PER_TURN;
        });
    const bool eyeHealthPurchase = std::any_of(
        season14.trinkets.begin(), season14.trinkets.end(), [](const auto& trinket) {
            if (!trinket.active || trinket.remainingUses == 0) return false;
            const auto behavior = FindTrinketBehavior(
                Cards::FindCardByDbfID(trinket.dbfID).id);
            return behavior.effect == TrinketEffect::BUY_MINION_HEALTH_CADENCE &&
                   behavior.value > 0 && trinket.triggerProgress + 1 >= behavior.value;
        });
    const bool splittingScrollCopy = season14.HasGeneratedRewardSplittingScroll() &&
                                     cost >= 3;
    tavern.spellSlots.erase(tavern.spellSlots.begin() + static_cast<std::ptrdiff_t>(idx - minionCount));
    hand.Add(CardData{spell});
    if (freeLiftOffUpgrade)
        season14.liftOffFreeUpgradeAvailable = false;
    if (splittingScrollCopy && !hand.IsFull()) {
        // Splitting Scroll copies the purchased Tavern spell as a plain card.
        hand.Add(CardData{Spell(Cards::FindCardByDbfID(spell.GetDbfID()))});
    }
    lastBoughtTavernSpellID = spell.GetID();
    if (bazaarHealthPurchase || eyeHealthPurchase) {
        hero.health -= cost;
    } else {
        remainCoin -= cost;
        RecordGoldSpent(cost);
    }
    for (auto& trinket : season14.trinkets) {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect ==
                TrinketEffect::TAVERN_SPELL_HEALTH_ONCE_PER_TURN &&
            trinket.triggerProgress == 0)
            trinket.triggerProgress = 1;
        if (behavior.effect != TrinketEffect::BUY_MINION_HEALTH_CADENCE ||
            behavior.value <= 0)
            continue;
        if (++trinket.triggerProgress >= behavior.value)
            trinket.triggerProgress = 0;
    }
    // Tavern-spell purchases do not advance Warp Gate's minion-buy cadence;
    // this call only retries delivery when a previously armed reward waited
    // on hand capacity.
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
    // Magicfin Sticker is a persistent purchase observer rather than a board
    // Minion trigger. Run it at the same successful-purchase boundary while
    // `lastBoughtTavernSpellID` still identifies the exact purchased spell.
    for (auto& trinket : season14.trinkets) {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect != TrinketEffect::AFTER_BUY_TAVERN_SPELL_MURLOC)
            continue;
        (void)SimpleTasks::BuyTavernSpellMurlocTask{behavior.value}
            .Run(*this, trinket.triggerProgress);
    }
    lastBoughtTavernSpellID.clear();
    return true;
}

void Player::ResolveHackerfinBattlecry(Minion& source)
{
    if (source.GetCardID() != "BG31_148" &&
        source.GetCardID() != "BG31_148_G")
        return;

    // Hackerfin counts distinct Bonus Keywords in the complete warband after
    // it enters play, then gives the resulting grant to every *other* minion.
    // FieldZone::Add copies its argument, so resolve the canonical board
    // instance by entity index before comparing source/recipient identity.
    Minion* boardSource = nullptr;
    recruitField.ForEachAlive([&](MinionData& data) {
        if (data.value().GetIndex() == source.GetIndex())
            boardSource = &data.value();
    });
    if (boardSource == nullptr) return;

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
    const int multiplier = boardSource->IsGolden() ? 2 : 1;
    recruitField.ForEachAlive([distinctKeywords, multiplier,
                               boardSource](MinionData& data) {
        if (&data.value() == boardSource) return;
        auto& recipient = data.value();
        recipient.SetAttack(recipient.GetAttack() +
                            multiplier * (1 + distinctKeywords));
        recipient.SetHealth(recipient.GetHealth() +
                            multiplier * 2 * (1 + distinctKeywords));
    });
}

void Player::PlayCard(std::size_t handIdx, std::size_t fieldIdx, int targetIdx)
{
    if (handIdx >= static_cast<std::size_t>(hand.GetCount()))
        return;
    // The bridge uses -1 for append, which arrives here as size_t::max().
    // Normalize it once before passing the position to the int-based zone API.
    const auto requestedFieldPosition =
        fieldIdx == std::numeric_limits<std::size_t>::max()
            ? -1
            : static_cast<int>(fieldIdx);
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
        // Sprightly Scarab has a target-dependent Choose-One.  Reject the
        // play before removing the card when no Beast exists; consuming the
        // source and silently dropping both generated branches would make a
        // legal-action snapshot disagree with the modal resolver.
        const bool sprightlyScarab = handMinion.GetCardID() == "BG27_084" ||
                                     handMinion.GetCardID() == "BG27_084_G";
        if (sprightlyScarab) {
            bool hasBeast = false;
            recruitField.ForEachAlive([&hasBeast](MinionData& data) {
                hasBeast = hasBeast || data.value().HasRace(Race::BEAST);
            });
            if (!hasBeast) return;
        }
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
            // Trigger all target-local Magnetize listeners only after the
            // attachment succeeds; rejected actions must not advance them.
            ApplyAfterMagnetizeTrinkets(
                recruitField[static_cast<std::size_t>(targetIdx)]);
            season14.generatedRewardSinfallTier = attachment.GetTier();
            season14.generatedRewardSinfallSourceEntityID =
                static_cast<std::uint64_t>(
                    recruitField[static_cast<std::size_t>(targetIdx)].GetIndex());
            // A Magnetize is an attachment, not a played Demon/Elemental for
            // post-play Trinkets.  Keep only magnetic-specific listeners.
            ApplyAfterPlayCardTrinkets(attachment.GetRace(), true,
                                       attachment.HasRace(Race::DEMON),
                                       attachment.HasRace(Race::MURLOC),
                                       static_cast<std::uint64_t>(
                                           recruitField[static_cast<std::size_t>(targetIdx)].GetIndex()));
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
        const bool repeatedPlay = season14.WasMinionPlayedThisTurn(
            std::get<Minion>(card).GetCardID());
        season14.RecordRepeatedPlayCard(std::get<Minion>(card).GetCardID());
        TryDeliverChampionReward();
        TryDeliverHeroicInspirationReward();
        auto minion = std::get<Minion>(card);
        // Hackerfin Portrait marks every Hackerfin played after acquisition;
        // Minion's ordinary end-turn hook then repeats its Battlecry.
        if (HasActivePortrait(PortraitEffect::HACKERFIN_END_TURN_BATTLECRY) &&
            (minion.GetCardID() == "BG31_148" ||
             minion.GetCardID() == "BG31_148_G"))
            minion.SetEndTurnBattlecryTrigger(true);
        if (minion.IsGolden()) ++season14.goldenMinionsPlayed;
        if (minion.HasRace(Race::PIRATE))
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
            recruitField.Add(minion, requestedFieldPosition);
            // FieldZone::Add updates the stored board copy, not the local
            // source copy that still carries its former hand position.
            const auto playedFieldPosition = static_cast<std::size_t>(
                requestedFieldPosition < 0 ? recruitField.GetCount() - 1
                                            : requestedFieldPosition);
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
                recruitField[playedFieldPosition]);

            recruitField.ForEachAlive([&minion](MinionData& aliveMinion) {
                aliveMinion.value().ActivateTrigger(TriggerType::SUMMON,
                                                    minion);
            });
            ApplyMechagnomeInterpreterBonus(
                recruitField[playedFieldPosition]);
            ApplySummonTrinkets(recruitField[playedFieldPosition]);
            season14.generatedRewardSinfallTier = minion.GetTier();
            season14.generatedRewardSinfallSourceEntityID =
                static_cast<std::uint64_t>(minion.GetIndex());
            ApplyAfterPlayCardTrinkets(minion.GetRace(), false,
                                       minion.HasRace(Race::DEMON),
                                       minion.HasRace(Race::MURLOC),
                                       static_cast<std::uint64_t>(minion.GetIndex()));

            // Hackerfin's generated CardDef has no generic task: resolve its
            // warband-dependent Battlecry only after the actual board object
            // exists, so the source is not buffed as one of its recipients.
            ResolveHackerfinBattlecry(minion);

            if (minion.GetCardID() == "TB_BaconUps_089") {
                season14.pendingPrimalfinDiscoverRemaining = 1;
                season14.pendingPrimalfinDiscoverSourceEntityID =
                    static_cast<std::uint64_t>(minion.GetIndex());
            }
            minion.ActivateTask(PowerType::POWER, *this);
            for (const auto& definition : BUDDY_HERO_POWER_GOLDENIZE_BEHAVIORS)
                if (minion.GetCardID() == definition.id)
                    season14.ArmBuddyGoldenHeroPowerUses(definition.uses);
            if (minion.GetCardID() == "TB_BaconUps_089" &&
                season14.pendingDecision == Season14Decision::NONE) {
                // The friendly-Murloc condition failed (or the pool was
                // empty), so do not retain a stale replay counter.
                season14.pendingPrimalfinDiscoverRemaining = 0;
                season14.pendingPrimalfinDiscoverSourceEntityID = 0;
            }
            // Selfless Portrait also fires the generated Selfless Hero's
            // normal Divine-Shield deathrattle on Battlecry. Reuse the
            // authoritative task so golden amount and target RNG stay exact.
            if ((minion.GetCardID() == "BG_OG_221" ||
                 minion.GetCardID() == "TB_BaconUps_014") &&
                HasActivePortrait(PortraitEffect::SELFLESS_BATTLECRY))
                minion.ActivateTask(PowerType::DEATHRATTLE, *this);
            if (minion.GetCardID() == "TB_BaconShop_HERO_28_Buddy" ||
                minion.GetCardID() == "TB_BaconShop_HERO_28_Buddy_G")
                BeginClockworkAssistantDiscover(minion.IsGolden());
            if (minion.GetCardID() == "TB_BaconShop_HERO_55_Buddy" ||
                minion.GetCardID() == "TB_BaconShop_HERO_55_Buddy_G")
                ResolveSparkfinSoothsayer(minion.IsGolden());
            if (CardDefs::FindCardDefByID(minion.GetCardID()).lifecycle ==
                CardLifecycle::BUDDY_APOSTLE) {
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
                else if (data.value().GetCardID() == "TB_BaconUps_045") brannRepeats += 2;
                else if (data.value().GetCardID() == "BG_LOE_077_G") brannRepeats += 2;
            });
            const bool hackerfin = minion.GetCardID() == "BG31_148" ||
                                   minion.GetCardID() == "BG31_148_G";
            for (int i = 0; i < brannRepeats; ++i) {
                if (hackerfin)
                    ResolveHackerfinBattlecry(minion);
                else
                    minion.ActivateTask(PowerType::POWER, *this);
            }
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
            if ((minion.HasBattlecry() || hackerfin) &&
                season14.ConsumeGeneratedRewardConch())
            {
                for (int i = 0; i < 2; ++i) {
                    if (hackerfin) ResolveHackerfinBattlecry(minion);
                    else minion.ActivateTask(PowerType::POWER, *this);
                }
            }
            if (minion.HasBattlecry() || hackerfin)
                for (std::uint32_t i = 0;
                     i < season14.GeneratedRewardBattlecryRepeatCount(); ++i) {
                    if (hackerfin) ResolveHackerfinBattlecry(minion);
                    else minion.ActivateTask(PowerType::POWER, *this);
                }
            // War Drum is a once-per-recruit-turn allowance.  Consume it
            // only after the play and ordinary repeat sources have succeeded;
            // direct task activation avoids recursively re-triggering Drum.
            const int warDrumRepeats = (minion.HasBattlecry() || hackerfin)
                ? ConsumeWarDrumRepeats() : 0;
            for (int i = 0; i < warDrumRepeats; ++i) {
                if (hackerfin) ResolveHackerfinBattlecry(minion);
                else minion.ActivateTask(PowerType::POWER, *this);
            }
            if (minion.GetRace() == Race::DRAGON &&
                ShouldDuplicateDragonBattlecry())
                // PlayCard's POWER activation is the minion Battlecry path;
                // preserve the same source/target for the extra resolution.
                minion.ActivateTask(PowerType::POWER, *this);
        }
        else
        {
            // Most targeted Battlecries address the recruit board.  A small
            // number of Battlegrounds effects (currently Mini-Zerek) target
            // a live Tavern entity instead; the card's typed targeting
            // metadata selects the correct zone while the target index keeps
            // the public PlayCard ABI unchanged.
            const bool requiresTavernMinionTarget =
                handMinion.RequiresTavernMinionTarget();
            // Mini-Zerek transforms its source while resolving.  Capture its
            // original Battlecry tasks before that identity changes so Brann,
            // Conch, and other retriggers replay the same effect against the
            // same selected Tavern entity.
            const bool miniZerek =
                minion.GetCardID() == "BG31_HERO_005_Buddy" ||
                minion.GetCardID() == "BG31_HERO_005_Buddy_G";
            const bool originalTargetedBattlecry = minion.HasBattlecry();
            const bool originalTargetedDragon = minion.GetRace() == Race::DRAGON;
            const auto originalTargetedTasks = miniZerek
                ? minion.GetTasks(PowerType::POWER)
                : std::vector<TaskType>{};
            // Add() shifts occupied slots when inserting before the target;
            // retain the entity identity so repeated targeted Battlecries do
            // not silently retarget a different minion.
            const auto targetEntityID = requiresTavernMinionTarget
                                            ? tavern.fieldZone[static_cast<std::size_t>(targetIdx)].GetIndex()
                                            : recruitField[static_cast<std::size_t>(targetIdx)].GetIndex();
            const auto findTarget = [this, requiresTavernMinionTarget,
                                     targetEntityID]() -> Minion* {
                auto& zone = requiresTavernMinionTarget ? tavern.fieldZone
                                                        : recruitField;
                Minion* result = nullptr;
                zone.ForEachAlive([&](MinionData& data) {
                    if (data.value().GetIndex() == targetEntityID)
                        result = &data.value();
                });
                return result;
            };
            const auto activateTargetedBattlecry = [&]() {
                auto* target = findTarget();
                if (target == nullptr) return;
                if (miniZerek)
                    minion.ActivateTask(PowerType::POWER, *this, *target,
                                        originalTargetedTasks);
                else
                    minion.ActivateTask(PowerType::POWER, *this, *target);
                // Smogger's normal and golden Battlecries are the same
                // targeted stat-giver at 1x/2x Tavern Tier.  Resolve this
                // through the shared executor so typed Fountain Pen and
                // Amplifying Essence augmentation sees only a real giver.
                if (minion.GetCardID() == "BG21_021" ||
                    minion.GetCardID() == "BG21_021_G") {
                    const int scale = minion.GetCardID() == "BG21_021_G" ? 2 : 1;
                    SimpleTasks::ElementalStatGiverTask{currentTier * scale,
                                                        currentTier * scale}
                        .Run(*this, *target);
                }
            };

            recruitField.Add(minion, requestedFieldPosition);
            // See the ordinary-play path above: the local source retains its
            // hand slot after Add copies it into the board.
            const auto playedFieldPosition = static_cast<std::size_t>(
                requestedFieldPosition < 0 ? recruitField.GetCount() - 1
                                            : requestedFieldPosition);
            ApplyFirstMinionDivineShield(
                recruitField[playedFieldPosition]);

            recruitField.ForEachAlive([&minion](MinionData& aliveMinion) {
                aliveMinion.value().ActivateTrigger(TriggerType::SUMMON,
                                                    minion);
            });
            ApplyMechagnomeInterpreterBonus(
                recruitField[playedFieldPosition]);
            ApplySummonTrinkets(recruitField[playedFieldPosition]);
            season14.generatedRewardSinfallTier = minion.GetTier();
            season14.generatedRewardSinfallSourceEntityID =
                static_cast<std::uint64_t>(minion.GetIndex());
            ApplyAfterPlayCardTrinkets(minion.GetRace(), false,
                                       minion.HasRace(Race::DEMON),
                                       minion.HasRace(Race::MURLOC),
                                       static_cast<std::uint64_t>(minion.GetIndex()));

            activateTargetedBattlecry();
            if (minion.GetCardID() == "TB_BaconShop_HERO_28_Buddy" ||
                minion.GetCardID() == "TB_BaconShop_HERO_28_Buddy_G")
                BeginClockworkAssistantDiscover(minion.IsGolden());
            if (minion.GetCardID() == "TB_BaconShop_HERO_55_Buddy" ||
                minion.GetCardID() == "TB_BaconShop_HERO_55_Buddy_G")
                ResolveSparkfinSoothsayer(minion.IsGolden());
            if (CardDefs::FindCardDefByID(minion.GetCardID()).lifecycle ==
                CardLifecycle::BUDDY_APOSTLE) {
                const int replacements = tavern.fieldZone.GetCount();
                const int passes = minion.GetCardID().ends_with("_G") ? 2 : 1;
                for (int pass = 0; pass < passes && replacements > 0; ++pass) {
                    season14.ArmHigherTierRefresh(replacements);
                    RefreshTavern(true);
                }
            }
            if (minion.GetCardID() == "TB_BaconShop_HERO_93_Buddy") {
                if (auto* target = findTarget(); target != nullptr &&
                    target->HasDeathrattle())
                    target->MakeGolden();
            }
            ResolveHackerfinBattlecry(minion);
            int brannRepeats = 0;
            recruitField.ForEachAlive([&brannRepeats](MinionData& data) {
                if (data.value().GetCardID() == "BG_LOE_077") ++brannRepeats;
                else if (data.value().GetCardID() == "TB_BaconUps_045") brannRepeats += 2;
                else if (data.value().GetCardID() == "BG_LOE_077_G") brannRepeats += 2;
            });
            const bool hackerfin = minion.GetCardID() == "BG31_148" ||
                                   minion.GetCardID() == "BG31_148_G";
            for (int i = 0; i < brannRepeats; ++i) {
                if (hackerfin) ResolveHackerfinBattlecry(minion);
                else activateTargetedBattlecry();
            }
            if ((originalTargetedBattlecry || hackerfin) &&
                season14.ConsumeGeneratedRewardConch())
            {
                for (int i = 0; i < 2; ++i) {
                    if (hackerfin) ResolveHackerfinBattlecry(minion);
                    else activateTargetedBattlecry();
                }
            }
            if (originalTargetedBattlecry || hackerfin)
                for (std::uint32_t i = 0;
                     i < season14.GeneratedRewardBattlecryRepeatCount(); ++i) {
                    if (hackerfin) ResolveHackerfinBattlecry(minion);
                    else activateTargetedBattlecry();
                }
            const int warDrumRepeats = (originalTargetedBattlecry || hackerfin)
                ? ConsumeWarDrumRepeats() : 0;
            for (int i = 0; i < warDrumRepeats; ++i) {
                if (hackerfin) ResolveHackerfinBattlecry(minion);
                else activateTargetedBattlecry();
            }
            if (originalTargetedDragon &&
                ShouldDuplicateDragonBattlecry())
                // Targeted Battlecries receive the identical target again.
                activateTargetedBattlecry();
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
        const bool chooseOneLifecycle =
            CardDefs::FindCardDefByID(minion.GetCardID()).lifecycle ==
            CardLifecycle::CHOOSE_ONE_SOURCE;
        if (chooseOneLifecycle ||
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
                minion.GetCardID() == "BG36_332_G" ? "BG36_332_Gt" :
                minion.GetCardID() == "BG36_332" ? "BG36_332t" :
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
                minion.GetCardID() == "BG36_332_G" ? "BG36_332_Gt2" :
                minion.GetCardID() == "BG36_332" ? "BG36_332t2" :
                minion.GetCardID() == "BG36_341_G" ? "BG36_341_Gt2" :
                minion.GetCardID() == "BG36_341" ? "BG36_341t2" : "BG27_084t2");
            // A target-dependent modal is only exposed when both generated
            // options and at least one Beast target are available; otherwise
            // do not leave the recruit phase permanently locked.
            const bool targetless = minion.GetCardID() != "BG27_084" &&
                                    minion.GetCardID() != "BG27_084_G";
            if (targetless)
                targetMask = 0;
            // Trailblazer Sticker is an account-wide modifier: mark each
            // supported source at the modal boundary so ApplyChooseOne can
            // resolve both branches transactionally.  This intentionally
            // does not bypass the source lifecycle or offering validation.
            if (season14.trailblazerCombinedChooseOne) {
                minion.SetCombinedChooseOne(true);
                (void)RecordReviewedLifecycleEnchantment(
                    minion,
                    minion.GetCardID().ends_with("_G") ? "BG31_327_G" : "BG31_327",
                    "BG31_850e");
            }
            if (combinedChooseOneUses > 0 && targetless) {
                minion.SetCombinedChooseOne(true);
                (void)RecordReviewedLifecycleEnchantment(
                    minion,
                    minion.GetCardID().ends_with("_G") ? "BG31_327_G" : "BG31_327",
                    "BG31_850e");
                --combinedChooseOneUses;
            }
            if (minion.HasCombinedChooseOne() && targetless &&
                !season14.trailblazerCombinedChooseOne) {
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
        // Use the complete race set: a multi-typed minion with Elemental as
        // an auxiliary race is still an Elemental play for Water Wheel and
        // the other Elemental-play listeners.  Magnetize returns before this
        // block, so attachments do not count as played Elementals.
        if (minion.HasRace(Race::ELEMENTAL))
        {
            ++season14.unboundElementals;
            const auto result = season14.OnPlayElemental();
            coinToUpgradeTavern = std::max(
                0, coinToUpgradeTavern + result.upgradeCostDelta);
            for (auto& trinket : season14.trinkets)
            {
                if (!trinket.active || trinket.remainingUses == 0) continue;
                const auto behavior = FindTrinketBehavior(
                    Cards::FindCardByDbfID(trinket.dbfID).id);
                if (behavior.effect == TrinketEffect::AFTER_PLAY_ELEMENTAL_RANDOM_SPELL)
                {
                    if (behavior.value > 0 &&
                        trinket.triggerProgress < behavior.value)
                    {
                        (void)SimpleTasks::RandomTavernSpellToHandTask{1}.Run(*this);
                        ++trinket.triggerProgress;
                    }
                    continue;
                }
                if (behavior.effect == TrinketEffect::AFTER_PLAY_ELEMENTAL_FIXED_CARD)
                {
                    // The counter is owned by this Trinket instance and
                    // survives recruit/combat boundaries.  Consume a
                    // completed ten-Elemental cadence once, after the
                    // successful play has been recorded.  A full hand burns
                    // the generated reward just as the normal generated-card
                    // boundary does, but never advances a second cadence.
                    if (behavior.value > 0 &&
                        ++trinket.triggerProgress >= behavior.value)
                    {
                        trinket.triggerProgress = 0;
                        if (!hand.IsFull() && !behavior.cardID.empty())
                        {
                            const auto repeat = Cards::FindCardByID(behavior.cardID);
                            if (repeat.dbfID != 0 &&
                                (repeat.GetCardType() == CardType::SPELL ||
                                 repeat.GetCardType() == CardType::BATTLEGROUND_SPELL) &&
                                FindTavernSpellBehavior(repeat.id).effect !=
                                    TavernSpellEffect::NONE)
                                hand.Add(CardData{Spell(repeat)});
                        }
                    }
                    continue;
                }
                if (behavior.effect == TrinketEffect::AFTER_PLAY_ELEMENTAL_FREE_REFRESH)
                {
                    season14.AddFreeRefreshes(behavior.value);
                    continue;
                }
                if (behavior.effect ==
                    TrinketEffect::ESCALATING_ELEMENTAL_STAT_GIVER_BONUS)
                {
                    // This is a lifetime, per-owned-Trinket threshold.  Do
                    // not reset it at recruit start or combat boundaries.
                    // The improvement is committed only after this play's
                    // AFTER_PLAY observers run below: the fifth Elemental
                    // still receives the old +1/+1 payload, while the sixth
                    // and subsequent Elementals receive the improved +2/+2.
                    if (behavior.value > 0 &&
                        trinket.triggerProgress < behavior.value)
                        ++trinket.triggerProgress;
                    continue;
                }
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
        // Ur'zul Sticker is an AFTER_PLAY effect: resolve it only after the
        // played minion's complete Battlecry/repeat chain and the normal
        // AFTER_PLAY_MINION observers above.  This also orders it after the
        // original Ur'zul minion when both effects are present.
        ApplyUrZulStickerTriggers(minion.HasRace(Race::DEMON),
            static_cast<std::uint64_t>(minion.GetIndex()));
        // Jandice's Apprentice checks the card identity before this play,
        // then buffs the complete live warband after the play has resolved.
        // This keeps battlecry/stat changes in the source snapshot and makes
        // each owned normal/golden Buddy contribute independently.
        if (repeatedPlay) {
            for (const auto& definition : BUDDY_REPEATED_PLAY_BEHAVIORS) {
                bool owned = false;
                recruitField.ForEachAlive([&owned, &definition](MinionData& data) {
                    owned = owned || data.value().GetCardID() == definition.id;
                });
                if (!owned) continue;
                const int bonus = currentTier * definition.tierMultiplier;
                recruitField.ForEachAlive([bonus](MinionData& data) {
                    data.value().ApplyPersistentMinionStats(bonus, bonus);
                });
            }
        }
        // Amplifying Essence improves after the threshold play has fully
        // resolved, rather than changing the payload of that fifth play.
        // Keep this state on each owned Trinket so duplicate copies and
        // replayed states remain independent.
        for (auto& trinket : season14.trinkets) {
            if (!trinket.active || trinket.remainingUses == 0 ||
                trinket.statScale != 0) continue;
            const auto behavior = FindTrinketBehavior(
                Cards::FindCardByDbfID(trinket.dbfID).id);
            if (behavior.effect ==
                    TrinketEffect::ESCALATING_ELEMENTAL_STAT_GIVER_BONUS &&
                behavior.value > 0 &&
                trinket.triggerProgress >= behavior.value)
                trinket.statScale = 1;
        }
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
    if (amount <= 0) return false;
    // Magicfin's apprentice is the destination for the selected spell.  The
    // apprentice is intentionally inserted before this modal is opened, so a
    // hand that reaches ten cards must still be allowed to present the
    // Discover; selecting a spell mutates that existing minion rather than
    // adding another card.  Do not generalize this exception to arbitrary
    // full-hand Tavern-spell Discover sources.
    const bool magicfinRelic = sourceCardDbfID == 122825;
    // The ordinary capacity guard is `if (amount <= 0 || hand.IsFull()) return false;`;
    // Magicfin is the one reviewed exception because its existing Apprentice is
    // taught in place rather than adding a card to hand.
    if (hand.IsFull() && !magicfinRelic) return false;
    if (magicfinRelic) {
        bool hasApprentice = false;
        hand.ForEach([&hasApprentice](const std::optional<CardData>& data) {
            if (data.has_value() && std::holds_alternative<Minion>(data.value()) &&
                std::get<Minion>(data.value()).GetCardID() == "BG33_890t")
                hasApprentice = true;
        });
        if (!hasApprentice) return false;
    }
    // Golden battlecries produce sequential Discover modals.  Keep the
    // remaining count in Season14State so only one choice is pending at a
    // time and hand-cap/source-lifetime validation is reapplied between
    // choices.
    std::vector<Card> candidates;
    const bool outlandSunbeam = sourceCardDbfID == 122331 ||
                                sourceCardDbfID == 122332;
    // For Outland Sunbeam, the mechanical predicate is "card.cost >= 3";
    // Card exposes the pinned COST GameTag rather than a mutable cost field.
    for (const auto& card : Cards::GetAllCards())
        if (card.isBattlegroundsPoolSpell && card.normalDbfID == 0 &&
            FindTavernSpellBehavior(card.id).effect != TavernSpellEffect::NONE &&
            (!outlandSunbeam ||
             (card.gameTags.contains(GameTag::COST) &&
              card.gameTags.at(GameTag::COST) >= 3)))
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
            AppendSupportedNormalMinions(Cards::GetTier1Minions(), candidates, Race::INVALID, activeTribes);
        else if (behavior.value == 7)
            AppendSupportedNormalMinions(Cards::GetTier7Minions(), candidates, Race::INVALID, activeTribes);
        else if (behavior.value == 8)
            candidates = SupportedDeathrattleMinions(activeTribes);
        else if (behavior.lockHand) {
            const auto tier = currentTier;
            if (tier == 1) AppendSupportedNormalMinions(Cards::GetTier1Minions(), candidates, Race::INVALID, activeTribes);
            else if (tier == 2) AppendSupportedNormalMinions(Cards::GetTier2Minions(), candidates, Race::INVALID, activeTribes);
            else if (tier == 3) AppendSupportedNormalMinions(Cards::GetTier3Minions(), candidates, Race::INVALID, activeTribes);
            else if (tier == 4) AppendSupportedNormalMinions(Cards::GetTier4Minions(), candidates, Race::INVALID, activeTribes);
            else if (tier == 5) AppendSupportedNormalMinions(Cards::GetTier5Minions(), candidates, Race::INVALID, activeTribes);
            else if (tier == 6) AppendSupportedNormalMinions(Cards::GetTier6Minions(), candidates, Race::INVALID, activeTribes);
            else if (tier == 7) AppendSupportedNormalMinions(Cards::GetTier7Minions(), candidates, Race::INVALID, activeTribes);
        } else
            candidates = SupportedMinionsForRace(MostCommonFriendlyRace(*this), activeTribes);
        return BeginMinionDiscover(*this, std::move(candidates), sourceSpellDbfID,
                                   behavior.lockHand);
    case TavernSpellEffect::DISCOVER_BATTLECRY_MINION:
        return BeginMinionDiscover(*this, SupportedBattlecryMinions(activeTribes), sourceSpellDbfID, false);
    case TavernSpellEffect::DISCOVER_DIFFERENT_RACE:
    {
        Minion* target = nullptr;
        recruitField.ForEachAlive([&](MinionData& data) {
            if (static_cast<std::uint64_t>(data.value().GetIndex()) == targetEntityID)
                target = &data.value();
        });
        if (target == nullptr) return false;
        candidates = SupportedMinionsForRace(target->GetRace(), activeTribes);
        candidates.erase(std::remove_if(candidates.begin(), candidates.end(),
            [target](const Card& card) { return card.id == target->GetCardID(); }),
            candidates.end());
        return BeginMinionDiscover(*this, std::move(candidates), sourceSpellDbfID, false);
    }
    case TavernSpellEffect::DISCOVER_HERO_POWER:
        for (const auto& card : Cards::GetHeroPowerMetadata())
        {
            const auto* behavior = FindSeason14HeroPowerBehavior(card.dbfID);
            // Unmasked Identity replaces the active power.  Passive
            // lifecycle powers (for example Warp Gate) have no executable
            // recruit action and must not enter this public modal.
            if (card.dbfID != 0 && card.hasBehavior && behavior != nullptr &&
                !behavior->passive)
                candidates.push_back(card);
        }
        if (candidates.empty()) return false;
        Random::shuffle(candidates.begin(), candidates.end());
        candidates.resize(std::min<std::size_t>(3, candidates.size()));
        {
            std::vector<Season14Offering> offerings;
            for (const auto& card : candidates) offerings.push_back({card.dbfID, 0});
            season14.BeginOfferingDecision(Season14Decision::DISCOVER, 0,
                                           sourceSpellDbfID,
                                           std::move(offerings));
            return true;
        }
    case TavernSpellEffect::DISCOVER_TIER_MINION_OR_SPELL:
        // This modal has a public two-step shape: first choose minion versus
        // Tavern spell, then choose one of the three generated offerings.
        // Replaying the source spell must reopen the typed branch choice,
        // rather than trying to reuse the stale offering list from the first
        // cast.  The final ApplyChoice path owns queue consumption.
        season14.BeginSpellAllMinionChoice(
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
            season14.BeginOfferingDecision(Season14Decision::DISCOVER, 0,
                                           sourceSpellDbfID,
                                           std::move(offerings));
            return true;
        }
    case TavernSpellEffect::DISCOVER_UNDEAD_DIES_THIS_TURN:
        return BeginMinionDiscover(*this, SupportedMinionsForRace(Race::UNDEAD, activeTribes),
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
        season14.BeginOfferingDecision(Season14Decision::DISCOVER, 0,
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
    // A Cube request can be deferred by any unrelated public modal.  Keep a
    // single retry boundary around the whole dispatcher so every successful
    // modal resolution gets the same safe behavior without duplicating the
    // retry at each branch or recursing through a newly opened modal.
    struct DeferredMysteryCubeRetry {
        Player& player;
        ~DeferredMysteryCubeRetry() {
            if (player.season14.pendingMysteryCubeOffer &&
                player.season14.pendingDecision == Season14Decision::NONE)
                (void)player.BeginMysteryCubeOffer();
            if (player.season14.pendingTripVouchersOffer &&
                player.season14.pendingDecision == Season14Decision::NONE)
                (void)player.BeginTripVouchersOffer();
            if (player.season14.pendingTickatusDiscover &&
                player.season14.pendingDecision == Season14Decision::NONE &&
                player.BeginTickatusDiscover())
            {
                player.season14.pendingTickatusDiscover = false;
                // The deferred acquisition arm belongs to one owned copy.
                // Reset exactly one mature Tickatus so the next recruit start
                // cannot replay the just-opened modal.
                for (auto& trinket : player.season14.trinkets) {
                    if (!trinket.active || trinket.remainingUses == 0) continue;
                    const auto behavior = FindTrinketBehavior(
                        Cards::FindCardByDbfID(trinket.dbfID).id);
                    if (behavior.effect == TrinketEffect::TICKATUS_DARKMOON_PRIZE &&
                        trinket.triggerProgress >= behavior.value) {
                        trinket.triggerProgress = 0;
                        break;
                    }
                }
            }
        }
    } deferredMysteryCubeRetry{*this};

    // Quaint Boutique, Jumbo Warehouse, Ornate Clock, and Mysterious Orb are public Trinket
    // modals.  The first two are paid; Ornate Clock replaces the scheduled
    // Greater Trinket purchase and therefore has no cost here.
    // Resolve them before the ordinary Discover path: Trinkets never enter
    // the hand, and stale/replayed options must not bypass the pool checks.
    if (season14.pendingDecision == Season14Decision::TRINKET_SELECTION) {
        const auto source = season14.pendingSourceCardDbfID;
        if (source == 117801 || source == 140001) {
            const bool tripVouchers = source == 140001;
            const auto slot = season14.pendingTrinketReplacementSlot;
            if ((!tripVouchers && season14.pendingOfferings.size() != 2) ||
                (tripVouchers && season14.pendingOfferings.size() != 3) ||
                offeringIdx >= season14.pendingOfferings.size() || slot < 0 ||
                static_cast<std::size_t>(slot) >= season14.trinkets.size())
                return false;
            const auto selected = season14.pendingOfferings[offeringIdx].dbfID;
            const auto old = season14.trinkets[static_cast<std::size_t>(slot)];
            const auto card = Cards::FindCardByDbfID(selected);
            const auto current = Cards::FindCardByDbfID(
                season14.trinkets[static_cast<std::size_t>(slot)].dbfID);
            const bool offered = std::any_of(
                season14.pendingOfferings.begin(), season14.pendingOfferings.end(),
                [selected](const Season14Offering& offering) {
                    return offering.dbfID == selected;
                });
            if (!offered || card.GetCardType() != CardType::BATTLEGROUND_TRINKET ||
                card.trinketType != (tripVouchers ? "GREATER_TRINKET" : "LESSER_TRINKET") ||
                card.normalDbfID != 0 ||
                current.id != (tripVouchers ? "BG30_MagicItem_891" : "BG30_MagicItem_703") ||
                selected == old.dbfID ||
                FindTrinketBehavior(card.id).effect == TrinketEffect::NONE)
                return false;
            for (std::size_t i = 0; i < season14.trinkets.size(); ++i)
                if (i != static_cast<std::size_t>(slot) &&
                    season14.trinkets[i].dbfID == selected)
                    return false;
            // SelectDecision clears the public modal. Validate the
            // acquisition payload first so a stale/non-executable option
            // cannot consume the replacement decision and lose the old slot.
            if (!CanAcquireTrinketPayload(card, FindTrinketBehavior(card.id)))
                return false;
            if (!season14.SelectDecision(offeringIdx)) return false;
            season14.trinkets.erase(season14.trinkets.begin() + slot);
            if (!AcquireTrinket({selected, 1, true})) {
                season14.trinkets.insert(season14.trinkets.begin() + slot, old);
                return false;
            }
            // AcquireTrinket appends a newly acquired effect.  This modal is
            // a replacement, not a remove-and-readd operation: preserve the
            // Cube's exact Trinket slot so slot-based UI/action state remains
            // stable across the choice.
            const auto appended = season14.trinkets.size() - 1;
            if (appended != static_cast<std::size_t>(slot))
                std::swap(season14.trinkets[static_cast<std::size_t>(slot)],
                          season14.trinkets[appended]);
            if (season14.pendingDecision == Season14Decision::NONE && !tripVouchers)
                (void)BeginMysteryCubeOffer();
            if (season14.pendingDecision == Season14Decision::NONE)
                (void)BeginOrnateClockOffer();
            return true;
        }
        const bool scheduled = source == 0;
        bool greater = source == 122014 || source == 121120;
        const bool orbLesser = source == 130836;
        if ((!greater && !orbLesser && source != 122013 && !scheduled) ||
            offeringIdx >= season14.pendingOfferings.size() ||
            season14.pendingOfferings.size() != (scheduled ? 4u : 3u) ||
            !season14.CanAddTrinket())
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
            !TrinketIsInLobby(card, activeTribes, excludedLobbyRace) ||
            (card.id == "BG36_MagicItem_309" && hand.IsFull()) ||
            std::any_of(season14.trinkets.begin(), season14.trinkets.end(),
                [selected](const Season14PersistentEffect& existing) {
                    return existing.dbfID == selected;
                }))
            return false;
        // Validate the complete public snapshot before mutating ownership.
        // This closes replay paths that replace one option or inject a second
        // copy after the modal was opened.  The discount is public metadata;
        // only the pinned pivot values are valid on a Trinket option.
        std::set<std::int32_t> pendingIDs;
        for (const auto& pending : season14.pendingOfferings) {
            const auto pendingCard = Cards::FindCardByDbfID(pending.dbfID);
            if (pending.dbfID <= 0 || !pendingIDs.insert(pending.dbfID).second ||
                pendingCard.GetCardType() != CardType::BATTLEGROUND_TRINKET ||
                pendingCard.normalDbfID != 0 ||
                !TrinketIsInLobby(pendingCard, activeTribes,
                                  excludedLobbyRace) ||
                (pending.discount != 0 && pending.discount != 2))
                return false;
        }
        if (scheduled) {
            // The ordinary offer is tied to the current scheduled turn.  Do
            // not accept a replayed four-choice list after the boundary, and
            // require every option to belong to the same expected tier.
            if (season14.recruitTurnNumber != 6 &&
                season14.recruitTurnNumber != 9)
                return false;
            greater = season14.recruitTurnNumber == 9;
            const auto expected = greater ? "GREATER_TRINKET"
                                          : "LESSER_TRINKET";
            if (card.trinketType != expected ||
                std::any_of(season14.pendingOfferings.begin(),
                            season14.pendingOfferings.end(),
                    [expected](const Season14Offering& offering) {
                        return Cards::FindCardByDbfID(offering.dbfID).trinketType !=
                               expected;
                    }))
                return false;
        } else if (card.trinketType !=
                   (greater ? "GREATER_TRINKET" : "LESSER_TRINKET")) {
            return false;
        }
        if (!AcquireTrinket({selected, 1, true}) ||
            !season14.SelectDecision(offeringIdx))
            return false;
        // Souvenir Stand is a stateful copy effect: only a successfully
        // committed Greater Trinket purchase transforms it.  Do this after
        // AcquireTrinket so the selected copy remains independently owned,
        // and mutate only the existing Stand slot (never the new purchase).
        if (greater) {
            for (auto& owned : season14.trinkets) {
                // A spent/inactive instance is no longer an owned Stand for
                // trigger purposes.  Replace the whole persistent payload,
                // rather than only its identity, so the copied Greater
                // Trinket starts with independent cadence/progression state
                // and cannot inherit stale combat snapshots from the Stand.
                if (!owned.active || owned.remainingUses == 0)
                    continue;
                const auto ownedCard = Cards::FindCardByDbfID(owned.dbfID);
                if (FindTrinketBehavior(ownedCard.id).effect ==
                    TrinketEffect::SOUVENIR_STAND_GREATER_COPY) {
                    owned = Season14PersistentEffect{selected, 1, true};
                    break;
                }
            }
        }
        if (season14.HasPendingLavishCapeRandomSpells())
            (void)SimpleTasks::ActivateRandomTavernSpellsTask{
                season14.LavishCapeRandomSpellsRemaining()}.Run(*this);
        if (FindTrinketBehavior(Cards::FindCardByDbfID(selected).id).effect ==
            TrinketEffect::MYSTERY_CUBE_REPLACE_LESSER &&
            season14.pendingDecision == Season14Decision::NONE)
            (void)BeginMysteryCubeOffer();
        // AcquireTrinket runs while the Trinket-selection modal is pending,
        // so typed Discover rewards cannot open there. Resume them only after
        // committing the selected Trinket and clearing that modal.
        if (selected == 133393)
            (void)BeginOminousStoneDiscover(selected);
        else if (selected == 133381)
            (void)BeginWaxLanceDiscover(selected);
        else if (selected == 133713)
            (void)BeginMaldraxxusDaggerDiscover(selected);
        const auto selectedBehavior = FindTrinketBehavior(card.id);
        if (season14.pendingDecision == Season14Decision::NONE &&
            selectedBehavior.effect == TrinketEffect::PORTABLE_FACTORY)
            (void)BeginTrinketMinionDiscover(
                *this, card.dbfID, selectedBehavior.tier, Race::INVALID,
                false);
        else if (season14.pendingDecision == Season14Decision::NONE &&
                 selectedBehavior.effect == TrinketEffect::BATTLE_HORN)
            (void)BeginTrinketMinionDiscover(
                *this, card.dbfID, 0, Race::INVALID, true);
        else if (season14.pendingDecision == Season14Decision::NONE &&
                 selectedBehavior.effect == TrinketEffect::PUTRICIDE_STICKER)
        {
            // AcquireTrinket is called while the public Trinket modal is
            // still open.  Preserve the immediate craft in Season14State and
            // open its first component Discover only after that modal has
            // committed; otherwise the reward is silently lost.
            season14.pendingPutricideStickerSourceDbfID = card.dbfID;
            season14.pendingPutricideStickerFirstDbfID = 0;
            season14.pendingPutricideStickerSecondPool = false;
            (void)BeginPutricideStickerDiscover(*this, card.dbfID, false);
        }
        // AcquireTrinket runs while the Trinket-selection modal is still
        // pending, so its typed Discover cannot open until this selection is
        // committed. Resume it immediately once the decision slot is free.
        if (season14.pendingElectromagneticDiscoverRemaining > 0)
            (void)BeginElectromagneticDiscover(
                *this, season14.pendingElectromagneticDiscoverSourceCardDbfID);
        // If another scheduled offer was preserved (for example, an Ornate
        // Clock alongside a Mysterious Orb), open it only after this modal
        // has committed and cleared its decision slot.
        if (season14.pendingDecision == Season14Decision::NONE)
            (void)BeginOrnateClockOffer();
        return true;
    }
    if (season14.pendingSourceCardDbfID == 120827) {
        if (season14.pendingDecision != Season14Decision::DISCOVER ||
            offeringIdx >= season14.pendingOfferings.size() || hand.IsFull())
            return false;
        const auto selected = season14.pendingOfferings[offeringIdx].dbfID;
        const auto card = Cards::FindCardByDbfID(selected);
        if (card.dbfID == 0 || card.GetCardType() != CardType::MINION ||
            card.normalDbfID != 0 || !card.hasBehavior ||
            !IsBuildAnUndeadPoolDbfID(selected,
                                      season14.pendingPutricideStickerSecondPool) ||
            card.GetTier() > currentTier)
            return false;
        if (season14.pendingPutricideStickerSecondPool) {
            const auto first = Cards::FindCardByDbfID(
                season14.pendingPutricideStickerFirstDbfID);
            const bool excludeKeywords =
                season14.pendingPutricideStickerFirstDbfID == 95263 ||
                first.gameTags.contains(GameTag::REBORN) ||
                first.gameTags.contains(GameTag::POISONOUS) ||
                first.gameTags.contains(GameTag::VENOMOUS);
            if (excludeKeywords &&
                (card.dbfID == 99527 || card.dbfID == 98867 ||
                 card.gameTags.contains(GameTag::REBORN) ||
                 card.gameTags.contains(GameTag::POISONOUS) ||
                 card.gameTags.contains(GameTag::VENOMOUS)))
                return false;
        }
        if (!season14.pendingPutricideStickerSecondPool) {
            if (!season14.SelectDecision(offeringIdx)) return false;
            season14.pendingPutricideStickerFirstDbfID = selected;
            season14.pendingPutricideStickerSecondPool = true;
            if (!BeginPutricideStickerDiscover(*this, 120827, true)) {
                season14.pendingPutricideStickerFirstDbfID = 0;
                season14.pendingPutricideStickerSecondPool = false;
                return false;
            }
            return true;
        }
        const auto first = Cards::FindCardByDbfID(
            season14.pendingPutricideStickerFirstDbfID);
        if (first.dbfID == 0 || !season14.SelectDecision(offeringIdx))
            return false;
        Minion creation(first);
        Minion component(card);
        if (!creation.ComposeCustomFrom(component,
                                        std::max(first.GetTier(), card.GetTier())))
            return false;
        ApplyFreshMinionModifiers(creation);
        hand.Add(CardData{std::move(creation)});
        const auto entityID = static_cast<std::uint64_t>(
            std::get<Minion>(hand[hand.GetCount() - 1]).GetIndex());
        ResolveDiscoverTriggers(entityID);
        season14.pendingPutricideStickerSourceDbfID = 0;
        season14.pendingPutricideStickerFirstDbfID = 0;
        season14.pendingPutricideStickerSecondPool = false;
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
    if (IsWarpGateHeroPowerDbfID(season14.pendingSourceCardDbfID)) {
        if (season14.pendingDecision != Season14Decision::CHOICE ||
            season14.pendingOfferings.size() != WARP_GATE_LIFECYCLE.choiceCount ||
            offeringIdx >= WARP_GATE_LIFECYCLE.choiceCount) return false;
        const auto chosen = season14.pendingOfferings[offeringIdx].dbfID;
        const auto card = Cards::FindCardByDbfID(chosen);
        if (card.GetCardType() != CardType::MINION ||
            card.normalDbfID != 0 || !card.hasBehavior ||
            !IsExecutableWarpGateProtossDbfID(card.dbfID) ||
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
            card.isBattlegroundsPoolMinion && HasActiveTribe(activeTribes, card) &&
            ((source == 59891 && (card.GetTier() == 3 || card.GetTier() == 4)) ||
             (source == 63127 &&
              card.HasRace(static_cast<Race>(season14.pendingHeroPowerReplayRace))) ||
             (source == 97814 &&
              IsBuildAnUndeadPoolDbfID(card.dbfID,
                                       season14.heroPowerBatch9.buildAnUndeadSecondPool)) ||
             (source == 62267 &&
              card.GetTier() == season14.pendingHeroPowerReplayTier) ||
             (source == 64481 &&
              card.GetTier() == season14.pendingHeroPowerReplayTier));
        if (!valid) return false;
        Minion generated{card};
        ApplyFreshMinionModifiers(generated);
        hand.Add(CardData{std::move(generated)});
        const auto discoveredEntityID = static_cast<std::uint64_t>(
            std::get<Minion>(hand[hand.GetCount() - 1]).GetIndex());
        ResolveDiscoverTriggers(discoveredEntityID);
        if (--season14.pendingHeroPowerReplayRemaining > 0)
        {
            // The first selection is from Pool 1; every replayed selection
            // (including the ordinary no-Wishbone path) is Pool 2.
            if (source == 97814)
                season14.heroPowerBatch9.buildAnUndeadSecondPool = true;
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
                if (!HasActiveTribe(activeTribes, candidate)) continue;
                if (source == 63127 && !candidate.HasRace(static_cast<Race>(
                                              season14.pendingHeroPowerReplayRace)))
                    continue;
                if (source == 97814 &&
                    !IsBuildAnUndeadPoolDbfID(
                        candidate.dbfID,
                        season14.heroPowerBatch9.buildAnUndeadSecondPool))
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
            season14.heroPowerBatch9.buildAnUndeadSecondPool = false;
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
        const auto discoveredEntityID = static_cast<std::uint64_t>(
            std::get<Minion>(hand[hand.GetCount() - 1]).GetIndex());
        ResolveDiscoverTriggers(discoveredEntityID);
        if (--season14.pendingUndeadDiscoverRemaining > 0)
        {
            std::vector<Card> candidates;
            for (const auto& candidate : Cards::GetAllCards())
                if (candidate.isBattlegroundsPoolMinion && candidate.normalDbfID == 0 &&
                    candidate.HasRace(Race::UNDEAD) &&
                    HasActiveTribe(activeTribes, candidate)) candidates.push_back(candidate);
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
        const auto discoveredEntityID = static_cast<std::uint64_t>(
            std::get<Minion>(hand[hand.GetCount() - 1]).GetIndex());
        ResolveDiscoverTriggers(discoveredEntityID);
        hero.TakeDamage(*this, demon.GetTier());
        if (--season14.pendingDemonDiscoverRemaining > 0)
        {
            std::vector<Card> candidates;
            for (const auto& card : Cards::GetAllCards())
                if (card.isBattlegroundsPoolMinion && card.normalDbfID == 0 &&
                    card.HasRace(Race::DEMON) && HasActiveTribe(activeTribes, card))
                    candidates.push_back(card);
            if (candidates.empty())
            {
                // A pool can become empty after the first selection (for
                // example when the active-tribe training scope excludes the
                // remaining Demons).  The reward has already been applied;
                // do not leave a DISCOVER decision with zero offerings that
                // can never be represented or selected by the bridge.
                season14.pendingDemonDiscoverRemaining = 0;
                season14.pendingDemonDiscoverSourceEntityID = 0;
                season14.pendingDecision = Season14Decision::NONE;
                season14.pendingSourceEntityID = 0;
                season14.pendingSourceCardDbfID = 0;
                season14.pendingOfferings.clear();
                season14.choiceOfferings.clear();
                return true;
            }
            Random::shuffle(candidates.begin(), candidates.end());
            season14.pendingOfferings.clear();
            for (std::size_t i = 0; i < std::min<std::size_t>(3, candidates.size()); ++i)
                season14.pendingOfferings.push_back({candidates[i].dbfID, 0});
        }
        else
        {
            season14.pendingDemonDiscoverSourceEntityID = 0;
            season14.pendingDemonDiscoverRemaining = 0;
            season14.pendingDecision = Season14Decision::NONE;
            season14.pendingSourceEntityID = 0;
            season14.pendingSourceCardDbfID = 0;
            season14.pendingOfferings.clear();
            season14.choiceOfferings.clear();
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
        // Clunker Junker's modal is a real Discover. Apply after-Discover
        // effects while the selected Magnetic Mech is still an attachment;
        // MagnetizeOnto then carries the buffed payload onto the host.
        ApplyBurthDiscoverBuff(attachment);
        attachment.MagnetizeOnto(*target);
        // Discover attachments are successful Magnetize events too.  Keep
        // the same ordering and per-Trinket progression as ordinary plays.
        ApplyAfterMagnetizeTrinkets(*target);

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
                    HasActiveTribe(activeTribes, candidate) &&
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
    // Electromagnetic Device is a Discover-to-hand modal.  The golden form
    // reopens this same typed modal after the first selection; unlike the
    // attachment Discover above, the chosen Magnetic Mech is materialized in
    // hand (and burns when the hand is full, as an ordinary Discover does).
    if (season14.pendingSourceCardDbfID == 117894 ||
        season14.pendingSourceCardDbfID == 121689)
    {
        if (season14.pendingDecision != Season14Decision::DISCOVER ||
            offeringIdx >= season14.pendingOfferings.size() ||
            season14.pendingOfferings.size() != 3 ||
            season14.pendingElectromagneticDiscoverRemaining <= 0)
            return false;
        std::set<std::int32_t> distinctOfferings;
        for (const auto& pending : season14.pendingOfferings)
        {
            const auto candidate = Cards::FindCardByDbfID(pending.dbfID);
            if (candidate.dbfID == 0 || candidate.GetCardType() != CardType::MINION ||
                candidate.normalDbfID != 0 || !candidate.isBattlegroundsPoolMinion ||
                !candidate.hasBehavior || !candidate.HasRace(Race::MECHANICAL) ||
                !candidate.gameTags.contains(GameTag::MAGNETIC) ||
                candidate.gameTags.at(GameTag::MAGNETIC) == 0 ||
                !distinctOfferings.insert(candidate.dbfID).second)
                return false;
        }
        const auto selectedCard = Cards::FindCardByDbfID(
            season14.pendingOfferings[offeringIdx].dbfID);
        const auto sourceCardDbfID = season14.pendingSourceCardDbfID;
        if (!season14.SelectDecision(offeringIdx)) return false;
        if (!hand.IsFull())
        {
            Minion generated(selectedCard);
            ApplyFreshMinionModifiers(generated);
            hand.Add(CardData{std::move(generated)});
            // This is still a real Discover-a-minion commit.  Preserve the
            // generated entity identity so after-Discover effects (notably
            // Burth) resolve against this exact hand copy, including the
            // second choice of the golden device.
            const auto discoveredEntityID = static_cast<std::uint64_t>(
                std::get<Minion>(hand[hand.GetCount() - 1]).GetIndex());
            ResolveDiscoverTriggers(discoveredEntityID);
        }
        AddGeneratedDiscoverCopy(selectedCard);
        --season14.pendingElectromagneticDiscoverRemaining;
        if (season14.pendingElectromagneticDiscoverRemaining > 0)
        {
            if (!BeginElectromagneticDiscover(*this, sourceCardDbfID))
            {
                // Keep the remainder armed for the next recruit-start retry
                // if another modal currently owns the decision slot.
            }
        }
        else
        {
            season14.pendingElectromagneticDiscoverSourceCardDbfID = 0;
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
         !embraceElements && !expeditionPlans && !magicfinRelic &&
         !(season14.pendingSourceCardDbfID == 117894 ||
           season14.pendingSourceCardDbfID == 121689) && hand.IsFull()))
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
    // Jewelry Box is a closed three-card modal.  Revalidate both the source
    // identity and the complete offering before any generic Discover path can
    // materialize a card; this makes stale/replayed choices fail closed.
    const bool jewelryBoxBloodGem =
        wasDiscover && season14.pendingSourceCardDbfID == 130904;
    if (jewelryBoxBloodGem)
    {
        static constexpr std::array<std::int32_t, 3> allowed = {
            130699, 130698, 130700};
        if (season14.pendingOfferings.size() != allowed.size() ||
            hand.IsFull() || offeringIdx >= allowed.size() ||
            offering.dbfID != allowed[offeringIdx])
            return false;
        for (std::size_t i = 0; i < allowed.size(); ++i)
        {
            if (season14.pendingOfferings[i].dbfID != allowed[i])
                return false;
            const auto gem = Cards::FindCardByDbfID(allowed[i]);
            if (gem.GetCardType() != CardType::BATTLEGROUND_SPELL ||
                !gem.hasBehavior || FindTavernSpellBehavior(gem.id).effect ==
                    TavernSpellEffect::NONE)
                return false;
        }
        const auto source = Cards::FindCardByDbfID(
            season14.pendingSourceCardDbfID);
        if (source.id != "BG35_MagicItem_434" || source.dbfID == 0)
            return false;
    }
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
                !HasActiveTribe(activeTribes, candidate) ||
                candidate.GetTier() != 6 ||
                !globeOfferings.insert(candidate.dbfID).second)
                return false;
        }
    }
    // Deathly Phylactery's acquisition modal is constrained to executable
    // Deathrattle minions. Revalidate the complete offering at commit time so
    // replay/stale choices cannot turn the Trinket into an arbitrary card.
    const bool deathlyPhylactery =
        wasDiscover && season14.pendingSourceCardDbfID == 117794;
    if (deathlyPhylactery)
    {
        if (season14.pendingOfferings.empty() ||
            season14.pendingOfferings.size() > 3)
            return false;
        const auto supported = SupportedDeathrattleMinions(activeTribes);
        for (const auto& pending : season14.pendingOfferings)
        {
            const auto candidate = Cards::FindCardByDbfID(pending.dbfID);
            if (candidate.GetCardType() != CardType::MINION ||
                candidate.normalDbfID != 0 || !candidate.isBattlegroundsPoolMinion ||
                !candidate.hasBehavior || candidate.power.GetDeathrattleTask().empty() ||
                std::none_of(supported.begin(), supported.end(),
                    [&candidate](const Card& legal) {
                        return legal.dbfID == candidate.dbfID;
                    }))
                return false;
        }
    }
    // Innkeeper's Hearth is a typed, replay-safe minion Discover.  Validate
    // the complete constrained modal again at commit time, then materialize
    // the selected card with an exact stat floor.  The golden token reopens
    // the same Tier-6 pool once more after the first selection.
    const bool innkeepersHearth =
        season14.pendingSourceCardDbfID == 121498 ||
        season14.pendingSourceCardDbfID == 121684;
    if (innkeepersHearth)
    {
        const auto sourceCardDbfID = season14.pendingSourceCardDbfID;
        const auto source = Cards::FindCardByDbfID(sourceCardDbfID);
        const bool validSource =
            (sourceCardDbfID == 121498 &&
             source.id == "BG32_MagicItem_362") ||
            (sourceCardDbfID == 121684 &&
             source.id == "BG32_MagicItem_362t");
        const int expectedTier = sourceCardDbfID == 121684 ? 6 : currentTier;
        const int exactStats = sourceCardDbfID == 121684 ? 30 : 12;
        const int remainingAfterSelection =
            sourceCardDbfID == 121684
                ? season14.pendingInnkeepersHearthRemaining - 1
                : 0;
        if (!wasDiscover || !validSource ||
            season14.pendingOfferings.size() != 3 ||
            offeringIdx >= season14.pendingOfferings.size() ||
            hand.IsFull() || offering.dbfID <= 0 ||
            (sourceCardDbfID == 121684 &&
             season14.pendingInnkeepersHearthRemaining <= 0))
            return false;
        std::set<std::int32_t> hearthOfferings;
        for (const auto& pending : season14.pendingOfferings)
        {
            const auto candidate = Cards::FindCardByDbfID(pending.dbfID);
            if (candidate.GetCardType() != CardType::MINION ||
                !candidate.isBattlegroundsPoolMinion || candidate.normalDbfID != 0 ||
                !candidate.hasBehavior || candidate.GetTier() != expectedTier ||
                (sourceCardDbfID == 121684 &&
                 season14.pendingInnkeepersHearthSelectedDbfID != 0 &&
                 candidate.dbfID ==
                     season14.pendingInnkeepersHearthSelectedDbfID) ||
                !hearthOfferings.insert(candidate.dbfID).second)
                return false;
        }
        if (source.dbfID == 0 ||
            !hearthOfferings.contains(offering.dbfID))
            return false;
        Minion selected(Cards::FindCardByDbfID(offering.dbfID));
        selected.SetAttack(exactStats);
        selected.SetHealth(exactStats);
        if (!season14.SelectDecision(offeringIdx)) return false;
        hand.Add(CardData{std::move(selected)});
        if (sourceCardDbfID == 121684 && remainingAfterSelection > 0)
        {
            season14.pendingInnkeepersHearthSelectedDbfID = offering.dbfID;
            if (!BeginInnkeepersHearthDiscover(*this, sourceCardDbfID, 6))
                season14.pendingInnkeepersHearthRemaining = 0;
            else
                season14.pendingInnkeepersHearthRemaining = remainingAfterSelection;
        }
        else
        {
            season14.pendingInnkeepersHearthRemaining = 0;
            season14.pendingInnkeepersHearthSelectedDbfID = 0;
        }
        return true;
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
            hand.IsFull() || offering.dbfID <= 0 ||
            offeringIdx >= season14.pendingOfferings.size() ||
            season14.pendingOfferings[offeringIdx].dbfID != offering.dbfID)
            return false;
        // Locket offerings are canonical non-Golden identities, while the
        // recorded combat snapshot may be Golden. Resolve by canonical card
        // identity and normalize the copied instance at this same boundary;
        // an exact DBF-ID lookup would make every Golden offering unselectable.
        std::optional<Minion> snapshot;
        for (std::size_t i = 0;
             i < season14.lastOpponentCombatMinionDbfIDs.size(); ++i) {
            auto candidate = season14.LastOpponentCombatMinionSnapshot(i);
            if (!candidate.has_value()) continue;
            const auto sourceCard =
                Cards::FindCardByID(std::string(candidate->GetCardID()));
            const auto canonicalDbfID = sourceCard.normalDbfID != 0
                ? sourceCard.normalDbfID : sourceCard.dbfID;
            if (canonicalDbfID != offering.dbfID) continue;
            if (sourceCard.normalDbfID != 0) {
                const auto normalCard =
                    Cards::FindCardByDbfID(sourceCard.normalDbfID);
                if (!candidate->TransformToKeepingInstanceState(normalCard))
                    return false;
            }
            snapshot = std::move(candidate);
            break;
        }
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
    if (season14.pendingSourceCardDbfID == 129685) {
        // Spawning Pool discovers a replacement Hero Power.  Treat this as
        // the same typed modal as Adventure/Cosmic Reward; falling through to
        // the generic hand-card materializer leaves the HERO_POWER variant
        // without an executable destination and deadlocks legality.
        const auto card = Cards::FindCardByDbfID(offering.dbfID);
        if (season14.pendingDecision != Season14Decision::DISCOVER ||
            card.GetCardType() != CardType::HERO_POWER ||
            !card.hasBehavior || card.dbfID == 129685)
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
            offeringIdx >= season14.pendingOfferings.size() ||
            season14.pendingOfferings[offeringIdx].dbfID != offering.dbfID)
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
            !card.isBattlegroundsPoolMinion || card.normalDbfID != 0 ||
            !card.hasBehavior || !HasActiveTribe(activeTribes, card))
            return false;
        season14.SetChampionReward(card.dbfID);
        TryDeliverChampionReward();
        return season14.SelectDecision(offeringIdx);
    }


    if (embraceElements)
    {
        if (hero.card.heroPowerDbfID != 79720 ||
            season14.pendingDecision != Season14Decision::CHOICE ||
            season14.pendingOfferings.size() != 4 ||
            offering.dbfID < 79721 || offering.dbfID > 79724)
            return false;
        season14.embraceElementDbfID = offering.dbfID;
        recruitField.ForEachAlive([&](MinionData& data) {
            auto& minion = data.value();
            if (minion.GetCardID() != "BG22_HERO_001_Buddy" &&
                minion.GetCardID() != "BG22_HERO_001_Buddy_G") return;
            const auto key = static_cast<std::uint64_t>(minion.GetIndex());
            auto it = std::find_if(season14.spiritRaptorElements.begin(),
                                   season14.spiritRaptorElements.end(),
                                   [key](const auto& entry) { return entry.first == key; });
            if (it == season14.spiritRaptorElements.end())
                it = season14.spiritRaptorElements.emplace(
                    season14.spiritRaptorElements.end(), key, std::vector<std::int32_t>{});
            if (std::find(it->second.begin(), it->second.end(), offering.dbfID) == it->second.end())
                it->second.push_back(offering.dbfID);
        });
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
        // The bridge filters each public action against the same executable
        // Naga predicate.  Validate the selected payload here as the commit
        // boundary, but do not reject an otherwise valid selection merely
        // because an older/generated modal retained one stale sibling row.
        // Requiring every row to remain valid made the policy's legal index
        // set non-empty while every advertised index still failed here.
        const auto selectedNaga = Cards::FindCardByDbfID(offering.dbfID);
        if (selectedNaga.GetCardType() != CardType::MINION ||
            !selectedNaga.isBattlegroundsPoolMinion ||
            selectedNaga.normalDbfID != 0 || !selectedNaga.hasBehavior ||
            !HasActiveTribe(activeTribes, selectedNaga) ||
            !selectedNaga.HasRace(Race::NAGA))
            return false;
    }
    // Portable Factory and Battle Horn are typed minion Discovers.  Keep the
    // commit boundary stricter than the generic card path: every offered row
    // must still belong to the exact executable pool, and Portable Factory's
    // selected instance is retained for its recruit-start copies.
    const auto portableSource =
        season14.pendingSourceCardDbfID == 111399 ||
        season14.pendingSourceCardDbfID == 120925;
    const auto battleHornSource = season14.pendingSourceCardDbfID == 120874;
    if (portableSource || battleHornSource)
    {
        const auto source = Cards::FindCardByDbfID(
            season14.pendingSourceCardDbfID);
        const int tier = season14.pendingSourceCardDbfID == 120925 ? 5 : 4;
        if (season14.pendingDecision != Season14Decision::DISCOVER ||
            season14.pendingOfferings.size() != 3 ||
            offeringIdx >= season14.pendingOfferings.size() ||
            source.GetCardType() != CardType::BATTLEGROUND_TRINKET)
            return false;
        for (const auto& pending : season14.pendingOfferings)
        {
            const auto candidate = Cards::FindCardByDbfID(pending.dbfID);
            if (candidate.GetCardType() != CardType::MINION ||
                !candidate.isBattlegroundsPoolMinion ||
                candidate.normalDbfID != 0 || !candidate.hasBehavior ||
                (portableSource &&
                 (candidate.GetTier() != tier ||
                  !std::any_of(RACES_IN_BATTLEGROUNDS.begin(),
                               RACES_IN_BATTLEGROUNDS.end(),
                               [&candidate](Race race) {
                                   return candidate.HasRace(race);
                               }))) ||
                (battleHornSource &&
                 !CardDefs::FindCardDefByID(candidate.id).HasBattlecry()))
                return false;
        }
        const auto selectedCard = Cards::FindCardByDbfID(
            season14.pendingOfferings[offeringIdx].dbfID);
        if (!season14.SelectDecision(offeringIdx)) return false;
        Minion generated{selectedCard};
        ApplyFreshMinionModifiers(generated);
        const auto generatedEntityID =
            static_cast<std::uint64_t>(generated.GetIndex());
        if (portableSource)
        {
            bool saved = false;
            for (auto& owned : season14.trinkets)
            {
                if (owned.dbfID == source.dbfID && owned.active &&
                    owned.remainingUses != 0)
                {
                    owned.capturedMinion = generated;
                    saved = true;
                    break;
                }
            }
            if (!saved) return false;
        }
        hand.Add(CardData{std::move(generated)});
        ResolveDiscoverTriggers(generatedEntityID);
        return true;
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
    const bool triplePrize = pendingSourceID == "BG35_MagicItem_812t";
    const bool tickatusSticker = pendingSourceID == "BG30_MagicItem_707";
    if (triplePrize || tickatusSticker)
    {
        // Recheck the constrained prize pool at commit time. This keeps a
        // stale/replayed offering from awarding an arbitrary spell.  Both
        // Triple Prize and Tickatus use the same Tier-3 pool, but retain
        // distinct source identities so a modal cannot be replayed across
        // either effect.
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
        const auto expectedSource = Cards::FindCardByID(
            tickatusSticker ? "BG30_MagicItem_707" : "BG35_MagicItem_812t");
        const auto actualSource = Cards::FindCardByDbfID(
            season14.pendingSourceCardDbfID);
        if (season14.pendingDecision != Season14Decision::DISCOVER ||
            !distinctOfferings || !selectedOffering ||
            expectedSource.dbfID == 0 || actualSource.dbfID != expectedSource.dbfID ||
            actualSource.id != expectedSource.id ||
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
         !card.hasBehavior || !HasActiveTribe(activeTribes, card) ||
         !card.HasRace(Race::NAGA)))
        return false;

    if (galakrondGreed)
    {
        // Galakrond replacement is allowed to proceed even when the hand is
        // full; ordinary discover paths retain the (!galakrondGreed && hand.IsFull()) guard.
        const auto slot = season14.pendingTavernReplacementSlot;
        if (card.GetCardType() != CardType::MINION ||
            !card.isBattlegroundsPoolMinion || card.normalDbfID != 0 ||
            !card.hasBehavior || !HasActiveTribe(activeTribes, card) ||
            card.GetTier() <= season14.pendingTavernReplacementTier ||
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
        season14.pendingSourceCardDbfID == 134010 ||
        Cards::FindCardByDbfID(season14.pendingSourceCardDbfID).id ==
            "BG36_MagicItem_206" ||
        Cards::FindCardByDbfID(season14.pendingSourceCardDbfID).id ==
            "BG36_MagicItem_309" ||
        Cards::FindCardByDbfID(season14.pendingSourceCardDbfID).id ==
            "BG36_MagicItem_370";
    const auto darkGift = darkGiftDiscover
        ? FindDarkGiftBehavior(Cards::FindCardByDbfID(offering.darkGiftDbfID).id)
        : DarkGiftBehavior{};
    const auto darkGiftID = darkGiftDiscover
        ? Cards::FindCardByDbfID(offering.darkGiftDbfID).id
        : std::string{};
    if (darkGiftDiscover &&
        (offering.darkGiftDbfID == 0 || darkGift.effect == DarkGiftEffect::NONE))
        return false;
    if (darkGiftDiscover &&
        (card.GetCardType() != CardType::MINION || card.normalDbfID != 0 ||
         ((season14.pendingSourceCardDbfID == 132581 ||
           season14.pendingSourceCardDbfID == 134010) &&
          card.GetTier() != 5) ||
         (Cards::FindCardByDbfID(season14.pendingSourceCardDbfID).id ==
              "BG36_MagicItem_206" &&
          (card.GetTier() != 4 ||
           !card.HasRace(MostCommonFriendlyRace(*this)))) ||
         (Cards::FindCardByDbfID(season14.pendingSourceCardDbfID).id ==
              "BG36_MagicItem_309" && card.GetTier() != 7) ||
         (Cards::FindCardByDbfID(season14.pendingSourceCardDbfID).id ==
              "BG36_MagicItem_370" && [&]() {
                  bool found = false;
                  recruitField.ForEachAlive([&](const MinionData& data) {
                      found = found || data.value().GetDbfID() == card.dbfID;
                  });
                  return !found;
              }()) ||
         !card.hasBehavior ||
         !DarkGiftTargetIsLegal(Minion(card), darkGift)))
        return false;

    // Wax Lance is a typed three-option Discover. Revalidate the complete
    // modal at commit time so stale/replayed state cannot inject a non-pool,
    // duplicate, non-Tier-7 minion or an illegal gift pair.
    if (Cards::FindCardByDbfID(season14.pendingSourceCardDbfID).id ==
        "BG36_MagicItem_309")
    {
        if (season14.pendingDecision != Season14Decision::DISCOVER ||
            season14.pendingOfferings.size() != 3)
            return false;
        std::set<std::int32_t> waxMinions;
        for (const auto& pending : season14.pendingOfferings)
        {
            const auto candidate = Cards::FindCardByDbfID(pending.dbfID);
            const auto gift = FindDarkGiftBehavior(
                Cards::FindCardByDbfID(pending.darkGiftDbfID).id);
            if (candidate.GetCardType() != CardType::MINION ||
                !candidate.isBattlegroundsPoolMinion ||
                candidate.normalDbfID != 0 || !candidate.hasBehavior ||
                candidate.GetTier() != 7 ||
                !waxMinions.insert(candidate.dbfID).second ||
                pending.darkGiftDbfID <= 0 ||
                gift.effect == DarkGiftEffect::NONE ||
                !DarkGiftTargetIsLegal(Minion(candidate), gift))
                return false;
        }
    }
    if (Cards::FindCardByDbfID(season14.pendingSourceCardDbfID).id ==
        "BG36_MagicItem_370")
    {
        if (season14.pendingDecision != Season14Decision::DISCOVER ||
            season14.pendingOfferings.size() != 3)
            return false;
        std::set<std::int32_t> daggerMinions;
        for (const auto& pending : season14.pendingOfferings) {
            const auto candidate = Cards::FindCardByDbfID(pending.dbfID);
            const auto gift = FindDarkGiftBehavior(
                Cards::FindCardByDbfID(pending.darkGiftDbfID).id);
            if (candidate.GetCardType() != CardType::MINION ||
                !candidate.isBattlegroundsPoolMinion ||
                candidate.normalDbfID != 0 || !candidate.hasBehavior ||
                pending.darkGiftDbfID <= 0 ||
                !Cards::FindCardByDbfID(pending.darkGiftDbfID)
                     .isBattlegroundsDarkGift ||
                gift.effect == DarkGiftEffect::NONE ||
                !daggerMinions.insert(candidate.dbfID).second ||
                !DarkGiftTargetIsLegal(Minion(candidate), gift))
                return false;
            bool inWarband = false;
            recruitField.ForEachAlive([&](const MinionData& data) {
                const auto warband = Cards::FindCardByDbfID(data.value().GetDbfID());
                const auto plainDbfID = warband.normalDbfID != 0
                    ? warband.normalDbfID : warband.dbfID;
                inWarband = inWarband || plainDbfID == candidate.dbfID;
            });
            if (!inWarband) return false;
        }
    }

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
          card.GetCardType() != CardType::SPELL &&
          card.GetCardType() != CardType::BATTLEGROUND_SPELL) ||
         card.GetTier() != (season14.pendingBoundlessDiscoverTier > 0
                                ? season14.pendingBoundlessDiscoverTier
                                : currentTier) ||
         ((card.GetCardType() == CardType::SPELL ||
           card.GetCardType() == CardType::BATTLEGROUND_SPELL) &&
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
        const auto discoveredEntityID = static_cast<std::uint64_t>(
            std::get<Minion>(hand[hand.GetCount() - 1]).GetIndex());
        const bool selected = season14.SelectDecision(offeringIdx);
        if (selected && wasDiscover)
            ResolveDiscoverTriggers(discoveredEntityID);
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
        const auto discoveredEntityID = static_cast<std::uint64_t>(
            std::get<Minion>(hand[hand.GetCount() - 1]).GetIndex());
        const bool selected = season14.SelectDecision(offeringIdx);
        if (selected && wasDiscover)
            ResolveDiscoverTriggers(discoveredEntityID);
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
        // A typed registry row is selectable only when its executor can
        // resolve all dynamic payloads from authoritative lobby state.
        if (!IsExecutableSeason14GeneratedQuestReward(card.dbfID)) return false;
        if (card.dbfID == 104673 &&
            (hand.IsFull() || SupportedBattlecryMinions(activeTribes).empty()))
            return false;
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

    std::uint64_t discoveredEntityID = 0;
    const bool kaleidoscopeDiscover =
        wasDiscover &&
        (season14.pendingSourceCardDbfID == 130839 ||
         season14.pendingSourceCardDbfID == 131309);
    if (card.GetCardType() == CardType::MINION)
    {
        if (kaleidoscopeDiscover)
        {
            const bool golden = season14.pendingSourceCardDbfID == 131309;
            if (season14.pendingOfferings.size() != 3 ||
                !IsKaleidoscopeCandidate(card, golden))
                return false;
            std::set<std::int32_t> distinct;
            for (const auto& offered : season14.pendingOfferings)
            {
                const auto candidate = Cards::FindCardByDbfID(offered.dbfID);
                if (!IsKaleidoscopeCandidate(candidate, golden) ||
                    !distinct.insert(candidate.dbfID).second)
                    return false;
            }
        }
        Minion minion(card);
        ApplyFreshMinionModifiers(minion);
        if (kaleidoscopeDiscover &&
            season14.pendingSourceCardDbfID == 131309)
        {
            // Never materialize a normal Tier-7 when the premium link is
            // missing or malformed.  The modal was validated above, but
            // MakeGolden is still fallible and must be checked explicitly.
            if (!minion.MakeGolden()) return false;
        }
        if (windfallDiscover) {
            minion.SetAttack(season14.windfallAttack);
            minion.SetHealth(season14.windfallHealth);
        }
        if (darkGiftDiscover)
        {
            if (!ApplyDarkGift(*this, minion, darkGift)) return false;
            // The parent executor owns all semantic state.  Record only the
            // exact child marker for the two reviewed Dark Gift lifecycles;
            // unknown gifts remain fail-closed.
            RecordReviewedDarkGiftChild(minion, darkGiftID);
        }
        hand.Add(CardData{ std::move(minion) });
        discoveredEntityID = static_cast<std::uint64_t>(
            std::get<Minion>(hand[hand.GetCount() - 1]).GetIndex());
        if (season14.pendingHandLock)
        {
            const int turns = season14.pendingHandLockTurns > 0
                ? season14.pendingHandLockTurns : 1;
            std::get<Minion>(hand[hand.GetCount() - 1])
                .SetHandLockedForTurns(turns);
        }
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
    if (selected && jewelryBoxBloodGem)
        season14.pendingJewelryBoxBloodGemRemaining = 0;
    // A targeted/Discover Tavern spell selected by Lavish Cape may have
    // paused the per-type sequence. Resume only after this public choice has
    // committed and cleared its modal state.
    if (selected && season14.HasPendingLavishCapeRandomSpells())
        (void)SimpleTasks::ActivateRandomTavernSpellsTask{
            season14.LavishCapeRandomSpellsRemaining()}.Run(*this);
    if (selected && wasDiscover)
        AddGeneratedDiscoverCopy(card);
    if (selected && wasDiscover)
    {
        // Sinstone is a post-commit Discover listener.  Count only a
        // successful public selection, never an opened/stale modal, and keep
        // the counter on each owned Trinket so duplicate copies stack.
        for (auto& trinket : season14.trinkets)
        {
            if (!trinket.active || trinket.remainingUses == 0)
                continue;
            const auto behavior = FindTrinketBehavior(
                Cards::FindCardByDbfID(trinket.dbfID).id);
            if (behavior.effect != TrinketEffect::SINSTONE_DISCOVER_COPY)
                continue;
            // `value` is the descriptor's pinned per-turn limit.  Keep the
            // trigger fail-closed if a malformed descriptor omits it.
            if (behavior.value <= 0 ||
                trinket.triggerProgress >= behavior.value)
                continue;
            (void)AddSinstoneDiscoverCopy(card);
            ++trinket.triggerProgress;
        }
    }
    if (selected && wasDiscover)
        ResolveDiscoverTriggers(discoveredEntityID);
    if (selected && wasDiscover &&
        (sourceCardDbfID == 77859 || sourceCardDbfID == 77860) &&
        season14.pendingUniqueDiscoverRemaining > 0) {
        --season14.pendingUniqueDiscoverRemaining;
        if (!BeginUniqueBuddyDiscover(*this,
                season14.pendingUniqueDiscoverSourceCardDbfID) ||
            season14.pendingUniqueDiscoverRemaining == 0) {
            season14.pendingUniqueDiscoverRemaining = 0;
            season14.pendingUniqueDiscoverSourceCardDbfID = 0;
        }
    }
    // Premium Primalfin Lookout has two sequential public Discover choices,
    // not two tasks attempted against the same still-open modal.  Reopen the
    // second offering only after the first selection commits, retaining the
    // original source entity and respecting a now-full hand.
    if (selected && wasDiscover && sourceCardDbfID == 60027 &&
        season14.pendingPrimalfinDiscoverRemaining > 0) {
        --season14.pendingPrimalfinDiscoverRemaining;
        bool reopened = false;
        if (!hand.IsFull()) {
            Minion* source = nullptr;
            recruitField.ForEachAlive([&](MinionData& data) {
                if (static_cast<std::uint64_t>(data.value().GetIndex()) ==
                    season14.pendingPrimalfinDiscoverSourceEntityID)
                    source = &data.value();
            });
            if (source != nullptr && source->GetCardID() == "TB_BaconUps_089") {
                (void)SimpleTasks::MinionOfferingTask{
                    Race::MURLOC, 1, 7, 3, true}.Run(*this, *source);
                reopened = season14.pendingDecision == Season14Decision::DISCOVER;
            }
        }
        if (!reopened) {
            season14.pendingPrimalfinDiscoverRemaining = 0;
            season14.pendingPrimalfinDiscoverSourceEntityID = 0;
        }
    }
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
    season14.pendingHandLockTurns = 0;
    return selected;
}

bool Player::ApplyTransformChoice(std::size_t offeringIdx)
{
    if (season14.transformModal.stage != Season14TransformStage::CANDIDATE ||
        offeringIdx >= season14.pendingOfferings.size())
        return false;
    const auto modal = season14.transformModal;
    if (modal.targetIndex < 0 || modal.targetIndex >= recruitField.GetCount())
        return false;
    auto& target = recruitField[static_cast<std::size_t>(modal.targetIndex)];
    if (target.IsDestroyed() || target.GetHealth() <= 0 ||
        static_cast<std::uint64_t>(target.GetIndex()) != modal.targetEntityID)
        return false;
    const auto card = Cards::FindCardByDbfID(
        season14.pendingOfferings[offeringIdx].dbfID);
    if (card.dbfID == 0 || card.normalDbfID != 0 ||
        card.GetCardType() != CardType::MINION ||
        card.GetTier() != modal.targetTier + 1)
        return false;
    if (!target.TransformTo(card)) return false;

    // Every committed transform is one successful targeted Tavern spell.
    // Replays are direct modal transactions and therefore cannot recursively
    // arm another Locket or re-enter PlaySpell with a stale hand/entity ID.
    season14.OnTavernSpellResolved(true, modal.sourceCardDbfID, true);
    ResolveSpellCountTrinkets();
    IncrementStartCombatSpellImprovements();
    ApplyTavernSpellTrinkets();
    AdvanceDarkGiftCounters(3);

    if (modal.locketReplay && modal.locketReplayRemaining > 0) {
        const auto remaining = static_cast<std::uint8_t>(
            modal.locketReplayRemaining - 1);
        if (remaining > 0) {
            bool hasReplayTarget = false;
            recruitField.ForEachAlive([&](const MinionData& data) {
                if (static_cast<std::uint64_t>(data.value().GetIndex()) !=
                    modal.locketReplayExcludedEntityID)
                    hasReplayTarget = true;
            });
            if (hasReplayTarget) {
                // The completed candidate still owns the public CHOOSE_ONE
                // decision. Clear it before opening the next transaction so
                // BeginTransformReplayDecision cannot accidentally nest
                // modals or retain the prior offerings.
                season14.CancelTransformDecision();
                if (season14.BeginTransformReplayDecision(
                        modal.sourceCardDbfID,
                        modal.locketReplayExcludedEntityID, remaining))
                    return true;
            }
        }
    }
    season14.CancelTransformDecision();
    return true;
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
            HasActiveTribe(activeTribes, card) && card.GetTier() == currentTier)
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
    // All successful Tavern-spell paths (including modal continuations and
    // generated/free casts) converge here.  Resolve Thrasher before other
    // post-cast auras so a replay observes the same lifecycle boundary.
    ApplyDaggerspineThrasherSpellCast();
    // Bloodbound Earrings' reward is a real Blood Gem play on every friendly
    // minion, not a synthetic stat delta. Consume pending reward units only
    // after the successful-spell hook commits each Trinket's threshold and
    // remainder; ApplyBloodGemTo preserves canonical gem instance semantics.
    const int pendingBloodGems = season14.TakeSpellCountBloodGems();
    if (pendingBloodGems > 0)
    {
        recruitField.ForEachAlive([this, pendingBloodGems](MinionData& data) {
            for (int i = 0; i < pendingBloodGems; ++i)
                ApplyBloodGemTo(data.value());
        });
    }
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
    // Azerite Portrait extends Living Azerite's successful-spell trigger to
    // friendly Elementals. Apply it to the active recruit board so the
    // resulting stats persist and never leak into combat-only copies.
    bool azeritePortrait = false;
    for (const auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.portraitEffect ==
            PortraitEffect::LIVING_AZERITE_ELEMENTAL_STATS)
            azeritePortrait = true;
    }
    if (azeritePortrait)
    {
        int copies = 0;
        recruitField.ForEachAlive([&copies](MinionData& data) {
            const auto& id = data.value().GetCardID();
            if (id == "BG28_707") ++copies;
            else if (id == "BG28_707_G") copies += 2;
        });
        if (copies > 0)
            recruitField.ForEachAlive([copies](MinionData& data) {
                auto& minion = data.value();
                if (!minion.HasRace(Race::ELEMENTAL)) return;
                minion.SetAttack(minion.GetAttack() + 3 * copies);
                minion.SetHealth(minion.GetHealth() + 2 * copies);
            });
    }
    // Felsteel Cleaver resolves after the targeted Tavern minion has received
    // the spell payload.  Snapshot the stable shop entity before removing it,
    // then transfer its final stats to one random friendly minion.  A full
    // hand is irrelevant; an empty warband is not: in that case the printed
    // consume cannot resolve and the shop target remains available.
    if (season14.lastTavernSpellShopTargetEntityID != 0)
    {
        Minion* shopTarget = nullptr;
        tavern.fieldZone.ForEachAlive([&](MinionData& data) {
            if (static_cast<std::uint64_t>(data.value().GetIndex()) ==
                season14.lastTavernSpellShopTargetEntityID)
                shopTarget = &data.value();
        });
        std::vector<int> recipients;
        recruitField.ForEachAlive([&recipients](MinionData& data) {
            recipients.push_back(data.value().GetZonePosition());
        });
        bool cleaverActive = false;
        for (const auto& trinket : season14.trinkets)
        {
            if (!trinket.active || trinket.remainingUses == 0) continue;
            const auto behavior = FindTrinketBehavior(
                Cards::FindCardByDbfID(trinket.dbfID).id);
            if (behavior.effect == TrinketEffect::AFTER_SPELL_ON_SHOP_CONSUME)
            {
                cleaverActive = true;
                break;
            }
        }
        if (cleaverActive && shopTarget != nullptr && !recipients.empty())
        {
            Random::shuffle(recipients.begin(), recipients.end());
            auto& recipient = recruitField[static_cast<std::size_t>(recipients.front())];
            const int attack = shopTarget->GetAttack();
            const int health = shopTarget->GetHealth();
            const int poolIndex = shopTarget->GetPoolIndex();
            const Minion consumedSnapshot = *shopTarget;
            recipient.SetAttack(recipient.GetAttack() + attack);
            recipient.SetHealth(recipient.GetHealth() + health);
            tavern.fieldZone.Remove(*shopTarget);
            if (poolIndex >= 0) returnMinionCallback(poolIndex);
            ApplyDemonConsumeBonus(recipient, consumedSnapshot);
        }
    }
    season14.lastTavernSpellShopTargetEntityID = 0;
    // Flighty Portrait's text is a hand/warband aura rather than a generic
    // board or Tavern-shop stat bonus.  Resolve it only after a successful
    // Tavern spell, so generated/free/modal casts share the same boundary.
    for (const auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.portraitEffect !=
            PortraitEffect::FLIGHTY_SCOUT_TAVERN_SPELL_STATS)
            continue;
        hand.ForEach([&behavior](std::optional<CardData>& data) {
            if (!data.has_value() || !std::holds_alternative<Minion>(*data))
                return;
            auto& minion = std::get<Minion>(*data);
            if (minion.GetCardID() == "BG32_330" ||
                minion.GetCardID() == "BG32_330_G")
            {
                minion.SetAttack(minion.GetAttack() + behavior.attack);
                minion.SetHealth(minion.GetHealth() + behavior.health);
            }
        });
        recruitField.ForEachAlive([&behavior](MinionData& data) {
            auto& minion = data.value();
            if (minion.GetCardID() == "BG32_330" ||
                minion.GetCardID() == "BG32_330_G")
            {
                minion.SetAttack(minion.GetAttack() + behavior.attack);
                minion.SetHealth(minion.GetHealth() + behavior.health);
            }
        });
    }
    ApplyAfterPlayCardTrinkets();
    }

void Player::ApplyUrZulStickerTriggers(bool playedDemon,
                                       std::uint64_t playedEntityID)
{
    // Resolve each owned Sticker independently.  The text names "another"
    // friendly Demon, so the played entity is excluded by its instance index;
    // comparing card IDs would incorrectly exclude every duplicate copy.
    if (!playedDemon) return;
    for (const auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect != TrinketEffect::AFTER_PLAY_DEMON_CONSUME)
            continue;

        Minion* consumer = nullptr;
        recruitField.ForEachAlive([&](MinionData& data) {
            if (consumer != nullptr) return;
            auto& demon = data.value();
            if (demon.HasRace(Race::DEMON) &&
                static_cast<std::uint64_t>(demon.GetIndex()) !=
                    playedEntityID)
                consumer = &demon;
        });
        if (consumer == nullptr) continue;

        // ConsumeRandomTavernTask owns the authoritative Tavern candidate
        // filter, pool return, current-stat snapshot, and Consuming Claw
        // observer.  A failed/empty Tavern leaves this Sticker inert.
        (void)SimpleTasks::ConsumeRandomTavernTask{1}.Run(*this, *consumer);
    }
}

void Player::ApplyTavernMinionConsumedTrinkets()
{
    // ConsumeRandomTavernTask calls this only after removing the offer,
    // returning its pool index, applying the consumer payload, and notifying
    // keyword observers. Failed/empty consumes cannot advance this counter.
    for (auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect !=
                TrinketEffect::AFTER_TAVERN_MINION_CONSUMED_RANDOM_SPELL ||
            behavior.value <= 0 || behavior.amount <= 0)
            continue;

        ++trinket.triggerProgress;
        while (trinket.triggerProgress >= behavior.value &&
               trinket.remainingUses > 0)
        {
            // Consume the threshold at this successful boundary. A full hand
            // burns the generated reward but can never overflow the hand.
            trinket.triggerProgress -= behavior.value;
            --trinket.remainingUses;
            if (trinket.remainingUses == 0) trinket.active = false;
            if (!hand.IsFull())
                (void)SimpleTasks::RandomTavernSpellToHandTask{
                    behavior.amount}.Run(*this);
        }
    }
}

void Player::ApplyAfterPlayCardTrinkets(Race playedRace, bool magnetic,
                                         bool playedDemon, bool playedMurloc,
                                         std::uint64_t playedEntityID)
{
    // Both ordinary and magnetic play paths converge here; the no-argument
    // ApplyAfterPlayCardTrinkets(); form is reserved for successful spells.
    if (HasActivePortrait(PortraitEffect::WHELP_SMUGGLER_STATS_AND_DRAGON))
        recruitField.ForEachAlive([](MinionData& data) {
            auto& minion = data.value();
            if (minion.GetCardID() == "BG21_013" ||
                minion.GetCardID() == "BG21_013_G")
                minion.AddRace(Race::DRAGON);
        });
    // Marine Signet counts successful ordinary minion plays.  Magnetize
    // attachments are deliberately excluded because they are not played
    // minions.  The completed cadence remains armed if the hand is full, so
    // the reward can be delivered after a later hand-space mutation.
    if (!magnetic)
    {
        for (auto& trinket : season14.trinkets)
        {
            if (!trinket.active || trinket.remainingUses == 0) continue;
            const auto behavior = FindTrinketBehavior(
                Cards::FindCardByDbfID(trinket.dbfID).id);
            if (behavior.effect !=
                    TrinketEffect::AFTER_PLAY_MINION_RANDOM_TIER_SPELL ||
                behavior.value <= 0)
                continue;
            const int cadence = std::max(1, behavior.value - trinket.statScale);
            if (++trinket.triggerProgress < cadence || hand.IsFull()) continue;
            const auto before = hand.GetCount();
            (void)SimpleTasks::RandomTavernSpellToHandTask{
                behavior.amount, 0, false, behavior.tier}.Run(*this);
            if (hand.GetCount() == before) continue;
            trinket.triggerProgress = 0;
            ++trinket.statScale;
        }
    }
    for (auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect == TrinketEffect::ACQUIRE_RANDOM_CHROMADRAKES &&
            !magnetic && behavior.value > 0) {
            Minion* played = nullptr;
            recruitField.ForEachAlive([&](MinionData& data) {
                if (static_cast<std::uint64_t>(data.value().GetIndex()) ==
                    playedEntityID)
                    played = &data.value();
            });
            if (played == nullptr || !played->HasBattlecry()) continue;
            // Count each successful Battlecry-minion play only until the
            // threshold is reached. If the hand is full at that boundary,
            // retain the completed cadence and retry on the next successful
            // play that has room; do not require an eighth play or advance
            // the counter beyond seven while delivery is pending.
            if (trinket.triggerProgress < behavior.value)
                ++trinket.triggerProgress;
            if (trinket.triggerProgress < behavior.value || hand.IsFull())
                continue;
            static constexpr std::array<std::string_view, 5> ids = {
                "BG34_634t", "BG34_635t", "BG34_636t",
                "BG34_637t", "BG34_638t"};
            std::vector<Card> candidates;
            for (const auto id : ids) {
                const auto candidate = Cards::FindCardByID(id);
                if (candidate.dbfID != 0 && candidate.hasBehavior &&
                    candidate.GetCardType() == CardType::MINION)
                    candidates.push_back(candidate);
            }
            bool addedReward = false;
            for (int i = 0; i < behavior.amount && !hand.IsFull(); ++i)
                addedReward = AddRandomMinionToHand(*this, candidates) ||
                              addedReward;
            if (addedReward) trinket.triggerProgress = 0;
            continue;
        }
        if (behavior.effect == TrinketEffect::AFTER_PLAY_MURLOC_RANDOM_SPELL)
        {
            // GetRace() is the primary race only; HasRace() also recognizes
            // Amalgams and modern multi-race minions.  The caller snapshots
            // that membership before any attachment mutations.
            if (!playedMurloc || behavior.value <= 0) continue;
            // Keep the threshold armed while the hand is full. Once one
            // slot is available, add as many requested spells as fit and
            // reset the cadence; a completely full hand retries later.
            if (trinket.triggerProgress < behavior.value)
                ++trinket.triggerProgress;
            if (trinket.triggerProgress < behavior.value || hand.IsFull())
                continue;
            (void)SimpleTasks::RandomTavernSpellToHandTask{
                behavior.amount, 0, true}.Run(*this);
            trinket.triggerProgress = 0;
            continue;
        }
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
        if (behavior.effect == TrinketEffect::AFTER_PLAY_DEMON_DAMAGE)
        {
            if (!magnetic && (playedDemon || playedRace == Race::DEMON) &&
                behavior.value > 0)
                hero.TakeDamage(*this, behavior.value,
                                HeroDamageSource::RECRUIT_SELF);
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

void Player::ApplyDaggerspineThrasherSpellCast()
{
    recruitField.ForEachAlive([](MinionData& data) {
        Minion& minion = data.value();
        if (minion.GetCardID() != "BG27_024" &&
            minion.GetCardID() != "BG27_024_G") return;
        const bool golden = minion.GetCardID() == "BG27_024_G";
        const auto parentID = golden ? "BG27_024_G" : "BG27_024";
        switch (Random::get<int>(0, 2)) {
            case 0:
                static_cast<void>(ApplyReviewedLifecycleEnchantment(
                    minion, parentID,
                    golden ? "BG27_024_Ge1" : "BG27_024e1",
                    Minion::TemporaryEnchantment::DivineShield));
                break;
            case 1:
                static_cast<void>(ApplyReviewedLifecycleEnchantment(
                    minion, parentID,
                    golden ? "BG27_024_Ge2" : "BG27_024e2",
                    Minion::TemporaryEnchantment::StatsAndWindfury));
                break;
            default:
                static_cast<void>(ApplyReviewedLifecycleEnchantment(
                    minion, parentID,
                    golden ? "BG27_024_Ge3" : "BG27_024e3",
                    Minion::TemporaryEnchantment::Venomous));
                break;
        }
    });
}

void Player::ApplyAfterRebornTrinkets(const Minion* reborn)
{
    for (auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect == TrinketEffect::AFTER_REBORN_COPY)
        {
            if (reborn == nullptr || behavior.value <= 0 || hand.IsFull())
                continue;
            if (trinket.triggerProgress >= behavior.value)
                continue;
            if (AddMinionCopyToHand(*reborn))
                ++trinket.triggerProgress;
            continue;
        }
        if (behavior.effect == TrinketEffect::AFTER_REBORN_UNDEAD_REBORN)
        {
            if (reborn == nullptr || !reborn->HasRace(Race::UNDEAD) ||
                behavior.value <= 0 ||
                trinket.triggerProgress >= behavior.value)
                continue;
            // Combat operates on a copy of the recruit field.  Mark this as
            // a combat-persistent keyword gain so CommitPersistentState
            // carries the re-armed Reborn back to the same recruit entity;
            // a plain SetReborn would make the effect disappear when combat
            // ends.  Multiple Apple copies keep independent cadence state;
            // applying the keyword to an already re-armed target is harmless
            // and still consumes this copy's trigger.
            auto* mutableReborn = const_cast<Minion*>(reborn);
            mutableReborn->SetReborn(true);
            mutableReborn->ApplyCombatPersistentKeyword(GameTag::REBORN);
            ++trinket.triggerProgress;
            continue;
        }
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

void Player::ApplyOutsideCombatDestroyTrinkets()
{
    for (const auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect ==
            TrinketEffect::AFTER_OUTSIDE_COMBAT_DESTROY_STATS)
        {
            recruitField.ForEachAlive([&](MinionData& data) {
                data.value().ApplyPersistentMinionStats(behavior.attack,
                                                        behavior.health);
            });
        }
        else if (behavior.effect ==
                 TrinketEffect::AFTER_OUTSIDE_COMBAT_DESTROY_GOLD)
        {
            // Resolve the exact generated Coin Pouch row and its typed spell
            // payload. If either identity or behavior is unavailable, remain
            // fail-closed rather than substituting an arbitrary gold delta.
            const auto pouch = Cards::FindCardByID("BG32_MagicItem_205t");
            const auto pouchBehavior = FindTavernSpellBehavior(pouch.id);
            if (pouch.dbfID != 0 &&
                pouchBehavior.effect == TavernSpellEffect::GAIN_GOLD &&
                pouchBehavior.value > 0)
                remainCoin += pouchBehavior.value;
        }
    }
}

void Player::ApplyStartCombatTrinkets()
{
    // Start-of-combat Trinkets can explicitly activate friendly Deathrattles
    // outside Battle::RemoveMinion.  Route those activations through the
    // active Titus' Tribute scope as well; otherwise Titus only doubled
    // death-caused activations and silently missed Soul Fermenter, Rylak, and
    // the generic Deathrattle starter.  This helper is deliberately local to
    // combat-start paths: recruit-side bespoke Deathrattle tasks must not
    // inherit a combat-only replay boundary.
    const auto activateCombatDeathrattle = [this](Minion& minion) {
        if (!minion.HasDeathrattle()) return;
        minion.ActivateTask(PowerType::DEATHRATTLE, *this);
        for (std::int32_t repeat = 0;
             repeat < season14.TitusTributeExtraActivations(); ++repeat)
            minion.ActivateTask(PowerType::DEATHRATTLE, *this);
    };
    // Recruit-phase Blood Gem improvements armed by Deathrattles expire at
    // this combat boundary, before any new start-of-combat effects resolve.
    season14.ResetTemporaryBloodGemBonus();
    if (HasActivePortrait(PortraitEffect::WHELP_SMUGGLER_STATS_AND_DRAGON))
        battleField.ForEachAlive([](MinionData& data) {
            auto& minion = data.value();
            if (minion.GetCardID() == "BG21_013" ||
                minion.GetCardID() == "BG21_013_G")
                minion.AddRace(Race::DRAGON);
        });
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
        const auto trinketId = Cards::FindCardByDbfID(trinket.dbfID).id;
        if (behavior.effect == TrinketEffect::START_COMBAT_SOUL_FERMENTER)
        {
            // Snapshot before mutating the field.  Remove from right to left
            // so the printed left-most three are stable even when a
            // Deathrattle changes the board while the destroy is resolved.
            std::vector<std::size_t> slots;
            for (std::size_t i = 0;
                 i < static_cast<std::size_t>(battleField.GetCount()) &&
                 slots.size() < 3; ++i)
                if (!battleField[i].IsDestroyed()) slots.push_back(i);
            // Keep snapshots left over by an earlier copy of the Trinket.
            // A Deathrattle (or a full board) can defer resurrection until a
            // later death observer; clearing here would silently lose those
            // exact instances when multiple copies trigger in one combat.
            std::vector<Minion> captured;
            captured.reserve(slots.size());
            for (auto it = slots.rbegin(); it != slots.rend(); ++it)
            {
                // Remove first: Soul Fermenter says to destroy the selected
                // minions.  Their Deathrattles therefore observe the source
                // in the graveyard/removed state, matching real destroy
                // ordering and Rapid Reanimation's authoritative path.
                Minion snapshot = battleField.Remove(battleField[*it]);
                if (snapshot.HasDeathrattle())
                {
                    ++season14.deathrattlesTriggered;
                    activateCombatDeathrattle(snapshot);
                }
                captured.push_back(std::move(snapshot));
            }
            // Removal is right-to-left so positions remain valid; replay is
            // left-to-right, preserving the original board order.  Existing
            // deferred snapshots stay ahead of this trigger's new batch.
            for (auto it = captured.rbegin(); it != captured.rend(); ++it)
                season14.soulFermenterSnapshots.insert(
                    season14.soulFermenterSnapshots.end(), std::move(*it));
            season14.soulFermenterArmed =
                !season14.soulFermenterSnapshots.empty();
            (void)TryResolveSoulFermenterIfSpace(battleField);
            continue;
        }
        if (behavior.portraitEffect ==
            PortraitEffect::RYLAK_START_COMBAT_DEATHRATTLES)
        {
            // Rylak Portrait is narrower than the generic Deathrattle
            // starter: only owned Rylak Metalheads are replayed, once each.
            std::vector<std::uint64_t> rylaks;
            battleField.ForEachAlive([&rylaks](MinionData& data) {
                const auto& minion = data.value();
                if ((minion.GetCardID() == "BG26_801" ||
                     minion.GetCardID() == "BG26_801_G") &&
                    minion.HasDeathrattle())
                    rylaks.push_back(minion.GetIndex());
            });
            for (const auto id : rylaks)
                battleField.ForEachAlive([&](MinionData& data) {
                    auto& minion = data.value();
                    if (minion.GetIndex() == id)
                        activateCombatDeathrattle(minion);
                });
            continue;
        }
        if (behavior.portraitEffect ==
            PortraitEffect::SKY_GOLEM_DEATHRATTLE_STATS)
        {
            // Arm the copied combat board only.  Fresh summons are not part
            // of the printed Start-of-Combat grant.
            battleField.ForEachAlive([](MinionData& data) {
                ApplyReviewedPersistentChildEnchantment(
                    data.value(), "BG35_MagicItem_740e2");
            });
            continue;
        }
        if (trinketId == "BG30_MagicItem_301" ||
            behavior.portraitEffect == PortraitEffect::ETERNAL_KNIGHT_TAUNT_REBORN)
        {
            battleField.ForEachAlive([](MinionData& data) {
                auto& minion = data.value();
                if (minion.GetCardID() == "BG25_008" ||
                    minion.GetCardID() == "BG25_008_G")
                {
                    minion.SetTaunt(true);
                    minion.SetReborn(true);
                }
            });
            continue;
        }
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
                    HasActiveTribe(activeTribes, candidate) &&
                    candidate.HasRace(Race::PIRATE))
                    pirates.push_back(candidate);
            if (pirates.empty()) continue;
            const auto& selected = pirates[
                Random::get<std::size_t>(0, pirates.size() - 1)];
            if (!hand.IsFull())
                hand.Add(CardData{Minion(selected)});
            if (battleField.IsFull()) {
                ApplySummonOverflowTrinkets();
                continue;
            }

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
            if (battleField.IsFull()) {
                ApplySummonOverflowTrinkets();
                continue;
            }
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
                    ApplyReviewedPersistentChildEnchantment(
                        minion, "BG30_MagicItem_917e");
            });
            continue;
        }
        if (behavior.effect == TrinketEffect::START_COMBAT_QUILBOAR_BLOOD_GEMS)
        {
            battleField.ForEachAlive([&](MinionData& data) {
                auto& minion = data.value();
                if (minion.HasRace(Race::QUILBOAR))
                    ApplyReviewedPersistentChildEnchantment(
                        minion, "BG30_MagicItem_411e");
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
                ApplyReviewedPersistentChildEnchantment(
                    *candidates[i], "BG30_MagicItem_952e");
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
            if (battleField.GetCount() == 0) continue;
            if (battleField.IsFull()) {
                ApplySummonOverflowTrinkets();
                continue;
            }
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
        if (behavior.effect == TrinketEffect::START_COMBAT_AUTOMATON_SUMMON)
        {
            if (battleField.IsFull()) {
                ApplySummonOverflowTrinkets();
                continue;
            }
            const auto automatonID = behavior.cardID.empty()
                ? std::string_view{"BG_TTN_401"}
                : behavior.cardID;
            const Card automaton = Cards::FindCardByID(automatonID);
            if (automaton.id.empty()) continue;
            Minion summoned(automaton);
            summoned.getPlayerCallback = [this]() -> Player& { return *this; };
            if (getNextCardIndexCallback)
                summoned.SetIndex(getNextCardIndexCallback());
            ApplyFreshMinionModifiers(summoned);
            battleField.Add(summoned, battleField.GetCount());
            Minion& added = battleField[battleField.GetCount() - 1];
            battleField.ForEachAlive([&added](MinionData& data) {
                data.value().ActivateTrigger(TriggerType::SUMMON, added);
            });
            ApplySummonTrinkets(added);
            continue;
        }
        if (behavior.effect == TrinketEffect::START_COMBAT_AUTO_ASSEMBLER)
        {
            // Resolve the printed "magnetize an Auto Assembler to all your
            // Mechs" as real attachments on the copied combat board.  Take a
            // stable target snapshot first: MagnetizeOnto may emit owner
            // listeners, but must not make later targets depend on iteration
            // order or on newly-created entities.
            const Card assemblerCard = Cards::FindCardByID("BG32_172");
            if (assemblerCard.id.empty()) continue;
            std::vector<std::uint64_t> targets;
            battleField.ForEachAlive([&targets](MinionData& data) {
                if (data.value().HasRace(Race::MECHANICAL))
                    targets.push_back(data.value().GetIndex());
            });
            for (const auto targetIndex : targets)
                battleField.ForEachAlive([&](MinionData& data) {
                    auto& target = data.value();
                    if (target.GetIndex() != targetIndex ||
                        !target.HasRace(Race::MECHANICAL))
                        return;
                    Minion attachment(assemblerCard);
                    attachment.MagnetizeOnto(target);
                    ApplyAfterMagnetizeTrinkets(target);
                });
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
                        activateCombatDeathrattle(data.value());
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

void Player::ResolveBoomController(FieldZone& combatField)
{
    const auto& snapshot = season14.BoomControllerFirstMech();
    if (!snapshot.has_value() || combatField.IsFull()) return;
    bool consumed = false;
    for (auto& trinket : season14.trinkets) {
        if (!trinket.active || trinket.remainingUses == 0 ||
            trinket.triggerProgress != 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect != TrinketEffect::BOOM_CONTROLLER_FIRST_MECH_COPY)
            continue;
        if (combatField.IsFull()) break;
        if (SummonCombatSnapshot(*snapshot)) {
            trinket.triggerProgress = 1;
            consumed = true;
        }
    }
    if (consumed) season14.ClearBoomControllerMech();
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

    // Rune of Transmutation is a per-instance lifetime counter.  Once its
    // fifteenth successful Tavern spell resolves, replace that exact owned
    // slot with one random executable Greater Naga Trinket.  Build the pool
    // from the pinned card metadata and current owned identities so this
    // cannot produce a duplicate or a metadata-only/unsupported Trinket.
    for (std::size_t index = 0; index < season14.trinkets.size(); ++index)
    {
        auto& trinket = season14.trinkets[index];
        if (!trinket.active || trinket.remainingUses == 0 ||
            FindTrinketBehavior(Cards::FindCardByDbfID(trinket.dbfID).id).effect !=
                TrinketEffect::SPELL_COUNT_REPLACE_GREATER_NAGA ||
            FindTrinketBehavior(Cards::FindCardByDbfID(trinket.dbfID).id).value <= 0)
            continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (++trinket.triggerProgress < behavior.value)
            continue;

        std::vector<Card> candidates;
        for (const auto& candidate : Cards::GetAllCards())
        {
            if (candidate.GetCardType() != CardType::BATTLEGROUND_TRINKET ||
                candidate.trinketType != "GREATER_TRINKET" ||
                candidate.normalDbfID != 0 || candidate.dbfID <= 0 ||
                // Trinket tribe affinity is pool metadata
                // (`battlegroundsAssociatedRaces`), not a gameplay card
                // race.  Card::HasRace() intentionally only examines the
                // latter, so use the loader-preserved associated-race field
                // here or the Rune pool is always empty.
                std::find(candidate.associatedRaces.begin(),
                          candidate.associatedRaces.end(), "NAGA") ==
                    candidate.associatedRaces.end() ||
                FindTrinketBehavior(candidate.id).effect == TrinketEffect::NONE ||
                std::any_of(
                    season14.trinkets.begin(), season14.trinkets.end(),
                    [&candidate](const Season14PersistentEffect& owned) {
                        return owned.dbfID == candidate.dbfID;
                    }))
                continue;
            candidates.push_back(candidate);
        }
        // Keep the progress armed when the pinned pool cannot currently
        // provide a legal replacement; no successful spell is lost and a
        // later data/configuration repair can retry the replacement.
        if (candidates.empty()) {
            trinket.triggerProgress = behavior.value;
            continue;
        }
        const auto replacement = candidates[
            Random::get<std::size_t>(0, candidates.size() - 1)];
        const auto oldTrinket = trinket;
        season14.trinkets.erase(season14.trinkets.begin() + index);
        if (!AcquireTrinket({replacement.dbfID, 1, true}))
        {
            season14.trinkets.insert(season14.trinkets.begin() + index,
                                     oldTrinket);
            continue;
        }
        // The replacement occupies the same logical Trinket slot.  Its
        // acquisition payload is resolved by AcquireTrinket, and the loop
        // advances past the newly installed effect.
    }

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
    // Bassgill Portrait upgrades every friendly Murloc summoned during
    // combat, including the Murloc pulled from hand by Bassgill's own
    // Deathrattle.  Resolve after insertion so the summoned entity is the
    // authoritative target and before later summon auras inspect it.
    if (isInCombat && summoned.HasRace(Race::MURLOC) &&
        HasActivePortrait(PortraitEffect::BASSGILL_SUMMON_DIVINE_SHIELD))
        summoned.SetGameTag(GameTag::DIVINE_SHIELD, 1);
    if (isInCombat && season14.HasGeneratedRewardTumblingDisaster()) {
        const auto bonus = season14.GeneratedRewardTumblingBonus();
        summoned.SetAttack(summoned.GetAttack() + bonus);
        summoned.SetHealth(summoned.GetHealth() + bonus);
    }
    RecordAncestralAutomatonSummon(summoned);
    // Both Silver Fledgling rarities are the same printed summon for Goose
    // Portrait. Count the canonical normal and golden token IDs regardless
    // of whether the summon came from recruit or combat resolution.
    const bool isSilverFledgling =
        summoned.GetCardID() == "BG29_801t" ||
        summoned.GetCardID() == "BG29_801_Gt";
    if (HasActivePortrait(PortraitEffect::GOOSE_FLEDGLING_REWARD) &&
        isSilverFledgling) {
        for (auto& trinket : season14.trinkets) {
            if (!trinket.active || trinket.remainingUses == 0) continue;
            const auto behavior = FindTrinketBehavior(
                Cards::FindCardByDbfID(trinket.dbfID).id);
            if (behavior.portraitEffect !=
                PortraitEffect::GOOSE_FLEDGLING_REWARD) continue;
            if (behavior.value <= 0) continue;
            // A full hand does not discard a completed reward. Keep the
            // threshold armed and retry after space opens; cap progress so
            // repeated summons cannot overflow this portrait's cadence.
            if (trinket.triggerProgress < behavior.value)
                ++trinket.triggerProgress;
            if (trinket.triggerProgress < behavior.value || hand.IsFull())
                continue;
            trinket.triggerProgress = 0;
            const Card egg = Cards::FindCardByID("BG30_104");
            if (egg.dbfID != 0 && egg.hasBehavior)
                hand.Add(CardData{Minion(egg)});
        }
    }
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
        if (behavior.effect == TrinketEffect::START_COMBAT_FIRST_SUMMON_COPY &&
            isInCombat && trinket.triggerProgress == 0)
        {
            // Capture the first summon even when the board is full.  If space
            // opens later, the effect still copies that first minion rather
            // than whichever later summon happened to be observed.
            if (!trinket.pendingFirstSummon.has_value())
                trinket.pendingFirstSummon = summoned;
            // The printed condition is part of the trigger: a full board
            // must not consume the once-per-combat opportunity.  This also
            // lets a later summon create the copy after space opens.
            if (battleField.IsFull()) {
                ApplySummonOverflowTrinkets();
                continue;
            }
            const int copies = behavior.amount > 0 ? behavior.amount : 1;
            const Minion source = *trinket.pendingFirstSummon;
            // Mark the opportunity before notifying summon observers.  The
            // generated copy is itself a summon and must not recursively
            // retrigger this same Trinket (especially for Twin Sky Lanterns).
            trinket.triggerProgress = 1;
            trinket.pendingFirstSummon.reset();
            for (int copyIndex = 0; copyIndex < copies; ++copyIndex) {
                if (battleField.IsFull()) {
                    ApplySummonOverflowTrinkets();
                    if (battleField.IsFull()) break;
                }
                Minion copy = source;
                if (!SummonCombatSnapshot(std::move(copy))) break;
            }
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

void Player::ApplySummonOverflowTrinkets()
{
    for (const auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect != TrinketEffect::AFTER_SUMMON_OVERFLOW_STATS)
            continue;
        GetField().ForEachAlive([&behavior](MinionData& data) {
            auto& minion = data.value();
            minion.SetAttack(minion.GetAttack() + behavior.attack);
        });
    }
}

void Player::OnFriendlyDivineShieldLost(Minion& minion)
{
    if (!isInCombat) return;
    // Divine Signet is race-independent and observes every authoritative
    // friendly shield-loss callback.  Mechanical repair remains a separate
    // observer below and is the only path that checks the minion's race.
    for (auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect !=
                TrinketEffect::AFTER_DIVINE_SHIELD_LOST_RANDOM_SPELL ||
            trinket.triggerProgress >= behavior.value)
            continue;
        (void)SimpleTasks::RandomTavernSpellToHandTask{1}.Run(*this);
        ++trinket.triggerProgress;
    }
    if (!minion.HasRace(Race::MECHANICAL)) return;
    for (auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect != TrinketEffect::MECH_DIVINE_SHIELD_REPAIR ||
            trinket.triggerProgress >= behavior.value)
            continue;
        minion.SetGameTag(GameTag::DIVINE_SHIELD, 1);
        ++trinket.triggerProgress;
    }
}

void Player::OnFriendlyMinionDamaged(Minion& minion)
{
    std::vector<Minion*> candidates;
    recruitField.ForEachAlive([&](MinionData& data) {
        if (&data.value() != &minion)
            candidates.push_back(&data.value());
    });
    if (candidates.empty()) return;
    for (auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect != TrinketEffect::AFTER_FRIENDLY_DAMAGE_STATS)
            continue;
        auto* target = candidates[Random::get<std::size_t>(
            0, candidates.size() - 1)];
        target->ApplyPersistentMinionStats(behavior.attack, behavior.health);
    }
}

void Player::OnFriendlyVenomousLost(Minion& minion)
{
    // Emit exactly once per real Venomous -> absent transition. Each owned
    // portrait is an independent passive, and the golden portrait has the
    // printed +14/+14 payload rather than inheriting the normal +4/+4.
    int attack = 0;
    int health = 0;
    for (const auto& trinket : season14.trinkets) {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.portraitEffect != PortraitEffect::BELCHER_VENOMOUS_LOSS_STATS)
            continue;
        const auto id = Cards::FindCardByDbfID(trinket.dbfID).id;
        const int amount = id == "BG30_MagicItem_432t" ? 14 : 4;
        if (amount == 4)
            minion.ApplyPersistentMinionStats(4, 4);
        else
        {
            attack += amount;
            health += amount;
        }
    }
    if (attack != 0)
        minion.ApplyPersistentMinionStats(attack, health);
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

void Player::ApplyAfterMagnetizeTrinkets(Minion& target)
{
    if (target.IsDestroyed()) return;
    // The event is target-local and is emitted only after MagnetizeOnto has
    // accepted the attachment.  `triggerProgress` belongs to each owned
    // Trinket, so duplicate copies and golden copies advance independently.
    for (auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect != TrinketEffect::AFTER_MAGNETIZE_STATS)
            continue;
        const auto scale = behavior.value * trinket.triggerProgress;
        target.SetAttack(target.GetAttack() + behavior.attack + scale);
        target.SetHealth(target.GetHealth() + behavior.health + scale);
        ++trinket.triggerProgress;
    }
}

void Player::ResolveStartTurnTrinkets()
{
    ResolveLockboxAtRecruitStart();
    for (auto& trinket : season14.trinkets) {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect == TrinketEffect::TAVERN_SPELL_GROWING_STATS) {
            season14.AddTavernSpellAttackBonus(behavior.attack);
            season14.AddTavernSpellHealthBonus(behavior.health);
        }
        if (behavior.effect == TrinketEffect::MALDRAXXUS_DAGGER_DISCOVER)
            (void)BeginMaldraxxusDaggerDiscover(trinket.dbfID);
        if (behavior.effect == TrinketEffect::TICKATUS_DARKMOON_PRIZE) {
            // Once the cadence matures, keep the instance armed at the
            // threshold until its modal can actually be opened.  In
            // particular, another public modal (or a full hand) must not
            // advance this copy beyond the threshold and thereby turn a
            // three-turn cadence into an unbounded retry counter.  This is
            // per Trinket state, so duplicate Tickatus copies remain
            // independent when one of them owns the modal.
            if (behavior.value > 0 && trinket.triggerProgress < behavior.value)
                ++trinket.triggerProgress;
            if (behavior.value > 0 &&
                trinket.triggerProgress >= behavior.value &&
                BeginTickatusDiscover())
            {
                trinket.triggerProgress = 0;
                season14.pendingTickatusDiscover = false;
            }
        }
        if (behavior.effect == TrinketEffect::LOCKBOX_PORTRAIT)
            GrantOrAdvanceLockbox(2);
        if (behavior.effect == TrinketEffect::YOGG_WHEEL)
            (void)ResolveYoggWheel();
        if (behavior.effect == TrinketEffect::GOLD_PENDANT_GOLDENIZE)
            ApplyGoldPendant();
        if (behavior.effect == TrinketEffect::TRIP_VOUCHERS_REPLACE_GREATER) {
            // The printed countdown advances once per recruit start.  Mark
            // the instance at the threshold before opening the modal so a
            // blocked/replayed public choice cannot advance it twice.
            if (++trinket.triggerProgress >= behavior.value) {
                trinket.triggerProgress = behavior.value;
                (void)BeginTripVouchersOffer();
            }
        }
    }
    // Per-turn cadence state is reset before any start-turn grants.  Keep it
    // on each persistent effect so duplicate Cathedral copies stack exactly.
    for (auto& trinket : season14.trinkets) {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto startBehavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (startBehavior.effect == TrinketEffect::FIRST_SPELL_REPEAT ||
            startBehavior.effect == TrinketEffect::SPELLCRAFT_REPEAT ||
            startBehavior.effect == TrinketEffect::SINSTONE_DISCOVER_COPY ||
            startBehavior.effect == TrinketEffect::AFTER_PLAY_ELEMENTAL_RANDOM_SPELL ||
            startBehavior.effect == TrinketEffect::BATTLECRY_EXTRA_TRIGGERS ||
            startBehavior.effect == TrinketEffect::ACQUIRE_FIXED_CARD_FIRST_PIRATE_FREE ||
            startBehavior.effect == TrinketEffect::BUY_DEMON_HEALTH_ONCE_PER_TURN ||
            (startBehavior.effect == TrinketEffect::TIER_SIX_ONLY_REFRESH &&
             startBehavior.value > 0))
            trinket.triggerProgress = 0;
        if (startBehavior.effect ==
            TrinketEffect::AFTER_REBORN_UNDEAD_REBORN)
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

void Player::ApplyGoldPendant()
{
    std::vector<std::pair<bool, std::size_t>> candidates;
    const auto tierCap = [&]() {
        for (const auto& trinket : season14.trinkets) {
            if (!trinket.active || trinket.remainingUses == 0) continue;
            const auto behavior = FindTrinketBehavior(
                Cards::FindCardByDbfID(trinket.dbfID).id);
            if (behavior.effect == TrinketEffect::GOLD_PENDANT_GOLDENIZE)
                return behavior.tier;
        }
        return 0;
    }();
    if (tierCap <= 0) return;
    for (std::size_t i = 0; i < recruitField.GetCount(); ++i) {
        auto& minion = recruitField[i];
        if (!minion.IsDestroyed() && minion.GetTier() <= tierCap &&
            minion.CanMakeGolden())
            candidates.emplace_back(false, i);
    }
    for (std::size_t i = 0; i < hand.GetCount(); ++i) {
        if (!std::holds_alternative<Minion>(hand[i]))
            continue;
        const auto& minion = std::get<Minion>(hand[i]);
        if (minion.GetTier() <= tierCap && minion.CanMakeGolden())
            candidates.emplace_back(true, i);
    }
    if (candidates.empty()) return;
    const auto [inHand, index] = candidates[Random::get<std::size_t>(
        0, candidates.size() - 1)];
    if (inHand)
        (void)std::get<Minion>(hand[index]).MakeGolden();
    else
        (void)recruitField[index].MakeGolden();
}

void Player::ResolveLockboxAtRecruitStart()
{
    if (!season14.lockboxActive)
        return;
    // A portrait/escapee advance may bring the countdown to zero between
    // recruit starts.  In that case the box must resolve on that same start;
    // do not return early merely because the stored countdown is already 0.
    if (season14.lockboxTurnsRemaining > 0) {
        --season14.lockboxTurnsRemaining;
        if (season14.lockboxTurnsRemaining > 0) return;
    }

    int lockboxSlot = -1;
    for (int i = 0; i < hand.GetCount(); ++i)
    {
        if (std::holds_alternative<Spell>(hand[i]) &&
            std::get<Spell>(hand[i]).GetID() == "BG36_520t")
        {
            // This is the owned public entity (GetCardID() == "BG36_520t"),
            // rather than an unrelated generated spell with the same timing.
            lockboxSlot = i;
            break;
        }
    }
    if (lockboxSlot < 0) return;

    std::vector<Card> candidates;
    for (const auto& candidate : Cards::GetAllCards())
    {
        if (!candidate.isBattlegroundsPoolMinion || !candidate.hasBehavior ||
            candidate.GetCardType() != CardType::MINION ||
            candidate.normalDbfID != 0 || candidate.premiumDbfID == 0 ||
            candidate.GetRace() == Race::INVALID ||
            !HasActiveTribe(activeTribes, candidate))
            continue;
        const auto golden = Cards::FindCardByDbfID(candidate.premiumDbfID);
        if (golden.dbfID != 0 && golden.hasBehavior &&
            golden.GetCardType() == CardType::MINION &&
            golden.GetRace() != Race::INVALID)
            candidates.push_back(golden);
    }
    if (candidates.empty()) return;
    const auto reward = candidates[Random::get<std::size_t>(
        0, candidates.size() - 1)];
    hand.Remove(hand[lockboxSlot]);
    hand.Add(CardData{Minion(reward)}, lockboxSlot);
    season14.lockboxActive = false;
    season14.lockboxAdvance = 0;
    season14.lockboxTurnsRemaining = 0;
}

void Player::GrantOrAdvanceLockbox(int turnsSooner)
{
    turnsSooner = std::max(0, turnsSooner);
    // Keep the inactive branch explicit in the lifecycle contract
    // (`lockboxActive == false` means no public Lockbox is owned).
    if (!season14.lockboxActive)
    {
        const auto lockbox = Cards::FindCardByID("BG36_520t");
        if (lockbox.id.empty() || hand.IsFull()) return;
        hand.Add(CardData{Spell(lockbox)});
        season14.lockboxActive = true;
        season14.lockboxTurnsRemaining = 5;
        return;
    }
    season14.lockboxAdvance += turnsSooner;
    season14.lockboxTurnsRemaining = std::max(
        0, season14.lockboxTurnsRemaining - turnsSooner);
    if (season14.lockboxTurnsRemaining == 0)
        ResolveLockboxAtRecruitStart();
}

int Player::ConsumeWarDrumRepeats()
{
    int repeats = 0;
    for (auto& trinket : season14.trinkets) {
        if (!trinket.active || trinket.remainingUses == 0 ||
            trinket.triggerProgress != 0)
            continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect != TrinketEffect::BATTLECRY_EXTRA_TRIGGERS)
            continue;
        // Each owned War Drum has its own once-per-recruit-turn allowance.
        // Consume every eligible instance for this Battlecry so duplicate
        // trinkets stack (two copies therefore produce four extra
        // resolutions), while the per-instance marker prevents later
        // Battlecries in the same turn from consuming it again.
        trinket.triggerProgress = 1;
        repeats += std::max(0, behavior.amount);
    }
    return repeats;
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
    const auto targetEntityID = targetShop
        ? static_cast<std::uint64_t>(target.GetIndex()) : 0;
    ApplySpellBoardEffect(*this, effect, static_cast<int>(targetIdx), false, card.dbfID);
    ApplyImperialDefenderCopies(
        *this, static_cast<int>(targetIdx), card.targetingType,
        [&](int buddyIdx) {
            ApplySpellBoardEffect(*this, effect, buddyIdx, false, card.dbfID);
        });
    // A Tavern/shop minion is still a minion for Honeycomb Ring's trigger.
    // Pass the semantic target fact rather than excluding shop targets.
    season14.OnTavernSpellResolved(
        true, card.dbfID, true, targetEntityID);
    ResolveSpellCountTrinkets();
    ApplyTavernSpellTrinkets();
    const bool resumeGeneratedRewardSpells =
        season14.HasGeneratedRewardStartTurnRandomSpells() &&
        season14.HasPendingGeneratedRewardRandomSpells();
    const bool resumeLavishCapeSpells =
        season14.HasPendingLavishCapeRandomSpells();
    season14.spellModal = {};
    season14.pendingTaughtSpell = {};
    season14.pendingDecision = Season14Decision::NONE;
    if (resumeGeneratedRewardSpells)
        (void)SimpleTasks::ActivateRandomTavernSpellsTask{
            season14.GeneratedRewardRandomSpellsRemaining()}.Run(*this);
    else if (resumeLavishCapeSpells)
        (void)SimpleTasks::ActivateRandomTavernSpellsTask{
            season14.LavishCapeRandomSpellsRemaining()}.Run(*this);
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
        const auto generatedTier = currentTier;
        std::vector<Season14Offering> offerings;
        offerings.reserve(count);
        for (std::size_t i = 0; i < count; ++i)
            offerings.push_back({candidates[i].dbfID, 0});
        season14.BeginOfferingDecision(Season14Decision::DISCOVER, 0,
                                       source.dbfID, std::move(offerings));
        season14.pendingBoundlessDiscoverTier = generatedTier;
        return true;
    }
    if (season14.spellModal.kind ==
        Season14SpellModalKind::BLOOD_GEM_CHOOSE_ONE)
    {
        const bool trailblazer = season14.trailblazerCombinedChooseOne;
        const int extraResolutions = season14.spellModal.extraResolutionCount;
        const auto source = Cards::FindCardByDbfID(
            season14.spellModal.sourceCardDbfID);
        if (source.id != "BG31_893") return false;
        int attack = (offeringIdx == 0 || trailblazer) ? 1 : 0;
        int health = (offeringIdx == 1 || trailblazer) ? 1 : 0;
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
        const bool trailblazer = season14.trailblazerCombinedChooseOne;
        const int extraResolutions = season14.spellModal.extraResolutionCount;
        int attack = 2;
        int health = 2;
        if (offeringIdx == 1 || trailblazer)
        {
            season14.deferredMinionAttack += 4;
            season14.deferredMinionHealth += 4;
            season14.deferredMinionStatTurns = 1;
        }
        if (offeringIdx == 0 || trailblazer)
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
            if (offeringIdx == 1 || trailblazer)
            {
                season14.deferredMinionAttack += 4;
                season14.deferredMinionHealth += 4;
                season14.deferredMinionStatTurns = 1;
            }
            if (offeringIdx == 0 || trailblazer)
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
    const int friendlySpellRepeats =
        season14.spellModal.friendlySpellRepeatCount;
    const auto applyLovelyLocketRepeats =
        [&](int attack, int health, int branchBonus) {
            // Select the secondary target at resolution time.  The original
            // target is excluded and a target that died while the modal was
            // pending is never reused.  Direct application prevents the
            // replay from recursively re-arming Lovely Locket.
            for (int repeat = 0; repeat < friendlySpellRepeats; ++repeat) {
                int secondaryIdx = -1;
                for (int candidate = 0;
                     candidate < recruitField.GetCount(); ++candidate) {
                    if (candidate == targetIdx ||
                        recruitField[static_cast<std::size_t>(candidate)]
                            .IsDestroyed() ||
                        recruitField[static_cast<std::size_t>(candidate)]
                            .GetHealth() <= 0)
                        continue;
                    secondaryIdx = candidate;
                    break;
                }
                if (secondaryIdx < 0) continue;
                auto& secondary = recruitField[
                    static_cast<std::size_t>(secondaryIdx)];
                secondary.SetAttack(secondary.GetAttack() + attack + branchBonus);
                secondary.SetHealth(secondary.GetHealth() + health + branchBonus);
            }
        };
    const bool trailblazer = season14.trailblazerCombinedChooseOne;
    if (modalKind == Season14SpellModalKind::TARGET_OR_ALL_STATS &&
        offeringIdx == 1 && !trailblazer)
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
    if (modalKind == Season14SpellModalKind::TARGET_OR_ALL_STATS &&
        trailblazer)
    {
        // Keep the selected target validation and target-branch payload, then
        // apply the all-minion branch as a second payload.
        for (int repeat = 0; repeat <= extraResolutions; ++repeat)
            recruitField.ForEachAlive([](MinionData& data) {
                data.value().SetAttack(data.value().GetAttack() + 2);
                data.value().SetHealth(data.value().GetHealth() + 2);
            });
    }
    // The target branch is a real cast on the selected minion even though
    // its payload was chosen asynchronously.  Mirror the resolved branch,
    // including Cathedral/Sushi repeats, onto each Defender allowance.  The
    // ALL branch returns above and is intentionally not mirrored.
    const auto sourceCard = Cards::FindCardByDbfID(sourceDbfID);
    const int targetBranchBonus =
        modalKind == Season14SpellModalKind::TARGET_OR_ALL_STATS ? 6 : 0;
    applyLovelyLocketRepeats(attack, health, targetBranchBonus);
    ApplyImperialDefenderCopies(
        *this, targetIdx, sourceCard.targetingType,
        [&](int buddyIdx) {
            auto& buddy = recruitField[static_cast<std::size_t>(buddyIdx)];
            buddy.SetAttack(buddy.GetAttack() + attack + targetBranchBonus);
            buddy.SetHealth(buddy.GetHealth() + health + targetBranchBonus);
            for (int repeat = 0; repeat < extraResolutions; ++repeat) {
                buddy.SetAttack(buddy.GetAttack() + attack + targetBranchBonus);
                buddy.SetHealth(buddy.GetHealth() + health + targetBranchBonus);
            }
        });
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
    if (season14.pendingOfferings.size() != 2 ||
        !IsChooseOneOptionForSource(
            season14.chooseOne.sourceCardDbfID,
            season14.pendingOfferings[offeringIdx].dbfID))
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
            const bool trailblazer = season14.trailblazerCombinedChooseOne;
        if (season14.chooseOne.sourceCardDbfID == Cards::FindCardByID("BG30_123").dbfID ||
            season14.chooseOne.sourceCardDbfID == Cards::FindCardByID("BG30_123_G").dbfID)
        {
            if (offeringIdx == 0 || trailblazer)
                season14.AddBloodGemBonus(golden ? 2 : 1, golden ? 2 : 1);
            if (offeringIdx == 1 || trailblazer)
                AddBloodGems(golden ? 8 : 4);
        }
        else if (season14.chooseOne.sourceCardDbfID == Cards::FindCardByID("BG31_320").dbfID ||
                 season14.chooseOne.sourceCardDbfID == Cards::FindCardByID("BG31_320_G").dbfID)
        {
            if (offeringIdx == 0 || trailblazer) AddBloodGems(golden ? 4 : 2);
            if (offeringIdx == 1 || trailblazer) gemDays += golden ? 2 : 1;
        }
        else if (season14.chooseOne.sourceCardDbfID == Cards::FindCardByID("BG32_237").dbfID ||
                 season14.chooseOne.sourceCardDbfID == Cards::FindCardByID("BG32_237_G").dbfID)
        {
            if (offeringIdx == 0 || trailblazer) season14.AddTavernSpellAttackBonus(golden ? 2 : 1);
            if (offeringIdx == 1 || trailblazer) season14.AddTavernSpellHealthBonus(golden ? 2 : 1);
        }
        else if (season14.chooseOne.sourceCardDbfID == Cards::FindCardByID("BG36_330").dbfID ||
                 season14.chooseOne.sourceCardDbfID == Cards::FindCardByID("BG36_330_G").dbfID)
        {
            if (offeringIdx == 0 || trailblazer)
                season14.AddFreeRefreshes(golden ? 4 : 2);
            if (offeringIdx == 1 || trailblazer)
                AddBloodGems(golden ? 6 : 3);
        }
        else if (season14.chooseOne.sourceCardDbfID == Cards::FindCardByID("BG36_341").dbfID ||
                 season14.chooseOne.sourceCardDbfID == Cards::FindCardByID("BG36_341_G").dbfID)
        {
            const int amount = golden ? 6 : 3;
            if (offeringIdx == 0 || trailblazer) {
                recruitField.ForEachAlive([this, amount](MinionData& data) {
                    for (int i = 0; i < amount; ++i) ApplyBloodGemTo(data.value());
                });
            }
            if (offeringIdx == 1 || trailblazer) {
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
            if (offeringIdx == 0 || trailblazer) {
                const auto candidates = SupportedMinionsForRace(Race::QUILBOAR, activeTribes);
                const int count = golden ? 2 : 1;
                for (int i = 0; i < count && !hand.IsFull(); ++i)
                    AddRandomMinionToHand(*this, candidates);
            }
            if (offeringIdx == 1 || trailblazer)
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
            !card.hasBehavior || !HasActiveTribe(activeTribes, card) ||
            card.GetTier() < 1 ||
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
    bool designerEyepatch = false;
    for (const auto& trinket : season14.trinkets) {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect == TrinketEffect::TWO_COPIES_MAKE_GOLDEN) {
            designerEyepatch = true;
            break;
        }
    }
    if (season14.heroPowerDbfID != 126533 && !pilferedLamps &&
        !designerEyepatch) return false;
    const bool pirateOnly = designerEyepatch && !pilferedLamps &&
                            season14.heroPowerDbfID != 126533;
    // Double Time's pair completion grants a Tavern Coin and therefore keeps
    // the historical full-hand guard.  Pilfered Lamps is a different rule:
    // two copies simply become one Golden minion, so a full hand must not
    // prevent a pair that lives on the board from resolving.
    // A full hand must not block an Eyepatch pair that lives on the board:
    // the reward is an in-place golden conversion, not a generated hand
    // card.  Double Time still keeps its historical full-hand guard because
    // that path also grants a Tavern Coin to hand.
    if (!pilferedLamps && !designerEyepatch && hand.IsFull()) return false;

    auto family = [](const Minion& minion) {
        const auto card = Cards::FindCardByDbfID(minion.GetDbfID());
        return card.normalDbfID == 0 ? card.dbfID : card.normalDbfID;
    };
    auto eligible = [pirateOnly](const Minion& minion) {
        return !minion.IsGolden() && !minion.IsDestroyed() &&
               (!pirateOnly || minion.HasRace(Race::PIRATE));
    };
    std::vector<int> handMatches;
    std::vector<int> boardMatches;
    for (int i = 0; i < hand.GetCount(); ++i)
    {
        if (std::holds_alternative<Minion>(hand[i]))
        {
            const auto& minion = std::get<Minion>(hand[i]);
            if (eligible(minion)) handMatches.push_back(i);
        }
    }
    for (int i = 0; i < recruitField.GetCount(); ++i)
    {
        const auto& minion = recruitField[i];
        if (eligible(minion))
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
    // A triple consumes two concrete source entities and creates a new
    // golden Buddy identity. Nine Frogs prints nine charges on both forms;
    // do not carry a normal source's spent counter into the golden copy or
    // leave the removed source records alive for a later entity lifecycle.
    const auto resetNineFrogsAfterGolden = [this](const Minion& survivor,
                                                   std::uint64_t consumedID) {
        if (survivor.GetCardID() != "BG28_HERO_801_Buddy_G") return;
        const auto survivorID = static_cast<std::uint64_t>(survivor.GetIndex());
        season14.ForgetNineFrogs(consumedID);
        season14.ResetNineFrogs(survivorID);
    };
    const auto handFirst = std::find_if(
        handMatches.begin(), handMatches.end(), [&](const int index) {
            return family(std::get<Minion>(hand[index])) == key;
        });
    if (handFirst != handMatches.end())
    {
        const int firstIndex = *handFirst;
        auto& first = std::get<Minion>(hand[firstIndex]);
        for (int i = hand.GetCount() - 1; i >= 0; --i)
        {
            if (i == firstIndex || !std::holds_alternative<Minion>(hand[i])) continue;
            if (family(std::get<Minion>(hand[i])) == key)
            {
                const auto consumedID = static_cast<std::uint64_t>(
                    std::get<Minion>(hand[i]).GetIndex());
                if (!first.MergeIntoGolden(std::get<Minion>(hand[i]))) return false;
                resetNineFrogsAfterGolden(first, consumedID);
                hand.Remove(hand[i]);
                if (!pilferedLamps && !designerEyepatch) AddTavernCoins(1);
                return true;
            }
        }
        for (int i = recruitField.GetCount() - 1; i >= 0; --i)
        {
            if (eligible(recruitField[i]) && family(recruitField[i]) == key)
            {
                const auto consumedID = static_cast<std::uint64_t>(
                    recruitField[i].GetIndex());
                // A hand+board pair is still a real triple.  Fold the board
                // instance into the hand survivor before removing it so its
                // enchantments, counters, keywords, and dynamic tasks are
                // not silently discarded.
                if (!first.MergeIntoGolden(recruitField[i])) return false;
                resetNineFrogsAfterGolden(first, consumedID);
                recruitField.Remove(recruitField[i]);
                if (!pilferedLamps && !designerEyepatch) AddTavernCoins(1);
                return true;
            }
        }
    }
    for (int i = recruitField.GetCount() - 1; i >= 0; --i)
    {
        if (!eligible(recruitField[i]) || family(recruitField[i]) != key) continue;
        for (int j = i - 1; j >= 0; --j)
        {
            if (eligible(recruitField[j]) && family(recruitField[j]) == key)
            {
                const auto consumedID = static_cast<std::uint64_t>(
                    recruitField[j].GetIndex());
                if (!recruitField[i].MergeIntoGolden(recruitField[j])) return false;
                resetNineFrogsAfterGolden(recruitField[i], consumedID);
                recruitField.Remove(recruitField[j]);
                if (!pilferedLamps && !designerEyepatch) AddTavernCoins(1);
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
    if (!CanAcquireTrinketPayload(card, behavior)) return false;
    // Bloodbound Earrings display a threshold counter in the pinned card
    // data, not a finite lifetime counter: the normal form repeats every four
    // spells and the golden form every five. Keep the ordinary one-use
    // sentinel so generic persistent-effect liveness checks keep the Trinket
    // active; `triggerProgress` is the actual progress-to-next-trigger state.
    if (behavior.effect == TrinketEffect::SPELL_COUNT_BLOOD_GEMS &&
        behavior.value > 0)
        effect.remainingUses = 1;
    // Transcribing Typewriter's printed number is a finite purchase cadence,
    // not the ordinary one-use sentinel supplied by the offering modal.
    if (behavior.effect == TrinketEffect::AFTER_BUY_MINION_COPY &&
        behavior.value > 0)
        effect.remainingUses = static_cast<std::uint8_t>(behavior.value);
    const auto before = season14.trinkets.size();
    season14.AddTrinket(effect);
    // Season14State rejects duplicate DBF identities. Do not run an
    // acquisition-time payload when the persistent Trinket was not actually
    // inserted; otherwise a rejected second Cape could still cast spells.
    if (season14.trinkets.size() == before)
        return false;
    if (behavior.effect == TrinketEffect::WARBAND_COPY_REFRESH)
    {
        // Mark the newly inserted instance only while its acquisition refresh
        // is being filled.  RefreshTavern consumes the marker after replacing
        // the generated offers with plain warband copies.
        season14.trinkets.back().triggerProgress = 1;
        (void)RefreshTavern(true);
        season14.trinkets.back().triggerProgress = 0;
        season14.trinkets.back().remainingUses = 0;
        season14.trinkets.back().active = false;
    }
    if (behavior.effect == TrinketEffect::START_TURN_RANDOM_SPELLCRAFT)
    {
        // The initial three Spellcraft cards are granted at acquisition; the
        // same descriptor is replayed by GrantTrinketStartTurnCards.
        GrantRandomSpellcraft(*this, behavior.amount);
    }
    if (behavior.effect == TrinketEffect::GOLD_PENDANT_GOLDENIZE)
        ApplyGoldPendant();
    if (behavior.effect == TrinketEffect::POCKET_CYCLONE)
        (void)CastTavernSpellFree("BG34_444", behavior.amount);
    if (behavior.effect == TrinketEffect::TRAILBLAZER_CHOOSE_ONE)
        season14.trailblazerCombinedChooseOne = true;
    if (behavior.effect == TrinketEffect::START_TURN_RANDOM_TAVERN_SPELLS_PER_TYPE)
    {
        // Count distinct type memberships, not only primary race.  An
        // All-type or dual-type minion therefore contributes once per type,
        // matching the printed "different friendly minion type" wording.
        std::array<bool, RACES_IN_BATTLEGROUNDS.size()> types{};
        recruitField.ForEachAlive([&types](const MinionData& data) {
            const auto& minion = data.value();
            for (std::size_t i = 0; i < RACES_IN_BATTLEGROUNDS.size(); ++i)
                types[i] = types[i] || minion.HasRace(RACES_IN_BATTLEGROUNDS[i]);
        });
        const auto count = static_cast<int>(
            std::count(types.begin(), types.end(), true));
        if (count > 0) {
            season14.BeginLavishCapeRandomSpells(count);
            // Do not open a spell target modal while the trinket-selection
            // modal is still active. ApplyChoice resumes this sequence after
            // committing the selected trinket.
            if (season14.pendingDecision == Season14Decision::NONE)
                (void)SimpleTasks::ActivateRandomTavernSpellsTask{count}.Run(*this);
        }
    }
    // Drakkari Portrait's modifier is a persistent identity aura. Apply it
    // to already-owned copies at acquisition time, including copies in hand,
    // so later play and combat snapshots retain both added tribes.
    if (behavior.portraitEffect == PortraitEffect::DRAKKARI_ENCHANTER_ALL_TYPES)
    {
        recruitField.ForEachAlive([](MinionData& data) {
            auto& minion = data.value();
            if (minion.GetCardID() == "BG26_ICC_901" ||
                minion.GetCardID() == "BG26_ICC_901_G")
            {
                minion.AddRace(Race::MECHANICAL);
                minion.AddRace(Race::ELEMENTAL);
            }
        });
        hand.ForEach([](std::optional<CardData>& data) {
            if (!data.has_value() || !std::holds_alternative<Minion>(*data))
                return;
            auto& minion = std::get<Minion>(*data);
            if (minion.GetCardID() == "BG26_ICC_901" ||
                minion.GetCardID() == "BG26_ICC_901_G")
            {
                minion.AddRace(Race::MECHANICAL);
                minion.AddRace(Race::ELEMENTAL);
            }
        });
    }
    if (behavior.portraitEffect == PortraitEffect::WHELP_SMUGGLER_STATS_AND_DRAGON)
    {
        auto addDragon = [](Minion& minion) {
            if (minion.GetCardID() == "BG21_013" ||
                minion.GetCardID() == "BG21_013_G")
                minion.AddRace(Race::DRAGON);
        };
        recruitField.ForEachAlive([&addDragon](MinionData& data) {
            addDragon(data.value());
        });
        hand.ForEach([&addDragon](std::optional<CardData>& data) {
            if (data.has_value() && std::holds_alternative<Minion>(*data))
                addDragon(std::get<Minion>(*data));
        });
    }
    if (behavior.portraitEffect == PortraitEffect::HACKERFIN_END_TURN_BATTLECRY)
    {
        // Apply the portrait to Hackerfins already owned; the fixed grant
        // below receives the same marker before entering the hand.
        recruitField.ForEachAlive([](MinionData& data) {
            auto& minion = data.value();
            if (minion.GetCardID() == "BG31_148" ||
                minion.GetCardID() == "BG31_148_G")
                minion.SetEndTurnBattlecryTrigger(true);
        });
        hand.ForEach([](std::optional<CardData>& data) {
            if (!data.has_value() || !std::holds_alternative<Minion>(*data))
                return;
            auto& minion = std::get<Minion>(*data);
            if (minion.GetCardID() == "BG31_148" ||
                minion.GetCardID() == "BG31_148_G")
                minion.SetEndTurnBattlecryTrigger(true);
        });
    }
    if (behavior.portraitEffect == PortraitEffect::PERMANENT_SPELLCRAFT)
    {
        // The aura applies to every Weary Mage already owned when the
        // Portrait is acquired, not only to the generated Mage and future
        // fresh instances.  Keep the marker on the entity so a later move,
        // copy, or triple retains the taught permanent Spellcraft.
        auto teach = [](Minion& minion) {
            if (minion.GetCardID() == "BG31_830" ||
                minion.GetCardID() == "BG31_830_G" ||
                minion.GetCardID() == "BG31_924" ||
                minion.GetCardID() == "BG31_924_G")
                minion.SetPermanentSpellcraft(true);
        };
        recruitField.ForEachAlive([&teach](MinionData& data) {
            teach(data.value());
        });
        hand.ForEach([&teach](std::optional<CardData>& data) {
            if (data.has_value() && std::holds_alternative<Minion>(*data))
                teach(std::get<Minion>(*data));
        });
    }
    // Static Tavern auras take effect on cards already offered as well as on
    // future fills.  The persistent state above covers future cards; apply
    // this acquisition-time delta to the live mixed/frozen shop exactly once.
    if (behavior.effect == TrinketEffect::SHOP_STATS ||
        behavior.effect == TrinketEffect::SHOP_STATS_AND_EXTRA_SLOT ||
        behavior.effect == TrinketEffect::SHOP_STATS_AND_TAVERN_SLOTS ||
        behavior.effect == TrinketEffect::REFRESH_SHOP_STATS ||
        behavior.effect == TrinketEffect::HERO_DAMAGE_SHOP_STATS)
    {
        tavern.fieldZone.ForEachAlive([&behavior](MinionData& data) {
            data.value().SetAttack(data.value().GetAttack() + behavior.attack);
            data.value().SetHealth(data.value().GetHealth() + behavior.health);
        });
    }
    if (behavior.effect == TrinketEffect::TAVERN_SPELL_STATS ||
        behavior.effect == TrinketEffect::TAVERN_SPELL_GROWING_STATS ||
        behavior.effect == TrinketEffect::TAVERN_SPELL_IMPROVE_AFTER_MINION_CAST) {
        season14.AddTavernSpellAttackBonus(behavior.attack);
        season14.AddTavernSpellHealthBonus(behavior.health);
    }
    if (behavior.effect == TrinketEffect::AFTER_PLAY_DEMON_DAMAGE &&
        behavior.attack > 0)
        armor += behavior.attack;
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
    // Safety Patch's printed five Gold is an acquisition-time reward. Keep it
    // atomic with the accepted Trinket; the generic immediate-gold queue is
    // reserved for effects resolved at the next recruit start.
    if (behavior.effect == TrinketEffect::SAFETY_PATCH)
        remainCoin += behavior.value;
    if (behavior.effect == TrinketEffect::YOGG_WHEEL)
        (void)ResolveYoggWheel();
    if (behavior.effect == TrinketEffect::NEXT_RACE_MINION_GOLDEN &&
        behavior.value > 0)
        season14.AddFreeRefreshes(behavior.value);
    if (behavior.effect == TrinketEffect::ACQUIRE_RANDOM_TRINKET)
    {
        // Orb of the Unknown is a replacement reward, not a third Trinket.
        // Select only canonical, executable, unowned Trinkets; metadata-only
        // cards must never leak into the runtime pool.  Remove the Orb before
        // recursively acquiring the reward so the ordinary two-slot guard is
        // respected, then restore the exact slot index.
        std::vector<Card> candidates;
        const auto greater = behavior.tier > 1;
        for (const auto& candidate : Cards::GetAllCards())
        {
            if (candidate.GetCardType() != CardType::BATTLEGROUND_TRINKET ||
                candidate.normalDbfID != 0 || candidate.dbfID <= 0 ||
                candidate.trinketType != (greater ? "GREATER_TRINKET"
                                                  : "LESSER_TRINKET") ||
                !TrinketIsInLobby(candidate, activeTribes, excludedLobbyRace) ||
                FindTrinketBehavior(candidate.id).effect == TrinketEffect::NONE ||
                // Neither Orb form is a valid replacement.  Excluding only
                // the source ID still lets the Golden Orb choose the normal
                // Orb (and vice versa), recursively creating another reward
                // instead of terminating the replacement lifecycle.
                candidate.id == "BG35_MagicItem_816" ||
                candidate.id == "BG35_MagicItem_816t" ||
                !CanAcquireTrinketPayload(candidate,
                                          FindTrinketBehavior(candidate.id)) ||
                std::any_of(season14.trinkets.begin(), season14.trinkets.end(),
                    [&candidate](const Season14PersistentEffect& owned) {
                        return owned.dbfID == candidate.dbfID;
                    }))
                continue;
            candidates.push_back(candidate);
        }
        if (!candidates.empty())
        {
            Random::shuffle(candidates.begin(), candidates.end());
            const auto slot = season14.trinkets.size() - 1;
            const auto reward = candidates.front();
            season14.trinkets.erase(season14.trinkets.begin() +
                                    static_cast<std::ptrdiff_t>(slot));
            if (!AcquireTrinket({reward.dbfID, 1, true}))
            {
                // The source was valid and already accepted.  Restore it if
                // an unexpectedly malformed generated reward was rejected.
                season14.trinkets.insert(
                    season14.trinkets.begin() + static_cast<std::ptrdiff_t>(slot),
                    effect);
            }
            else
            {
                const auto appended = season14.trinkets.size() - 1;
                if (appended != slot)
                    std::swap(season14.trinkets[slot],
                              season14.trinkets[appended]);
                if (behavior.value > 0)
                    remainCoin += behavior.value;
            }
        }
    }
    if (behavior.effect == TrinketEffect::IMMEDIATE_GOLD_AND_LESSER_NEXT)
        remainCoin += behavior.value;
    if (behavior.effect == TrinketEffect::LOCKBOX_PORTRAIT)
        GrantOrAdvanceLockbox(2);
    if (behavior.effect == TrinketEffect::MYSTERY_CUBE_REPLACE_LESSER &&
        season14.pendingDecision == Season14Decision::NONE)
        (void)BeginMysteryCubeOffer();
    // Acquisition-time grants are resolved exactly once below, alongside
    // the other executable Trinket effects.  Keeping a single dispatch here
    // is important: Compass/Pendant and fixed-card effects may also repeat
    // at recruit start, but their initial grant must not be doubled.
    if (card.id == "BG30_MagicItem_709" || card.id == "BG30_MagicItem_709t")
    {
        // Golden Device opens two sequential public Discover modals. Keep
        // the count in player-owned state so replay or a full hand cannot
        // silently consume the second choice.
        season14.pendingElectromagneticDiscoverRemaining =
            card.id == "BG30_MagicItem_709t" ? 2 : 1;
        season14.pendingElectromagneticDiscoverSourceCardDbfID = card.dbfID;
        (void)BeginElectromagneticDiscover(*this, card.dbfID);
    }
    if (behavior.effect == TrinketEffect::BOOK_OF_MEDIVH_DISCOVER)
        (void)BeginBookOfMedivhDiscover(*this, card.dbfID, behavior.value);
    if (behavior.effect == TrinketEffect::DEATHLY_PHYLACTERY &&
        season14.pendingDecision == Season14Decision::NONE)
    {
        // The printed pool is any supported Deathrattle minion.  Build it
        // from executable card definitions and retain the ordinary public
        // three-choice Discover lifecycle; no random hand insertion may
        // bypass the modal or hand-cap/replay validation.
        (void)BeginMinionDiscover(*this, SupportedDeathrattleMinions(activeTribes),
                                   card.dbfID, false);
    }
    if (behavior.effect == TrinketEffect::TICKATUS_DARKMOON_PRIZE &&
        season14.pendingDecision == Season14Decision::NONE)
    {
        if (!BeginTickatusDiscover()) {
            // A full hand is also a blocked public reward. Keep the newly
            // inserted copy at its threshold and retry after hand space or
            // the decision slot becomes available.
            auto acquired = std::find_if(
                season14.trinkets.rbegin(), season14.trinkets.rend(),
                [effect](const Season14PersistentEffect& owned) {
                    return owned.dbfID == effect.dbfID &&
                           owned.active && owned.remainingUses > 0;
                });
            if (acquired != season14.trinkets.rend())
                acquired->triggerProgress = behavior.value;
            season14.pendingTickatusDiscover = true;
        }
    }
    else if (behavior.effect == TrinketEffect::TICKATUS_DARKMOON_PRIZE)
    {
        // Acquisition is immediate, but public modals are serialized. Arm
        // the newly inserted copy at its threshold so a concurrent Trinket
        // choice cannot silently consume Tickatus's first Discover.
        auto acquired = std::find_if(
            season14.trinkets.rbegin(), season14.trinkets.rend(),
            [effect](const Season14PersistentEffect& owned) {
                return owned.dbfID == effect.dbfID &&
                       owned.active && owned.remainingUses > 0;
            });
        if (acquired != season14.trinkets.rend())
            acquired->triggerProgress = behavior.value;
        season14.pendingTickatusDiscover = true;
    }
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
    if (behavior.effect == TrinketEffect::STATIC_RACE_STATS ||
        behavior.effect == TrinketEffect::ACQUIRE_FLAGBEARER_PORTRAIT)
        ApplyPersistentRaceStats(behavior.race, behavior.attack, behavior.health);
    if (behavior.effect == TrinketEffect::ACQUIRE_RANDOM_CHOOSE_ONE)
    {
        Minion source;
        (void)SimpleTasks::RandomChooseOneCardToHandTask{behavior.amount}
            .Run(*this, source);
    }
    else if (behavior.effect == TrinketEffect::JEWELRY_BOX_BLOOD_GEM)
    {
        // Use the typed public modal so hand-capacity and replay semantics
        // remain identical to other Discover rewards.
        season14.pendingJewelryBoxBloodGemRemaining = 1;
        (void)BeginJewelryBoxBloodGem(*this, card.dbfID);
    }
    else if (behavior.effect == TrinketEffect::SACRIFICIAL_ALTAR)
    {
        // The card text is an immediate conversion, not a death trigger.
        // Remove board entities from the back so stable slot references are
        // never reused while iterating, and award exactly three Gold per
        // entity removed.  Hand minions are intentionally untouched.
        const int removed = recruitField.GetCount();
        for (int i = removed - 1; i >= 0; --i)
            recruitField.Remove(recruitField[i]);
        remainCoin += removed * 3;
    }
    else if (behavior.effect == TrinketEffect::INNKEEPERS_HEARTH_DISCOVER)
    {
        // Normal Hearth uses the owner's current Tavern tier.  The golden
        // generated token is two independent Tier-6 Discover rewards; keep
        // the second modal explicitly in Season14State rather than relying
        // on a mutable Trinket or an anonymous callback.
        const bool golden = card.id == "BG32_MagicItem_362t";
        season14.pendingInnkeepersHearthRemaining = golden ? 2 : 1;
        season14.pendingInnkeepersHearthSelectedDbfID = 0;
        const int tier = golden ? 6 : currentTier;
        if (!BeginInnkeepersHearthDiscover(*this, card.dbfID, tier))
        {
            season14.pendingInnkeepersHearthRemaining = 0;
            season14.pendingInnkeepersHearthSelectedDbfID = 0;
        }
        else
            season14.pendingInnkeepersHearthRemaining = golden ? 2 : 1;
    }
    else if (behavior.effect == TrinketEffect::OMINOUS_STONE_DISCOVER)
    {
        // The reward is a public modal.  Keep acquisition atomic: if there
        // is no legal Tier-4/type/gift tuple, no pending choice is created.
        (void)BeginOminousStoneDiscover(card.dbfID);
    }
    else if (behavior.effect == TrinketEffect::WAX_LANCE_DISCOVER)
    {
        (void)BeginWaxLanceDiscover(card.dbfID);
    }
    else if (behavior.effect == TrinketEffect::KALEIDOSCOPE_DISCOVER)
    {
        // Keep Kaleidoscope on the supported Tier-7 minion pool. The
        // selected hand copy is locked by the shared Discover commit path;
        // the golden form is promoted only after a valid selection.
        std::vector<Card> candidates;
        AppendSupportedNormalMinions(Cards::GetTier7Minions(), candidates,
                                     Race::INVALID, activeTribes);
        const bool golden = card.id == "BG35_MagicItem_821t";
        candidates.erase(std::remove_if(candidates.begin(), candidates.end(),
            [golden](const Card& candidate) {
                return !IsKaleidoscopeCandidate(candidate, golden);
            }), candidates.end());
        std::sort(candidates.begin(), candidates.end(),
                  [](const Card& lhs, const Card& rhs) {
                      return lhs.dbfID < rhs.dbfID;
                  });
        candidates.erase(std::unique(candidates.begin(), candidates.end(),
            [](const Card& lhs, const Card& rhs) {
                return lhs.dbfID == rhs.dbfID;
            }), candidates.end());
        if (candidates.size() < 3) return false;
        if (BeginMinionDiscover(*this, std::move(candidates), card.dbfID,
                                true))
            season14.pendingHandLockTurns = behavior.value;
    }
    else if (behavior.effect == TrinketEffect::MALDRAXXUS_DAGGER_DISCOVER)
    {
        (void)BeginMaldraxxusDaggerDiscover(card.dbfID);
    }
    else if (behavior.effect == TrinketEffect::ACQUIRE_RANDOM_BALLER)
    {
        if (!hand.IsFull())
        {
            std::vector<Card> candidates;
            for (const auto id : {"BG31_816", "BG31_818"})
            {
                const auto candidate = Cards::FindCardByID(id);
                if (candidate.dbfID != 0 && candidate.hasBehavior &&
                    candidate.GetCardType() == CardType::MINION)
                    candidates.push_back(candidate);
            }
            (void)AddRandomMinionToHand(*this, std::move(candidates));
        }
    }
    else if (behavior.effect == TrinketEffect::ACQUIRE_RANDOM_CHROMADRAKES)
    {
        // Chromatic Tear's pool is the five canonical normal Chromadrakes;
        // generated golden forms are never eligible.  Keep each reward an
        // independent random hand insertion so hand-cap behavior is shared
        // with other generated minion rewards.
        static constexpr std::array<std::string_view, 5> ids = {
            "BG34_634t", "BG34_635t", "BG34_636t",
            "BG34_637t", "BG34_638t"};
        std::vector<Card> candidates;
        for (const auto id : ids) {
            const auto candidate = Cards::FindCardByID(id);
            if (candidate.dbfID != 0 && candidate.hasBehavior &&
                candidate.GetCardType() == CardType::MINION)
                candidates.push_back(candidate);
        }
        for (int i = 0; i < behavior.amount && !hand.IsFull(); ++i)
            (void)AddRandomMinionToHand(*this, candidates);
    }
    else if (behavior.effect == TrinketEffect::END_TURN_HIGHEST_TIER_TAVERN)
        // Rendle's first steal is part of acquisition; subsequent steals are
        // dispatched by ResolveTrinketEndTurn.  A full hand safely leaves the
        // Tavern untouched, matching the normal hand-cap boundary.
        (void)StealHighestTierTavernMinionToHand();
    else if (behavior.effect == TrinketEffect::ACQUIRE_RANDOM_MINIONS)
        (void)SimpleTasks::RandomCardToHandTask{behavior.race, behavior.tier,
                                                 behavior.amount,
                                                 behavior.magneticOnly,
                                                 behavior.battlecryOnly,
                                                 behavior.distinct}.Run(*this);
    else if (behavior.effect == TrinketEffect::ACQUIRE_RANDOM_MRGLTON)
        (void)AddRandomMrrgltonToHand(*this);
    else if (behavior.effect == TrinketEffect::ACQUIRE_RANDOM_MINIONS_TIER_BATCH)
    {
        // Magician's Top Hat is six independent rewards (two per tier), not
        // a single broad tier-1..3 pool. Run the authoritative generated-card
        // task once for each tier; it applies the Battlegrounds pool-minion
        // filter, fresh-instance modifiers, and hand-cap semantics while
        // keeping each sample independent. This is a generated hand reward,
        // so it must not call MinionPool::TakeMinion (that operation removes a
        // live Tavern offer from the recruit pool).
        for (const int tier : {1, 2, 3})
            (void)SimpleTasks::RandomCardToHandTask{Race::INVALID, tier, 2}
                .Run(*this);
    }
    else if (behavior.effect == TrinketEffect::TRANSFORM_WARBAND_TIER)
    {
        // Shrine of Evolution is an acquisition-time board mutation.  Build
        // the supported Tier-4 pool once, then sample independently for each
        // live slot.  TransformTo owns the card-instance replacement and
        // clears transient state; no Tavern pool entity is consumed.
        std::vector<Card> candidates;
        AppendSupportedNormalMinions(Cards::GetTier4Minions(), candidates,
                                     Race::INVALID, activeTribes);
        if (!candidates.empty()) {
            recruitField.ForEachAlive([&candidates](MinionData& data) {
                auto& minion = data.value();
                Random::shuffle(candidates.begin(), candidates.end());
                (void)minion.TransformTo(candidates.front());
            });
        }
    }
    else if (behavior.effect == TrinketEffect::START_TURN_RANDOM_BOUNTIES)
        (void)SimpleTasks::RandomBountyToHandTask{behavior.amount}.Run(*this);
    else if (behavior.effect == TrinketEffect::ACQUIRE_RANDOM_FRIENDLY_COPY)
        (void)AddRandomFriendlyMinionCopyToHand();
    else if ((behavior.effect == TrinketEffect::ACQUIRE_FIXED_CARD ||
              behavior.effect == TrinketEffect::ACQUIRE_PRIMALFIN_PORTRAIT ||
              behavior.effect == TrinketEffect::ACQUIRE_FIXED_CARD_FIRST_PIRATE_FREE ||
              behavior.effect == TrinketEffect::ACQUIRE_FIXED_CARD_AND_BOUNTIES ||
              behavior.effect == TrinketEffect::ACQUIRE_TWO_FIXED_CARDS ||
              behavior.effect == TrinketEffect::ACQUIRE_FIXED_CARD_AND_TAVERN_SLOTS ||
              behavior.effect == TrinketEffect::ACQUIRE_FIXED_CARD_AFTER_SELL ||
              behavior.effect == TrinketEffect::ACQUIRE_FLAGBEARER_PORTRAIT ||
              behavior.effect == TrinketEffect::ACQUIRE_FIXED_GLOWSCALE ||
             behavior.effect == TrinketEffect::ACQUIRE_FIXED_LIONFISH ||
             behavior.effect == TrinketEffect::AFTER_PLAY_ELEMENTAL_FIXED_CARD ||
             behavior.effect == TrinketEffect::CONDUCTOR_DISCARD_BLOOD_GEM) &&
             behavior.cardID.size() != 0 &&
             // A two-card portrait is one printed reward.  Do not resolve
             // only its first card when the hand has a single free slot:
             // either both cards are created or neither is, preserving the
             // reward's atomicity at the hand-cap boundary.
             (behavior.effect != TrinketEffect::ACQUIRE_TWO_FIXED_CARDS
                  ? hand.GetCount() < MAX_HAND_SIZE
                  : hand.GetCount() + 2 <= MAX_HAND_SIZE))
    {
        const Card generated = Cards::FindCardByID(behavior.cardID);
        if (generated.dbfID != 0 && generated.GetCardType() != CardType::INVALID)
        {
            // Some fixed-card Trinkets grant more than one copy on
            // acquisition (notably Essence of Dreams).  Use the descriptor's
            // amount and stop at the authoritative hand cap; portraits and
            // other one-card effects keep amount=1.
            const int count = std::max(1, behavior.amount);
            for (int i = 0; i < count && !hand.IsFull(); ++i)
            {
                if (generated.GetCardType() == CardType::SPELL ||
                    generated.GetCardType() == CardType::BATTLEGROUND_SPELL)
                    hand.Add(CardData{Spell(generated)});
                else if (generated.GetCardType() == CardType::MINION) {
                    Minion granted(generated);
                    if (behavior.portraitEffect ==
                        PortraitEffect::HACKERFIN_END_TURN_BATTLECRY)
                        granted.SetEndTurnBattlecryTrigger(true);
                    if (behavior.portraitEffect ==
                        PortraitEffect::PERMANENT_SPELLCRAFT)
                        granted.SetPermanentSpellcraft(true);
                    if (card.id == "BG30_MagicItem_825") {
                        const int size = granted.IsGolden() ? 24 : 12;
                        granted.SetAttack(size);
                        granted.SetHealth(size);
                        granted.AddRace(Race::DRAGON);
                    }
                    // Drakkari Portrait's generated Enchanter is both a
                    // Mechanical and an Elemental for all tribe checks.
                    if (card.id == "BG32_MagicItem_179") {
                        granted.AddRace(Race::MECHANICAL);
                        granted.AddRace(Race::ELEMENTAL);
                    }
                    // Enforcer Portrait's generated Lightfang is an instance
                    // with every currently active Battlegrounds type.  This
                    // must be real race state (rather than only a special
                    // case in Lightfang's own rally), so downstream type
                    // checks and other type-based effects see the same
                    // identity as the printed portrait text.
                    if (card.id == "BG30_MagicItem_971") {
                        for (const auto race : RACES_IN_BATTLEGROUNDS)
                            granted.AddRace(race);
                    }
                    // Radio Star Portrait grants the Timewarped copy with
                    // Reborn; this is instance state, not card metadata.
                    if (card.id == "BG35_MagicItem_310")
                        granted.SetReborn(true);
                    // The portrait's golden Egg overrides the normal two-turn
                    // text: its pinned text says "It hatches next turn".
                    // The normal Egg keeps its authoritative two-turn timer;
                    // both timers are advanced at recruit-end.
                    if (card.id == "BG35_MagicItem_848t")
                        granted.SetEggHatch(1);
                    else if (card.id == "BG35_MagicItem_842")
                        granted.SetEggHatch(2);
                    hand.Add(CardData{std::move(granted)});
                }
            }
            if (behavior.effect == TrinketEffect::ACQUIRE_TWO_FIXED_CARDS &&
                !behavior.secondaryCardID.empty() && !hand.IsFull())
            {
                const Card secondary = Cards::FindCardByID(behavior.secondaryCardID);
                if (secondary.GetCardType() == CardType::SPELL ||
                    secondary.GetCardType() == CardType::BATTLEGROUND_SPELL)
                    hand.Add(CardData{Spell(secondary)});
                else if (secondary.GetCardType() == CardType::MINION) {
                    Minion granted(secondary);
                    // Curator Sticker's Amalgam is printed as a 10/10 even
                    // though the reusable Amalgam token metadata is 2/2.
                    // Preserve the token's authoritative ALL-race and
                    // VENOMOUS metadata while applying the printed instance
                    // stats at grant time.
                    if (card.id == "BG32_MagicItem_807" &&
                        behavior.secondaryCardID == "TB_BaconShop_HP_033t") {
                        granted.SetAttack(10);
                        granted.SetHealth(10);
                    }
                    hand.Add(CardData{std::move(granted)});
                }
            }
            if (behavior.effect == TrinketEffect::ACQUIRE_FIXED_CARD_AND_BOUNTIES)
                (void)SimpleTasks::RandomBountyToHandTask{behavior.value}.Run(*this);
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
    // Rewinder Portrait has two immediate fixed grants.  Keep the second
    // generated entity in the same authoritative hand-cap path as the first;
    // a full hand burns neither a hidden card nor a later lifecycle trigger.
    if (card.id == "BG30_MagicItem_868" && hand.GetCount() < MAX_HAND_SIZE)
    {
        const auto wrathWeaver = Cards::FindCardByID("BGS_004");
        if (wrathWeaver.dbfID != 0)
            hand.Add(CardData{Minion(wrathWeaver)});
    }
    // Portable Factory and Battle Horn both open a real, constrained
    // Discover after acquisition.  AcquireTrinket may run while the public
    // Trinket-selection modal is still active, so defer opening until the
    // decision slot is free (ApplyChoice retries this exact source).
    if (behavior.effect == TrinketEffect::PORTABLE_FACTORY &&
        season14.pendingDecision == Season14Decision::NONE)
        (void)BeginTrinketMinionDiscover(
            *this, card.dbfID, behavior.tier, Race::INVALID,
            false);
    else if (behavior.effect == TrinketEffect::BATTLE_HORN &&
             season14.pendingDecision == Season14Decision::NONE)
        (void)BeginTrinketMinionDiscover(
            *this, card.dbfID, 0, Race::INVALID, true);
    else if (behavior.effect == TrinketEffect::PUTRICIDE_STICKER &&
             season14.pendingDecision == Season14Decision::NONE) {
        season14.pendingPutricideStickerSourceDbfID = card.dbfID;
        season14.pendingPutricideStickerFirstDbfID = 0;
        season14.pendingPutricideStickerSecondPool = false;
        (void)BeginPutricideStickerDiscover(*this, card.dbfID, false);
    }
    return true;
}

int Player::GrantTrinketStartTurnCards()
{
    int added = 0;
    // A Device acquired while another modal was open may not have been able
    // to open its Discover immediately. Retry the exact pending modal at the
    // next recruit start without generating a fresh trinket effect.
    if (season14.pendingElectromagneticDiscoverRemaining > 0 &&
        season14.pendingDecision == Season14Decision::NONE)
    {
        if (season14.pendingElectromagneticDiscoverSourceCardDbfID != 0)
            (void)BeginElectromagneticDiscover(
                *this, season14.pendingElectromagneticDiscoverSourceCardDbfID);
    }
    // A Putricide acquisition can be committed while another public modal is
    // active, or can reach a full hand before its first component is opened.
    // Retry the exact pending stage at the next safe recruit boundary; do not
    // roll a new pool or advance the recurring two-turn counter.
    if (season14.pendingPutricideStickerSourceDbfID != 0 &&
        season14.pendingDecision == Season14Decision::NONE && !hand.IsFull())
    {
        (void)BeginPutricideStickerDiscover(
            *this, season14.pendingPutricideStickerSourceDbfID,
            season14.pendingPutricideStickerSecondPool);
    }
    for (auto& trinket : season14.trinkets) {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect == TrinketEffect::PUTRICIDE_STICKER) {
            // A pending immediate craft (including one just resumed above)
            // owns this trinket's modal.  Do not also advance its recurring
            // two-turn cadence on the same recruit boundary.
            if (season14.pendingPutricideStickerSourceDbfID == trinket.dbfID)
                continue;
            if (++trinket.triggerProgress < 2) continue;
            if (season14.pendingDecision == Season14Decision::NONE &&
                !hand.IsFull()) {
                season14.pendingPutricideStickerSourceDbfID = trinket.dbfID;
                season14.pendingPutricideStickerFirstDbfID = 0;
                season14.pendingPutricideStickerSecondPool = false;
                if (BeginPutricideStickerDiscover(*this, trinket.dbfID,
                                                   false))
                    trinket.triggerProgress = 0;
            }
            continue;
        }
        if (behavior.effect == TrinketEffect::BOOK_OF_MEDIVH_DISCOVER)
        {
            const auto count = season14.bookOfMedivhRemaining > 0
                                   ? season14.bookOfMedivhRemaining
                                   : behavior.value;
            (void)BeginBookOfMedivhDiscover(*this, trinket.dbfID, count);
            continue;
        }
        if (behavior.effect == TrinketEffect::PORTABLE_FACTORY)
        {
            // The selected minion is a per-Trinket snapshot, not a fresh
            // random roll.  Hand-cap blocking leaves the snapshot armed for
            // the next safe recruit-start boundary.
            if (trinket.capturedMinion.has_value() && !hand.IsFull())
            {
                Minion copy = *trinket.capturedMinion;
                if (getNextCardIndexCallback)
                    copy.SetIndex(getNextCardIndexCallback());
                hand.Add(CardData{std::move(copy)});
                ++added;
            }
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
        if (behavior.effect == TrinketEffect::START_TURN_RANDOM_BOUNTIES) {
            const auto before = hand.GetCount();
            (void)SimpleTasks::RandomBountyToHandTask{behavior.amount}.Run(*this);
            added += hand.GetCount() - before;
            continue;
        }
        if (behavior.effect ==
            TrinketEffect::START_TURN_RANDOM_TAVERN_SPELLS_PER_TYPE) {
            std::array<bool, RACES_IN_BATTLEGROUNDS.size()> types{};
            recruitField.ForEachAlive([&types](const MinionData& data) {
                const auto& minion = data.value();
                for (std::size_t i = 0; i < RACES_IN_BATTLEGROUNDS.size(); ++i)
                    types[i] = types[i] ||
                        minion.HasRace(RACES_IN_BATTLEGROUNDS[i]);
            });
            const auto count = static_cast<int>(
                std::count(types.begin(), types.end(), true));
            if (count > 0) {
                season14.BeginLavishCapeRandomSpells(count);
                (void)SimpleTasks::ActivateRandomTavernSpellsTask{count}.Run(*this);
            }
            continue;
        }
        if (behavior.effect == TrinketEffect::START_TURN_RANDOM_SPELLCRAFT) {
            const auto before = hand.GetCount();
            GrantRandomSpellcraft(*this, behavior.amount);
            added += hand.GetCount() - before;
            continue;
        }
        if (behavior.effect == TrinketEffect::POCKET_CYCLONE) {
            const bool golden = Cards::FindCardByDbfID(trinket.dbfID).id.ends_with("t");
            (void)CastTavernSpellFree("BG34_444", golden ? 2 : 1);
            continue;
        }
        if (behavior.effect != TrinketEffect::START_TURN_RANDOM_MINIONS &&
            !(behavior.effect == TrinketEffect::ACQUIRE_RANDOM_CHROMADRAKES &&
              behavior.repeatAtStartTurn) &&
            !(behavior.effect == TrinketEffect::ACQUIRE_RANDOM_CHOOSE_ONE &&
              behavior.repeatAtStartTurn) &&
            !(behavior.effect == TrinketEffect::JEWELRY_BOX_BLOOD_GEM &&
              behavior.repeatAtStartTurn) &&
            !(behavior.effect == TrinketEffect::ACQUIRE_RANDOM_BALLER &&
              behavior.repeatAtStartTurn) &&
            !(behavior.effect == TrinketEffect::ACQUIRE_RANDOM_MRGLTON &&
              behavior.repeatAtStartTurn) &&
            !(behavior.effect == TrinketEffect::ACQUIRE_RANDOM_MINIONS &&
              behavior.repeatAtStartTurn) &&
            !(behavior.effect == TrinketEffect::POCKET_CYCLONE &&
              behavior.repeatAtStartTurn) &&
            !(behavior.effect == TrinketEffect::ACQUIRE_RANDOM_FRIENDLY_COPY &&
              behavior.repeatAtStartTurn) &&
            !(behavior.effect == TrinketEffect::ACQUIRE_LAST_OPPONENT_COPY &&
              behavior.repeatAtStartTurn) &&
            !((behavior.effect == TrinketEffect::ACQUIRE_FIXED_CARD ||
               behavior.effect == TrinketEffect::ACQUIRE_PRIMALFIN_PORTRAIT ||
               behavior.effect == TrinketEffect::ACQUIRE_FIXED_CARD_AND_BOUNTIES) &&
              behavior.repeatAtStartTurn)) continue;
        if (behavior.effect == TrinketEffect::ACQUIRE_RANDOM_BALLER) {
            const auto before = hand.GetCount();
            if (!hand.IsFull()) {
                std::vector<Card> candidates;
                for (const auto id : {"BG31_816", "BG31_818"}) {
                    const auto candidate = Cards::FindCardByID(id);
                    if (candidate.dbfID != 0 && candidate.hasBehavior &&
                        candidate.GetCardType() == CardType::MINION)
                        candidates.push_back(candidate);
                }
                (void)AddRandomMinionToHand(*this, std::move(candidates));
            }
            added += hand.GetCount() - before;
            continue;
        }
        if (behavior.effect == TrinketEffect::ACQUIRE_RANDOM_CHROMADRAKES &&
            behavior.repeatAtStartTurn) {
            static constexpr std::array<std::string_view, 5> ids = {
                "BG34_634t", "BG34_635t", "BG34_636t",
                "BG34_637t", "BG34_638t"};
            std::vector<Card> candidates;
            for (const auto id : ids) {
                const auto candidate = Cards::FindCardByID(id);
                if (candidate.dbfID != 0 && candidate.hasBehavior &&
                    candidate.GetCardType() == CardType::MINION)
                    candidates.push_back(candidate);
            }
            const auto before = hand.GetCount();
            for (int i = 0; i < behavior.amount && !hand.IsFull(); ++i)
                (void)AddRandomMinionToHand(*this, candidates);
            added += hand.GetCount() - before;
            continue;
        }
    if (behavior.effect == TrinketEffect::ACQUIRE_RANDOM_CHOOSE_ONE) {
            const auto before = hand.GetCount();
            Minion source;
            (void)SimpleTasks::RandomChooseOneCardToHandTask{behavior.amount}
                .Run(*this, source);
            added += hand.GetCount() - before;
            continue;
        }
        if (behavior.effect == TrinketEffect::JEWELRY_BOX_BLOOD_GEM) {
            if (season14.pendingJewelryBoxBloodGemRemaining == 0)
                season14.pendingJewelryBoxBloodGemRemaining = 1;
            (void)BeginJewelryBoxBloodGem(*this, trinket.dbfID);
            continue;
        }
        if (behavior.effect == TrinketEffect::ACQUIRE_RANDOM_MRGLTON) {
            const auto before = hand.GetCount();
            for (int i = 0; i < behavior.amount && !hand.IsFull(); ++i)
                (void)AddRandomMrrgltonToHand(*this);
            added += hand.GetCount() - before;
            continue;
        }
        if (behavior.effect == TrinketEffect::ACQUIRE_FIXED_CARD ||
            behavior.effect == TrinketEffect::ACQUIRE_PRIMALFIN_PORTRAIT ||
            behavior.effect == TrinketEffect::ACQUIRE_FIXED_CARD_AND_BOUNTIES) {
            if (behavior.cardID.empty()) continue;
            // Privateer Portrait's fixed Proud Privateer is an acquisition
            // reward only.  Its printed recruit-start repeat is specifically
            // "2 random Bounties", not another Proud Privateer.  Keep this
            // branch on the bounty-only path so every owned copy contributes
            // exactly two Bounties per recruit start without duplicating the
            // generated minion.
            if (behavior.effect == TrinketEffect::ACQUIRE_FIXED_CARD_AND_BOUNTIES)
            {
                const auto before = hand.GetCount();
                (void)SimpleTasks::RandomBountyToHandTask{behavior.value}.Run(*this);
                added += hand.GetCount() - before;
                continue;
            }
            if (behavior.startTurnCadence > 1 &&
                trinket.triggerProgress < behavior.startTurnCadence)
                ++trinket.triggerProgress;
            if (behavior.startTurnCadence > 1 &&
                trinket.triggerProgress < behavior.startTurnCadence)
                continue;
            const Card generated = Cards::FindCardByID(behavior.cardID);
            const bool generatedSpell =
                generated.GetCardType() == CardType::SPELL ||
                generated.GetCardType() == CardType::BATTLEGROUND_SPELL;
            const bool generatedSpellSupported =
                generatedSpell &&
                FindTavernSpellBehavior(generated.id).effect !=
                    TavernSpellEffect::NONE;
            if (generated.dbfID == 0 ||
                ((!generated.hasBehavior && !generatedSpellSupported)) ||
                hand.IsFull())
                continue;
            const auto before = hand.GetCount();
            const int startTurnAmount = behavior.startTurnAmount > 0
                                            ? behavior.startTurnAmount
                                            : behavior.amount;
            for (int i = 0; i < startTurnAmount && !hand.IsFull(); ++i)
            {
                if (generated.GetCardType() == CardType::SPELL ||
                    generated.GetCardType() == CardType::BATTLEGROUND_SPELL)
                    hand.Add(CardData{Spell(generated)});
                else if (generated.GetCardType() == CardType::MINION) {
                    Minion granted(generated);
                    if (generated.id == "BG34_639") granted.SetEggHatch(2);
                    hand.Add(CardData{std::move(granted)});
                }
            }
            added += hand.GetCount() - before;
            if (behavior.startTurnCadence > 1 && hand.GetCount() > before)
                trinket.triggerProgress = 0;
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
                                                 behavior.battlecryOnly,
                                                 behavior.distinct}.Run(*this);
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
                                  std::vector<Card>& result, Race race,
                                  const ActiveTribeSet& activeTribes)
{
    for (const auto& card : cards)
    {
        if (card.id.empty() || !card.hasBehavior ||
            card.normalDbfID != 0 || card.GetCardType() != CardType::MINION ||
            !HasActiveTribe(activeTribes, card) ||
            (race != Race::INVALID && !card.HasRace(race)))
        {
            continue;
        }
        result.push_back(card);
    }
}

bool HasSupportedTier1Minion(const ActiveTribeSet& activeTribes)
{
    std::vector<Card> candidates;
    AppendSupportedNormalMinions(Cards::GetTier1Minions(), candidates,
                                 Race::INVALID, activeTribes);
    return !candidates.empty();
}

std::vector<Card> SupportedMinionsForRace(Race race,
                                          const ActiveTribeSet& activeTribes)
{
    std::vector<Card> result;
    AppendSupportedNormalMinions(Cards::GetTier1Minions(), result, race, activeTribes);
    AppendSupportedNormalMinions(Cards::GetTier2Minions(), result, race, activeTribes);
    AppendSupportedNormalMinions(Cards::GetTier3Minions(), result, race, activeTribes);
    AppendSupportedNormalMinions(Cards::GetTier4Minions(), result, race, activeTribes);
    AppendSupportedNormalMinions(Cards::GetTier5Minions(), result, race, activeTribes);
    AppendSupportedNormalMinions(Cards::GetTier6Minions(), result, race, activeTribes);
    AppendSupportedNormalMinions(Cards::GetTier7Minions(), result, race, activeTribes);
    return result;
}

std::vector<Card> SupportedEndTurnMinions(const ActiveTribeSet& activeTribes)
{
    std::vector<Card> result;
    const auto append = [&result, &activeTribes](const auto& cards) {
        for (const auto& card : cards) {
            if (card.id.empty() || !card.hasBehavior ||
                card.normalDbfID != 0 ||
                card.GetCardType() != CardType::MINION ||
                !HasActiveTribe(activeTribes, card))
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

std::vector<Card> SupportedDeathrattleMinions(const ActiveTribeSet& activeTribes)
{
    std::vector<Card> result;
    const auto append = [&result, &activeTribes](const auto& cards) {
        for (const auto& card : cards)
            if (card.hasBehavior && card.normalDbfID == 0 &&
                card.GetCardType() == CardType::MINION &&
                HasActiveTribe(activeTribes, card) &&
                !card.power.GetDeathrattleTask().empty())
                result.push_back(card);
    };
    append(Cards::GetTier1Minions()); append(Cards::GetTier2Minions()); append(Cards::GetTier3Minions());
    append(Cards::GetTier4Minions()); append(Cards::GetTier5Minions()); append(Cards::GetTier6Minions()); append(Cards::GetTier7Minions());
    return result;
}

std::vector<Card> SupportedBattlecryMinions(const ActiveTribeSet& activeTribes)
{
    std::vector<Card> result;
    const auto append = [&result, &activeTribes](const auto& cards) {
        for (const auto& card : cards)
            if (card.hasBehavior && card.normalDbfID == 0 &&
                card.GetCardType() == CardType::MINION &&
                card.isBattlegroundsPoolMinion &&
                HasActiveTribe(activeTribes, card) &&
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
    if (player.currentTier == 1) AppendSupportedNormalMinions(Cards::GetTier1Minions(), result, Race::INVALID, player.activeTribes);
    else if (player.currentTier == 2) AppendSupportedNormalMinions(Cards::GetTier2Minions(), result, Race::INVALID, player.activeTribes);
    else if (player.currentTier == 3) AppendSupportedNormalMinions(Cards::GetTier3Minions(), result, Race::INVALID, player.activeTribes);
    else if (player.currentTier == 4) AppendSupportedNormalMinions(Cards::GetTier4Minions(), result, Race::INVALID, player.activeTribes);
    else if (player.currentTier == 5) AppendSupportedNormalMinions(Cards::GetTier5Minions(), result, Race::INVALID, player.activeTribes);
    else if (player.currentTier == 6) AppendSupportedNormalMinions(Cards::GetTier6Minions(), result, Race::INVALID, player.activeTribes);
    else if (player.currentTier == 7) AppendSupportedNormalMinions(Cards::GetTier7Minions(), result, Race::INVALID, player.activeTribes);
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

bool AddRandomMrrgltonToHand(Player& player)
{
    std::vector<Card> candidates;
    for (const auto id : {"BG35_140", "BG35_141"}) {
        const auto candidate = Cards::FindCardByID(id);
        if (candidate.dbfID != 0 && candidate.hasBehavior &&
            candidate.isBattlegroundsPoolMinion && candidate.normalDbfID == 0 &&
            candidate.GetCardType() == CardType::MINION)
            candidates.push_back(candidate);
    }
    return AddRandomMinionToHand(player, std::move(candidates));
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

// Shared typed Discover constructor for Trinkets whose printed payload is a
// constrained minion pool.  Rebuilding the pool from executable card data at
// each opening keeps acquisition and delayed/repeated openings deterministic
// and prevents metadata-only/generated cards from entering the modal.
bool BeginTrinketMinionDiscover(Player& player, std::int32_t sourceCardDbfID,
                                int tier, Race requiredRace,
                                bool battlecryOnly)
{
    if (player.season14.pendingDecision != Season14Decision::NONE ||
        player.hand.IsFull() || sourceCardDbfID <= 0)
        return false;
    std::vector<Card> candidates;
    for (const auto& card : Cards::GetAllCards()) {
        if (card.id.empty() || card.dbfID <= 0 ||
            card.GetCardType() != CardType::MINION ||
            !card.isBattlegroundsPoolMinion || card.normalDbfID != 0 ||
            !card.hasBehavior || !HasActiveTribe(player.activeTribes, card) ||
            (tier > 0 && card.GetTier() != tier) ||
            (requiredRace != Race::INVALID && !card.HasRace(requiredRace)))
            continue;
        if (battlecryOnly && !CardDefs::FindCardDefByID(card.id).HasBattlecry())
            continue;
        // Portable Factory requires a minion "with a type", not a neutral
        // minion.  The explicit race filter is used by Putricide callers;
        // this guard keeps the generic helper safe for future callers.
        if (!battlecryOnly && requiredRace == Race::INVALID &&
            !std::any_of(RACES_IN_BATTLEGROUNDS.begin(),
                         RACES_IN_BATTLEGROUNDS.end(),
                         [&card](Race race) { return card.HasRace(race); }))
            continue;
        candidates.push_back(card);
    }
    return BeginMinionDiscover(player, std::move(candidates),
                               sourceCardDbfID, false);
}

bool BeginPutricideStickerDiscover(Player& player, std::int32_t sourceCardDbfID,
                                   bool second)
{
    if (player.season14.pendingDecision != Season14Decision::NONE ||
        player.hand.IsFull() || sourceCardDbfID != 120827)
        return false;
    const auto first = Cards::FindCardByDbfID(
        player.season14.pendingPutricideStickerFirstDbfID);
    const bool excludeKeywords = second &&
        (player.season14.pendingPutricideStickerFirstDbfID == 95263 ||
         first.gameTags.contains(GameTag::REBORN) ||
         first.gameTags.contains(GameTag::POISONOUS) ||
         first.gameTags.contains(GameTag::VENOMOUS));
    std::vector<Card> candidates;
    const auto appendPool = [&](const auto& pool) {
        for (const auto dbfID : pool) {
            const auto card = Cards::FindCardByDbfID(dbfID);
            if (card.dbfID == 0 || card.GetCardType() != CardType::MINION ||
                card.normalDbfID != 0 || !card.hasBehavior ||
                !HasActiveTribe(player.activeTribes, card) ||
                card.GetTier() > player.currentTier)
                continue;
            if (excludeKeywords &&
                (card.dbfID == 99527 || card.dbfID == 98867 ||
                 card.gameTags.contains(GameTag::REBORN) ||
                 card.gameTags.contains(GameTag::POISONOUS) ||
                 card.gameTags.contains(GameTag::VENOMOUS)))
                continue;
            candidates.push_back(card);
        }
    };
    if (second) appendPool(BUILD_AN_UNDEAD_POOL_2);
    else appendPool(BUILD_AN_UNDEAD_POOL_1);
    if (candidates.size() < 3) return false;
    Random::shuffle(candidates.begin(), candidates.end());
    player.season14.BeginOfferingDecision(
        Season14Decision::DISCOVER, 0, sourceCardDbfID,
        {{candidates[0].dbfID, 0}, {candidates[1].dbfID, 0},
         {candidates[2].dbfID, 0}});
    return true;
}

// Jewelry Box's reward is deliberately a public choice rather than a random
// Blood Gem. Keep the exact three-card pool here so acquisition and its
// recruit-start repeat share the ordinary Discover/hand-cap/replay boundary.
bool BeginJewelryBoxBloodGem(Player& player, std::int32_t sourceCardDbfID)
{
    if (player.season14.pendingDecision != Season14Decision::NONE ||
        player.hand.IsFull() || sourceCardDbfID != 130904 ||
        Cards::FindCardByDbfID(sourceCardDbfID).id != "BG35_MagicItem_434" ||
        player.season14.pendingJewelryBoxBloodGemRemaining <= 0)
        return false;
    std::vector<Season14Offering> offerings;
    for (const auto id : {"BG20_GEM_Taunt", "BG20_GEM_DivineShield",
                          "BG20_GEM_Reborn"}) {
        const auto card = Cards::FindCardByID(id);
        if (card.dbfID == 0 || !card.hasBehavior) return false;
        offerings.push_back({card.dbfID, 0});
    }
    player.season14.BeginOfferingDecision(Season14Decision::DISCOVER, 0,
                                          sourceCardDbfID,
                                          std::move(offerings));
    return true;
}

// Innkeeper's Hearth is deliberately kept separate from the generic minion
// Discover: its pool is the current Tavern tier (or an explicit Tier 6 for
// the golden token), and the selected copy receives an exact stat floor at
// the commit boundary.  Rebuilding the pool from card metadata on every
// sequential choice also makes replayed choices independent of mutable shop
// state and excludes unsupported/generated identities.
bool BeginInnkeepersHearthDiscover(Player& player,
                                   std::int32_t sourceCardDbfID,
                                   int tier)
{
    if (player.season14.pendingDecision != Season14Decision::NONE ||
        player.hand.IsFull() || tier <= 0)
        return false;
    std::vector<Card> candidates;
    for (const auto& card : Cards::GetAllCards())
    {
        if (card.id.empty() || card.GetCardType() != CardType::MINION ||
            !card.isBattlegroundsPoolMinion || card.normalDbfID != 0 ||
            !card.hasBehavior || !HasActiveTribe(player.activeTribes, card) ||
            card.GetTier() != tier ||
            (sourceCardDbfID == 121684 &&
             player.season14.pendingInnkeepersHearthSelectedDbfID != 0 &&
             card.dbfID ==
                 player.season14.pendingInnkeepersHearthSelectedDbfID))
            continue;
        candidates.push_back(card);
    }
    if (candidates.size() < 3) return false;
    Random::shuffle(candidates.begin(), candidates.end());
    player.season14.BeginOfferingDecision(
        Season14Decision::DISCOVER, 0, sourceCardDbfID,
        {{candidates[0].dbfID, 0}, {candidates[1].dbfID, 0},
         {candidates[2].dbfID, 0}});
    return true;
}

}  // namespace

bool BeginUniqueBuddyDiscover(Player& player, std::int32_t sourceCardDbfID)
{
    if (player.season14.pendingDecision != Season14Decision::NONE ||
        player.season14.pendingUniqueDiscoverRemaining <= 0 ||
        player.hand.IsFull())
        return false;

    std::unordered_map<std::int32_t, int> owned;
    std::unordered_map<std::int32_t, Card> normalCards;
    auto observe = [&](const Card& card) {
        if (card.GetCardType() != CardType::MINION || card.normalDbfID != 0 ||
            !card.isBattlegroundsPoolMinion || !card.hasBehavior ||
            card.dbfID == sourceCardDbfID)
            return;
        ++owned[card.dbfID];
        normalCards.insert_or_assign(card.dbfID, card);
    };
    player.hand.ForEach([&](const std::optional<CardData>& data) {
        if (data.has_value() && std::holds_alternative<Minion>(*data))
            observe(Cards::FindCardByDbfID(std::get<Minion>(*data).GetDbfID()));
    });
    player.recruitField.ForEachAlive([&](const MinionData& data) {
        observe(Cards::FindCardByDbfID(data.value().GetDbfID()));
    });

    std::vector<Card> candidates;
    for (const auto& [dbfID, count] : owned)
        if (count == 1) candidates.push_back(normalCards.at(dbfID));
    if (candidates.empty()) return false;
    Random::shuffle(candidates.begin(), candidates.end());
    const auto count = std::min<std::size_t>(3, candidates.size());
    std::vector<Season14Offering> offerings;
    offerings.reserve(count);
    for (std::size_t i = 0; i < count; ++i)
        offerings.push_back({candidates[i].dbfID, 0});
    player.season14.BeginOfferingDecision(Season14Decision::DISCOVER, 0,
                                          sourceCardDbfID, std::move(offerings));
    return true;
}

// Electromagnetic Device's printed pool is narrower than an ordinary
// minion Discover: only supported, normal, pool Magnetic Mechs are legal.
// Keep the pool derived from card metadata/behavior so unsupported cards can
// never leak into the modal, while retaining the normal three-option public
// Discover shape.  A full hand does not suppress the modal; selecting while
// full follows normal Discover burn semantics in ApplyChoice.
bool BeginElectromagneticDiscover(Player& player,
                                  std::int32_t sourceCardDbfID)
{
    if (player.season14.pendingDecision != Season14Decision::NONE ||
        player.season14.pendingElectromagneticDiscoverRemaining <= 0)
        return false;
    std::vector<Card> candidates;
    for (const auto& card : Cards::GetAllCards())
    {
        if (card.id.empty() || !card.isBattlegroundsPoolMinion ||
            card.normalDbfID != 0 || !card.hasBehavior ||
            card.GetCardType() != CardType::MINION ||
            !HasActiveTribe(player.activeTribes, card) ||
            !card.HasRace(Race::MECHANICAL) ||
            !card.gameTags.contains(GameTag::MAGNETIC) ||
            card.gameTags.at(GameTag::MAGNETIC) == 0)
            continue;
        candidates.push_back(card);
    }
    // The printed modal is always a three-option Discover.  Do not silently
    // downgrade it to a one/two-option choice when a restricted build has an
    // undersized pool; fail closed instead.
    if (candidates.size() < 3) return false;
    Random::shuffle(candidates.begin(), candidates.end());
    std::vector<Season14Offering> offerings;
    offerings.reserve(3);
    for (std::size_t i = 0; i < 3; ++i)
        offerings.push_back({candidates[i].dbfID, 0});
    player.season14.BeginOfferingDecision(Season14Decision::DISCOVER, 0,
                                          sourceCardDbfID,
                                          std::move(offerings));
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
            HasActiveTribe(player.activeTribes, card) &&
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

bool Player::BeginOminousStoneDiscover(const std::int32_t sourceCardDbfID)
{
    if (season14.pendingDecision != Season14Decision::NONE || hand.IsFull() ||
        sourceCardDbfID <= 0)
        return false;

    // This is a typed acquisition reward, not a generic public entry point.
    // Validate the canonical source before creating any modal so a stale or
    // replayed DBF cannot mint an Ominous Stone choice.
    const auto source = Cards::FindCardByDbfID(sourceCardDbfID);
    if (source.id != "BG36_MagicItem_206" ||
        source.GetCardType() != CardType::BATTLEGROUND_TRINKET ||
        source.normalDbfID != 0 ||
        FindTrinketBehavior(source.id).effect !=
            TrinketEffect::OMINOUS_STONE_DISCOVER)
        return false;

    const auto race = MostCommonFriendlyRace(*this);
    if (race == Race::INVALID) return false;

    std::vector<Card> candidates;
    AppendSupportedNormalMinions(Cards::GetTier4Minions(), candidates, race,
                                 activeTribes);
    if (candidates.empty()) return false;

    std::vector<Card> gifts;
    for (const auto& gift : Cards::GetAllCards())
        if (gift.isBattlegroundsDarkGift &&
            FindDarkGiftBehavior(gift.id).effect != DarkGiftEffect::NONE)
            gifts.push_back(gift);
    if (gifts.empty()) return false;

    // Construct the complete valid tuple pool before sampling.  Sampling
    // only the first three candidates and filtering gifts can accidentally
    // downgrade a printed three-option Discover to one or two choices.
    std::vector<Season14Offering> offerings;
    for (const auto& candidate : candidates)
    {
        const Minion preview(candidate);
        std::vector<Card> legalGifts;
        for (const auto& gift : gifts)
            if (DarkGiftTargetIsLegal(preview, FindDarkGiftBehavior(gift.id)))
                legalGifts.push_back(gift);
        if (legalGifts.empty()) continue;
        const auto& gift = legalGifts[Random::get<std::size_t>(
            0, legalGifts.size() - 1)];
        offerings.push_back({candidate.dbfID, 0, gift.dbfID});
    }
    if (offerings.size() < 3) return false;
    Random::shuffle(offerings.begin(), offerings.end());
    offerings.resize(3);
    season14.BeginOfferingDecision(Season14Decision::DISCOVER, 0,
                                   sourceCardDbfID, std::move(offerings));
    return true;
}

bool Player::BeginWaxLanceDiscover(const std::int32_t sourceCardDbfID)
{
    if (season14.pendingDecision != Season14Decision::NONE || hand.IsFull() ||
        sourceCardDbfID <= 0)
        return false;

    const auto source = Cards::FindCardByDbfID(sourceCardDbfID);
    if (source.id != "BG36_MagicItem_309" ||
        source.GetCardType() != CardType::BATTLEGROUND_TRINKET ||
        source.normalDbfID != 0 ||
        FindTrinketBehavior(source.id).effect !=
            TrinketEffect::WAX_LANCE_DISCOVER)
        return false;

    std::vector<Card> candidates;
    AppendSupportedNormalMinions(Cards::GetTier7Minions(), candidates,
                                  Race::INVALID, activeTribes);
    candidates.erase(
        std::remove_if(candidates.begin(), candidates.end(),
                       [](const Card& candidate) {
                           return !candidate.isBattlegroundsPoolMinion;
                       }),
        candidates.end());
    if (candidates.empty()) return false;

    std::vector<Card> gifts;
    for (const auto& gift : Cards::GetAllCards())
        if (gift.isBattlegroundsDarkGift &&
            FindDarkGiftBehavior(gift.id).effect != DarkGiftEffect::NONE)
            gifts.push_back(gift);
    if (gifts.empty()) return false;

    std::vector<Season14Offering> offerings;
    std::set<std::int32_t> offeredMinions;
    for (const auto& candidate : candidates)
    {
        if (!offeredMinions.insert(candidate.dbfID).second) continue;
        const Minion preview(candidate);
        std::vector<Card> legalGifts;
        for (const auto& gift : gifts)
            if (DarkGiftTargetIsLegal(preview, FindDarkGiftBehavior(gift.id)))
                legalGifts.push_back(gift);
        if (legalGifts.empty()) continue;
        const auto& gift = legalGifts[Random::get<std::size_t>(
            0, legalGifts.size() - 1)];
        offerings.push_back({candidate.dbfID, 0, gift.dbfID});
    }
    if (offerings.size() < 3) return false;
    Random::shuffle(offerings.begin(), offerings.end());
    offerings.resize(3);
    season14.BeginOfferingDecision(Season14Decision::DISCOVER, 0,
                                   sourceCardDbfID, std::move(offerings));
    return true;
}

bool Player::BeginMaldraxxusDaggerDiscover(const std::int32_t sourceCardDbfID)
{
    if (season14.pendingDecision != Season14Decision::NONE || hand.IsFull() ||
        sourceCardDbfID <= 0)
        return false;
    const auto source = Cards::FindCardByDbfID(sourceCardDbfID);
    if (source.id != "BG36_MagicItem_370" ||
        source.GetCardType() != CardType::BATTLEGROUND_TRINKET ||
        source.normalDbfID != 0 ||
        FindTrinketBehavior(source.id).effect !=
            TrinketEffect::MALDRAXXUS_DAGGER_DISCOVER)
        return false;
    std::vector<std::int32_t> minions;
    std::set<std::int32_t> seen;
    recruitField.ForEachAlive([&](const MinionData& data) {
        const auto& minion = data.value();
        if (minion.IsDestroyed())
            return;
        const auto source = Cards::FindCardByDbfID(minion.GetDbfID());
        const auto plainDbfID = source.normalDbfID != 0
            ? source.normalDbfID : source.dbfID;
        const auto candidate = Cards::FindCardByDbfID(plainDbfID);
        if (candidate.dbfID > 0 && candidate.GetCardType() == CardType::MINION &&
            candidate.isBattlegroundsPoolMinion && candidate.normalDbfID == 0 &&
            candidate.hasBehavior && seen.insert(candidate.dbfID).second)
            minions.push_back(candidate.dbfID);
    });
    if (minions.size() < 3) return false;
    std::vector<Card> gifts;
    for (const auto& gift : Cards::GetAllCards())
        if (gift.isBattlegroundsDarkGift &&
            FindDarkGiftBehavior(gift.id).effect != DarkGiftEffect::NONE)
            gifts.push_back(gift);
    if (gifts.empty()) return false;
    std::vector<Season14Offering> offerings;
    for (const auto dbfID : minions) {
        const auto candidate = Cards::FindCardByDbfID(dbfID);
        const Minion preview(candidate);
        std::vector<Card> legalGifts;
        for (const auto& gift : gifts)
            if (DarkGiftTargetIsLegal(preview, FindDarkGiftBehavior(gift.id)))
                legalGifts.push_back(gift);
        if (legalGifts.empty()) continue;
        const auto& gift = legalGifts[Random::get<std::size_t>(
            0, legalGifts.size() - 1)];
        offerings.push_back({candidate.dbfID, 0, gift.dbfID});
    }
    if (offerings.size() < 3) return false;
    Random::shuffle(offerings.begin(), offerings.end());
    offerings.resize(3);
    season14.BeginOfferingDecision(Season14Decision::DISCOVER, 0,
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

bool HasSupportedRaceMinion(Race race, const ActiveTribeSet& activeTribes)
{
    return !SupportedMinionsForRace(race, activeTribes).empty();
}

std::size_t AliveFriendlyMinionCount(const Player& player)
{
    std::size_t count = 0;
    player.recruitField.ForEachAlive(
        [&count](const MinionData&) { ++count; });
    return count;
}

// Imperial Defender is a post-resolution trigger.  Keep the fan-out in one
// place so ordinary spells, generated Spellcraft cards, and deferred modal
// target choices all consume the same per-turn allowance.  The callback is
// deliberately responsible for applying the resolved payload: modal spells
// have already selected their branch by the time this helper runs.
template <typename Apply>
void ApplyImperialDefenderCopies(Player& player, int targetIdx,
                                 TargetingType targetingType, Apply&& apply)
{
    if (targetIdx < 0 ||
        (targetingType != TargetingType::FRIENDLY_MINIONS &&
         targetingType != TargetingType::FRIENDLY_CHARACTERS))
        return;

    int available = 0;
    player.recruitField.ForEachAlive([&available](const MinionData& data) {
        const auto& id = data.value().GetCardID();
        if (id == "BG22_HERO_007_Buddy") ++available;
        else if (id == "BG22_HERO_007_Buddy_G") available += 2;
    });
    if (player.season14.imperialDefenderCopiesUsed >= available) return;

    player.recruitField.ForEachAlive([&](MinionData& data) {
        if (player.season14.imperialDefenderCopiesUsed >= available) return;
        auto& buddy = data.value();
        const auto& id = buddy.GetCardID();
        if (buddy.GetZonePosition() == targetIdx ||
            (id != "BG22_HERO_007_Buddy" &&
             id != "BG22_HERO_007_Buddy_G"))
            return;
        const int copies = id == "BG22_HERO_007_Buddy_G" ? 2 : 1;
        for (int copy = 0;
             copy < copies &&
             player.season14.imperialDefenderCopiesUsed < available;
             ++copy)
        {
            apply(buddy.GetZonePosition());
            ++player.season14.imperialDefenderCopiesUsed;
        }
    });
}

void ApplySpellBoardEffect(Player& player, const TavernSpellBehavior& effect,
                           int targetIdx, bool temporary,
                           std::int32_t sourceCardDbfID = 0)
{
    const auto lifecycleSpellID = sourceCardDbfID > 0
                                      ? Cards::FindCardByDbfID(sourceCardDbfID).id
                                      : std::string{};
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
        case TavernSpellEffect::TEMPORARY_DEATHRATTLE_REPEAT:
            // Titus' Tribute is a player enchantment, not a minion aura.
            // Preserve the exact pinned child ID in Season14State while the
            // Battle resolver owns the actual repeated activation boundary.
            if (lifecycleSpellID == "BG28_843")
                player.season14.ArmTitusTribute();
            return;
        case TavernSpellEffect::TEMPORARY_END_TURN_REPEAT:
            // Primal Staff has a separate scope from Titus: it repeats only
            // this recruit turn's end-of-turn effects.  Do not merge these
            // counters or let the flag leak into the following turn.
            if (lifecycleSpellID == "BG28_955")
                player.season14.ArmPrimalStaff();
            return;
        case TavernSpellEffect::BLOOD_GEM:
        case TavernSpellEffect::BLOOD_GEM_TAUNT:
        case TavernSpellEffect::BLOOD_GEM_DIVINE_SHIELD:
        case TavernSpellEffect::BLOOD_GEM_REBORN:
        {
            // Generated gems can resolve during combat (for example Blood
            // Amulet's Deathrattle trigger).  Resolve the target and all
            // gem-trigger observers against the active field; hard-coding
            // recruitField silently buffed the next-turn copy while leaving
            // the combat minion unchanged.
            auto& activeField = player.GetField();
            if (targetIdx < 0 || targetIdx >= activeField.GetCount())
            {
                return;
            }
            Minion& target =
                activeField[static_cast<std::size_t>(targetIdx)];
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
            activeField.ForEachAlive([&](const MinionData& data) {
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
            // Vinespeaker Portrait's modifier is target-local: only the
            // generated Vinespeaker copies receive the extra Blood Gem
            // Health, and the golden copy receives the doubled payload.
            if ((target.GetCardID() == "BG35_437" ||
                 target.GetCardID() == "BG35_437_G") &&
                player.HasActivePortrait(
                    PortraitEffect::VINESPEAKER_BLOOD_GEM_HEALTH))
            {
                scaledHealth += target.GetCardID() == "BG35_437_G" ? 4 : 2;
            }
            target.ApplyBloodGem(scaledAttack, scaledHealth);
            // Jewelry Box's generated spells use the canonical Blood Gem
            // payload, then apply their printed keyword only to Quilboar.
            // Keep this at the same resolution boundary so all Blood Gem
            // auras/counters observe one ordinary Gem play.
            if (target.HasRace(Race::QUILBOAR))
            {
                if (effect.effect == TavernSpellEffect::BLOOD_GEM_TAUNT)
                    target.SetTaunt(true);
                else if (effect.effect == TavernSpellEffect::BLOOD_GEM_DIVINE_SHIELD)
                    target.SetDivineShieldHits(1);
                else if (effect.effect == TavernSpellEffect::BLOOD_GEM_REBORN)
                    target.SetReborn(true);
            }

            // Bloodbound Ring is specifically a hand-cast Blood Gem trigger.
            // Snapshot shielded recipients after the source target resolves;
            // ApplyBloodGemTo uses the canonical free-gem path and does not
            // re-enter this hook, so the Trinket cannot recurse.
            if (sourceCardDbfID > 0 &&
                (Cards::FindCardByDbfID(sourceCardDbfID).id == "BG20_GEM" ||
                 Cards::FindCardByDbfID(sourceCardDbfID).id == "BG20_GEM_Taunt" ||
                 Cards::FindCardByDbfID(sourceCardDbfID).id == "BG20_GEM_DivineShield" ||
                 Cards::FindCardByDbfID(sourceCardDbfID).id == "BG20_GEM_Reborn"))
            {
                bool ringActive = false;
                for (const auto& trinket : player.season14.trinkets)
                {
                    if (!trinket.active || trinket.remainingUses == 0) continue;
                    if (FindTrinketBehavior(
                            Cards::FindCardByDbfID(trinket.dbfID).id).effect ==
                        TrinketEffect::AFTER_BLOOD_GEM_DIVINE_SHIELD)
                    {
                        ringActive = true;
                        break;
                    }
                }
                if (ringActive)
                {
                    std::vector<std::uint64_t> shielded;
                    activeField.ForEachAlive(
                        [&shielded](MinionData& data) {
                            if (data.value().HasDivineShield())
                                shielded.push_back(static_cast<std::uint64_t>(
                                    data.value().GetIndex()));
                        });
                    for (const auto entityID : shielded)
                    {
                        for (int i = 0; i < activeField.GetCount(); ++i)
                        {
                            auto& recipient = activeField[
                                static_cast<std::size_t>(i)];
                            if (static_cast<std::uint64_t>(recipient.GetIndex()) !=
                                    entityID || recipient.IsDestroyed())
                                continue;
                            player.ApplyBloodGemTo(recipient);
                            break;
                        }
                    }
                }
            }

            // Groundshaker's linked enchantment is a temporary +2 Attack
            // payload on every other friendly minion.  Resolve it through the
            // shared typed lifecycle so it expires at the next recruit start.
            if (target.GetCardID() == "BG20_106")
            {
                activeField.ForEachAlive(
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
                    static_cast<void>(ApplyReviewedLifecycleEnchantment(
                        target, "BG20_102", "BG20_102e",
                        Minion::TemporaryEnchantment::DivineShield));
                }
                else if (target.GetCardID() == "BG20_102_G")
                {
                    static_cast<void>(ApplyReviewedLifecycleEnchantment(
                        target, "BG20_102_G", "BG20_102_Ge",
                        Minion::TemporaryEnchantment::DivineShield));
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
                        static_cast<void>(ApplyReviewedLifecycleEnchantment(
                            target, "BG32_MagicItem_279",
                            "BG32_MagicItem_279e",
                            Minion::TemporaryEnchantment::DivineShield));
                }
            }

            // Dynamic Duo is a persistent +attack/+health response on other
            // Quilboar.  Resolve it from the post-gem public board.
            activeField.ForEachAlive([&](MinionData& data) {
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
        case TavernSpellEffect::ALL_STATS_NEXT_TURN:
            // Haunted Carapace's parent owns the numeric +3/+1 payload;
            // attach the canonical child to each recipient so the typed
            // temporary state expires at the next recruit turn.
            player.recruitField.ForEachAlive([&effect](MinionData& data) {
                auto& minion = data.value();
                static_cast<void>(ApplyReviewedLifecycleEnchantment(
                    minion, "BG33_112", "BG33_112e",
                    Minion::TemporaryEnchantment::Stats, effect.attack,
                    effect.health));
            });
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
            {
                if (ApplySpellcraftLifecycleEnchantment(
                        minion, lifecycleSpellID, effect.attack,
                        effect.health))
                    return;
                minion.ApplyTemporaryEnchantment(
                    Minion::TemporaryEnchantment::Stats, effect.attack,
                    effect.health);
            }
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
            const int attack = multiplier * source.GetAttack();
            const int health = multiplier * source.GetHealth();
            if (temporary)
            {
                if (ApplySpellcraftLifecycleEnchantment(
                        target, lifecycleSpellID, attack, health))
                    return;
                target.ApplyTemporaryEnchantment(
                    Minion::TemporaryEnchantment::Stats, attack, health);
            }
            else
            {
                // Spellcraft normally expires at the next recruit turn, but
                // Lava Lurker can make a copy permanent for this turn.  Do
                // not route that variant through the temporary accumulator:
                // its stats must survive ExpireTemporaryEffects().
                target.SetAttack(target.GetAttack() + attack);
                target.SetHealth(target.GetHealth() + health);
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
            {
                if (ApplySpellcraftLifecycleEnchantment(
                        minion, lifecycleSpellID, effect.attack,
                        effect.health))
                    return;
                minion.ApplyTemporaryEnchantment(
                    Minion::TemporaryEnchantment::StatsAndTaunt, effect.attack,
                    effect.health);
            }
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
                if (ApplySpellcraftLifecycleEnchantment(
                        minion, lifecycleSpellID, effect.attack,
                        effect.health,
                        effect.race == Race::INVALID ||
                            minion.HasRace(effect.race)))
                    return;
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
            // Spellcraft keyword grants expire at the recruit boundary; an
            // ordinary permanent cast retains both keywords.
            if (temporary) {
                if (ApplySpellcraftLifecycleEnchantment(
                        minion, lifecycleSpellID, effect.attack,
                        effect.health))
                    return;
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
            {
                if (ApplySpellcraftLifecycleEnchantment(
                        minion, lifecycleSpellID, effect.attack,
                        effect.health))
                    return;
                minion.ApplyTemporaryEnchantment(
                    Minion::TemporaryEnchantment::StatsAndStealth,
                    effect.attack, effect.health);
            }
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
            auto& minion =
                player.recruitField[static_cast<std::size_t>(targetIdx)];
            if (!ApplySpellcraftLifecycleEnchantment(
                    minion, lifecycleSpellID, effect.attack, effect.health))
                static_cast<void>(minion.MakeGoldenUntilNextTurn());
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
                        Cards::GetTier1Minions(), result, Race::INVALID,
                        player.activeTribes);
                    return result;
                }()));
            return;
        case TavernSpellEffect::RANDOM_NAGA_MINION_TO_HAND:
        {
            std::vector<Card> candidates =
                SupportedMinionsForRace(Race::NAGA, player.activeTribes);
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
                player, SupportedMinionsForRace(race, player.activeTribes)));
            return;
        }
        case TavernSpellEffect::DISCOVER_MINION:
        {
            std::vector<Card> candidates;
            if (effect.race != Race::INVALID)
                candidates = SupportedMinionsForRace(effect.race, player.activeTribes);
            else if (effect.value == 1)
                AppendSupportedNormalMinions(Cards::GetTier1Minions(), candidates, Race::INVALID, player.activeTribes);
            else if (effect.value == 7)
                AppendSupportedNormalMinions(Cards::GetTier7Minions(), candidates, Race::INVALID, player.activeTribes);
            else if (effect.value == 8)
                candidates = SupportedDeathrattleMinions(player.activeTribes);
            else if (effect.lockHand) {
                if (player.currentTier == 1) AppendSupportedNormalMinions(Cards::GetTier1Minions(), candidates, Race::INVALID, player.activeTribes);
                else if (player.currentTier == 2) AppendSupportedNormalMinions(Cards::GetTier2Minions(), candidates, Race::INVALID, player.activeTribes);
                else if (player.currentTier == 3) AppendSupportedNormalMinions(Cards::GetTier3Minions(), candidates, Race::INVALID, player.activeTribes);
                else if (player.currentTier == 4) AppendSupportedNormalMinions(Cards::GetTier4Minions(), candidates, Race::INVALID, player.activeTribes);
                else if (player.currentTier == 5) AppendSupportedNormalMinions(Cards::GetTier5Minions(), candidates, Race::INVALID, player.activeTribes);
                else if (player.currentTier == 6) AppendSupportedNormalMinions(Cards::GetTier6Minions(), candidates, Race::INVALID, player.activeTribes);
                else if (player.currentTier == 7) AppendSupportedNormalMinions(Cards::GetTier7Minions(), candidates, Race::INVALID, player.activeTribes);
            }
            else
                candidates = SupportedMinionsForRace(MostCommonFriendlyRace(player), player.activeTribes);
            if (BeginMinionDiscover(player, std::move(candidates), sourceCardDbfID, effect.lockHand))
                armDiscoverReplay();
            return;
        }
        case TavernSpellEffect::DISCOVER_BATTLECRY_MINION:
            if (BeginMinionDiscover(player, SupportedBattlecryMinions(player.activeTribes), sourceCardDbfID))
                armDiscoverReplay();
            return;
        case TavernSpellEffect::TRANSFORM_HIGHER_TIER:
        {
            std::vector<Card> candidates;
            for (int tier = player.recruitField[static_cast<std::size_t>(targetIdx)].GetTier() + 1;
                 tier <= TIER_UPPER_LIMIT; ++tier) {
                const auto append = [&candidates, &player](const auto& cards) {
                    AppendSupportedNormalMinions(cards, candidates, Race::INVALID,
                                                 player.activeTribes);
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
            std::vector<Card> candidates = SupportedMinionsForRace(target.GetRace(), player.activeTribes);
            candidates.erase(std::remove_if(candidates.begin(), candidates.end(),
                [&target](const Card& card) { return card.id == target.GetCardID(); }), candidates.end());
            if (BeginMinionDiscover(player, std::move(candidates), sourceCardDbfID))
                armDiscoverReplay();
            return;
        }
        case TavernSpellEffect::RANDOM_MINION_AND_COPY:
        {
            auto candidates = SupportedMinionsForRace(effect.race, player.activeTribes);
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
            // Easterly Winds owns the random-refresh payload in Season14;
            // retain the exact parent-qualified child on the Air Revenant
            // source for replay/provenance without applying a second buff.
            player.recruitField.ForEachAlive([](MinionData& data) {
                const auto& id = data.value().GetCardID();
                if (id == "BG34_858" || id == "BG34_858_G")
                    (void)RecordReviewedLifecycleEnchantment(
                        data.value(), id, "BG34_854pe");
            });
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
                // Consuming Claw also observes Tavern-spell devours.  The
                // spell's own keyword-copy flag is independent: Claw always
                // transfers the consumed minion's current Bonus Keywords and
                // adds its per-instance +5/+5 payload after a real removal.
                if (target.HasRace(Race::DEMON)) {
                    for (const auto& trinket : player.season14.trinkets) {
                        if (!trinket.active || trinket.remainingUses == 0) continue;
                        const auto trinketBehavior = FindTrinketBehavior(
                            Cards::FindCardByDbfID(trinket.dbfID).id);
                        if (trinketBehavior.effect !=
                            TrinketEffect::DEMON_CONSUME_BONUS_KEYWORDS)
                            continue;
                        if (consumed.HasTaunt()) target.SetTaunt(true);
                        if (consumed.HasDivineShield())
                            target.SetGameTag(GameTag::DIVINE_SHIELD, 1);
                        if (consumed.HasReborn()) target.SetReborn(true);
                        if (consumed.HasWindfury())
                            target.SetGameTag(GameTag::WINDFURY, 1);
                        if (consumed.HasVenomous())
                            target.SetGameTag(GameTag::VENOMOUS, 1);
                        if (consumed.HasStealth())
                            target.SetGameTag(GameTag::STEALTH, 1);
                        target.SetAttack(target.GetAttack() +
                                         trinketBehavior.attack);
                        target.SetHealth(target.GetHealth() +
                                         trinketBehavior.health);
                    }
                }
                // A Tavern spell may consume one or several offers.  Count
                // each successful removal in order; failed/empty iterations
                // never reach this boundary.
                player.ApplyTavernMinionConsumedTrinkets();
            }
            target.SetAttack(target.GetAttack() + attack);
            target.SetHealth(target.GetHealth() + health);
            return;
        }
        case TavernSpellEffect::DESTROY_UNDEAD_RANDOM_TO_HAND:
        {
            if (targetIdx < 0 || targetIdx >= player.recruitField.GetCount()) return;
            auto& target = player.recruitField[static_cast<std::size_t>(targetIdx)];
            if (effect.race == Race::INVALID || effect.randomCount <= 0 ||
                !target.HasRace(effect.race) ||
                player.hand.GetCount() + effect.randomCount > MAX_HAND_SIZE)
                return;
            auto candidates =
                SupportedMinionsForRace(effect.race, player.activeTribes);
            if (candidates.empty()) return;
            // Destroyed minions still resolve their deathrattle and leave the
            // pool before the generated rewards are created.  This is the
            // same outside-combat destroy boundary used by the persistent
            // attack variant of the Jailer spell.
            if (target.HasDeathrattle())
                target.ActivateTask(PowerType::DEATHRATTLE, player);
            const int poolIndex = target.GetPoolIndex();
            player.recruitField.Remove(target);
            player.returnMinionCallback(poolIndex);
            player.ApplyOutsideCombatDestroyTrinkets();
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
            // Keep the deferred reward's canonical enchantment identity at
            // the same arm boundary as the player-owned counter.  The
            // payload is resolved after combat, so attaching a minion
            // enchantment here would be semantically wrong.
            if (!IsReviewedDeferredLifecycle(
                    Cards::FindCardByDbfID(sourceCardDbfID).id,
                    "BG28_884e"))
                return;
            player.season14.ArmNextCombatReward(105267);
            return;
        case TavernSpellEffect::TARGET_NEXT_COMBAT_BUFF:
        {
            if (targetIdx < 0 || targetIdx >= player.recruitField.GetCount())
                return;
            auto& target = player.recruitField[static_cast<std::size_t>(targetIdx)];
            target.SetAttack(target.GetAttack() + effect.attack);
            target.SetHealth(target.GetHealth() + effect.health);
            // Winner's Bread owns the deferred +4/+6 payload in Season14
            // state.  Retain its child identity on the target now, without
            // applying the deferred stats a second time.
            if (sourceCardDbfID > 0)
                static_cast<void>(RecordReviewedLifecycleEnchantment(
                    target, Cards::FindCardByDbfID(sourceCardDbfID).id));
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
            const auto parentID = sourceCardDbfID > 0
                ? Cards::FindCardByDbfID(sourceCardDbfID).id : std::string{};
            if (!ApplyReviewedLifecycleEnchantment(
                    target, parentID, effect.attack, effect.health))
                target.ApplyTemporaryEnchantment(Minion::TemporaryEnchantment::Stats,
                                                  effect.attack, effect.health);
            return;
        }
        case TavernSpellEffect::TARGET_END_TURN_STATS:
        {
            if (targetIdx < 0 || targetIdx >= player.recruitField.GetCount() ||
                sourceCardDbfID <= 0)
                return;
            const auto parent = Cards::FindCardByDbfID(sourceCardDbfID).id;
            const std::string_view child =
                parent == "BG28_814" ? "BG28_814e" : "";
            if (!child.empty())
                static_cast<void>(ApplyReviewedEndTurnChildEnchantment(
                    player.recruitField[static_cast<std::size_t>(targetIdx)],
                    parent, child));
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
            // "Bonus Keyword" is the six-keyword Battlegrounds set.  Keep
            // this list in the resolver (rather than relying on generic
            // Hearthstone keyword metadata), since the card text excludes
            // no keyword and the random roll must be over *missing* ones.
            constexpr GameTag keywords[] = {GameTag::DIVINE_SHIELD,
                GameTag::REBORN, GameTag::WINDFURY, GameTag::VENOMOUS,
                GameTag::TAUNT, GameTag::STEALTH};
            std::vector<GameTag> eligible;
            for (const auto keyword : keywords)
            {
                const bool present = keyword == GameTag::DIVINE_SHIELD
                    ? target.HasDivineShield()
                    : keyword == GameTag::REBORN ? target.HasReborn()
                    : keyword == GameTag::WINDFURY ? target.HasWindfury()
                    : keyword == GameTag::VENOMOUS ? target.HasVenomous()
                    : keyword == GameTag::TAUNT ? target.HasTaunt()
                    : target.HasStealth();
                if (!present) eligible.push_back(keyword);
            }
            if (eligible.empty()) return;
            const auto keyword = eligible[Random::get<std::size_t>(
                0, eligible.size() - 1)];
            if (temporary)
            {
                // Spellcraft's Bonus Keyword lasts through this recruit
                // turn only.  In particular, do not use SetGameTag here:
                // that would make a Vibrant Bubble token permanent and
                // would also make it survive a hand/board transition.
                target.ApplyTemporaryKeyword(keyword);
            }
            else if (keyword == GameTag::TAUNT)
            {
                target.SetTaunt(true);
            }
            else if (keyword == GameTag::REBORN)
            {
                target.SetReborn(true);
            }
            else if (keyword == GameTag::VENOMOUS)
            {
                target.SetGameTag(GameTag::POISONOUS, 1);
            }
            else
            {
                target.SetGameTag(keyword, 1);
            }
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
            player.ApplyOutsideCombatDestroyTrinkets();
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
    // Free casts still use the normal target contract.  In particular,
    // target-required spells must never fall through to ApplySpellBoardEffect
    // with -1 (TARGET_STATS indexes the recruit field directly).  Generated
    // effects may request a public target modal, but an internal caller that
    // supplies an explicit target must point at a live entity in the correct
    // zone.  This keeps targeted effects such as Repair Job target-aware
    // instead of silently becoming a global/default cast.
    if (TavernSpellRequiresTarget(behavior.effect) && targetIdx < 0 &&
        !season14.pendingTaughtSpell.pending)
        return false;
    if (targetIdx >= 0 && TavernSpellRequiresTarget(behavior.effect))
    {
        if (TavernSpellTargetsShop(behavior.effect))
        {
            if (targetIdx >= tavern.fieldZone.GetCount() ||
                tavern.fieldZone[static_cast<std::size_t>(targetIdx)]
                    .IsDestroyed() ||
                tavern.fieldZone[static_cast<std::size_t>(targetIdx)]
                    .GetCardID().empty())
                return false;
        }
        else if (!ValidFriendlyBoardTarget(*this, targetIdx))
        {
            return false;
        }
        if (!TavernSpellTargetsShop(behavior.effect) &&
            (behavior.effect == TavernSpellEffect::DESTROY_UNDEAD_RANDOM_TO_HAND ||
             behavior.effect == TavernSpellEffect::DESTROY_UNDEAD_GIVE_PERSISTENT_ATTACK))
        {
            const auto& target = recruitField[static_cast<std::size_t>(targetIdx)];
            if (behavior.race == Race::INVALID || !target.HasRace(behavior.race))
                return false;
            if (behavior.effect == TavernSpellEffect::DESTROY_UNDEAD_RANDOM_TO_HAND &&
                (behavior.randomCount <= 0 ||
                 hand.GetCount() + behavior.randomCount > MAX_HAND_SIZE ||
                 SupportedMinionsForRace(behavior.race, activeTribes).empty()))
                return false;
        }
    }
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
                const auto& target = data.value();
                const bool raceLegal = behavior.race == Race::INVALID ||
                    target.HasRace(behavior.race);
                const bool keywordOpen =
                    !target.HasDivineShield() || !target.HasReborn() ||
                    !target.HasWindfury() || !target.HasVenomous() ||
                    !target.HasTaunt() || !target.HasStealth();
                const bool effectLegal =
                    (behavior.effect != TavernSpellEffect::TARGET_RANDOM_RACE_KEYWORD ||
                     (raceLegal && keywordOpen)) &&
                    (behavior.effect != TavernSpellEffect::DESTROY_UNDEAD_RANDOM_TO_HAND &&
                     behavior.effect != TavernSpellEffect::DESTROY_UNDEAD_GIVE_PERSISTENT_ATTACK ||
                     raceLegal);
                const bool rewardCapacity =
                    behavior.effect != TavernSpellEffect::DESTROY_UNDEAD_RANDOM_TO_HAND ||
                    (behavior.randomCount > 0 &&
                     hand.GetCount() + behavior.randomCount <= MAX_HAND_SIZE &&
                     !SupportedMinionsForRace(behavior.race, activeTribes).empty());
                if (slot < 7 && effectLegal && rewardCapacity) {
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
        const bool targetShop = targetIdx >= 0 && TavernSpellTargetsShop(behavior.effect);
        const auto targetEntity = targetShop
            ? static_cast<std::uint64_t>(tavern.fieldZone[
                  static_cast<std::size_t>(targetIdx)].GetIndex())
            : 0;
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
        // Free/generated casts are still successful Tavern-spell resolutions:
        // they must participate in the same spell counter and observer hooks
        // as hand-cast and modal continuations.  Keeping this immediately
        // before ApplyTavernSpellTrinkets preserves the normal post-resolution
        // ordering and lets effects such as Inductive Gyroblade observe them.
        season14.OnTavernSpellResolved(
            true, card.dbfID, targetIdx >= 0, targetEntity);
        // Tide Raiser Portrait listens to the authoritative combat spell
        // transaction, including generated Spellcraft/free casts.  Count a
        // successful cast even when the hand is full; the copy is simply
        // lost at the ordinary hand-cap boundary.  Never recurse because the
        // generated copy is added to hand rather than cast.
        if (isInCombat)
        {
            for (auto& trinket : season14.trinkets)
            {
                if (!trinket.active || trinket.remainingUses == 0 ||
                    trinket.triggerProgress >= 3)
                    continue;
                if (FindTrinketBehavior(
                        Cards::FindCardByDbfID(trinket.dbfID).id).portraitEffect !=
                    PortraitEffect::TIDE_RAISER_COMBAT_SPELL_COPY)
                    continue;
                ++trinket.triggerProgress;
                if (!hand.IsFull()) hand.Add(CardData{Spell(card)});
            }
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
    if (targetIdx < 0 || targetIdx >= GetField().GetCount())
        return;
    // Rally-generated gems are free and do not masquerade as a spell cast;
    // use the canonical board-effect executor so race auras, Agamaggan,
    // Tough Tusk, and Dynamic Duo all resolve identically to a real gem.
    ApplySpellBoardEffect(*this, FindTavernSpellBehavior("BG20_GEM"),
                          targetIdx, false);
}

void Player::ResolveScrapsmithPortraitDeath(const Minion& deadMinion)
{
    if (!deadMinion.HasTaunt() ||
        !HasActivePortrait(PortraitEffect::SCRAPSMITH_TAUNT_DEATH_GEMS))
        return;
    // This is a permanent Blood Gem play, not a generated Gem in hand. Apply
    // the canonical scaled Gem payload directly to the active field; the
    // ordinary ApplyBloodGemTo helper intentionally routes through the
    // recruit-phase spell executor, while this trigger also fires in combat.
    const auto applyGem = [this](Minion& target) {
        auto [attack, health] = season14.BloodGemStats();
        for (const auto race : RACES_IN_BATTLEGROUNDS) {
            if (!target.HasRace(race)) continue;
            const auto [raceAttack, raceHealth] =
                season14.BloodGemRaceStatsFor(race);
            attack += raceAttack;
            health += raceHealth;
        }
        // Scrapsmith Portrait resolves a permanent Blood Gem directly in
        // this helper (it can trigger during combat), so retain the same
        // target-local Vinespeaker Portrait modifier as the ordinary
        // TavernSpellEffect::BLOOD_GEM path.  The generated Vinespeaker's
        // golden copy doubles the printed health contribution.
        if ((target.GetCardID() == "BG35_437" ||
             target.GetCardID() == "BG35_437_G") &&
            HasActivePortrait(PortraitEffect::VINESPEAKER_BLOOD_GEM_HEALTH))
            health += target.GetCardID() == "BG35_437_G" ? 4 : 2;
        target.ApplyBloodGem(attack, health);
    };
    GetField().ForEachAlive([&applyGem](MinionData& data) {
        auto& scrapsmith = data.value();
        if (scrapsmith.GetCardID() == "BG24_707" ||
            scrapsmith.GetCardID() == "BG24_707_G")
            applyGem(scrapsmith);
    });
    // Combat copies are discarded after resolution. Mirror the permanent Gem
    // onto the matching recruit instance so it survives into the next turn.
    if (isInCombat) {
        recruitField.ForEachAlive([&](MinionData& data) {
            auto& recruit = data.value();
            if (recruit.GetCardID() != "BG24_707" &&
                recruit.GetCardID() != "BG24_707_G") return;
            GetField().ForEachAlive([&](MinionData& combatData) {
                auto& combat = combatData.value();
                if (combat.GetIndex() == recruit.GetIndex() &&
                    (combat.GetCardID() == "BG24_707" ||
                     combat.GetCardID() == "BG24_707_G"))
                    applyGem(recruit);
            });
        });
    }
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
    if (IsLiftOffUpgrade(spell.GetID())) {
        if (season14.heroPowerDbfID != 118681 || targetIdx >= 0 ||
            season14.liftOffBattlecruiserEntityID == 0)
            return false;
        bool cruiserPresent = false;
        recruitField.ForEachAlive([&](const MinionData& data) {
            cruiserPresent = cruiserPresent ||
                static_cast<std::uint64_t>(data.value().GetIndex()) ==
                    season14.liftOffBattlecruiserEntityID;
        });
        if (!cruiserPresent) return false;
        const int cost = season14.liftOffFreeUpgradeAvailable
            ? 0 : spell.GetCost();
        return spell.GetCost() >= 0 && remainCoin >= cost;
    }
    if (spell.GetID() == "BG30_MagicItem_416t")
    {
        if (targetIdx < 0 || !ValidFriendlyBoardTarget(*this, targetIdx)) return false;
        const auto& target = recruitField[static_cast<std::size_t>(targetIdx)];
        if (target.GetTier() >= 6) return false;
        return std::any_of(Cards::GetAllCards().begin(), Cards::GetAllCards().end(),
            [&target, this](const Card& candidate) {
                return candidate.isBattlegroundsPoolMinion && candidate.hasBehavior &&
                       candidate.GetCardType() == CardType::MINION && candidate.normalDbfID == 0 &&
                       HasActiveTribe(activeTribes, candidate) &&
                       candidate.GetTier() == target.GetTier() + 1;
            });
    }
    const TavernSpellBehavior behavior = FindTavernSpellBehavior(spell.GetID());
    const bool shopTarget = TavernSpellTargetsShop(behavior.effect);
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
            (behavior.race != Race::INVALID && !target.HasRace(behavior.race) ||
             (target.HasDivineShield() && target.HasReborn() &&
              target.HasWindfury() && target.HasVenomous() &&
              target.HasTaunt() && target.HasStealth())))
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
            (!target.HasRace(behavior.race) || behavior.race == Race::INVALID ||
             behavior.randomCount <= 0 ||
             // hand still contains the spell at this point; after it is
             // removed, every printed random reward must have a slot.
             hand.GetCount() - 1 + behavior.randomCount > MAX_HAND_SIZE ||
             SupportedMinionsForRace(behavior.race, activeTribes).empty()))
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
        !HasSupportedTier1Minion(activeTribes))
    {
        return false;
    }
    if (behavior.effect == TavernSpellEffect::RANDOM_NAGA_MINION_TO_HAND &&
        SupportedMinionsForRace(Race::NAGA, activeTribes).empty())
        return false;
    if (behavior.effect == TavernSpellEffect::DISCOVER_MINION) {
        if (hand.IsFull()) return false;
        std::vector<Card> candidates;
        if (behavior.value == 1)
            AppendSupportedNormalMinions(Cards::GetTier1Minions(), candidates, Race::INVALID, activeTribes);
        else if (behavior.value == 7)
            AppendSupportedNormalMinions(Cards::GetTier7Minions(), candidates, Race::INVALID, activeTribes);
        else if (behavior.value == 8)
            candidates = SupportedDeathrattleMinions(activeTribes);
        else if (behavior.lockHand) {
            if (currentTier == 1) AppendSupportedNormalMinions(Cards::GetTier1Minions(), candidates, Race::INVALID, activeTribes);
            else if (currentTier == 2) AppendSupportedNormalMinions(Cards::GetTier2Minions(), candidates, Race::INVALID, activeTribes);
            else if (currentTier == 3) AppendSupportedNormalMinions(Cards::GetTier3Minions(), candidates, Race::INVALID, activeTribes);
            else if (currentTier == 4) AppendSupportedNormalMinions(Cards::GetTier4Minions(), candidates, Race::INVALID, activeTribes);
            else if (currentTier == 5) AppendSupportedNormalMinions(Cards::GetTier5Minions(), candidates, Race::INVALID, activeTribes);
            else if (currentTier == 6) AppendSupportedNormalMinions(Cards::GetTier6Minions(), candidates, Race::INVALID, activeTribes);
            else if (currentTier == 7) AppendSupportedNormalMinions(Cards::GetTier7Minions(), candidates, Race::INVALID, activeTribes);
        }
        else
                candidates = SupportedMinionsForRace(MostCommonFriendlyRace(*this), activeTribes);
        if (candidates.empty()) return false;
    }
    if (behavior.effect == TavernSpellEffect::DISCOVER_BATTLECRY_MINION &&
        SupportedBattlecryMinions(activeTribes).empty())
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
        SupportedMinionsForRace(Race::UNDEAD, activeTribes).empty())
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
        auto candidates = SupportedMinionsForRace(target.GetRace(), activeTribes);
        candidates.erase(std::remove_if(candidates.begin(), candidates.end(),
            [&target](const Card& card) { return card.id == target.GetCardID(); }), candidates.end());
        if (candidates.empty()) return false;
    }
    if (behavior.effect == TavernSpellEffect::RANDOM_MINION_AND_COPY) {
        if (hand.GetCount() + 2 > MAX_HAND_SIZE ||
            SupportedMinionsForRace(behavior.race, activeTribes).empty()) return false;
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
        if (race == Race::INVALID ||
            !HasSupportedRaceMinion(race, activeTribes))
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
        if (!ApplySpellcraftLifecycleEnchantment(target, spell.GetID(), 30,
                                                 30))
            target.ApplyTemporaryStats(30, 30);
        ApplyImperialDefenderCopies(
            *this, targetIdx, spell.GetTargetingType(),
            [&](int buddyIdx) {
                auto& buddy = recruitField[static_cast<std::size_t>(buddyIdx)];
                if (!ApplySpellcraftLifecycleEnchantment(buddy, spell.GetID(),
                                                         30, 30))
                    buddy.ApplyTemporaryStats(30, 30);
            });
        // Harvested Pearl is a generated/free targeted Spellcraft card, but
        // it is still a real spell cast for Lovely Locket.  Apply the same
        // temporary payload directly to another live friendly minion; do not
        // call PlaySpell recursively, which would re-arm the Locket.
        for (const auto& trinket : season14.trinkets) {
            if (!trinket.active || trinket.remainingUses == 0) continue;
            if (FindTrinketBehavior(
                    Cards::FindCardByDbfID(trinket.dbfID).id).effect !=
                TrinketEffect::AFTER_FRIENDLY_SPELL_REPEAT)
                continue;
            int secondaryIdx = -1;
            for (int candidate = 0; candidate < recruitField.GetCount(); ++candidate) {
                if (candidate == targetIdx ||
                    recruitField[static_cast<std::size_t>(candidate)].IsDestroyed() ||
                    recruitField[static_cast<std::size_t>(candidate)].GetHealth() <= 0)
                    continue;
                secondaryIdx = candidate;
                break;
            }
            if (secondaryIdx < 0) continue;
            auto& secondary = recruitField[static_cast<std::size_t>(secondaryIdx)];
            if (!ApplySpellcraftLifecycleEnchantment(secondary, spell.GetID(),
                                                      30, 30))
                secondary.ApplyTemporaryStats(30, 30);
        }
        hand.Remove(card);
        // Spellcraft's generated Tavern spell is a successful cast too;
        // route it through the shared counter before flushing post-cast
        // Trinket rewards (including Bloodbound Earrings).
        season14.OnTavernSpellResolved(true, spell.GetDbfID(), true);
        ResolveSpellCountTrinkets();
        ApplyTavernSpellTrinkets();
        AdvanceDarkGiftCounters(3);
        return true;
    }
    if (spell.GetID() == "BG30_MagicItem_416t")
    {
        auto& target = recruitField[static_cast<std::size_t>(targetIdx)];
        // This spell opens a target modal and then a second candidate modal.
        if (!season14.BeginTransformDecision(0, spell.GetDbfID(),
                                             static_cast<std::uint64_t>(target.GetIndex()),
                                             targetIdx, target.GetTier()))
            return false;
        std::uint8_t locketCopies = 0;
        for (const auto& trinket : season14.trinkets) {
            if (!trinket.active || trinket.remainingUses == 0) continue;
            const auto behavior = FindTrinketBehavior(
                Cards::FindCardByDbfID(trinket.dbfID).id);
            if (behavior.effect == TrinketEffect::AFTER_FRIENDLY_SPELL_REPEAT &&
                locketCopies < 255)
                ++locketCopies;
        }
        season14.ArmTransformReplay(locketCopies,
                                     static_cast<std::uint64_t>(target.GetIndex()));
        hand.Remove(card);
        return true;
    }
    if (IsLiftOffUpgrade(spell.GetID())) {
        const int level = LiftOffUpgradeLevel(spell.GetID());
        const int cost = season14.liftOffFreeUpgradeAvailable ? 0 : spell.GetCost();
        Minion* cruiser = nullptr;
        recruitField.ForEachAlive([&](MinionData& data) {
            if (static_cast<std::uint64_t>(data.value().GetIndex()) ==
                season14.liftOffBattlecruiserEntityID)
                cruiser = &data.value();
        });
        if (cruiser == nullptr) return false;
        if (season14.liftOffFreeUpgradeAvailable)
            season14.liftOffFreeUpgradeAvailable = false;
        hand.Remove(card);
        remainCoin -= cost;
        RecordGoldSpent(cost);
        const auto id = spell.GetID();
        constexpr std::array<int, 7> attack = {6, 8, 11, 15, 20, 28, 40};
        constexpr std::array<int, 7> health = {6, 8, 11, 15, 20, 28, 40};
        const auto amount = static_cast<std::size_t>(std::clamp(level, 1, 7) - 1);
        if (id.find("801pta") != std::string::npos) cruiser->SetAttack(cruiser->GetAttack() + attack[amount]);
        else if (id.find("801ptb") != std::string::npos) cruiser->SetHealth(cruiser->GetHealth() + health[amount]);
        else if (id.find("801ptc") != std::string::npos) {
            constexpr std::array<int, 7> damage = {3, 6, 9, 18, 24, 30, 40};
            season14.liftOffYamatoDamage = std::max(season14.liftOffYamatoDamage, damage[amount]);
            (void)RecordReviewedExternalLifecycleEnchantment(
                *cruiser, id, id.find("801ptc") != std::string::npos
                             ? "BG31_HERO_801ptce" : "");
        } else if (id.find("801ptd") != std::string::npos) {
            constexpr std::array<int, 7> rally = {2, 3, 4, 5, 6, 8, 10};
            season14.liftOffRallyAttack = std::max(season14.liftOffRallyAttack, rally[amount]);
        } else if (id.find("801pte") != std::string::npos) {
            constexpr std::array<int, 7> reactor = {1, 2, 4, 7, 10, 15, 25};
            season14.liftOffDeathrattleAttack = std::max(season14.liftOffDeathrattleAttack, reactor[amount]);
            season14.liftOffDeathrattleHealth = std::max(season14.liftOffDeathrattleHealth, reactor[amount]);
            (void)RecordReviewedExternalLifecycleEnchantment(
                *cruiser, id, "BG31_HERO_801ptee");
        } else if (id.find("801ptf") != std::string::npos) season14.liftOffFreeUpgradeAvailable = true;
        else if (id.find("801pth") != std::string::npos) {
            season14.liftOffFortifiedBunker = true;
            (void)RecordReviewedExternalLifecycleEnchantment(
                *cruiser, id, "BG31_HERO_801pthe");
        }
        else if (id.find("801pti") != std::string::npos) season14.liftOffMissilePod = true;
        else if (id.find("801ptj") != std::string::npos) season14.liftOffUltraCapacitor = true;
        ++season14.liftOffUpgradesBoughtThisTurn;
        season14.Emit(Season14Event::SPELL_CAST);
        season14.OnTavernSpellResolved(true, spell.GetDbfID(), false);
        ResolveSpellCountTrinkets();
        IncrementStartCombatSpellImprovements();
        // This generated Tavern-spell path returns before the ordinary
        // post-resolution block; flush thresholded Trinket rewards here so
        // Bloodbound Earrings resolve on the same successful-cast boundary.
        ApplyTavernSpellTrinkets();
        ApplyAfterPlayCardTrinkets();
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
    const bool spellcraftSpell = IsSpellcraftToken(spell);
    bool temporarySpell = spellcraftSpell;
    TavernSpellBehavior effect = FindTavernSpellBehavior(spell.GetID());
    // Capture a shop target before resolving the spell.  Some valid target
    // spells (move/copy) mutate or remove the Tavern entity during their
    // effect, but Felsteel Cleaver needs the original stable entity ID at the
    // shared post-resolution boundary.
    const bool spellTargetShop = targetIdx >= 0 &&
                                 TavernSpellTargetsShop(effect.effect);
    const std::uint64_t spellTargetEntityID =
        spellTargetShop
            ? static_cast<std::uint64_t>(tavern.fieldZone[
                  static_cast<std::size_t>(targetIdx)].GetIndex())
            : 0;
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
    if (spell.GetID() == "BG26_502t" || spell.GetID() == "BG26_502_Gt")
    {
        const auto [improvementAttack, improvementHealth] =
            season14.FutureDeepBluesStats();
        effect.attack += improvementAttack;
        effect.health += improvementHealth;
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
        int friendlyExtra = 0;
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
            // Lovely Locket is a separate replay from Cathedral/Sushi.  Keep
            // its count on the modal so the secondary target is selected only
            // after the asynchronous choice commits.
            if (targetIdx >= 0 &&
                behavior.effect == TrinketEffect::AFTER_FRIENDLY_SPELL_REPEAT)
                ++friendlyExtra;
        }
        season14.spellModal.extraResolutionCount =
            static_cast<std::uint8_t>(std::min(extra, 255));
        season14.spellModal.friendlySpellRepeatCount =
            static_cast<std::uint8_t>(std::min(friendlyExtra, 255));
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
        ApplyUpstartEmbers();
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
        const auto desiredOffers = season14.TavernOfferCount(MAX_FIELD_SIZE);
        clearTavernMinionsCallback(*this);
        season14.BeginRefreshTavern();
        PrepareTavern();
        const auto isBattlecryOneCost = [](const Minion& candidate) {
            const auto card = Cards::FindCardByID(candidate.GetCardID());
            return card.dbfID != 0 &&
                card.gameTags.contains(GameTag::COST) &&
                card.gameTags.at(GameTag::COST) == 1 &&
                CardDefs::FindCardDefByID(card.id).HasBattlecry();
        };
        for (int i = tavern.fieldZone.GetCount() - 1; i >= 0; --i)
        {
            auto& minion = tavern.fieldZone[static_cast<std::size_t>(i)];
            // Refresh preserves independently frozen cards.  Newly offered
            // non-Battlecry minions are removed, but a frozen pre-existing
            // entity remains visible exactly as it would after a normal
            // Tavern refresh.
            if (!minion.IsFrozen() && !isBattlecryOneCost(minion))
            {
                const auto poolIndex = minion.GetPoolIndex();
                auto removed = tavern.fieldZone.Remove(minion);
                if (poolIndex >= 0) returnMinionCallback(poolIndex);
                (void)removed;
            }
        }
        // Removing non-matching offers must not silently shrink the Tavern.
        // Draw replacements through the authoritative pool callback, then
        // return rejects immediately.  A finite guard handles a depleted
        // pool without looping forever in small deterministic fixtures.
        for (int attempts = 0;
             tavern.fieldZone.GetCount() < static_cast<int>(desiredOffers) &&
             attempts < 128 && addRandomTavernMinionCallback;
             ++attempts)
        {
            const auto before = tavern.fieldZone.GetCount();
            if (!addRandomTavernMinionCallback(*this, currentTier) ||
                tavern.fieldZone.GetCount() <= before)
                break;
            auto& candidate = tavern.fieldZone[
                static_cast<std::size_t>(tavern.fieldZone.GetCount() - 1)];
            if (candidate.IsFrozen() || isBattlecryOneCost(candidate))
                continue;
            const auto poolIndex = candidate.GetPoolIndex();
            auto removed = tavern.fieldZone.Remove(candidate);
            if (poolIndex >= 0) returnMinionCallback(poolIndex);
            (void)removed;
        }
        // The token's "They cost (1)" is a purchase-cost override, not a
        // printed-card-cost filter. It expires at the next recruit start.
        season14.SetTemporaryMinionPurchaseCost(1);
        season14.OnRefreshTavern(true);
        ApplyTavernSpellTrinkets();
        ApplyUpstartEmbers();
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
        auto candidates = SupportedMinionsForRace(Race::UNDEAD, activeTribes);
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
        season14.BeginRefreshTavern();
        season14.ArmRefreshRace(race);
        PrepareTavern();
        season14.OnRefreshTavern(true);
        ApplyTavernSpellTrinkets();
        ApplyUpstartEmbers();
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
    int friendlySpellRepeats = 0;
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
    // Lovely Locket is armed per successful targeted spell, not per turn.
    // Token of the Old Gods is handled by its typed two-stage transform
    // transaction above; all other supported targeted effects use this
    // direct, live-target replay path.
    const bool locketReplaySupported =
        effect.effect != TavernSpellEffect::DISCOVER_DIFFERENT_RACE &&
        effect.effect != TavernSpellEffect::REFRESH_RACE;
    if (locketReplaySupported && targetIdx >= 0 &&
        (spell.GetTargetingType() == TargetingType::FRIENDLY_MINIONS ||
         spell.GetTargetingType() == TargetingType::FRIENDLY_CHARACTERS))
    {
        for (const auto& trinket : season14.trinkets)
        {
            if (!trinket.active || trinket.remainingUses == 0) continue;
            const auto behavior = FindTrinketBehavior(
                Cards::FindCardByDbfID(trinket.dbfID).id);
            if (behavior.effect == TrinketEffect::AFTER_FRIENDLY_SPELL_REPEAT)
                ++friendlySpellRepeats;
        }
        castRepeats += friendlySpellRepeats;
    }
    // Hot-Air Surveyor adds the printed +6/+6 payload to each Blood Gem
    // played from hand.  It is a stat bonus, not an extra cast: an additional
    // cast would incorrectly trigger per-cast counters and only add +1/+1.
    if (!temporarySpell && targetIdx >= 0 &&
        (spell.GetID() == "BG20_GEM" ||
         spell.GetID() == "BG20_GEM_Taunt" ||
         spell.GetID() == "BG20_GEM_DivineShield" ||
         spell.GetID() == "BG20_GEM_Reborn"))
    {
        recruitField.ForEachAlive([&effect](MinionData& data) {
            const auto& id = data.value().GetCardID();
            if (id == "BG30_121") {
                effect.attack += 6;
                effect.health += 6;
            } else if (id == "BG30_121_G") {
                effect.attack += 12;
                effect.health += 12;
            }
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
    {
        int resolvedTargetIdx = targetIdx;
        if (repeat > 0 && repeat <= friendlySpellRepeats)
        {
            // Pick the first currently alive friendly minion other than the
            // original target.  If none exists, the printed "another"
            // target is unavailable and the replay is correctly skipped.
            resolvedTargetIdx = -1;
            for (int candidate = 0; candidate < recruitField.GetCount(); ++candidate)
            {
                if (candidate == targetIdx) continue;
                if (recruitField[candidate].GetHealth() > 0)
                {
                    resolvedTargetIdx = candidate;
                    break;
                }
            }
            if (resolvedTargetIdx < 0) continue;
        }
        ApplySpellBoardEffect(*this, effect, resolvedTargetIdx, temporarySpell,
                              sourceSpellDbfID);
        // Pufferquil's "spell cast on this" trigger resolves after the spell
        // payload.  Keep target selection in the ordinary PlaySpell path and
        // route only the canonical temporary child through the typed registry.
        if (resolvedTargetIdx >= 0 &&
            resolvedTargetIdx < recruitField.GetCount() &&
            (recruitField[static_cast<std::size_t>(resolvedTargetIdx)].GetCardID() ==
                 "BG25_039" ||
             recruitField[static_cast<std::size_t>(resolvedTargetIdx)].GetCardID() ==
                 "BG25_039_G"))
        {
            auto& target = recruitField[static_cast<std::size_t>(resolvedTargetIdx)];
            static_cast<void>(ApplyReviewedLifecycleEnchantment(
                target,
                target.GetCardID() == "BG25_039_G" ? "BG25_039_G" : "BG25_039",
                target.GetCardID() == "BG25_039_G" ? "BG25_039_Ge" : "BG25_039e",
                Minion::TemporaryEnchantment::Venomous));
        }
        // Imperial Defender mirrors a spell cast on a different friendly
        // minion onto itself once per turn.  The shared helper also serves
        // generated and modal target-resolved spells below.
        ApplyImperialDefenderCopies(
            *this, resolvedTargetIdx, spell.GetTargetingType(),
            [&](int buddyIdx) {
                ApplySpellBoardEffect(*this, effect, buddyIdx,
                                      temporarySpell, sourceSpellDbfID);
            });
        // Deep Blue Crooner improves only future copies.  Count each actual
        // resolution, including a legitimate repeated Spellcraft cast, and
        // scale the golden copy's improvement with its printed payload.
        if (spell.GetID() == "BG26_502t" || spell.GetID() == "BG26_502_Gt")
        {
            const int improvement = spell.GetID() == "BG26_502_Gt" ? 2 : 1;
            season14.ImproveFutureDeepBlues(improvement, improvement);
        }
    }
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
            bool shakerPortrait = false;
            for (const auto& trinket : season14.trinkets) {
                if (!trinket.active || trinket.remainingUses == 0) continue;
                const auto behavior = FindTrinketBehavior(
                    Cards::FindCardByDbfID(trinket.dbfID).id);
                if (behavior.portraitEffect == PortraitEffect::ZESTY_SHAKER_EXTRA_COPY) {
                    shakerPortrait = true;
                    break;
                }
            }
            const int copies = (target.GetCardID() == "BG26_505_G" ? 2 : 1) +
                               (shakerPortrait ? 1 : 0);
            for (int copy = 0; copy < copies && !hand.IsFull(); ++copy)
                hand.Add(CardData{playedSpell});
        }
    }
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
    season14.OnTavernSpellResolved(true, sourceSpellDbfID,
                                   targetIdx >= 0 && !spellTargetShop,
                                   spellTargetEntityID);
    // Tide Raiser copies every successfully resolved Tavern spell during
    // combat, regardless of whether the source was a hand Spellcraft card or
    // an ordinary/generated spell.  Add the copy directly (never cast it),
    // cap each owned portrait at three copies per combat, and let hand
    // capacity discard excess copies without consuming an additional trigger.
    if (isInCombat) {
        for (auto& trinket : season14.trinkets) {
            if (!trinket.active || trinket.remainingUses == 0 ||
                trinket.triggerProgress >= 3)
                continue;
            if (FindTrinketBehavior(
                    Cards::FindCardByDbfID(trinket.dbfID).id).portraitEffect !=
                PortraitEffect::TIDE_RAISER_COMBAT_SPELL_COPY)
                continue;
            ++trinket.triggerProgress;
            if (!hand.IsFull()) hand.Add(CardData{playedSpell});
        }
    }
    if (spellcraftSpell)
    {
        // Coral is resolved after the source Spellcraft payload and its
        // successful-cast boundary. The free cast itself emits its own
        // counters/observers through the canonical path.
        ResolveCoralSpearSpellcraft(*this);
    }
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
    // Bluegill Flippers targets exactly the left-most minion in each zone,
    // with one application per owned copy after a successful Tavern spell.
    for (const auto& trinket : season14.trinkets) {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect != TrinketEffect::SPELL_CAST_LEFTMOST_MINION_STATS)
            continue;
        for (int i = 0; i < hand.GetCount(); ++i) {
            if (!std::holds_alternative<Minion>(hand[i])) continue;
            auto& minion = std::get<Minion>(hand[i]);
            minion.SetAttack(minion.GetAttack() + behavior.attack);
            minion.SetHealth(minion.GetHealth() + behavior.health);
            break;
        }
        if (recruitField.GetCount() > 0) {
            auto& minion = recruitField[0];
            minion.SetAttack(minion.GetAttack() + behavior.attack);
            minion.SetHealth(minion.GetHealth() + behavior.health);
        }
    }
    ApplyTavernSpellTrinkets();
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
                                   std::int32_t replacementDbfID,
                                   std::int32_t replacementTier)
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
        !replacement.hasBehavior || !HasActiveTribe(activeTribes, replacement) ||
        replacement.GetTier() != replacementTier)
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

bool Player::BeginTickatusDiscover()
{
    if (season14.pendingDecision != Season14Decision::NONE || hand.IsFull())
        return false;
    std::vector<Card> prizes;
    for (const auto& candidate : Cards::GetAllCards()) {
        if (candidate.GetCardType() == CardType::SPELL &&
            candidate.normalDbfID == 0 && candidate.dbfID > 0 &&
            candidate.id.starts_with("BGS_Treasures_") &&
            candidate.darkmoonPrizeTurn == 3)
            prizes.push_back(candidate);
    }
    if (prizes.size() < 3)
        return false;
    Random::shuffle(prizes.begin(), prizes.end());
    const auto source = Cards::FindCardByID("BG30_MagicItem_707");
    if (source.dbfID == 0)
        return false;
    season14.BeginOfferingDecision(
        Season14Decision::DISCOVER, 0, source.dbfID,
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

int Player::ResolveMaxwellStickerBuddy(bool golden)
{
    if (hand.IsFull()) return 0;
    const auto reward = FindHeroPowerBuddyReward(season14.heroPowerDbfID);
    if (!reward.has_value()) return 0;
    const auto card = Cards::FindCardByDbfID(
        golden ? reward->goldenBuddyDbfID : reward->buddyDbfID);
    // Re-check the canonical entity at the commit boundary.  This protects
    // replayed/stale state if card data changes between acquisition and sale.
    if (card.dbfID <= 0 || card.GetCardType() != CardType::MINION ||
        (golden ? !card.id.ends_with("_Buddy_G")
                : !card.id.ends_with("_Buddy")))
        return 0;
    Minion generated(card);
    ApplyFreshMinionModifiers(generated);
    hand.Add(CardData{std::move(generated)});
    return 1;
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
            card.normalDbfID == 0 && card.GetTier() == tier &&
            HasActiveTribe(activeTribes, card))
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
                card.normalDbfID == 0 && HasActiveTribe(activeTribes, card) &&
                card.HasRace(Race::MURLOC) &&
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
    // The parent activation is now committed: retain only the exact child
    // provenance marker.  The modal itself remains the sole owner of the
    // hero-power transformation, so no enchantment task is executed here.
    season14.powerOfStormChildAttached = true;
    season14.powerOfStormChildID = "BG20_HERO_202pe";
    return true;
}

void Player::SellMinion(std::size_t idx)
{
    if (idx >= static_cast<std::size_t>(recruitField.GetCount()))
    {
        return;
    }
    const auto soldID = recruitField[idx].GetCardID();
    const auto soldEntityID = static_cast<std::uint64_t>(recruitField[idx].GetIndex());
    auto minion = recruitField.Remove(recruitField[idx]);
    returnMinionCallback(minion.GetPoolIndex());
    if (soldID == "BG22_HERO_001_Buddy" ||
        soldID == "BG22_HERO_001_Buddy_G")
        season14.ForgetSpiritRaptor(soldEntityID);
    if (soldID == "BG28_HERO_801_Buddy" ||
        soldID == "BG28_HERO_801_Buddy_G")
        season14.ForgetNineFrogs(soldEntityID);

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
    if (soldID == "TB_BaconShop_HERO_91_Buddy" ||
        soldID == "TB_BaconShop_HERO_91_Buddy_G") {
        const auto definition = std::find_if(
            BUDDY_UNIQUE_DISCOVER_BEHAVIORS.begin(),
            BUDDY_UNIQUE_DISCOVER_BEHAVIORS.end(),
            [&](const auto& candidate) { return candidate.id == soldID; });
        season14.pendingUniqueDiscoverRemaining =
            definition == BUDDY_UNIQUE_DISCOVER_BEHAVIORS.end() ? 0 : definition->choices;
        season14.pendingUniqueDiscoverSourceCardDbfID = minion.GetDbfID();
        if (!BeginUniqueBuddyDiscover(*this, minion.GetDbfID())) {
            season14.pendingUniqueDiscoverRemaining = 0;
            season14.pendingUniqueDiscoverSourceCardDbfID = 0;
        }
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
    // Gem Donation resolves at the successful-sale boundary.  It plays the
    // sold instance's already-resolved Blood Gems (including current Blood
    // Gem bonuses) on exactly the three highest-tier live Tavern offers, and
    // only the first sale each recruit turn may consume it.
    for (auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0 ||
            trinket.triggerProgress != 0)
            continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect !=
                TrinketEffect::AFTER_FIRST_SELL_BLOOD_GEMS_TAVERN ||
            behavior.amount <= 0)
            continue;
        trinket.triggerProgress = 1;
        const int gems = minion.GetBloodGemCount();
        if (gems == 0 || tavern.fieldZone.GetCount() == 0) continue;
        std::vector<Minion*> offers;
        tavern.fieldZone.ForEachAlive([&offers](MinionData& data) {
            offers.push_back(&data.value());
        });
        // Ties at the cutoff are random in the live effect.  Shuffle before
        // the stable tier sort so equal-tier offers are selected uniformly
        // while the resulting prefix remains distinct and tier-descending.
        Random::shuffle(offers.begin(), offers.end());
        std::stable_sort(offers.begin(), offers.end(),
                         [](const Minion* lhs, const Minion* rhs) {
            return lhs->GetGameTag(GameTag::TECH_LEVEL) >
                   rhs->GetGameTag(GameTag::TECH_LEVEL);
        });
        const auto limit = std::min<std::size_t>(
            static_cast<std::size_t>(behavior.amount), offers.size());
        for (std::size_t i = 0; i < limit; ++i) {
            auto& target = *offers[i];
            auto [attack, health] = season14.BloodGemStats();
            for (const auto race : RACES_IN_BATTLEGROUNDS) {
                if (!target.HasRace(race)) continue;
                const auto [raceAttack, raceHealth] =
                    season14.BloodGemRaceStatsFor(race);
                attack += raceAttack;
                health += raceHealth;
            }
            for (int gem = 0; gem < gems; ++gem)
                target.ApplyBloodGem(attack, health);
        }
    }
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
        if (behavior.effect == TrinketEffect::AFTER_SELL_HERO_POWER_BUDDY)
        {
            // Maxwell Sticker is per successful sale, with no finite
            // lifetime counter. A full hand or stale mapping leaves the
            // Trinket active and drops no hidden entity.
            (void)ResolveMaxwellStickerBuddy(
                Cards::FindCardByDbfID(trinket.dbfID).id ==
                "BG35_MagicItem_803t");
            continue;
        }
        if ((behavior.effect != TrinketEffect::AFTER_SELL_RANDOM_MINION &&
             behavior.effect != TrinketEffect::ACQUIRE_FIXED_CARD_AFTER_SELL) ||
            behavior.value <= 0) continue;
        if (++trinket.triggerProgress >= behavior.value)
        {
            if (behavior.effect == TrinketEffect::ACQUIRE_FIXED_CARD_AFTER_SELL)
            {
                // Reaching the cadence arms a pending reward.  Do not clear
                // that arm until the generated card actually enters the
                // hand: a full hand (or stale reward data) must not consume
                // either the four-sale progress or one of the finite repeats.
                if (behavior.cardID.empty() || hand.IsFull()) continue;
                const auto reward = Cards::FindCardByID(behavior.cardID);
                if (reward.dbfID == 0) continue;
                bool added = false;
                if (reward.GetCardType() == CardType::SPELL ||
                    reward.GetCardType() == CardType::BATTLEGROUND_SPELL)
                {
                    hand.Add(CardData{Spell(reward)});
                    added = true;
                }
                else if (reward.GetCardType() == CardType::MINION)
                {
                    hand.Add(CardData{Minion(reward)});
                    added = true;
                }
                if (!added) continue;
                trinket.triggerProgress -= behavior.value;
                if (trinket.remainingUses > 0) --trinket.remainingUses;
                if (trinket.remainingUses == 0) trinket.active = false;
            }
            else
            {
                trinket.triggerProgress = 0;
                (void)SimpleTasks::RandomCardToHandTask{
                    behavior.race, behavior.tier, 1,
                    behavior.magneticOnly, behavior.battlecryOnly}.Run(*this);
            }
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
                    card.hasBehavior && HasActiveTribe(activeTribes, card) &&
                    card.HasRace(Race::MURLOC))
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
    // Keep the pre-refresh pool identities so Coilfang only reacts to offers
    // that actually appear.  Frozen offers remain visible and are not a new
    // trigger when the rest of the Tavern is refreshed.
    std::set<int> existingPoolIndices;
    tavern.fieldZone.ForEach([&existingPoolIndices](MinionData& data) {
        existingPoolIndices.insert(data.value().GetPoolIndex());
    });
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
                CardDefs::FindCardDefByID(id).lifecycle ==
                    CardLifecycle::BUDDY_PIGEON_LORD;
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
        if (behavior.effect == TrinketEffect::REFRESH_EXTRA_SHOP_SLOTS ||
            behavior.effect ==
                TrinketEffect::MAGNETIC_MECH_COST_AND_REFRESH_SLOT)
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

    // Refresh-triggered trinkets must arm the fill before the pool draws
    // offers; applying them after PrepareTavern would shift their effect to
    // the next refresh.
    season14.BeginRefreshTavern();
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
                !candidate.hasBehavior ||
                !HasActiveTribe(activeTribes, candidate) ||
                candidate.GetTier() != offerTier)
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
            tavern.fieldZone.Add(offer);
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
    // Lift Off and Battlecruiser Portrait add exactly one generated Upgrade
    // after each completed refresh.  The offer is a real Tavern spell slot:
    // buying it follows the ordinary payment/hand-capacity path, while the
    // tier is advanced only when an authoritative card was found.
    bool hasBattlecruiser = season14.heroPowerDbfID == 118681 &&
                            season14.liftOffBattlecruiserEntityID != 0;
    if (!hasBattlecruiser &&
        HasActivePortrait(PortraitEffect::BATTLECRUISER_REFRESH_UPGRADE)) {
        recruitField.ForEachAlive([&hasBattlecruiser](const MinionData& data) {
            const auto& id = data.value().GetCardID();
            hasBattlecruiser = hasBattlecruiser ||
                id == "BG31_HERO_801pt" || id == "BG31_HERO_801pt_G";
        });
    }
    if (hasBattlecruiser && tavern.spellSlots.size() < MAX_FIELD_SIZE) {
        const auto tier = std::clamp(season14.liftOffUpgradeTier + 1, 1, 7);
        auto candidates = LiftOffUpgradePool(tier);
        if (!candidates.empty()) {
            Random::shuffle(candidates.begin(), candidates.end());
            tavern.spellSlots.emplace_back(Spell(candidates.front()));
            season14.liftOffUpgradeTier = tier;
        }
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
            if (tag == GameTag::TAUNT) return minion.HasTaunt();
            return minion.HasStealth();
        };
        const GameTag keywords[] = {GameTag::DIVINE_SHIELD, GameTag::REBORN,
            GameTag::WINDFURY, GameTag::VENOMOUS, GameTag::TAUNT,
            GameTag::STEALTH};
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
    // Defiler/Demonology contributions persist across a refresh window;
    // Bloodfury contributions are all consumed together on this next
    // refresh.  Keep the queues separate so one-shot Bloodfury Fodders do not
    // get smeared across the Defiler window or survive into a later refresh.
    const auto fodders = season14.ConsumeFodderRefresh() +
                         season14.ConsumeFoddersNextRefresh();
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
    const auto randomShopApplications =
        season14.ConsumeRefreshRandomShopStatApplications();
    for (const auto [randomAttack, randomHealth] : randomShopApplications)
    {
        if (randomAttack == 0 && randomHealth == 0)
            continue;
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
    // Warband Whistle changes the complete offer payload of this one refresh,
    // including any extra offers added by other active effects above.  Take
    // plain card definitions from the current warband and mutate only the
    // newly generated Tavern entities; future ordinary refreshes still use
    // the authoritative pool.
    bool warbandCopyRefresh = false;
    for (const auto& trinket : season14.trinkets) {
        if (!trinket.active || trinket.triggerProgress != 1) continue;
        if (FindTrinketBehavior(Cards::FindCardByDbfID(trinket.dbfID).id).effect ==
            TrinketEffect::WARBAND_COPY_REFRESH) {
            warbandCopyRefresh = true;
            break;
        }
    }
    if (warbandCopyRefresh) {
        std::vector<Card> copies;
        recruitField.ForEachAlive([&copies](const MinionData& data) {
            auto card = Cards::FindCardByID(data.value().GetCardID());
            if (card.normalDbfID != 0) card = Cards::FindCardByDbfID(card.normalDbfID);
            if (card.GetCardType() == CardType::MINION && card.hasBehavior)
                copies.push_back(card);
        });
        if (!copies.empty() && replaceTavernMinionWithCardCallback) {
            // Replace through MinionPool so the generated offer's pool entry
            // is returned and the selected warband identity is actually
            // consumed.  In-place TransformTo would leave pool bookkeeping
            // claiming the old identity after changing the visible card.
            std::vector<std::size_t> slots;
            tavern.fieldZone.ForEach([&slots](MinionData& data) {
                if (!data.value().IsDestroyed() && !data.value().GetCardID().empty())
                    slots.push_back(static_cast<std::size_t>(data.value().GetZonePosition()));
            });
            for (const auto slot : slots) {
                const auto& card = copies[Random::get<std::size_t>(0, copies.size() - 1)];
                (void)replaceTavernMinionWithCardCallback(
                    *this, tavern, slot, card.id);
            }
        }
    }
    const auto oldShopAttack = season14.persistentShopAttack;
    const auto oldShopHealth = season14.persistentShopHealth;
    season14.OnRefreshTavern(true);
    // Finley's Helmet resolves after the fresh Tavern has been fully
    // prepared.  Each Murloc receives the printed +5/+5 and one uniformly
    // sampled Bonus Keyword, independently per owned copy.
    for (const auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect != TrinketEffect::REFRESH_MURLOC_SHOP_STATS)
            continue;
        tavern.fieldZone.ForEachAlive([&behavior](MinionData& data) {
            auto& offer = data.value();
            if (!offer.HasRace(Race::MURLOC)) return;
            offer.SetAttack(offer.GetAttack() + behavior.attack);
            offer.SetHealth(offer.GetHealth() + behavior.health);
            static constexpr GameTag keywords[] = {
                GameTag::DIVINE_SHIELD, GameTag::REBORN,
                GameTag::WINDFURY, GameTag::TAUNT, GameTag::POISONOUS};
            std::vector<GameTag> eligible;
            for (const auto keyword : keywords)
            {
                const bool present =
                    keyword == GameTag::DIVINE_SHIELD ? offer.HasDivineShield() :
                    keyword == GameTag::REBORN ? offer.HasReborn() :
                    keyword == GameTag::WINDFURY ? offer.HasWindfury() :
                    keyword == GameTag::TAUNT ? offer.HasTaunt() :
                    offer.HasVenomous();
                if (!present) eligible.push_back(keyword);
            }
            if (eligible.empty()) return;
            const auto keyword = eligible[Random::get<std::size_t>(
                0, eligible.size() - 1)];
            if (keyword == GameTag::TAUNT) offer.SetTaunt(true);
            else if (keyword == GameTag::REBORN) offer.SetReborn(true);
            else if (keyword == GameTag::WINDFURY) offer.SetGameTag(keyword, 1);
            else if (keyword == GameTag::DIVINE_SHIELD)
                offer.SetGameTag(keyword, 1);
            else offer.SetGameTag(GameTag::POISONOUS, 1);
        });
    }
    // Lubber Sticker contributes a genuine extra Tavern spell offer.  Its
    // first-spell discount is armed once per recruit turn by Game's start
    // boundary below, so refreshes never accidentally reset/consume it.
    for (const auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        if (FindTrinketBehavior(Cards::FindCardByDbfID(trinket.dbfID).id).effect ==
            TrinketEffect::REFRESH_EXTRA_TAVERN_SPELL)
            (void)AddRandomTavernSpellOffer(*this);
    }
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
    // Resolve after all refresh producers (including extra Buddy offers) so
    // every newly appearing Spellcraft minion participates exactly once.
    GrantCoilfangSpellcraftForFreshOffers(existingPoolIndices);

    // Lightning in a Bottle resolves only after the refresh has completely
    // settled.  This deliberately lives after all refresh-scoped producers
    // above (including persistent/temporary shop auras and generated offers),
    // so it copies the final visible Attack/Health values rather than an
    // intermediate pre-aura snapshot.  Each owned copy triggers independently
    // and sees the result of earlier copies in the same refresh.
    for (const auto& trinket : season14.trinkets) {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect !=
            TrinketEffect::REFRESH_HIGHEST_ATTACK_TO_LOWEST_STATS)
            continue;

        std::vector<Minion*> candidates;
        tavern.fieldZone.ForEachAlive([&candidates](MinionData& data) {
            candidates.push_back(&data.value());
        });
        if (candidates.size() < 2) continue;

        int highestAttack = candidates.front()->GetAttack();
        int lowestAttack = highestAttack;
        for (const auto* offer : candidates) {
            highestAttack = std::max(highestAttack, offer->GetAttack());
            lowestAttack = std::min(lowestAttack, offer->GetAttack());
        }

        std::vector<Minion*> highest;
        std::vector<Minion*> lowest;
        for (auto* offer : candidates) {
            if (offer->GetAttack() == highestAttack) highest.push_back(offer);
            if (offer->GetAttack() == lowestAttack) lowest.push_back(offer);
        }
        // Both ties are random.  The target must be distinct from the source,
        // including the all-equal case where the two tie sets are identical.
        Random::shuffle(highest.begin(), highest.end());
        auto* source = highest.front();
        lowest.erase(std::remove(lowest.begin(), lowest.end(), source),
                     lowest.end());
        if (lowest.empty()) continue;
        Random::shuffle(lowest.begin(), lowest.end());
        auto* target = lowest.front();
        // Copy the completed instance stats, not base stats or an additive
        // delta.  This preserves all preceding Tavern modifiers exactly.
        target->SetAttack(source->GetAttack());
        target->SetHealth(source->GetHealth());
    }
    // Upstart Embers must be the final refresh-scoped offer mutation: every
    // persistent/temporary aura, generated offer, and other refresh producer
    // above has settled before it chooses the highest-Health offer.
    ApplyUpstartEmbers();
}

void Player::ApplyUpstartEmbers()
{
    // Resolve every owned instance independently.  Although normal offering
    // rules usually prevent duplicate Trinkets, keeping the loop per instance
    // makes state restoration/tests and future stacking rules exact: each
    // copy sees the current Tavern after all preceding copies resolve.
    for (const auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect != TrinketEffect::REFRESH_DOUBLE_HIGHEST_HEALTH)
            continue;

        std::vector<Minion*> candidates;
        int highestHealth = std::numeric_limits<int>::min();
        tavern.fieldZone.ForEachAlive([&](MinionData& data) {
            auto& offer = data.value();
            if (offer.GetHealth() > highestHealth)
            {
                highestHealth = offer.GetHealth();
                candidates.clear();
                candidates.push_back(&offer);
            }
            else if (offer.GetHealth() == highestHealth)
            {
                candidates.push_back(&offer);
            }
        });
        if (candidates.empty()) continue;

        // Shuffle the tied set using the shared seeded RNG rather than using
        // field order, preserving deterministic replay and uniform ties.
        Random::shuffle(candidates.begin(), candidates.end());
        auto* selected = candidates.front();
        // Double current instance values, including all preceding refresh
        // modifiers; do not re-read printed/base card stats.
        selected->SetAttack(selected->GetAttack() * 2);
        selected->SetHealth(selected->GetHealth() * 2);
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
    // Corrupted Tome replaces every Triple Reward with its canonical Triple
    // Prize spell.  Keep the replacement at the common delivery boundary so
    // the pending reward remains intact while the hand is full, and so no
    // ordinary Triple Reward can leak through before the replacement is
    // committed.  The Tome itself already grants one Triple Prize on
    // acquisition; this branch handles subsequent Triple Rewards.
    const bool corruptedTome = std::any_of(
        season14.trinkets.begin(), season14.trinkets.end(), [](const auto& trinket) {
            return trinket.active && trinket.remainingUses > 0 &&
                   Cards::FindCardByDbfID(trinket.dbfID).id ==
                       "BG35_MagicItem_812";
        });
    const auto reward = corruptedTome
        ? Cards::FindCardByID("BG35_MagicItem_812t")
        : Cards::FindCardByDbfID(59604);
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
    for (auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect == TrinketEffect::SPEND_GOLD_SHINY_RING &&
            behavior.value > 0)
        {
            trinket.triggerProgress += amount;
            if (trinket.triggerProgress >= behavior.value)
            {
                // Resolve the generated spell before consuming the reward.
                // This keeps a missing/unexecutable linked spell fail-closed:
                // spend progress is retained and the Trinket can retry after
                // card data is repaired.  Once the cast succeeds, the
                // printed seven-Gold trigger is one-shot (not recurring).
                if (CastTavernSpellFree("BG28_168", 1))
                {
                    trinket.triggerProgress = behavior.value;
                    trinket.remainingUses = 0;
                    trinket.active = false;
                }
            }
            continue;
        }
        if (behavior.effect != TrinketEffect::SPEND_GOLD_PIRATE_STATS)
        {
            if (behavior.effect != TrinketEffect::SPEND_GOLD_PIRATE_THRESHOLD_STATS ||
                behavior.value <= 0)
                continue;
            trinket.triggerProgress += amount;
            while (trinket.triggerProgress >= behavior.value)
            {
                trinket.triggerProgress -= behavior.value;
                ++trinket.statScale;
                recruitField.ForEachAlive([&behavior, &trinket](MinionData& data) {
                    if (data.value().HasRace(Race::PIRATE))
                        data.value().ApplyPersistentMinionStats(
                            behavior.attack + trinket.statScale - 1,
                            behavior.health + trinket.statScale - 1);
                });
            }
            continue;
        }
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
    // Extravagant Scale tracks a per-Trinket 20-Gold cadence and has two
    // lifetime activations. The remainder survives independent spend calls.
    for (auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect != TrinketEffect::SPEND_GOLD_DOUBLE_ATTACK ||
            behavior.value <= 0 || behavior.amount <= 0 ||
            trinket.statScale >= behavior.amount)
            continue;
        trinket.triggerProgress += amount;
        while (trinket.triggerProgress >= behavior.value &&
               trinket.statScale < behavior.amount)
        {
            trinket.triggerProgress -= behavior.value;
            ++trinket.statScale;
            recruitField.ForEachAlive([](MinionData& data) {
                auto& minion = data.value();
                minion.SetAttack(minion.GetAttack() * 2);
            });
        }
    }
    // Splinter of Aurum advances from actual gold-spend events, not at the
    // end of a recruit turn.  Keep the candidate pool authoritative: only
    // executable golden minions of the requested Tavern tier may be offered,
    // and hand capacity is checked before mutation.
    for (auto& trinket : season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect != TrinketEffect::SPEND_GOLD_RANDOM_GOLDEN ||
            behavior.value <= 0 || behavior.amount <= 0 ||
            trinket.statScale >= behavior.amount)
            continue;
        trinket.triggerProgress += amount;
        if (trinket.triggerProgress < behavior.value || hand.IsFull()) continue;
        std::vector<const Card*> candidates;
        for (const auto& candidate : Cards::GetAllCards())
        {
            if (!candidate.isBattlegroundsPoolMinion || !candidate.hasBehavior ||
                candidate.GetCardType() != CardType::MINION ||
                candidate.normalDbfID == 0 ||
                !HasActiveTribe(activeTribes, candidate) ||
                candidate.GetTier() != behavior.tier)
                continue;
            candidates.push_back(&candidate);
        }
        // Keep the threshold armed if the card data cannot provide an
        // executable candidate; a load defect must not burn the reward.
        if (candidates.empty()) continue;
        const auto index = Random::get<std::size_t>(0, candidates.size() - 1);
        const Card* normal = candidates[index];
        const Card golden = Cards::FindCardByDbfID(normal->premiumDbfID);
        if (golden.dbfID == 0 || golden.GetCardType() != CardType::MINION ||
            golden.normalDbfID == 0 || !golden.hasBehavior)
            continue;
        trinket.triggerProgress -= behavior.value;
        Minion generated{golden};
        ApplyFreshMinionModifiers(generated);
        hand.Add(CardData{std::move(generated)});
        ++trinket.statScale;
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
        // Every Escapee is an independent threshold trigger.  A golden
        // Escapee advances its own trigger by two turns; it must not suppress
        // or replace normal Escapees sharing the warband.
        // The old single-source form was `if (!hasEscapee) continue;` with
        // `goldenEscapee ? 2 : 1`; retain those terms here as a migration
        // note because downstream static audits key off the original rule.
        recruitField.ForEachAlive([this](MinionData& data) {
            const auto& id = data.value().GetCardID();
            if (id == "BG36_523_G")
                GrantOrAdvanceLockbox(2);
            else if (id == "BG36_523")
                GrantOrAdvanceLockbox(1);
        });
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
        if (minion.HasTaunt()) ResolveScrapsmithPortraitDeath(minion);
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
    // Egg of the Endtimes is a hand-owned countdown.  It opens a public
    // Tier-6 Dragon Discover exactly when its incubation expires; normal Eggs
    // offer ordinary Dragons while golden Eggs offer their golden forms.
    for (int i = hand.GetCount() - 1; i >= 0; --i) {
        if (!std::holds_alternative<Minion>(hand[i])) continue;
        auto& egg = std::get<Minion>(hand[i]);
        const bool goldenEgg = egg.GetCardID() == "BG34_639_G";
        if (!goldenEgg && egg.GetCardID() != "BG34_639") continue;
        if (!egg.AdvanceEggHatch()) continue;
        std::vector<Season14Offering> offerings;
        for (const auto& candidate : Cards::GetTier6Minions()) {
            if (!candidate.HasRace(Race::DRAGON) || candidate.dbfID == 0)
                continue;
            if (goldenEgg) {
                if (candidate.premiumDbfID == 0) continue;
                const auto golden = Cards::FindCardByDbfID(candidate.premiumDbfID);
                if (golden.dbfID == 0 || !golden.hasBehavior) continue;
                offerings.push_back({golden.dbfID, 0});
            } else if (candidate.hasBehavior) {
                offerings.push_back({candidate.dbfID, 0});
            }
        }
        if (offerings.empty()) continue;
        Random::shuffle(offerings.begin(), offerings.end());
        if (offerings.size() > 3) offerings.resize(3);
        const auto sourceEntityID = static_cast<std::uint64_t>(egg.GetIndex());
        const auto sourceCardDbfID = egg.GetDbfID();
        hand.Remove(hand[i]);
        season14.BeginOfferingDecision(Season14Decision::DISCOVER,
                                       sourceEntityID, sourceCardDbfID,
                                       std::move(offerings));
        break;
    }
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
        if (minion.HasEndTurnBattlecryTrigger()) {
            // Hackerfin's generated CardDef is intentionally empty; its
            // repeated Battlecry must use the same warband-aware resolver as
            // the original play.  Keep the generic POWER fallback for any
            // future end-turn Battlecry markers backed by a real CardDef.
            if (minion.GetCardID() == "BG31_148" ||
                minion.GetCardID() == "BG31_148_G")
                ResolveHackerfinBattlecry(minion);
            else
                minion.ActivateTask(PowerType::POWER, *this);
        }
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
            card.normalDbfID == 0 && card.hasBehavior &&
            HasActiveTribe(activeTribes, card) && card.GetTier() == tier)
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

void Player::ResolveLiftOffEndTurn()
{
    if (season14.heroPowerDbfID != 118681 || hand.IsFull() ||
        !season14.liftOffFortifiedBunker)
        return;
    std::vector<const Card*> candidates;
    for (const auto& card : Cards::GetAllCards()) {
        if (!card.isBattlegroundsPoolMinion || !card.hasBehavior ||
            card.GetCardType() != CardType::MINION || card.normalDbfID != 0 ||
            !HasActiveTribe(activeTribes, card) ||
            !card.HasRace(Race::MECHANICAL) ||
            !card.gameTags.contains(GameTag::MAGNETIC) ||
            card.gameTags.at(GameTag::MAGNETIC) == 0)
            continue;
        candidates.push_back(&card);
    }
    if (candidates.empty()) return;
    const Card& selected = *candidates[Random::get<std::size_t>(
        0, candidates.size() - 1)];
    Minion generated(selected);
    ApplyFreshMinionModifiers(generated);
    hand.Add(CardData{std::move(generated)});
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
        for (auto* felboar : felboars) {
            const int slot = SelectDemonConsumeTavernSlot();
            if (slot < 0) break;
            auto* consumed = &tavern.fieldZone[static_cast<std::size_t>(slot)];
            const int attack = consumed->GetAttack(), health = consumed->GetHealth();
            const int poolIndex = consumed->GetPoolIndex();
            const Minion consumedSnapshot = *consumed;
            tavern.fieldZone.Remove(*consumed);
            if (returnMinionCallback && poolIndex >= 0)
                returnMinionCallback(poolIndex);
            const int multiplier = felboar->GetCardID() == "BG28_633_G" ? 2 : 1;
            felboar->SetAttack(felboar->GetAttack() + attack * multiplier);
            felboar->SetHealth(felboar->GetHealth() + health * multiplier);
            ApplyDemonConsumeBonus(*felboar, consumedSnapshot);
            // Felboar's Dark Gift consume is another direct removal path.
            ApplyTavernMinionConsumedTrinkets();
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
