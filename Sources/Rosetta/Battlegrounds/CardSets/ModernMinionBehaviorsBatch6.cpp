// Copyright (c) 2026 Hearthstone BG AI contributors

#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch6.hpp>

namespace RosettaStone::Battlegrounds
{
namespace
{
void AddStatic(std::map<std::string, CardDef>& cards, const char* id)
{
    // These entities have no custom trigger/effect chain in the pinned
    // metadata. CardLoader supplies their stats and keyword tags; the empty
    // CardDef is therefore the complete simulator behavior for this family.
    cards.emplace(id, CardDef{});
}
}  // namespace

void ModernMinionBehaviorsBatch6::AddAll(
    std::map<std::string, CardDef>& cards)
{
    // These generated minions have no text/effect chain in the pinned
    // snapshot. Their producers are registered in other behavior batches;
    // keeping their definitions here makes every generated entity that can
    // reach the simulator resolvable without duplicating producer effects.
}
}  // namespace RosettaStone::Battlegrounds
