// Copyright (c) 2026 Hearthstone BG AI contributors

#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviors.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/HighestHealthHandMurlocSummonTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/GoldenizeTierMinionTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/DemonDiscoverDamageTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ConsumeUndeadBattlecryTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RallyAdjacentEnemyDamageTask.hpp>

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
    // Eternal Knight's stats are maintained by authoritative death
    // processing in Battle.cpp; register both pool entities so its dynamic
    // wherever-this-is aura is not mistaken for unsupported content.
    AddStatic(cards, "BG25_008");
    AddStatic(cards, "BG25_008_G");
    // Warpwing's Immune-while-attacking keyword is carried by card metadata;
    // registration keeps both pool entities in the supported behavior set.
    AddStatic(cards, "BG24_004");
    AddStatic(cards, "BG24_004_G");
    // Rot Hide Gnoll and Soul Rewinder are resolved by the authoritative
    // combat/deferred hero-damage hooks above.
    AddStatic(cards, "BG25_013");
    AddStatic(cards, "BG25_013_G");
    AddStatic(cards, "BG26_174");
    AddStatic(cards, "BG26_174_G");
    for (const auto& spec : {std::pair{"BG26_817", false}, std::pair{"BG26_817_G", false},
                             std::pair{"BG27_017", false}, std::pair{"BG27_017_G", true}}) {
        Power power;
        if (spec.first == "BG27_017" || spec.first == "BG27_017_G")
            power.AddRallyTask(SimpleTasks::RallyAdjacentEnemyDamageTask{spec.second});
        cards.emplace(spec.first, CardDef{std::move(power)});
    }
    AddStatic(cards, "BG26_ICC_901");
    AddStatic(cards, "BG26_ICC_901_G");
    AddStatic(cards, "BG26_149");
    AddStatic(cards, "BG26_149_G");
    AddStatic(cards, "BG26_505");
    AddStatic(cards, "BG26_505_G");
    AddStatic(cards, "BG26_524");
    AddStatic(cards, "BG26_524_G");
    AddStatic(cards, "BG28_633");
    AddStatic(cards, "BG28_633_G");
    AddStatic(cards, "BG27_514");
    AddStatic(cards, "BG27_514_G");
    AddStatic(cards, "BG29_813");
    AddStatic(cards, "BG29_813_G");
    AddStatic(cards, "BG31_320");
    AddStatic(cards, "BG31_320_G");
    AddStatic(cards, "BG31_323");
    AddStatic(cards, "BG31_323_G");
    AddStatic(cards, "BG31_327");
    AddStatic(cards, "BG31_327_G");
    AddStatic(cards, "BG32_237");
    AddStatic(cards, "BG32_237_G");
    struct DeathrattleSummonSpec { const char* id; int count; };
    constexpr DeathrattleSummonSpec handSpecs[] = {
        {"BG26_350", 1}, {"BG26_350_G", 2},
    };
    for (const auto& spec : handSpecs)
    {
        Power power;
        power.AddDeathrattleTask(
            SimpleTasks::HighestHealthHandMurlocSummonTask{spec.count});
        cards.emplace(spec.id, CardDef{std::move(power)});
    }
    struct GoldenizeSpec { const char* id; int count; };
    constexpr GoldenizeSpec goldenizeSpecs[] = {
        {"BG25_034", 1}, {"BG25_034_G", 2},
    };
    for (const auto& spec : goldenizeSpecs)
    {
        Power power;
        power.AddBattlecryTask(SimpleTasks::GoldenizeTierMinionTask{spec.count});
        cards.emplace(spec.id, CardDef{std::move(power)});
    }
    struct DemonSpec { const char* id; int count; };
    constexpr DemonSpec demonSpecs[] = {
        {"BG26_525", 1}, {"BG26_525_G", 2},
    };
    for (const auto& spec : demonSpecs)
    {
        Power power;
        power.AddBattlecryTask(SimpleTasks::DemonDiscoverDamageTask{spec.count});
        cards.emplace(spec.id, CardDef{std::move(power)});
    }
    struct ConsumeSpec { const char* id; bool discover; int copies; };
    constexpr ConsumeSpec consumeSpecs[] = {
        {"BG28_303", false, 1}, {"BG28_303_G", false, 2},
        {"BG32_340", true, 1}, {"BG32_340_G", true, 2},
    };
    for (const auto& spec : consumeSpecs)
    {
        Power power;
        power.AddBattlecryTask(SimpleTasks::ConsumeUndeadBattlecryTask{spec.discover, spec.copies});
        cards.emplace(spec.id, CardDef{std::move(power)});
    }

    // Patch 36.4 deathrattle summon family.
    // BG31_803 Buzzing Vermin: summon a 2/2 Beetle; golden summons two
    // golden Beetles whose metadata supplies their 4/4 stats.

    // BG29_611 Cord Puller: summon a 1/1 Microbot; its golden token is the
    // checked-in 2/2 TB_BaconUps_032t entity in the pinned data set.

    // BG28_300 Harmless Bonehead: summon two 1/1 Skeletons; golden summons
    // four 2/2 Skeletons.

    // Static-only modern minions. Their keyword state is authoritative in
    // Patch 36.4 metadata and therefore needs no custom task chain.
    AddStatic(cards, "BGS_119");    // Crackling Cyclone, DS/Windfury
    AddStatic(cards, "BGS_131");    // Deadly Spore, Venomous
    AddStatic(cards, "BG_BOT_911"); // Annoy-o-Module, DS/Taunt
}
}  // namespace RosettaStone::Battlegrounds
