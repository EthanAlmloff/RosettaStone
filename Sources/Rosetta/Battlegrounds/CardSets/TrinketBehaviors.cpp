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
    if (id == "BG30_MagicItem_841") return {TrinketEffect::SHOP_STATS_AND_EXTRA_SLOT, 3, 3, 1};
    return {};
}

void TrinketBehaviors::AddAll(std::map<std::string, CardDef>& cards)
{
    cards.emplace("BG30_MagicItem_996", CardDef{});
    cards.emplace("BG30_MagicItem_841", CardDef{});
}
}  // namespace RosettaStone::Battlegrounds
