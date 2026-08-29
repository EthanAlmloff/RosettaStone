// Copyright (c) 2026 Hearthstone BG AI contributors

#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch2.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonTask.hpp>

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
    AddDeathrattleSummon(cards, "BG30_125", "BG_ICC_026t", 3);
    AddDeathrattleSummon(cards, "BG30_125_G", "BG_ICC_026t", 6);

    // BG32_172 Auto Assembler summons an Ancestral Automaton. The golden
    // form explicitly summons the golden generated entity.
    AddDeathrattleSummon(cards, "BG32_172", "BG_TTN_401", 1);
    AddDeathrattleSummon(cards, "BG32_172_G", "BG_TTN_401_G", 1);

    // BG35_604 Sewer Lord summons two rats. Registering the rat definitions
    // below makes the nested rat -> Half-Shell deathrattle executable too.
    AddDeathrattleSummon(cards, "BG35_604", "BG19_010", 2);
    AddDeathrattleSummon(cards, "BG35_604_G", "BG19_010_G", 2);
    AddDeathrattleSummon(cards, "BG19_010", "BG19_010t", 1);
    AddDeathrattleSummon(cards, "BG19_010_G", "BG19_010_Gt", 1);
    AddStatic(cards, "BG19_010t");
    AddStatic(cards, "BG19_010_Gt");

    // Aureate Laureate is always golden but has no distinct behavior beyond
    // the Divine Shield keyword represented by the pinned metadata.
    AddStatic(cards, "BG32_236");
    AddStatic(cards, "BG32_236_G");
}
}  // namespace RosettaStone::Battlegrounds
