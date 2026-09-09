// This code is based on Sabberstone project.
// Copyright (c) 2017-2021 SabberStone Team, darkfriend77 & rnilva
// RosettaStone is hearthstone simulator using C++ with reinforcement learning.
// Copyright (c) 2017-2024 Chris Ohk

#include <Rosetta/Battlegrounds/CardSets/BattlegroundsCardsGen.hpp>
#include <Rosetta/Battlegrounds/CardSets/DarkGiftBehaviors.hpp>
#include <Rosetta/Battlegrounds/CardSets/BloodGemBehaviors.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviors.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch2.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch3.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch4.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch5.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch6.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch7.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch8.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch9.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch10.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch11.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch12.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsSimpleBatch.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch16.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch13.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch15.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch17.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch19.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch20.hpp>
#include <Rosetta/Battlegrounds/CardSets/SpellcraftMinionBehaviors.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch21.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch22.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch23.hpp>
#include <Rosetta/Battlegrounds/CardSets/MagneticMinionBehaviors.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch24.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch28.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch30.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch32.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch33.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch34.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch35.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch36.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch37.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch39.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch40.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch41.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch42.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch43.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch44.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch45.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch46.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch47.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch48.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch50.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch51.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch52.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch53.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch54.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch55.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch56.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch57.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch58.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch59.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch60.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch61.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch62.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch63.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch64.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch65.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch66.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernTokenBehaviorsBatch67.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernTokenBehaviorsBatch70.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernTokenBehaviorsBatch71.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernTokenBehaviorsBatch72.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernTokenBehaviorsBatch73.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernTokenBehaviorsBatch74.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernTokenBehaviorsBatch75.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernTokenBehaviorsBatch76.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernTokenBehaviorsBatch77.hpp>
#include <Rosetta/Battlegrounds/CardSets/GeneratedBehaviorMappings.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch31.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch26.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch27.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch25.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch29.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatchBaller.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch14.hpp>
#include <Rosetta/Battlegrounds/CardSets/EventCounterBehaviors.hpp>
#include <Rosetta/Battlegrounds/CardSets/ActivateBehaviors.hpp>
#include <Rosetta/Battlegrounds/CardSets/TrinketBehaviors.hpp>
#include <Rosetta/Battlegrounds/Cards/CardDefs.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/DamageTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ProtossBehaviorTask.hpp>

#include <array>

namespace RosettaStone::Battlegrounds
{
std::map<std::string, CardDef> CardDefs::m_data;

CardDefs::CardDefs()
{
    BattlegroundsCardsGen::AddAll(m_data);
    DarkGiftBehaviors::AddAll(m_data);
    BloodGemBehaviors::AddAll(m_data);
    ModernMinionBehaviors::AddAll(m_data);
    ModernMinionBehaviorsBatch2::AddAll(m_data);
    ModernMinionBehaviorsBatch3::AddAll(m_data);
    ModernMinionBehaviorsBatch4::AddAll(m_data);
    ModernMinionBehaviorsBatch5::AddAll(m_data);
    ModernMinionBehaviorsBatch6::AddAll(m_data);
    ModernMinionBehaviorsBatch7::AddAll(m_data);
    ModernMinionBehaviorsBatch8::AddAll(m_data);
    ModernMinionBehaviorsBatch9::AddAll(m_data);
    ModernMinionBehaviorsBatch10::AddAll(m_data);
    ModernMinionBehaviorsBatch11::AddAll(m_data);
    ModernMinionBehaviorsBatch12::AddAll(m_data);
    ModernMinionBehaviorsSimpleBatch::AddAll(m_data);
    ModernMinionBehaviorsBatch16::AddAll(m_data);
    ModernMinionBehaviorsBatch13::AddAll(m_data);
    ModernMinionBehaviorsBatch15::AddAll(m_data);
    ModernMinionBehaviorsBatch17::AddAll(m_data);
    ModernMinionBehaviorsBatch19::AddAll(m_data);
    ModernMinionBehaviorsBatch20::AddAll(m_data);
    SpellcraftMinionBehaviors::AddAll(m_data);
    ModernMinionBehaviorsBatch21::AddAll(m_data);
    ModernMinionBehaviorsBatch22::AddAll(m_data);
    ModernMinionBehaviorsBatch23::AddAll(m_data);
    MagneticMinionBehaviors::AddAll(m_data);
    ModernMinionBehaviorsBatch24::AddAll(m_data);
    ModernMinionBehaviorsBatch28::AddAll(m_data);
    ModernMinionBehaviorsBatch30::AddAll(m_data);
    ModernMinionBehaviorsBatch32::AddAll(m_data);
    ModernMinionBehaviorsBatch33::AddAll(m_data);
    ModernMinionBehaviorsBatch34::AddAll(m_data);
    ModernMinionBehaviorsBatch35::AddAll(m_data);
    ModernMinionBehaviorsBatch36::AddAll(m_data);
    ModernMinionBehaviorsBatch37::AddAll(m_data);
    ModernMinionBehaviorsBatch39::AddAll(m_data);
    ModernMinionBehaviorsBatch40::AddAll(m_data);
    ModernMinionBehaviorsBatch41::AddAll(m_data);
    ModernMinionBehaviorsBatch42::AddAll(m_data);
    ModernMinionBehaviorsBatch43::AddAll(m_data);
    ModernMinionBehaviorsBatch44::AddAll(m_data);
    ModernMinionBehaviorsBatch45::AddAll(m_data);
    ModernMinionBehaviorsBatch46::AddAll(m_data);
    ModernMinionBehaviorsBatch47::AddAll(m_data);
    ModernMinionBehaviorsBatch48::AddAll(m_data);
    ModernMinionBehaviorsBatch50::AddAll(m_data);
    ModernMinionBehaviorsBatch51::AddAll(m_data);
    ModernMinionBehaviorsBatch52::AddAll(m_data);
    ModernMinionBehaviorsBatch53::AddAll(m_data);
    ModernMinionBehaviorsBatch54::AddAll(m_data);
    ModernMinionBehaviorsBatch55::AddAll(m_data);
    ModernMinionBehaviorsBatch56::AddAll(m_data);
    ModernMinionBehaviorsBatch57::AddAll(m_data);
    ModernMinionBehaviorsBatch58::AddAll(m_data);
    ModernMinionBehaviorsBatch59::AddAll(m_data);
    ModernMinionBehaviorsBatch60::AddAll(m_data);
    ModernMinionBehaviorsBatch61::AddAll(m_data);
    ModernMinionBehaviorsBatch62::AddAll(m_data);
    ModernMinionBehaviorsBatch63::AddAll(m_data);
    ModernMinionBehaviorsBatch64::AddAll(m_data);
    ModernMinionBehaviorsBatch65::AddAll(m_data);
    ModernMinionBehaviorsBatch66::AddAll(m_data);
    ModernTokenBehaviorsBatch67::AddAll(m_data);
    ModernTokenBehaviorsBatch70::AddAll(m_data);
    ModernTokenBehaviorsBatch71::AddAll(m_data);
    ModernTokenBehaviorsBatch72::AddAll(m_data);
    ModernTokenBehaviorsBatch73::AddAll(m_data);
    ModernTokenBehaviorsBatch74::AddAll(m_data);
    ModernTokenBehaviorsBatch75::AddAll(m_data);
    ModernTokenBehaviorsBatch76::AddAll(m_data);
    ModernTokenBehaviorsBatch77::AddAll(m_data);
    GeneratedBehaviorMappings::AddAll(m_data);
    { Power p; p.AddDeathrattleTask(SimpleTasks::SummonTask{"BG19_010t", 1}); m_data.emplace("BG19_010", CardDef{std::move(p)}); }
    { Power p; p.AddDeathrattleTask(SimpleTasks::SummonTask{"BG19_010_Gt", 1}); m_data.emplace("BG19_010_G", CardDef{std::move(p)}); }
    { Power p; p.AddDeathrattleTask(SimpleTasks::SummonTask{"BG25_010t", 1}); m_data.emplace("BG25_010", CardDef{std::move(p)}); }
    { Power p; p.AddDeathrattleTask(SimpleTasks::SummonTask{"BG25_010_Gt", 2}); m_data.emplace("BG25_010_G", CardDef{std::move(p)}); }
    // Relics of the Deep is a passive hero power whose start-turn effect is
    // resolved by Player::ResolveRelicsOfTheDeepStartTurn.
    m_data.emplace("BG23_HERO_304p", CardDef{});
    // Token of the Old Gods is consumed by Player::PlaySpell only after its
    // two-stage transform modal is successfully opened.
    m_data.emplace("BG30_MagicItem_416t", CardDef{});
    // Season 14 Spellcraft Trinket tokens resolve through the reviewed
    // TavernSpellBehavior target executors; expose their CardDefs so the
    // generated hand cards are accepted by CardLoader.
    m_data.emplace("BG30_MagicItem_429t", CardDef{});
    m_data.emplace("BG36_MagicItem_208t", CardDef{});
    // MechGyver's threshold/reward is owned by Player::ResolveMechGyverDeath;
    // this marker makes CardLoader expose the passive hero power.
    m_data.emplace("BG22_HERO_200p", CardDef{});
    // Lift Off upgrades are resolved by Player because they mutate the
    // hero-owned Battlecruiser and carry a refresh/tier counter.  Register
    // every pinned normal and progressive spell here so CardLoader exposes
    // them as executable generated entities instead of metadata-only cards.
    constexpr std::array<const char*, 46> liftOffUpgrades = {
        "BG31_HERO_801pta", "BG31_HERO_801pta2", "BG31_HERO_801pta3",
        "BG31_HERO_801pta4", "BG31_HERO_801pta5", "BG31_HERO_801pta6",
        "BG31_HERO_801pta7", "BG31_HERO_801ptb", "BG31_HERO_801ptb2",
        "BG31_HERO_801ptb3", "BG31_HERO_801ptb4", "BG31_HERO_801ptb5",
        "BG31_HERO_801ptb6", "BG31_HERO_801ptb7", "BG31_HERO_801ptc",
        "BG31_HERO_801ptc2", "BG31_HERO_801ptc3", "BG31_HERO_801ptc4",
        "BG31_HERO_801ptc5", "BG31_HERO_801ptc6", "BG31_HERO_801ptc7",
        "BG31_HERO_801ptd", "BG31_HERO_801ptd2", "BG31_HERO_801ptd3",
        "BG31_HERO_801ptd4", "BG31_HERO_801ptd5", "BG31_HERO_801ptd6",
        "BG31_HERO_801ptd7", "BG31_HERO_801pte", "BG31_HERO_801pte2",
        "BG31_HERO_801pte3", "BG31_HERO_801pte4", "BG31_HERO_801pte5",
        "BG31_HERO_801pte6", "BG31_HERO_801pte7", "BG31_HERO_801ptf",
        "BG31_HERO_801ptf2", "BG31_HERO_801ptf3", "BG31_HERO_801ptf4",
        "BG31_HERO_801pth", "BG31_HERO_801pth2", "BG31_HERO_801pth3",
        "BG31_HERO_801pti", "BG31_HERO_801pti2", "BG31_HERO_801ptj",
        "BG31_HERO_801ptj2"};
    for (const auto id : liftOffUpgrades) m_data.emplace(id, CardDef{});
    // Warp Gate's Protoss offerings are hero-generated (not Tavern-pool
    // cards), so register the exact pinned SC entities as executable static
    // card definitions.  Their generation/selection remains owned by Player;
    // an absent definition must never be substituted with another race.
    constexpr std::array<const char*, 9> warpGateProtoss = {
        "SC_751t", "SC_752", "SC_756", "SC_758", "SC_762",
        "SC_763", "SC_764", "SC_765", "SC_783"};
    for (const auto id : warpGateProtoss) m_data.emplace(id, CardDef{});
    // Carrier's Interceptor and the Templar merge result are generated
    // entities, not Warp Gate choices.  Register both exact source IDs so
    // CardLoader metadata can instantiate them through the normal CardDefs
    // path when Carrier combat or the Archon transformation resolves.
    m_data.emplace("SC_756t", CardDef{});
    m_data.emplace("SC_671t1", CardDef{});
    // Curator Sticker's two fixed rewards are generated entities owned by
    // ModernTokenBehaviorsBatch76, which is registered above.
    // Protoss Battlecries that affect "all enemies" use a dedicated task so
    // the enemy hero is included as well as the enemy board.  A plain
    // DamageTask only visits minions and would silently under-apply these
    // printed effects.
    {
        Power p;
        p.AddBattlecryTask(SimpleTasks::ProtossBehaviorTask{
            SimpleTasks::ProtossBehaviorTask::Effect::COLOSSUS_DAMAGE});
        p.AddBattlecryTask(SimpleTasks::ProtossBehaviorTask{
            SimpleTasks::ProtossBehaviorTask::Effect::COLOSSUS_DAMAGE});
        m_data.insert_or_assign("SC_758", CardDef{std::move(p)});
    }
    {
        Power p;
        p.AddBattlecryTask(SimpleTasks::ProtossBehaviorTask{
            SimpleTasks::ProtossBehaviorTask::Effect::HIGH_TEMPLAR_DAMAGE});
        m_data.insert_or_assign("SC_765", CardDef{std::move(p)});
    }
    {
        Power p;
        p.AddBattlecryTask(SimpleTasks::ProtossBehaviorTask{
            SimpleTasks::ProtossBehaviorTask::Effect::MOTHERSHIP_REWARD});
        p.AddDeathrattleTask(SimpleTasks::ProtossBehaviorTask{
            SimpleTasks::ProtossBehaviorTask::Effect::MOTHERSHIP_REWARD});
        m_data.insert_or_assign("SC_762", CardDef{std::move(p)});
    }
    {
        Power p;
        p.AddBattlecryTask(SimpleTasks::ProtossBehaviorTask{
            SimpleTasks::ProtossBehaviorTask::Effect::IMMORTAL_DOUBLE});
        m_data.insert_or_assign("SC_763", CardDef{std::move(p)});
    }
    {
        Power p;
        p.AddDeathrattleTask(SimpleTasks::ProtossBehaviorTask{
            SimpleTasks::ProtossBehaviorTask::Effect::SENTRY_DISCOUNT});
        m_data.insert_or_assign("SC_764", CardDef{std::move(p)});
    }
    {
        Power p;
        p.AddBattlecryTask(SimpleTasks::ProtossBehaviorTask{
            SimpleTasks::ProtossBehaviorTask::Effect::VOID_RAY_BONUS});
        m_data.insert_or_assign("SC_783", CardDef{std::move(p)});
    }
    {
        Power p;
        p.AddBattlecryTask(SimpleTasks::ProtossBehaviorTask{
            SimpleTasks::ProtossBehaviorTask::Effect::DARK_TEMPLAR_DESTROY});
        m_data.insert_or_assign("SC_752", CardDef{std::move(p)});
    }
    {
        Power p;
        Trigger t{TriggerType::TURN_END};
        t.SetTriggerSource(TriggerSource::SELF);
        t.SetTasks({SimpleTasks::ProtossBehaviorTask{
            SimpleTasks::ProtossBehaviorTask::Effect::CARRIER_END_TURN}});
        p.AddTrigger(std::move(t));
        m_data.insert_or_assign("SC_756", CardDef{std::move(p)});
    }
    {
        Power p;
        Trigger t{TriggerType::TURN_END};
        t.SetTriggerSource(TriggerSource::SELF);
        t.SetTasks({SimpleTasks::ProtossBehaviorTask{
            SimpleTasks::ProtossBehaviorTask::Effect::ARCHON_END_TURN}});
        p.AddTrigger(std::move(t));
        m_data.insert_or_assign("SC_671t1", CardDef{std::move(p)});
    }
    ModernMinionBehaviorsBatch31::AddAll(m_data);
    ModernMinionBehaviorsBatch26::AddAll(m_data);
    ModernMinionBehaviorsBatch27::AddAll(m_data);
    ModernMinionBehaviorsBatch25::AddAll(m_data);
    ModernMinionBehaviorsBatch29::AddAll(m_data);
    ModernMinionBehaviorsBatchBaller::AddAll(m_data);
    ModernMinionBehaviorsBatch14::AddAll(m_data);
    EventCounterBehaviors::AddAll(m_data);
    ActivateBehaviors::AddAll(m_data);
    TrinketBehaviors::AddAll(m_data);
}

CardDefs::~CardDefs()
{
    m_data.clear();
}

CardDefs& CardDefs::GetInstance()
{
    static CardDefs instance;
    return instance;
}

CardDef CardDefs::FindCardDefByID(const std::string_view& id)
{
    for (auto& data : m_data)
    {
        if (data.first == id)
        {
            return data.second;
        }
    }

    return CardDef();
}

bool CardDefs::HasDefinition(const std::string_view& id)
{
    (void)GetInstance();
    return m_data.contains(std::string(id));
}
}  // namespace RosettaStone::Battlegrounds
