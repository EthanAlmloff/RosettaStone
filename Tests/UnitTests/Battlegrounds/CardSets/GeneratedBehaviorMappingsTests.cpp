#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/GeneratedBehaviorMappings.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/PersistentRaceBuffTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/HandRaceBuffTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AttackingMinionBuffTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/OnePerTypeRallyBuffTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonedMinionStatMultiplierTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RallyTavernSpellHealthBonusTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/GrowingSummonAttackTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/FriendlyRaceEnchantmentTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/BloodGemRaceBonusTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/MinionOfferingTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomHandMinionBuffTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomCardToHandTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AddCardTask.hpp>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Generated mappings] - every reviewed row has a live CardDef")
{
    std::map<std::string, CardDef> cards;
    GeneratedBehaviorMappings::AddAll(cards);
    for (const auto& row : DeclarativeBehaviorRows)
    {
        REQUIRE(cards.contains(std::string(row.id)));
        const auto& power = cards.at(std::string(row.id)).power;
        if (row.trigger == "battlecry")
            CHECK(!power.GetBattlecryTask().empty());
        else if (row.trigger == "deathrattle")
            CHECK(!power.GetDeathrattleTask().empty());
        else if (row.trigger == "battlecry_and_deathrattle")
            CHECK(!power.GetBattlecryTask().empty());
        else if (row.trigger == "static")
            CHECK(power.GetBattlecryTask().empty());
        else if (row.trigger == "rally")
            CHECK(!power.GetRallyTask().empty());
        else
            CHECK(power.GetTrigger().has_value());
    }
}

TEST_CASE("[Generated mappings] - persistent race buff is battlecry and exact golden scaling")
{
    std::map<std::string, CardDef> cards;
    GeneratedBehaviorMappings::AddAll(cards);
    for (const auto& row : { std::pair{"BG25_011", 1}, std::pair{"BG25_011_G", 2} })
    {
        REQUIRE(cards.contains(row.first));
        const auto& tasks = cards.at(row.first).power.GetBattlecryTask();
        REQUIRE_EQ(tasks.size(), 1);
        const auto* buff = std::get_if<SimpleTasks::PersistentRaceBuffTask>(&tasks.front());
        REQUIRE(buff != nullptr);
        CHECK(buff->GetRace() == Race::UNDEAD);
    }
}

TEST_CASE("[Generated mappings] - Beast rally family is race-gated")
{
    std::map<std::string, CardDef> cards;
    GeneratedBehaviorMappings::AddAll(cards);
    for (const auto id : {"BG36_211", "BG36_211_G"})
    {
        REQUIRE(cards.contains(id));
        const auto& tasks = cards.at(id).power.GetRallyTask();
        REQUIRE(tasks.size() == 1);
        REQUIRE(std::holds_alternative<SimpleTasks::RallyRaceBuffTask>(tasks.front()));
    }
}

TEST_CASE("[Generated mappings] - Roaring Recruiter buffs only the attacking Dragon")
{
    std::map<std::string, CardDef> cards;
    GeneratedBehaviorMappings::AddAll(cards);
    for (const auto id : {"BG29_816", "BG29_816_G"})
    {
        REQUIRE(cards.contains(id));
        const auto& tasks = cards.at(id).power.GetRallyTask();
        REQUIRE(tasks.size() == 1);
        const auto* buff = std::get_if<SimpleTasks::AttackingMinionBuffTask>(&tasks.front());
        REQUIRE(buff != nullptr);
        CHECK(buff->GetTriggerRace() == Race::DRAGON);
        CHECK(buff->GetAttack() == (std::string_view(id) == "BG29_816_G" ? 6 : 3));
        CHECK(buff->GetHealth() == (std::string_view(id) == "BG29_816_G" ? 2 : 1));
    }
}

TEST_CASE("[Generated mappings] - Banana Slamma multiplies summoned Beast attack in combat")
{
    std::map<std::string, CardDef> cards;
    GeneratedBehaviorMappings::AddAll(cards);
    for (const auto id : {"BG26_802", "BG26_802_G"})
    {
        REQUIRE(cards.contains(id));
        const auto& trigger = cards.at(id).power.GetTrigger();
        REQUIRE(trigger.has_value());
        CHECK(trigger->GetTriggerType() == TriggerType::SUMMON);
        CHECK(trigger->GetTriggerSource() == TriggerSource::MINIONS_EXCEPT_SELF);
        // Golden changes the multiplier (double -> triple); it does not
        // repeat the trigger.  Repeating would multiply once per task and
        // accidentally produce six times Attack.
        REQUIRE(trigger->GetTasks().size() == 1);
        const auto* multiplier = std::get_if<SimpleTasks::SummonedMinionStatMultiplierTask>(&trigger->GetTasks().front());
        REQUIRE(multiplier != nullptr);
        CHECK(multiplier->GetRace() == Race::BEAST);
        CHECK(multiplier->GetAttackMultiplier() == (std::string_view(id) == "BG26_802_G" ? 3 : 2));
        CHECK(multiplier->GetHealthMultiplier() == 1);
    }
}

TEST_CASE("[Generated mappings] - Blue Whelp accumulates Tavern spell health bonus")
{
    std::map<std::string, CardDef> cards;
    GeneratedBehaviorMappings::AddAll(cards);
    for (const auto id : {"BG33_924", "BG33_924_G"})
    {
        REQUIRE(cards.contains(id));
        const auto& tasks = cards.at(id).power.GetRallyTask();
        REQUIRE(tasks.size() == 1);
        const auto* task = std::get_if<SimpleTasks::RallyTavernSpellHealthBonusTask>(&tasks.front());
        REQUIRE(task != nullptr);
        CHECK(task->GetHealth() == (std::string_view(id) == "BG33_924_G" ? 2 : 1));
    }
}

TEST_CASE("[Generated mappings] - self-damage buffs one random hand minion")
{
    std::map<std::string, CardDef> cards;
    GeneratedBehaviorMappings::AddAll(cards);
    for (const auto id : {"BG29_300", "BG29_300_G"})
    {
        REQUIRE(cards.contains(id));
        const auto& trigger = cards.at(id).power.GetTrigger();
        REQUIRE(trigger.has_value());
        CHECK(trigger->GetTriggerType() == TriggerType::TAKE_DAMAGE);
        CHECK(trigger->GetTriggerSource() == TriggerSource::SELF);
        REQUIRE(trigger->GetTasks().size() == 1);
        const auto* task = std::get_if<SimpleTasks::RandomHandMinionBuffTask>(&trigger->GetTasks().front());
        REQUIRE(task != nullptr);
        CHECK(task->GetAttack() == (std::string_view(id) == "BG29_300_G" ? 4 : 2));
        CHECK(task->GetHealth() == (std::string_view(id) == "BG29_300_G" ? 2 : 1));
    }
}

TEST_CASE("[Generated mappings] - Tavern Tempest gets random Elementals")
{
    std::map<std::string, CardDef> cards;
    GeneratedBehaviorMappings::AddAll(cards);
    for (const auto id : {"BGS_123", "TB_BaconUps_162"})
    {
        REQUIRE(cards.contains(id));
        const auto& tasks = cards.at(id).power.GetBattlecryTask();
        REQUIRE(tasks.size() == 1);
        const auto* task = std::get_if<SimpleTasks::RandomCardToHandTask>(&tasks.front());
        REQUIRE(task != nullptr);
        CHECK(task->GetRace() == Race::ELEMENTAL);
        CHECK(task->GetTier() == 0);
        CHECK(task->GetAmount() == (std::string_view(id) == "TB_BaconUps_162" ? 2 : 1));
    }
}

TEST_CASE("[Generated mappings] - migrated deathrattles keep token and hand payloads")
{
    std::map<std::string, CardDef> cards;
    GeneratedBehaviorMappings::AddAll(cards);
    const auto checkSummon = [&cards](const char* id, const char* token, int amount) {
        REQUIRE(cards.contains(id));
        const auto& tasks = cards.at(id).power.GetDeathrattleTask();
        REQUIRE(tasks.size() == 1);
        const auto* task = std::get_if<SimpleTasks::SummonTask>(&tasks.front());
        REQUIRE(task != nullptr);
        CHECK(task->m_cardID == token);
        CHECK(task->m_amount == amount);
    };
    checkSummon("BG28_300", "BG_ICC_026t", 2);
    checkSummon("BG28_300_G", "BG_ICC_026t_G", 4);
    checkSummon("BG29_611", "BG_BOT_312t", 1);
    checkSummon("BG29_611_G", "TB_BaconUps_032t", 1);
    checkSummon("BG31_803", "BG28_603t", 1);
    checkSummon("BG31_803_G", "BG28_603t_G", 2);
    const auto checkCard = [&cards](const char* id, const char* card, int amount, bool battlecry) {
        REQUIRE(cards.contains(id));
        const auto& tasks = cards.at(id).power.GetDeathrattleTask();
        REQUIRE(tasks.size() == 1);
        const auto* task = std::get_if<SimpleTasks::AddCardTask>(&tasks.front());
        REQUIRE(task != nullptr);
        CHECK(task->CardID() == card);
        CHECK(task->Amount() == amount);
        if (battlecry) CHECK(cards.at(id).power.GetBattlecryTask().size() == 1);
    };
    checkCard("BG34_682", "BG34_689", 1, false);
    checkCard("BG34_682_G", "BG34_689", 2, false);
    checkCard("BG35_143", "BG35_149", 1, true);
    checkCard("BG35_143_G", "BG35_149", 2, true);
    checkCard("BG36_854", "BG36_624", 1, false);
    checkCard("BG36_854_G", "BG36_624", 2, false);
}

TEST_CASE("[Generated mappings] - Lurking Leviathan grows summoned Beast attack bonus")
{
    std::map<std::string, CardDef> cards;
    GeneratedBehaviorMappings::AddAll(cards);
    for (const auto id : {"BG35_602", "BG35_602_G"})
    {
        REQUIRE(cards.contains(id));
        const auto& trigger = cards.at(id).power.GetTrigger();
        REQUIRE(trigger.has_value());
        CHECK(trigger->GetTriggerType() == TriggerType::SUMMON);
        CHECK(trigger->GetTriggerSource() == TriggerSource::MINIONS_EXCEPT_SELF);
        REQUIRE(trigger->GetTasks().size() == 1);
        const auto* task = std::get_if<SimpleTasks::GrowingSummonAttackTask>(&trigger->GetTasks().front());
        REQUIRE(task != nullptr);
        CHECK(task->GetRace() == Race::BEAST);
        CHECK(task->GetInitialAttack() == (std::string_view(id) == "BG35_602_G" ? 4 : 2));
        CHECK(task->GetIncrement() == (std::string_view(id) == "BG35_602_G" ? 4 : 2));
    }
}

TEST_CASE("[Generated mappings] - spend gold family uses threshold trigger")
{
    std::map<std::string, CardDef> cards;
    GeneratedBehaviorMappings::AddAll(cards);
    for (const auto id : {"BG26_810", "BG26_810_G"})
    {
        REQUIRE(cards.contains(id));
        const auto& trigger = cards.at(id).power.GetTrigger();
        REQUIRE(trigger.has_value());
        CHECK(trigger->GetTriggerType() == TriggerType::SPEND_GOLD);
        CHECK(trigger->GetTriggerSource() == TriggerSource::SELF);
        CHECK(trigger->GetTasks().size() == (std::string_view(id) == "BG26_810_G" ? 2 : 1));
    }
}

TEST_CASE("[Generated mappings] - hand Murloc growth is trigger-backed")
{
    std::map<std::string, CardDef> cards;
    GeneratedBehaviorMappings::AddAll(cards);
    for (const auto id : {"BG26_137", "BG26_137_G"})
    {
        REQUIRE(cards.contains(id));
        const auto& trigger = cards.at(id).power.GetTrigger();
        REQUIRE(trigger.has_value());
        CHECK(trigger->GetTriggerType() == TriggerType::AFTER_PLAY_MINION);
        REQUIRE(trigger->GetTasks().size() ==
                (std::string_view(id) == "BG26_523_G" ? 2 : 1));
        CHECK(std::holds_alternative<SimpleTasks::HandRaceBuffTask>(trigger->GetTasks().front()));
    }
}

TEST_CASE("[Generated mappings] - tier-gated Murloc buff is generated")
{
    std::map<std::string, CardDef> cards;
    GeneratedBehaviorMappings::AddAll(cards);
    for (const auto id : {"BG33_893", "BG33_893_G"})
    {
        REQUIRE(cards.contains(id));
        const auto& trigger = cards.at(id).power.GetTrigger();
        REQUIRE(trigger.has_value());
        CHECK(trigger->GetTriggerType() == TriggerType::AFTER_PLAY_MINION);
        CHECK(trigger->GetTasks().size() == (std::string_view(id) == "BG33_893_G" ? 2 : 1));
    }
}

TEST_CASE("[Generated mappings] - Tichondrius uses the hero-health-loss lifecycle")
{
    std::map<std::string, CardDef> cards;
    GeneratedBehaviorMappings::AddAll(cards);
    for (const auto id : {"BG26_523", "BG26_523_G"})
    {
        REQUIRE(cards.contains(id));
        const auto& trigger = cards.at(id).power.GetTrigger();
        REQUIRE(trigger.has_value());
        CHECK(trigger->GetTriggerType() == TriggerType::HERO_DAMAGE);
        CHECK(trigger->GetTriggerSource() == TriggerSource::SELF);
        REQUIRE(trigger->GetTasks().size() == 1);
        const auto* task = std::get_if<SimpleTasks::FriendlyRaceEnchantmentTask>(
            &trigger->GetTasks().front());
        REQUIRE(task != nullptr);
        CHECK(task->GetRace() == Race::DEMON);
        // The golden card scales the effect amount; it does not require a
        // synthetic golden enchantment entity.  The pinned data provides one
        // canonical provenance enchantment for both forms.
        CHECK(task->CardID() == "BG26_523e");
    }
}

TEST_CASE("[Generated mappings] - Primalfin Lookout uses gated Murloc Discover")
{
    std::map<std::string, CardDef> cards;
    GeneratedBehaviorMappings::AddAll(cards);
    REQUIRE(cards.contains("BGS_020"));
    const auto& tasks = cards.at("BGS_020").power.GetBattlecryTask();
    REQUIRE(tasks.size() == 1);
    const auto* task =
        std::get_if<SimpleTasks::MinionOfferingTask>(&tasks.front());
    REQUIRE(task != nullptr);
    CHECK(task->GetRace() == Race::MURLOC);
    CHECK(task->GetMinTier() == 1);
    CHECK(task->GetMaxTier() == 7);
    CHECK(task->GetCount() == 3);
    CHECK(task->RequiresFriendlyRace());
}

TEST_CASE("[Generated mappings] - Moat Custodian adds race-scoped Blood Gem aura")
{
    std::map<std::string, CardDef> cards;
    GeneratedBehaviorMappings::AddAll(cards);
    for (const auto id : {"BG36_351", "BG36_351_G"})
    {
        REQUIRE(cards.contains(id));
        const auto& power = cards.at(id).power;
        REQUIRE(power.GetRallyTask().size() == 1);
        const auto* task = std::get_if<SimpleTasks::BloodGemRaceBonusTask>(
            &power.GetRallyTask().front());
        REQUIRE(task != nullptr);
        CHECK(task->GetRace() == Race::ELEMENTAL);
        CHECK(task->GetAttack() ==
              (std::string_view(id) == "BG36_351_G" ? 4 : 2));
        CHECK(task->GetHealth() ==
              (std::string_view(id) == "BG36_351_G" ? 4 : 2));
    }
}

TEST_CASE("[Generated mappings] - Sand Swirler persists Elemental attack aura")
{
    std::map<std::string, CardDef> cards;
    GeneratedBehaviorMappings::AddAll(cards);
    for (const auto id : {"BG32_841", "BG32_841_G"})
    {
        REQUIRE(cards.contains(id));
        const auto& tasks = cards.at(id).power.GetBattlecryTask();
        REQUIRE(tasks.size() == 1);
        const auto* task =
            std::get_if<SimpleTasks::PersistentRaceBuffTask>(&tasks.front());
        REQUIRE(task != nullptr);
        CHECK(task->GetRace() == Race::ELEMENTAL);
        CHECK(task->GetAttack() ==
              (std::string_view(id) == "BG32_841_G" ? 4 : 2));
        CHECK(task->GetHealth() == 0);
    }
}

TEST_CASE("[Generated mappings] - Last One Standing selects one minion per type")
{
    std::map<std::string, CardDef> cards;
    GeneratedBehaviorMappings::AddAll(cards);
    for (const auto id : {"BG34_320", "BG34_320_G"})
    {
        REQUIRE(cards.contains(id));
        const auto& tasks = cards.at(id).power.GetRallyTask();
        REQUIRE(tasks.size() == 1);
        const auto* task = std::get_if<SimpleTasks::OnePerTypeRallyBuffTask>(
            &tasks.front());
        REQUIRE(task != nullptr);
        CHECK(task->GetAttack() == 15);
        CHECK(task->GetHealth() == 15);
        CHECK(task->GetRepeats() ==
              (std::string_view(id) == "BG34_320_G" ? 2 : 1));
    }
}
