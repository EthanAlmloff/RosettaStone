// Copyright (c) 2026 Hearthstone BG AI contributors

#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch11.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RallyBuffTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonTask.hpp>

#include <utility>

namespace RosettaStone::Battlegrounds
{
namespace
{
using SimpleTasks::RallyBuffTask;
using SimpleTasks::SummonTask;

void AddRallySummon(std::map<std::string, CardDef>& cards, const char* id,
                    const char* tokenID, int amount)
{
    Power power;
    power.AddRallyTask(SummonTask{ tokenID, amount });
    cards.emplace(id, CardDef{ std::move(power) });
}

void AddRallyBuff(std::map<std::string, CardDef>& cards, const char* id,
                  int attack, int health)
{
    Power power;
    power.AddRallyTask(RallyBuffTask{ attack, health });
    cards.emplace(id, CardDef{ std::move(power) });
}
}  // namespace

void ModernMinionBehaviorsBatch11::AddAll(
    std::map<std::string, CardDef>& cards)
{
    // Flittering Bat: Rally: Summon a 1/1 Beast (golden summons two).
    AddRallySummon(cards, "BG36_200", "BG36_200t", 1);
    AddRallySummon(cards, "BG36_200_G", "BG36_200_Gt", 2);

    // Wolf Pup: Rally: Give your other minions +4/+1 (golden +8/+2).
    AddRallyBuff(cards, "BG36_207", 4, 1);
    AddRallyBuff(cards, "BG36_207_G", 8, 2);

    // Hoarding Hyena: Rally: Summon a Tasty Lobster (golden version).
    AddRallySummon(cards, "BG36_210", "BG36_202", 1);
    AddRallySummon(cards, "BG36_210_G", "BG36_202_G", 1);
}
}  // namespace RosettaStone::Battlegrounds
