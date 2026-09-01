#include <Rosetta/Battlegrounds/CardSets/EventCounterBehaviors.hpp>

namespace RosettaStone::Battlegrounds
{
void EventCounterBehaviors::AddAll(std::map<std::string, CardDef>& cards)
{
    // Keep registration data-driven.  Empty definitions are intentional for
    // effects whose simulator executor is not yet available; CardLoader can
    // still recognize the card while execution fails closed.
    for (const auto& spec : EventCounterSpecs)
        cards.emplace(std::string(spec.id), CardDef{});
}
}  // namespace RosettaStone::Battlegrounds
