// Copyright (c) 2026 Hearthstone BG AI contributors

#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch2.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonTask.hpp>
#include <Rosetta/Battlegrounds/CardSets/SewerRatTokenBehaviors.hpp>

#include <utility>

namespace RosettaStone::Battlegrounds
{
namespace
{
using SimpleTasks::SummonTask;

void AddStatic(std::map<std::string, CardDef>& cards, const char* id)
{
    // Keyword state is loaded by CardLoader. The registration prevents the
    // pool from treating a metadata-backed static card as an unknown entity.
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

void ModernMinionBehaviorsBatch2::AddAll(
    std::map<std::string, CardDef>& cards)
{
    // ---------------------------------------------------------------------
    // Pure deathrattle-summon families from the pinned Patch 36.4 text.
    // ---------------------------------------------------------------------
    // BG30_125 Cadaver Caretaker summons 1/1 Skeletons at both rarities.

    // BG32_172 Auto Assembler summons an Ancestral Automaton. The golden
    // form explicitly summons the golden generated entity.

    // BG35_604 Sewer Lord summons two rats. The rat family is itself
    // executable: each generated Rat deathrattle summons its pinned
    // Half-Shell token, including the golden pair.
    cards.emplace("BG19_010t", CardDef{});
    cards.emplace("BG19_010_Gt", CardDef{});

    // Aureate Laureate is always golden but has no distinct behavior beyond
    // the Divine Shield keyword represented by the pinned metadata.
}
}  // namespace RosettaStone::Battlegrounds
