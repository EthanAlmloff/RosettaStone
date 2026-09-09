#include <Rosetta/Battlegrounds/CardSets/ModernTokenBehaviorsBatch76.hpp>

namespace RosettaStone::Battlegrounds {
void ModernTokenBehaviorsBatch76::AddAll(
    std::map<std::string, CardDef>& cards) {
  // Curator Sticker's Mishmash is resolved by Player's authoritative stat-gain
  // dispatch hook.  Keep all generated identities in the normal CardDefs path
  // so the reward is executable rather than metadata-only.  The Amalgam token
  // is a generated source snapshot and intentionally has no CardDef task.
  cards.insert_or_assign("TB_BaconShop_HERO_33_Buddy", CardDef{});
  cards.insert_or_assign("TB_BaconShop_HERO_33_Buddy_G", CardDef{});
  cards.insert_or_assign("TB_BaconShop_HP_033t", CardDef{});
}
}  // namespace RosettaStone::Battlegrounds
