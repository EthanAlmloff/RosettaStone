// Copyright (c) 2026 Hearthstone BG AI contributors

#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviors.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonTask.hpp>

#include <utility>

namespace RosettaStone::Battlegrounds
{
namespace
{
using SimpleTasks::SummonTask;

void AddStatic(std::map<std::string, CardDef>& cards, const char* id)
{
    // Static keywords are loaded into GameTag by CardLoader. An empty Power
    // is the complete behavior for a static-only minion; registering it is
    // still required because MinionPool rejects unregistered pool entities.
    cards.emplace(id, CardDef{});
}

void AddDeathrattleSummon(std::map<std::string, CardDef>& cards, const char* id,
                          const char* tokenID, int amount)
{
    Power power;
    power.AddDeathrattleTask(SummonTask{ tokenID, amount });
    cards.emplace(id, CardDef{ std::move(power) });
}
}  // namespace

void ModernMinionBehaviors::AddAll(std::map<std::string, CardDef>& cards)
{
    // Patch 36.4 deathrattle summon family.
    // BG31_803 Buzzing Vermin: summon a 2/2 Beetle; golden summons two
    // golden Beetles whose metadata supplies their 4/4 stats.
    AddDeathrattleSummon(cards, "BG31_803", "BG28_603t", 1);
    AddDeathrattleSummon(cards, "BG31_803_G", "BG28_603t_G", 2);

    // BG29_611 Cord Puller: summon a 1/1 Microbot; its golden token is the
    // checked-in 2/2 TB_BaconUps_032t entity in the pinned data set.
    AddDeathrattleSummon(cards, "BG29_611", "BG_BOT_312t", 1);
    AddDeathrattleSummon(cards, "BG29_611_G", "TB_BaconUps_032t", 1);

    // BG28_300 Harmless Bonehead: summon two 1/1 Skeletons; golden summons
    // four 2/2 Skeletons.
    AddDeathrattleSummon(cards, "BG28_300", "BG_ICC_026t", 2);
    AddDeathrattleSummon(cards, "BG28_300_G", "BG_ICC_026t_G", 4);

    // Static-only modern minions. Their keyword state is authoritative in
    // Patch 36.4 metadata and therefore needs no custom task chain.
    AddStatic(cards, "BGS_119");    // Crackling Cyclone, DS/Windfury
    AddStatic(cards, "BGS_131");    // Deadly Spore, Venomous
    AddStatic(cards, "BG_BOT_911"); // Annoy-o-Module, DS/Taunt
}
}  // namespace RosettaStone::Battlegrounds
