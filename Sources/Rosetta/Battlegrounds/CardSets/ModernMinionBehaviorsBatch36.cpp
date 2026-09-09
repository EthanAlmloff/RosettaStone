#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch36.hpp>

namespace RosettaStone::Battlegrounds
{
void ModernMinionBehaviorsBatch36::AddAll(
    std::map<std::string, CardDef>& cards)
{
    // The source minion and generated option identities are resolved by the
    // dedicated Choose One state machine in Player.cpp. Do not attach a
    // second Power graph here: that would execute the branch twice. The
    // lifecycle tags below are executable ownership metadata, not empty
    // registration markers: CardDefs and Player agree on the modal owner.
    cards.emplace("BG27_084", CardDef{CardLifecycle::CHOOSE_ONE_SOURCE});
    cards.emplace("BG27_084_G", CardDef{CardLifecycle::CHOOSE_ONE_SOURCE});
    cards.emplace("BG27_084t", CardDef{CardLifecycle::CHOOSE_ONE_OPTION});
    cards.emplace("BG27_084t2", CardDef{CardLifecycle::CHOOSE_ONE_OPTION});
    cards.emplace("BG27_084_Gt", CardDef{CardLifecycle::CHOOSE_ONE_OPTION});
    cards.emplace("BG27_084_Gt2", CardDef{CardLifecycle::CHOOSE_ONE_OPTION});
    cards.emplace("BG30_123", CardDef{CardLifecycle::CHOOSE_ONE_SOURCE});
    cards.emplace("BG30_123_G", CardDef{CardLifecycle::CHOOSE_ONE_SOURCE});
    cards.emplace("BG30_123t", CardDef{CardLifecycle::CHOOSE_ONE_OPTION});
    cards.emplace("BG30_123_Gt", CardDef{CardLifecycle::CHOOSE_ONE_OPTION});
    cards.emplace("BG30_123t2", CardDef{CardLifecycle::CHOOSE_ONE_OPTION});
    cards.emplace("BG30_123_Gt2", CardDef{CardLifecycle::CHOOSE_ONE_OPTION});
    cards.emplace("BG36_330", CardDef{CardLifecycle::CHOOSE_ONE_SOURCE});
    cards.emplace("BG36_330_G", CardDef{CardLifecycle::CHOOSE_ONE_SOURCE});
    cards.emplace("BG36_330t", CardDef{CardLifecycle::CHOOSE_ONE_OPTION});
    cards.emplace("BG36_330_Gt", CardDef{CardLifecycle::CHOOSE_ONE_OPTION});
    cards.emplace("BG36_330t2", CardDef{CardLifecycle::CHOOSE_ONE_OPTION});
    cards.emplace("BG36_330_Gt2", CardDef{CardLifecycle::CHOOSE_ONE_OPTION});
    cards.emplace("BG36_341", CardDef{CardLifecycle::CHOOSE_ONE_SOURCE});
    cards.emplace("BG36_341_G", CardDef{CardLifecycle::CHOOSE_ONE_SOURCE});
    cards.emplace("BG36_341t", CardDef{CardLifecycle::CHOOSE_ONE_OPTION});
    cards.emplace("BG36_341_Gt", CardDef{CardLifecycle::CHOOSE_ONE_OPTION});
    cards.emplace("BG36_341t2", CardDef{CardLifecycle::CHOOSE_ONE_OPTION});
    cards.emplace("BG36_341_Gt2", CardDef{CardLifecycle::CHOOSE_ONE_OPTION});

    // These older Choose One minions use the same public modal state machine
    // as the Season 14 sources above.  Register both normal and golden
    // generated options so replay validation cannot reject a valid choice.
    cards.emplace("BG31_320", CardDef{CardLifecycle::CHOOSE_ONE_SOURCE});
    cards.emplace("BG31_320_G", CardDef{CardLifecycle::CHOOSE_ONE_SOURCE});
    cards.emplace("BG31_320t", CardDef{CardLifecycle::CHOOSE_ONE_OPTION});
    cards.emplace("BG31_320_Gt", CardDef{CardLifecycle::CHOOSE_ONE_OPTION});
    cards.emplace("BG31_320t2", CardDef{CardLifecycle::CHOOSE_ONE_OPTION});
    cards.emplace("BG31_320_Gt2", CardDef{CardLifecycle::CHOOSE_ONE_OPTION});
    cards.emplace("BG32_237", CardDef{CardLifecycle::CHOOSE_ONE_SOURCE});
    cards.emplace("BG32_237_G", CardDef{CardLifecycle::CHOOSE_ONE_SOURCE});
    cards.emplace("BG32_237t", CardDef{CardLifecycle::CHOOSE_ONE_OPTION});
    cards.emplace("BG32_237_Gt", CardDef{CardLifecycle::CHOOSE_ONE_OPTION});
    cards.emplace("BG32_237t2", CardDef{CardLifecycle::CHOOSE_ONE_OPTION});
    cards.emplace("BG32_237_Gt2", CardDef{CardLifecycle::CHOOSE_ONE_OPTION});
    cards.emplace("BG36_332", CardDef{CardLifecycle::CHOOSE_ONE_SOURCE});
    cards.emplace("BG36_332_G", CardDef{CardLifecycle::CHOOSE_ONE_SOURCE});
    cards.emplace("BG36_332t", CardDef{CardLifecycle::CHOOSE_ONE_OPTION});
    cards.emplace("BG36_332_Gt", CardDef{CardLifecycle::CHOOSE_ONE_OPTION});
    cards.emplace("BG36_332t2", CardDef{CardLifecycle::CHOOSE_ONE_OPTION});
    cards.emplace("BG36_332_Gt2", CardDef{CardLifecycle::CHOOSE_ONE_OPTION});
}
}  // namespace RosettaStone::Battlegrounds
