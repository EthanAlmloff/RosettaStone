// Copyright (c) 2026 Hearthstone BG AI contributors

#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviors.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/HighestHealthHandMurlocSummonTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/GoldenizeTierMinionTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/DemonDiscoverDamageTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ConsumeUndeadBattlecryTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RallyAdjacentEnemyDamageTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RallyRandomRaceKeywordTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/TriggerLeftmostDeathrattleTask.hpp>
#include <Rosetta/Battlegrounds/Triggers/Trigger.hpp>

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
    cards.emplace("BG26_817", CardDef{});
    cards.emplace("BG26_817_G", CardDef{});
    Power rallyAdjacent;
    rallyAdjacent.AddRallyTask(SimpleTasks::RallyAdjacentEnemyDamageTask{false});
    cards.emplace("BG27_017", CardDef{std::move(rallyAdjacent)});
    Power rallyAdjacentGolden;
    rallyAdjacentGolden.AddRallyTask(SimpleTasks::RallyAdjacentEnemyDamageTask{true});
    cards.emplace("BG27_017_G", CardDef{std::move(rallyAdjacentGolden)});
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
    AddStatic(cards, "BG32_873"); AddStatic(cards, "BG32_873_G");
    AddStatic(cards, "BG34_322"); AddStatic(cards, "BG34_322_G");
    // Brann is a metadata-only reward card whose Battlecry multiplier is
    // applied at the authoritative Player dispatch boundary.  Registration
    // keeps the generated reward in the supported pool without duplicating
    // that multiplier in a CardDef task.
    cards.emplace("BG_LOE_077", CardDef{});
    // Token/rarity derivatives below are resolved by their authoritative
    // Player/Battle hooks; explicit CardDefs keep generated copies in the
    // supported pool while preserving the same normal/golden payload.
    cards.emplace("BG22_HERO_000_Buddy", CardDef{});
    cards.emplace("BG22_HERO_000_Buddy_G", CardDef{});
    cards.emplace("BG23_HERO_303_Buddy", CardDef{});
    cards.emplace("BG23_HERO_303_Buddy_G", CardDef{});
    cards.emplace("BG26_537", CardDef{});
    cards.emplace("BG26_537_G", CardDef{});
    cards.emplace("BG30_MagicItem_416t", CardDef{});
    cards.emplace("BG33_890t", CardDef{});
    cards.emplace("BG36_520t", CardDef{});
    cards.emplace("EBG_Spell_014", CardDef{});
    // Operatic Belcher: preserve the Venomous metadata keyword and grant it
    // to one/two friendly Murlocs from the deathrattle boundary.
    Power belcher;
    belcher.AddDeathrattleTask(SimpleTasks::RallyRandomRaceKeywordTask{Race::MURLOC, GameTag::VENOMOUS, 1});
    cards.emplace("BG26_888", CardDef{std::move(belcher)});
    Power belcherGolden;
    belcherGolden.AddDeathrattleTask(SimpleTasks::RallyRandomRaceKeywordTask{Race::MURLOC, GameTag::VENOMOUS, 2});
    cards.emplace("BG26_888_G", CardDef{std::move(belcherGolden)});
    // Silver Goose's TAKE_DAMAGE path summons the rarity-specific Fledgling.
    Power goose;
    Trigger gooseDamage{TriggerType::TAKE_DAMAGE};
    gooseDamage.SetTriggerSource(TriggerSource::SELF);
    gooseDamage.SetTasks({SimpleTasks::SummonTask{"BG29_801t", 1}});
    goose.AddTrigger(std::move(gooseDamage));
    cards.emplace("BG29_801", CardDef{std::move(goose)});
    Power gooseGolden;
    Trigger gooseGoldenDamage{TriggerType::TAKE_DAMAGE};
    gooseGoldenDamage.SetTriggerSource(TriggerSource::SELF);
    gooseGoldenDamage.SetTasks({SimpleTasks::SummonTask{"BG29_801_Gt", 1}});
    gooseGolden.AddTrigger(std::move(gooseGoldenDamage));
    cards.emplace("BG29_801_G", CardDef{std::move(gooseGolden)});
    cards.emplace("BG29_801t", CardDef{});
    cards.emplace("BG29_801_Gt", CardDef{});
    Power macaw;
    macaw.AddRallyTask(SimpleTasks::TriggerLeftmostDeathrattleTask{});
    cards.emplace("BGS_078", CardDef{std::move(macaw)});
    Power bristlebach;
    bristlebach.AddAvenge({AvengeEffect::PLAY_BLOOD_GEMS_RACE, 2, 2, 0, Race::QUILBOAR, false});
    cards.emplace("BG26_157", CardDef{std::move(bristlebach)});
    Power bristlebachGolden;
    bristlebachGolden.AddAvenge({AvengeEffect::PLAY_BLOOD_GEMS_RACE, 2, 4, 0, Race::QUILBOAR, false});
    cards.emplace("BG26_157_G", CardDef{std::move(bristlebachGolden)});
    // Keep each rarity as an explicit registration.  Besides making the
    // normal/golden pairing auditable, this prevents a generic loop/table
    // scanner from mistaking linked token IDs for the owning CardDef.
    Power bassgill;
    bassgill.AddDeathrattleTask(SimpleTasks::HighestHealthHandMurlocSummonTask{1});
    cards.emplace("BG26_350", CardDef{std::move(bassgill)});
    Power bassgillGolden;
    bassgillGolden.AddDeathrattleTask(SimpleTasks::HighestHealthHandMurlocSummonTask{2});
    cards.emplace("BG26_350_G", CardDef{std::move(bassgillGolden)});

    Power goldenizer;
    goldenizer.AddBattlecryTask(SimpleTasks::GoldenizeTierMinionTask{1});
    cards.emplace("BG25_034", CardDef{std::move(goldenizer)});
    Power goldenizerGolden;
    goldenizerGolden.AddBattlecryTask(SimpleTasks::GoldenizeTierMinionTask{2});
    cards.emplace("BG25_034_G", CardDef{std::move(goldenizerGolden)});

    Power demonDiscover;
    demonDiscover.AddBattlecryTask(SimpleTasks::DemonDiscoverDamageTask{1});
    cards.emplace("BG26_525", CardDef{std::move(demonDiscover)});
    Power demonDiscoverGolden;
    demonDiscoverGolden.AddBattlecryTask(SimpleTasks::DemonDiscoverDamageTask{2});
    cards.emplace("BG26_525_G", CardDef{std::move(demonDiscoverGolden)});

    Power consumeUndead;
    consumeUndead.AddBattlecryTask(SimpleTasks::ConsumeUndeadBattlecryTask{false, 1});
    cards.emplace("BG28_303", CardDef{std::move(consumeUndead)});
    Power consumeUndeadGolden;
    consumeUndeadGolden.AddBattlecryTask(SimpleTasks::ConsumeUndeadBattlecryTask{false, 2});
    cards.emplace("BG28_303_G", CardDef{std::move(consumeUndeadGolden)});
    Power discoverUndead;
    discoverUndead.AddBattlecryTask(SimpleTasks::ConsumeUndeadBattlecryTask{true, 1});
    cards.emplace("BG32_340", CardDef{std::move(discoverUndead)});
    Power discoverUndeadGolden;
    discoverUndeadGolden.AddBattlecryTask(SimpleTasks::ConsumeUndeadBattlecryTask{true, 2});
    cards.emplace("BG32_340_G", CardDef{std::move(discoverUndeadGolden)});

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
