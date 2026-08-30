// Copyright (c) 2026 Hearthstone BG AI contributors

#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch8.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Conditions/SelfCondition.hpp>
#include <Rosetta/Battlegrounds/Enchants/Effects.hpp>
#include <Rosetta/Battlegrounds/Enchants/Enchant.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AddEnchantmentTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/FriendlyRaceEnchantmentTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/FreeRefreshTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SetGameTagTask.hpp>

#include <utility>
#include <string>
#include <vector>

namespace RosettaStone::Battlegrounds
{
namespace
{
using SimpleTasks::AddEnchantmentTask;
using SimpleTasks::FriendlyRaceEnchantmentTask;
using SimpleTasks::FreeRefreshTask;
using SimpleTasks::SetGameTagTask;

void AddRaceDeathrattleBuff(std::map<std::string, CardDef>& cards,
                            const char* id, const char* enchantmentID,
                            Race race)
{
    Power power;
    power.AddDeathrattleTask(
        FriendlyRaceEnchantmentTask{ enchantmentID, race });
    cards.emplace(id, CardDef{ std::move(power) });
}

void AddStatEnchantment(std::map<std::string, CardDef>& cards, const char* id,
                        int attack, int health)
{
    std::vector<Effect> effects;
    if (attack != 0)
    {
        effects.emplace_back(Effects::AttackN(attack));
    }
    if (health != 0)
    {
        effects.emplace_back(Effects::HealthN(health));
    }
    Power power;
    power.AddEnchant(Enchant{ std::move(effects) });
    cards.emplace(id, CardDef{ std::move(power) });
}

void AddKalecgos(std::map<std::string, CardDef>& cards, const char* id,
                 const char* enchantmentID)
{
    Power power;
    Trigger trigger{ TriggerType::AFTER_PLAY_MINION };
    trigger.SetTriggerSource(TriggerSource::MINIONS_EXCEPT_SELF);
    trigger.SetTasks(std::vector<TaskType>{
        FriendlyRaceEnchantmentTask{ enchantmentID, Race::DRAGON } });
    // AFTER_PLAY_MINION is also used by non-Battlecry cards.  The source
    // metadata is authoritative for this condition; match the keyword marker
    // rather than a loose "Battlecry" substring so cards that merely mention
    // Battlecries do not incorrectly award Kalecgos' aura.
    trigger.SetCondition(SelfCondition{ [](Minion& source) {
        const auto card = Cards::FindCardByID(source.GetCardID());
        return card.text.find("<b>Battlecry:</b>") != std::string::npos;
    } });
    power.AddTrigger(std::move(trigger));
    cards.emplace(id, CardDef{ std::move(power) });
}

void AddDeflectOBot(std::map<std::string, CardDef>& cards, const char* id,
                    const char* enchantmentID)
{
    Power power;
    Trigger trigger{ TriggerType::SUMMON };
    trigger.SetTriggerSource(TriggerSource::MINIONS_EXCEPT_SELF);
    trigger.SetTasks(std::vector<TaskType>{
        AddEnchantmentTask{ enchantmentID, EntityType::SOURCE },
        SetGameTagTask{ EntityType::SOURCE, GameTag::DIVINE_SHIELD, 1 } });
    trigger.SetCondition(SelfCondition{ [](Minion& source) {
        return source.getPlayerCallback().isInCombat &&
               source.HasRace(Race::MECHANICAL);
    } });
    power.AddTrigger(std::move(trigger));
    cards.emplace(id, CardDef{ std::move(power) });
}

void AddMoltenRock(std::map<std::string, CardDef>& cards, const char* id,
                   const char* enchantmentID)
{
    Power power;
    Trigger trigger{ TriggerType::AFTER_PLAY_MINION };
    trigger.SetTriggerSource(TriggerSource::MINIONS_EXCEPT_SELF);
    trigger.SetTasks(std::vector<TaskType>{
        AddEnchantmentTask{ enchantmentID, EntityType::SOURCE } });
    trigger.SetCondition(SelfCondition{ [](Minion& source) {
        return source.HasRace(Race::ELEMENTAL);
    } });
    power.AddTrigger(std::move(trigger));
    cards.emplace(id, CardDef{ std::move(power) });
}
}  // namespace

void ModernMinionBehaviorsBatch8::AddAll(
    std::map<std::string, CardDef>& cards)
{
    // Goldrinn, the Great Wolf: Deathrattle: Give your Beasts +5/+5;
    // TB_BaconUps_085 is the linked golden entity and gives +10/+10.
    AddRaceDeathrattleBuff(cards, "BGS_018", "BGS_018e", Race::BEAST);
    AddRaceDeathrattleBuff(cards, "TB_BaconUps_085", "TB_BaconUps_085e",
                           Race::BEAST);
    AddStatEnchantment(cards, "BGS_018e", 5, 5);
    AddStatEnchantment(cards, "TB_BaconUps_085e", 10, 10);

    // Kalecgos, Arcane Aspect: after a Battlecry minion is played, give your
    // Dragons +1/+1.  The linked golden copy doubles the enchantment.
    AddKalecgos(cards, "BGS_041", "BGS_041e");
    AddKalecgos(cards, "TB_BaconUps_109", "TB_BaconUps_109e");
    AddStatEnchantment(cards, "BGS_041e", 1, 1);
    AddStatEnchantment(cards, "TB_BaconUps_109e", 2, 2);

    // Deflect-o-Bot: during combat, after summoning a Mech, gain attack and
    // Divine Shield.  The Divine Shield tag is applied as a mutable instance
    // flag; the enchantment carries the attack delta.
    AddDeflectOBot(cards, "BGS_071", "BGS_071e");
    AddDeflectOBot(cards, "TB_BaconUps_123", "TB_BaconUps_123e");
    AddStatEnchantment(cards, "BGS_071e", 2, 0);
    AddStatEnchantment(cards, "TB_BaconUps_123e", 4, 0);

    // Refreshing Anomaly: the next refresh (two for golden) is free.
    Power refreshPower;
    refreshPower.AddBattlecryTask(FreeRefreshTask{ 1 });
    cards.emplace("BGS_116", CardDef{ refreshPower });
    refreshPower.ClearData();
    refreshPower.AddBattlecryTask(FreeRefreshTask{ 2 });
    cards.emplace("TB_BaconUps_167", CardDef{ std::move(refreshPower) });

    // Molten Rock: after playing an Elemental, gain +1 Health (+2 Health for
    // the linked golden copy).  The source is the owner of the trigger.
    AddMoltenRock(cards, "BGS_127", "BGS_127e");
    AddMoltenRock(cards, "TB_Baconups_202", "TB_Baconups_203e");
    AddStatEnchantment(cards, "BGS_127e", 0, 1);
    AddStatEnchantment(cards, "TB_Baconups_203e", 0, 2);
}
}  // namespace RosettaStone::Battlegrounds
