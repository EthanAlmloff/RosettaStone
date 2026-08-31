#include <Rosetta/Battlegrounds/CardSets/TrinketBehaviors.hpp>

namespace RosettaStone::Battlegrounds
{
TrinketBehavior FindTrinketBehavior(std::string_view id) noexcept
{
    // Only effects whose complete acquisition and lifecycle semantics are
    // implemented are registered here.  Nether Pendant and both Cheese
    // Wheel forms improve on a counter; Innkeeper's Stein and Minion Bait
    // trigger on refresh; Goblin Wallet triggers at end of turn.  Treating
    // any of those as an unconditional aura/bonus would silently change the
    // game, so they remain fail-closed until their trigger state is modeled.
    if (id == "BG30_MagicItem_996") return {TrinketEffect::GOLD_AND_MAX_GOLD, 0, 0, 4};
    if (id == "BG30_MagicItem_998") return {TrinketEffect::IMMEDIATE_GOLD, 0, 0, 2};
    if (id == "BG30_MagicItem_841") return {TrinketEffect::SHOP_STATS_AND_EXTRA_SLOT, 3, 3, 1};
    if (id == "BG36_MagicItem_220") return {TrinketEffect::START_TURN_GOLD_PER_MINION_TYPE};
    if (id == "BG32_MagicItem_858") return {TrinketEffect::ACQUIRE_RANDOM_MINIONS, 0, 0, 0, Race::INVALID, 4, 3};
    if (id == "BG30_MagicItem_993") return {TrinketEffect::ACQUIRE_RANDOM_MINIONS, 0, 0, 0, Race::INVALID, 7, 1, true};
    if (id == "BG30_MagicItem_430") return {TrinketEffect::ACQUIRE_RANDOM_MINIONS, 0, 0, 0, Race::INVALID, 0, 1, true, true, false};
    if (id == "BG32_MagicItem_172") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, true, false, false, "BG31_176"};
    if (id == "BG30_MagicItem_543") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, true, false, false, "EBG_Spell_032"};
    if (id == "BG35_MagicItem_303") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG34_Giant_072"};
    // These effects are resolved at recruit-end by Game, after all player
    // actions have completed.  Keeping them out of the generic passive path
    // prevents Goblin Wallet from increasing the cap before its first turn.
    if (id == "BG30_MagicItem_847") return {TrinketEffect::END_TURN_MAX_GOLD, 0, 0, 1};
    if (id == "BG32_MagicItem_231") return {TrinketEffect::END_TURN_GOLDEN_STATS, 3, 3};
    if (id == "BG32_MagicItem_231t") return {TrinketEffect::END_TURN_GOLDEN_STATS, 10, 10};
    if (id == "BG30_MagicItem_914") return {TrinketEffect::AFTER_PLAY_HAND_BUFF, 3, 3};
    if (id == "BG30_MagicItem_914t") return {TrinketEffect::AFTER_PLAY_HAND_BUFF, 6, 6};
    if (id == "BG30_MagicItem_544") return {TrinketEffect::AFTER_PLAY_ELEMENTAL_SHOP_BUFF, 2, 2};
    if (id == "BG30_MagicItem_544t") return {TrinketEffect::AFTER_PLAY_ELEMENTAL_SHOP_BUFF, 5, 5};
    if (id == "BG36_MagicItem_800") return {TrinketEffect::AFTER_TAVERN_SPELL_SHOP_BUFF, 1, 1};
    if (id == "BG35_MagicItem_710") return {TrinketEffect::AFTER_TAVERN_SPELL_RACE_BUFF, 2, 2, 0, Race::PIRATE};
    if (id == "BG30_MagicItem_414") return {TrinketEffect::AFTER_BUY_RANDOM_FRIENDLY_BUFF, 2, 1, 2};
    if (id == "BG30_MagicItem_414t") return {TrinketEffect::AFTER_BUY_RANDOM_FRIENDLY_BUFF, 4, 4, 2};
    if (id == "BG30_MagicItem_984") return {TrinketEffect::END_TURN_DIVINE_SHIELD_ATTACK, 3, 0};
    if (id == "BG30_MagicItem_984t") return {TrinketEffect::END_TURN_DIVINE_SHIELD_ATTACK, 7, 0};
    if (id == "BG30_MagicItem_900") return {TrinketEffect::AFTER_PLAY_CARD_RANDOM_RACE_BUFF, 4, 4, 1, Race::DRAGON};
    if (id == "BG30_MagicItem_900t") return {TrinketEffect::AFTER_PLAY_CARD_RANDOM_RACE_BUFF, 6, 4, 1, Race::DRAGON};
    if (id == "BG30_MagicItem_989") return {TrinketEffect::STATIC_RACE_STATS, 3, 0, 0, Race::UNDEAD};
    if (id == "BG30_MagicItem_989t") return {TrinketEffect::STATIC_RACE_STATS, 15, 0, 0, Race::UNDEAD};
    if (id == "BG36_MagicItem_205") return {TrinketEffect::AFTER_REBORN_STATS, 2, 2};
    if (id == "BG36_MagicItem_215") return {TrinketEffect::DUPLICATE_DRAGON_BATTLECRY};
    if (id == "BG36_MagicItem_811") return {TrinketEffect::FIRST_MINION_DIVINE_SHIELD};
    if (id == "BG36_MagicItem_202") return {TrinketEffect::BATTLECRY_BUY_DISCOUNT, 0, 0, 2};
    return {};
}

void TrinketBehaviors::AddAll(std::map<std::string, CardDef>& cards)
{
    cards.emplace("BG30_MagicItem_996", CardDef{});
    cards.emplace("BG30_MagicItem_998", CardDef{});
    cards.emplace("BG30_MagicItem_841", CardDef{});
    cards.emplace("BG36_MagicItem_220", CardDef{});
    cards.emplace("BG32_MagicItem_858", CardDef{});
    cards.emplace("BG30_MagicItem_993", CardDef{});
    cards.emplace("BG30_MagicItem_430", CardDef{});
    cards.emplace("BG32_MagicItem_172", CardDef{});
    cards.emplace("BG30_MagicItem_543", CardDef{});
    cards.emplace("BG35_MagicItem_303", CardDef{});
    cards.emplace("BG30_MagicItem_847", CardDef{});
    cards.emplace("BG32_MagicItem_231", CardDef{});
    cards.emplace("BG32_MagicItem_231t", CardDef{});
    cards.emplace("BG30_MagicItem_914", CardDef{});
    cards.emplace("BG30_MagicItem_914t", CardDef{});
    cards.emplace("BG30_MagicItem_544", CardDef{});
    cards.emplace("BG30_MagicItem_544t", CardDef{});
    cards.emplace("BG36_MagicItem_800", CardDef{});
    cards.emplace("BG35_MagicItem_710", CardDef{});
    cards.emplace("BG30_MagicItem_414", CardDef{});
    cards.emplace("BG30_MagicItem_414t", CardDef{});
    cards.emplace("BG30_MagicItem_984", CardDef{});
    cards.emplace("BG30_MagicItem_984t", CardDef{});
    cards.emplace("BG30_MagicItem_900", CardDef{});
    cards.emplace("BG30_MagicItem_900t", CardDef{});
    cards.emplace("BG30_MagicItem_989", CardDef{});
    cards.emplace("BG30_MagicItem_989t", CardDef{});
    cards.emplace("BG36_MagicItem_205", CardDef{});
    cards.emplace("BG36_MagicItem_215", CardDef{});
    cards.emplace("BG36_MagicItem_811", CardDef{});
    cards.emplace("BG36_MagicItem_202", CardDef{});
}
}  // namespace RosettaStone::Battlegrounds
