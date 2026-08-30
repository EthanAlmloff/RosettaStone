#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch26.hpp>
#include <utility>
namespace RosettaStone::Battlegrounds
{
namespace
{
void AddActivate(std::map<std::string, CardDef>& cards, const char* id,
                 ActivateDefinition definition)
{
    Power power;
    power.AddActivate(std::move(definition));
    cards.emplace(id, CardDef{ std::move(power) });
}
}

void ModernMinionBehaviorsBatch26::AddAll(std::map<std::string, CardDef>& cards)
{
    AddActivate(cards, "BG36_509", { ActivateEffect::GAIN_GOLD, 1, 0, 0, 3, true });
    AddActivate(cards, "BG36_509_G", { ActivateEffect::GAIN_GOLD, 1, 0, 0, 6, true });
    AddActivate(cards, "BG36_346", { ActivateEffect::ADD_CARD, 1, 0, 0, 2, false, "BG28_897" });
    AddActivate(cards, "BG36_346_G", { ActivateEffect::ADD_CARD, 1, 0, 0, 4, false, "BG28_897" });
}
}
