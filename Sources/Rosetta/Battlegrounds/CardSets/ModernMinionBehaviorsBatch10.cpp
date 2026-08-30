// Copyright (c) 2026 Hearthstone BG AI contributors

#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch10.hpp>
#include <Rosetta/Battlegrounds/Enchants/Effects.hpp>
#include <Rosetta/Battlegrounds/Enchants/Enchant.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AddEnchantmentTask.hpp>

#include <utility>
#include <vector>

namespace RosettaStone::Battlegrounds
{
namespace
{
using SimpleTasks::AddEnchantmentTask;

void AddStatEnchantment(std::map<std::string, CardDef>& cards, const char* id,
                        int health)
{
    Power power;
    power.AddEnchant(Enchant{ std::vector<Effect>{ Effects::HealthN(health) } });
    cards.emplace(id, CardDef{ std::move(power) });
}

void AddLullabot(std::map<std::string, CardDef>& cards, const char* id,
                 const char* enchantmentID)
{
    Power power;
    Trigger trigger{ TriggerType::TURN_END };
    trigger.SetTriggerSource(TriggerSource::SELF);
    trigger.SetTasks(std::vector<TaskType>{
        AddEnchantmentTask{ enchantmentID, EntityType::SOURCE } });
    power.AddTrigger(std::move(trigger));
    cards.emplace(id, CardDef{ std::move(power) });
}
}  // namespace

void ModernMinionBehaviorsBatch10::AddAll(
    std::map<std::string, CardDef>& cards)
{
    // Lullabot: Magnetic. At the end of your turn, gain +1 Health; golden
    // gains +2 Health. These are the linked enchantments in the pinned 36.4
    // inventory.
    AddLullabot(cards, "BG26_146", "BG26_146e2");
    AddLullabot(cards, "BG26_146_G", "BG26_146_Ge2");
    AddStatEnchantment(cards, "BG26_146e2", 1);
    AddStatEnchantment(cards, "BG26_146_Ge2", 2);
}
}  // namespace RosettaStone::Battlegrounds
