#include <Rosetta/Battlegrounds/CardSets/ModernTokenBehaviorsBatch77.hpp>

namespace RosettaStone::Battlegrounds
{
void ModernTokenBehaviorsBatch77::AddAll(std::map<std::string, CardDef>& cards)
{
    // Imperial Defender copies the resolved spell effect onto itself.  The
    // once-per-turn guard and target identity live in Player::PlaySpell, so
    // an empty CardDef here is intentional but still executable: the entity
    // is registered and cannot fall through as unknown metadata.
    cards.emplace("BG22_HERO_007_Buddy", CardDef{});
    cards.emplace("BG22_HERO_007_Buddy_G", CardDef{});
}
}
