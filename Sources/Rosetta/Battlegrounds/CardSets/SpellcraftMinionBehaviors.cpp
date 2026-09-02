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
    cards.emplace("BG27_024", CardDef{});
    cards.emplace("BG26_171", CardDef{});
    cards.emplace("BG26_171_G", CardDef{});
    cards.emplace("BG27_024_G", CardDef{});
    // Golden Thrasher uses the same typed temporary keyword payloads; the
    // golden enchantment identities remain explicit for replay/coverage.
    cards.emplace("BG27_024_Ge1", CardDef{});
    cards.emplace("BG27_024_Ge2", CardDef{});
    cards.emplace("BG27_024_Ge3", CardDef{});
    cards.emplace("BG31_830", CardDef{});
    cards.emplace("BG31_830_G", CardDef{});
    cards.emplace("BG31_924", CardDef{});
    cards.emplace("BG31_924_G", CardDef{});
    cards.emplace("BG26_501", CardDef{});
    cards.emplace("BG26_501_G", CardDef{});
    // Tranquil Meditative is a continuous Tavern-spell stat aura.  Its
    // effect is resolved in Player::PlaySpell so generated and modal spells
    // receive the same bonus; no standalone trigger is required here.
    cards.emplace("BG32_835", CardDef{});
    cards.emplace("BG32_835_G", CardDef{});
    // Nalaa and Charging Czarina resolve their Tavern-spell triggers at the
    // shared successful-cast boundary in Player, with golden scaling and
    // seeded target selection kept in that central path.
    cards.emplace("BG28_551", CardDef{});
    cards.emplace("BG28_551_G", CardDef{});
    cards.emplace("BG28_741", CardDef{});
    cards.emplace("BG28_741_G", CardDef{});
}
}
