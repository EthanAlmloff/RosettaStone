#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch14.hpp>
namespace RosettaStone::Battlegrounds
{
void ModernMinionBehaviorsBatch14::AddAll(
    std::map<std::string, CardDef>& cards)
{
    // Accord-o-Tron is owned by ModernMinionBehaviorsBatch16.  This batch
    // originally duplicated those rows, making ownership depend on order.
    static_cast<void>(cards);
}
}  // namespace RosettaStone::Battlegrounds
