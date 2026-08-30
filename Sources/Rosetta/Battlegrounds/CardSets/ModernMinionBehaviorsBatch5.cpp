// Copyright (c) 2026 Hearthstone BG AI contributors

#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch5.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomFriendlyRaceTask.hpp>

#include <utility>

namespace RosettaStone::Battlegrounds
{
namespace
{
using SimpleTasks::RandomFriendlyRaceTask;

void AddScarletSkull(std::map<std::string, CardDef>& cards, const char* id,
                     int attack, int health)
{
    Power power;
    power.AddDeathrattleTask(
        RandomFriendlyRaceTask{ Race::UNDEAD, attack, health });
    cards.emplace(id, CardDef{ std::move(power) });
}

void AddMummifier(std::map<std::string, CardDef>& cards, const char* id,
                  int amount)
{
    Power power;
    power.AddDeathrattleTask(RandomFriendlyRaceTask{ Race::UNDEAD, amount });
    cards.emplace(id, CardDef{ std::move(power) });
}
}  // namespace

void ModernMinionBehaviorsBatch5::AddAll(
    std::map<std::string, CardDef>& cards)
{
    // BG25_022 Scarlet Skull: Reborn is supplied by the pinned metadata;
    // its deathrattle buffs one random other friendly Undead.
    AddScarletSkull(cards, "BG25_022", 1, 2);
    AddScarletSkull(cards, "BG25_022_G", 2, 4);

    // BG28_309 Mummifier: grant Reborn to one different friendly Undead;
    // the golden form selects two distinct Undead.
    AddMummifier(cards, "BG28_309", 1);
    AddMummifier(cards, "BG28_309_G", 2);
}
}  // namespace RosettaStone::Battlegrounds
