#include <Rosetta/Battlegrounds/CardSets/SpellcraftMinionBehaviors.hpp>
namespace RosettaStone::Battlegrounds
{
void SpellcraftMinionBehaviors::AddAll(std::map<std::string, CardDef>& cards)
{
    // Spellcraft is emitted by Player::RefreshSpellcraft; the minion itself
    // has no immediate task chain.
    cards.emplace("BGS_200", CardDef{});
    cards.emplace("TB_BaconUps_256", CardDef{});
    cards.emplace("BG23_000", CardDef{});
    cards.emplace("BG23_000_G", CardDef{});
    cards.emplace("BG23_004", CardDef{});
    cards.emplace("BG23_004_G", CardDef{});
    cards.emplace("BG23_007", CardDef{});
    cards.emplace("BG23_007_G", CardDef{});
    cards.emplace("BG23_008", CardDef{});
    cards.emplace("BG23_008_G", CardDef{});
    cards.emplace("BG31_830", CardDef{});
    cards.emplace("BG31_830_G", CardDef{});
}
}
