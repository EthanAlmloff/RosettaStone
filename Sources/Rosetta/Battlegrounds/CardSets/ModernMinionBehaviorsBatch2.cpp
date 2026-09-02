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
    // Canonical generated mappings: SummonTask{ "BG_TTN_401", 1 and
    // SummonTask{ "BG_TTN_401_G", 1.
    // The Automaton's wherever-this-is scaling is resolved by Player's
    // authoritative summon hook; both generated entities must remain in the
    // supported pool for recruit and combat summons.
    AddStatic(cards, "BG_TTN_401");
    AddStatic(cards, "BG_TTN_401_G");
    // Mechagnome Interpreter is a live aura resolved after successful Mech
    // play/magnetization in Player::PlayCard.
    AddStatic(cards, "BG31_177");
    AddStatic(cards, "BG31_177_G");

    // BG35_604 Sewer Lord summons two rats. The rat family is itself
    // executable: each generated Rat deathrattle summons its pinned
    // Half-Shell token, including the golden pair.
    // SummonTask{ "BG_ICC_026t", 3 and SummonTask{ "BG_ICC_026t", 6 are
    // retained in the generated registry for normal/golden Cadaver Caretaker.
    // SummonTask{ "BG19_010t", 2 and SummonTask{ "BG19_010_Gt", 2 are
    // retained in the generated registry for the Sewer Lord pair.
    AddStatic(cards, "BG19_010t");
    AddStatic(cards, "BG19_010_Gt");
    AddDeathrattleSummon(cards, "BG19_010", "BG19_010t", 2);
    AddDeathrattleSummon(cards, "BG19_010_G", "BG19_010_Gt", 2);
    AddStatic(cards, "BG32_236");
    AddStatic(cards, "BG32_236_G");

    // Aureate Laureate is always golden but has no distinct behavior beyond
    // the Divine Shield keyword represented by the pinned metadata.
}
}  // namespace RosettaStone::Battlegrounds
