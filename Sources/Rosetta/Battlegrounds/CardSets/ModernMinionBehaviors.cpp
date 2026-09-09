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
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/TriggerAdjacentBattlecryTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/CopyTargetBattlecryTask.hpp>
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
    // Buddy lifecycle effects are resolved at their authoritative event
    // boundaries in Player/Battle; explicit empty CardDefs keep these pool
    // entities supported rather than silently metadata-only.
    AddStatic(cards, "BG24_HERO_204_Buddy");
    AddStatic(cards, "BG24_HERO_204_Buddy_G");
    AddStatic(cards, "BG25_HERO_103_Buddy");
    AddStatic(cards, "BG25_HERO_103_Buddy_G");
    AddStatic(cards, "BG26_HERO_104_Buddy");
    AddStatic(cards, "BG26_HERO_104_Buddy_G");
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
    // Tide Oracle Morgl's confirmed attack-kill hand stat transfer is
    // resolved by Battle.cpp using the exact attacker entity and slain
    // minion snapshot; keep both pool entities registered here.
    AddStatic(cards, "BG27_513");
    AddStatic(cards, "BG27_513_G");
    AddStatic(cards, "BG29_813");
    AddStatic(cards, "BG29_813_G");
    // Sklibb, Snow Elemental, and Ticket Collector are owned by the
    // refresh/sale phase boundaries in Player.cpp; register their normal and
    // golden pool entities explicitly so those hooks are executable rather
    // than metadata-only.
    AddStatic(cards, "TB_BaconShop_HERO_59_Buddy");
    AddStatic(cards, "TB_BaconShop_HERO_59_Buddy_G");
    AddStatic(cards, "TB_BaconShop_HERO_78_Buddy");
    AddStatic(cards, "TB_BaconShop_HERO_78_Buddy_G");
    AddStatic(cards, "TB_BaconShop_HERO_94_Buddy");
    AddStatic(cards, "TB_BaconShop_HERO_94_Buddy_G");
    // Timewarped Poet is a static Divine Shield/trigger-visual minion; its
    // all-Dragon combat-persistence aura is applied by Battle.cpp snapshots.
    AddStatic(cards, "BG34_Giant_314");
    AddStatic(cards, "BG34_Giant_314_G");
    // Egg of the Endtimes' delayed Tier-6 Dragon modal is resolved by the
    // recruit-hand timer path; registration prevents an unsupported token
    // from entering the pool while that modal is pending.
    AddStatic(cards, "BG34_639");
    AddStatic(cards, "BG34_639_G");
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
    // Karl the Lost buffs friendly Divine Shield minions after a successful
    // Hero Power use; the lifecycle hook lives on Player.
    AddStatic(cards, "TB_BaconShop_HERO_15_Buddy");
    AddStatic(cards, "TB_BaconShop_HERO_15_Buddy_G");
    AddStatic(cards, "BG26_HERO_102_Buddy");
    AddStatic(cards, "BG26_HERO_102_Buddy_G");
    // Weebomination resolves its end-of-turn health gain against final board
    // positions and the hero's current missing health in Game.
    AddStatic(cards, "TB_BaconShop_HERO_34_Buddy");
    AddStatic(cards, "TB_BaconShop_HERO_34_Buddy_G");
    // Jr. Navigator discounts the Lead Explorer hero power once per
    // resolved sale trigger; Sharkbait refreshes the hero power when sold.
    AddStatic(cards, "TB_BaconShop_HERO_42_Buddy");
    AddStatic(cards, "TB_BaconShop_HERO_42_Buddy_G");
    AddStatic(cards, "TB_BaconShop_HERO_68_Buddy");
    AddStatic(cards, "TB_BaconShop_HERO_68_Buddy_G");
    // Clockwork Assistant and Sparkfin Soothsayer have explicit Battlecry
    // resolution in Player::PlayMinion (their modal/pool rules are player
    // state, not fixed CardDef tasks).
    AddStatic(cards, "TB_BaconShop_HERO_28_Buddy");
    AddStatic(cards, "TB_BaconShop_HERO_28_Buddy_G");
    AddStatic(cards, "TB_BaconShop_HERO_55_Buddy");
    AddStatic(cards, "TB_BaconShop_HERO_55_Buddy_G");
    // Maxwell's sale trigger grants plain copies of the active hero's Buddy;
    // Player resolves it immediately after the minion is committed.
    AddStatic(cards, "TB_BaconShop_HERO_40_Buddy");
    AddStatic(cards, "TB_BaconShop_HERO_40_Buddy_G");
    // Loyal Henchman grants a plain copy of the second enemy killed each
    // combat; the threshold is tracked in Season14State and resolved by
    // Battle after deathrattle/Reborn ordering.
    AddStatic(cards, "TB_BaconShop_HERO_45_Buddy");
    AddStatic(cards, "TB_BaconShop_HERO_45_Buddy_G");
    // Hunter of Old and Lil' K.T. resolve their public opponent copies at
    // recruit-start, where hand capacity and plain-copy semantics are known.
    AddStatic(cards, "TB_BaconShop_HERO_50_Buddy");
    AddStatic(cards, "TB_BaconShop_HERO_50_Buddy_G");
    AddStatic(cards, "TB_BaconShop_HERO_70_Buddy");
    AddStatic(cards, "TB_BaconShop_HERO_70_Buddy_G");
    // Icesnarl and Tamuzo have their event-boundary stat resolution in
    // Battle.cpp/Player.cpp.  Register both rarities here so these buddies
    // are admitted to the supported pool and retain their catalog identity.
    AddStatic(cards, "BG20_HERO_100_Buddy");
    AddStatic(cards, "BG20_HERO_100_Buddy_G");
    AddStatic(cards, "BG23_HERO_201_Buddy");
    AddStatic(cards, "BG23_HERO_201_Buddy_G");
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
    // Rylak Metalhead triggers each surviving adjacent Battlecry when its
    // Deathrattle resolves. The task uses the removed entity's last field
    // position, so zone compaction cannot retarget the wrong slot.
    Power rylak;
    rylak.AddDeathrattleTask(SimpleTasks::TriggerAdjacentBattlecryTask{false});
    cards.emplace("BG26_801", CardDef{std::move(rylak)});
    Power rylakGolden;
    rylakGolden.AddDeathrattleTask(SimpleTasks::TriggerAdjacentBattlecryTask{true});
    cards.emplace("BG26_801_G", CardDef{std::move(rylakGolden)});
    // Faceless Manipulator copies the selected minion's card identity and
    // base stats in-place; golden Faceless upgrades the copied identity.
    Power faceless;
    faceless.AddBattlecryTask(SimpleTasks::CopyTargetBattlecryTask{});
    cards.emplace("BG_EX1_564", CardDef{
        std::move(faceless),
        {{PlayReq::REQ_TARGET_TO_PLAY, 0},
         {PlayReq::REQ_MINION_TARGET, 0},
         {PlayReq::REQ_FRIENDLY_TARGET, 0},
         {PlayReq::REQ_NONSELF_TARGET, 0}}});
    Power facelessGolden;
    facelessGolden.AddBattlecryTask(SimpleTasks::CopyTargetBattlecryTask{true});
    cards.emplace("BG_EX1_564_G", CardDef{
        std::move(facelessGolden),
        {{PlayReq::REQ_TARGET_TO_PLAY, 0},
         {PlayReq::REQ_MINION_TARGET, 0},
         {PlayReq::REQ_FRIENDLY_TARGET, 0},
         {PlayReq::REQ_NONSELF_TARGET, 0}}});
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
