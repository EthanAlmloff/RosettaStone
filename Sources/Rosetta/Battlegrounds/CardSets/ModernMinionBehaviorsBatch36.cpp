#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch36.hpp>

namespace RosettaStone::Battlegrounds
{
void ModernMinionBehaviorsBatch36::AddAll(
    std::map<std::string, CardDef>& cards)
{
    // The source minion and generated option identities are resolved by the
    // dedicated Choose One state machine in Player.cpp. Do not attach a
    // second Power graph here: that would execute the branch twice.
    cards.emplace("BG27_084", CardDef{});
    cards.emplace("BG27_084_G", CardDef{});
    cards.emplace("BG27_084t", CardDef{});
    cards.emplace("BG27_084t2", CardDef{});
    cards.emplace("BG27_084_Gt", CardDef{});
    cards.emplace("BG27_084_Gt2", CardDef{});
    cards.emplace("BG30_123", CardDef{});
    cards.emplace("BG30_123_G", CardDef{});
    cards.emplace("BG30_123t", CardDef{});
    cards.emplace("BG30_123_Gt", CardDef{});
    cards.emplace("BG30_123t2", CardDef{});
    cards.emplace("BG30_123_Gt2", CardDef{});
    cards.emplace("BG36_330", CardDef{});
    cards.emplace("BG36_330_G", CardDef{});
    cards.emplace("BG36_330t", CardDef{});
    cards.emplace("BG36_330_Gt", CardDef{});
    cards.emplace("BG36_330t2", CardDef{});
    cards.emplace("BG36_330_Gt2", CardDef{});
}
}  // namespace RosettaStone::Battlegrounds
