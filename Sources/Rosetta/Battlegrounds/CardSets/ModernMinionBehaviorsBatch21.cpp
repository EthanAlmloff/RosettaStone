#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch21.hpp>
namespace RosettaStone::Battlegrounds
{
void ModernMinionBehaviorsBatch21::AddAll(std::map<std::string, CardDef>& cards)
{
    // Minted Corsair is resolved by the explicit sell lookup in Player.
    static_cast<void>(cards);
}
}
