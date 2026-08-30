// Copyright (c) 2026 Hearthstone BG AI contributors

#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch12.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomFriendlyRaceTask.hpp>

#include <utility>

namespace RosettaStone::Battlegrounds
{
namespace
{
using SimpleTasks::RandomFriendlyRaceTask;

void AddLobsterDeathrattle(std::map<std::string, CardDef>& cards,
                           const char* id, int attack, int health)
{
    Power power;
    // Tasty Lobster: Deathrattle: give a random friendly Beast +1/+1.
    // Improve your future Tasty Lobsters.  The golden copy doubles the
    // stat grant and the persistent improvement.
    power.AddDeathrattleTask(
        RandomFriendlyRaceTask{ Race::BEAST, attack, health, 1, false, true });
    cards.emplace(id, CardDef{ std::move(power) });
}
}  // namespace

void ModernMinionBehaviorsBatch12::AddAll(
    std::map<std::string, CardDef>& cards)
{
    AddLobsterDeathrattle(cards, "BG36_202", 1, 1);
    AddLobsterDeathrattle(cards, "BG36_202_G", 2, 2);
}
}  // namespace RosettaStone::Battlegrounds
