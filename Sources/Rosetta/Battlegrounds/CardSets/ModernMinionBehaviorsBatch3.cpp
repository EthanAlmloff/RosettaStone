// Copyright (c) 2026 Hearthstone BG AI contributors

#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch3.hpp>

namespace RosettaStone::Battlegrounds
{
namespace
{
void AddStatic(std::map<std::string, CardDef>& cards, const char* id)
{
    // CardLoader copies the authoritative keyword tags (including Divine
    // Shield) from the pinned metadata snapshot. The empty definition is
    // therefore the complete custom behavior for this static-only card.
    cards.emplace(id, CardDef{});
}
}  // namespace

void ModernMinionBehaviorsBatch3::AddAll(
    std::map<std::string, CardDef>& cards)
{
    // BG26_175 Elemental of Surprise is a static Divine Shield minion. Its
    // normal and golden forms share the same keyword behavior; their stats
    // and golden identity remain metadata-owned.
}
}  // namespace RosettaStone::Battlegrounds
