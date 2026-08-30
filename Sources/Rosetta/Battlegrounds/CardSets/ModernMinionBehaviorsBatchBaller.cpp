#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatchBaller.hpp>
namespace RosettaStone::Battlegrounds
{
void ModernMinionBehaviorsBatchBaller::AddAll(std::map<std::string, CardDef>& cards)
{
    cards.emplace("BG31_816", CardDef{});
    cards.emplace("BG31_816_G", CardDef{});
    cards.emplace("BG31_818", CardDef{});
    cards.emplace("BG31_818_G", CardDef{});
}
}
