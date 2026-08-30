// Copyright (c) 2026 Hearthstone BG AI contributors

#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch7.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonTask.hpp>

#include <utility>

namespace RosettaStone::Battlegrounds
{
namespace
{
using SimpleTasks::SummonTask;

void AddDeathrattleSummon(std::map<std::string, CardDef>& cards, const char* id,
                          const char* tokenID, int amount)
{
    Power power;
    power.AddDeathrattleTask(SummonTask{ tokenID, amount });
    cards.emplace(id, CardDef{ std::move(power) });
}
}  // namespace

void ModernMinionBehaviorsBatch7::AddAll(
    std::map<std::string, CardDef>& cards)
{
    // ---------------------------------------------------------------------
    // Fixed deathrattle summon family.
    // ---------------------------------------------------------------------
    // Eternal Summoner (Patch 36.4): Reborn. Deathrattle: Summon 1 Eternal
    // Knight. The golden form summons a Golden Eternal Knight. Reborn is a
    // metadata keyword; only the deterministic deathrattle belongs here.

    // The remaining Batch 7 inventory is deliberately not registered here.
    // Those entities have non-static text (Auras, Devour/sell triggers, or
    // Activate) whose complete simulator behavior is not implemented yet.
    // Their IDs are kept in the explicitly experimental metadata-only catalog
    // in the parent project, but that catalog is never loaded by CardDefs.
    // In particular, Eternal Knight is summoned from Card metadata by
    // SummonTask, while its unresolved aura remains launch-blocking.
}
}  // namespace RosettaStone::Battlegrounds
