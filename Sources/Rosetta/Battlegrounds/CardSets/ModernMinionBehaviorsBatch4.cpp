// Copyright (c) 2026 Hearthstone BG AI contributors

#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch4.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonTask.hpp>

#include <utility>

namespace RosettaStone::Battlegrounds
{
namespace
{
using SimpleTasks::SummonTask;

void AddStatic(std::map<std::string, CardDef>& cards, const char* id)
{
    // Taunt, Reborn, Divine Shield, and Windfury are loaded from the pinned
    // card metadata. The empty definition is intentional: it registers the
    // entity without pretending that an unimplemented triggered effect exists.
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

void ModernMinionBehaviorsBatch4::AddAll(
    std::map<std::string, CardDef>& cards)
{
    // ---------------------------------------------------------------------
    // Static keyword family.
    // ---------------------------------------------------------------------
    // BG25_001 Risen Rider is only Taunt + Reborn in the pinned 36.4 data.
    // Both rarities are explicit because generated golden entities are
    // independently checked by the behavior coverage tooling.
    AddStatic(cards, "BG25_001");
    AddStatic(cards, "BG25_001_G");

    // The previous static batches registered the normal records. These are
    // their pinned golden records and share exactly the same metadata-backed
    // keyword behavior.
    AddStatic(cards, "TB_BaconUps_099");  // Annoy-o-Module
    AddStatic(cards, "TB_BaconUps_159");  // Crackling Cyclone
    AddStatic(cards, "TB_BaconUps_251");  // Deadly Spore

    // ---------------------------------------------------------------------
    // Fixed deathrattle-summon family.
    // ---------------------------------------------------------------------
    // BG25_010 Handless Forsaken summons Helping Hand. The golden form uses
    // the explicitly linked golden token and doubles the summon count.
    AddDeathrattleSummon(cards, "BG25_010", "BG25_010t", 1);
    AddDeathrattleSummon(cards, "BG25_010_G", "BG25_010_Gt", 2);
    AddStatic(cards, "BG25_010t");
    AddStatic(cards, "BG25_010_Gt");
}
}  // namespace RosettaStone::Battlegrounds
