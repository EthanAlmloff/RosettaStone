// Copyright (c) 2026 Hearthstone BG AI contributors

#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch9.hpp>
#include <Rosetta/Battlegrounds/Enchants/Effects.hpp>
#include <Rosetta/Battlegrounds/Enchants/Enchant.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/DamageTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/FriendlyRaceEnchantmentTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/LeftmostFriendlyRaceTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomFriendlyRaceTask.hpp>

#include <utility>
#include <vector>

namespace RosettaStone::Battlegrounds
{
namespace
{
using SimpleTasks::DamageTask;
using SimpleTasks::FriendlyRaceEnchantmentTask;
using SimpleTasks::LeftmostFriendlyRaceTask;
using SimpleTasks::RandomFriendlyRaceTask;

void AddStatEnchantment(std::map<std::string, CardDef>& cards,
                        const char* id, int attack, int health)
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

void AddRaceStartBuff(std::map<std::string, CardDef>& cards, const char* id,
                      const char* enchantmentID, Race race, int repetitions,
                      bool excludeSource)
{
    Power power;
    for (int i = 0; i < repetitions; ++i)
    {
        power.AddStartCombatTask(FriendlyRaceEnchantmentTask{
            enchantmentID, race, excludeSource });
    }
    cards.emplace(id, CardDef{ std::move(power) });
}

void AddAmberGuardian(std::map<std::string, CardDef>& cards, const char* id,
                      int amount)
{
    Power power;
    power.AddStartCombatTask(
        RandomFriendlyRaceTask{ Race::DRAGON, 2, 2, amount, true });
    cards.emplace(id, CardDef{ std::move(power) });
}

void AddPaperDrake(std::map<std::string, CardDef>& cards, const char* id,
                   int amount)
{
    Power power;
    power.AddStartCombatTask(
        LeftmostFriendlyRaceTask{ Race::DRAGON, 1, 2, amount, true });
    cards.emplace(id, CardDef{ std::move(power) });
}

void AddBoomInABox(std::map<std::string, CardDef>& cards, const char* id,
                  int repetitions)
{
    Power power;
    for (int i = 0; i < repetitions; ++i)
    {
        // DamageTask's MINIONS_NOSOURCE selector is friendly-only; pair it
        // with ENEMY_MINIONS so the semantic "all other minions" effect does
        // not accidentally omit the opposing combat field.
        power.AddStartCombatTask(
            DamageTask{ EntityType::MINIONS_NOSOURCE, 3 });
        power.AddStartCombatTask(
            DamageTask{ EntityType::ENEMY_MINIONS, 3 });
    }
    cards.emplace(id, CardDef{ std::move(power) });
}
}  // namespace

void ModernMinionBehaviorsBatch9::AddAll(
    std::map<std::string, CardDef>& cards)
{
    // Humming Bird: give all friendly Beasts +1 Attack for this combat;
    // golden gives +2.  The enchantment is attached to the combat copy.
    AddRaceStartBuff(cards, "BG26_805", "BG26_805e", Race::BEAST, 1, false);
    AddRaceStartBuff(cards, "BG26_805_G", "BG26_805e", Race::BEAST, 2,
                     false);
    AddStatEnchantment(cards, "BG26_805e", 1, 0);

    // Amber Guardian chooses one (two for golden) other friendly Dragon(s),
    // giving +2/+2 and Divine Shield.
    AddAmberGuardian(cards, "BG24_500", 1);
    AddAmberGuardian(cards, "BG24_500_G", 2);

    // Thousandth Paper Drake chooses the left-most Dragon(s), one normal and
    // two golden, granting +1/+2 and Windfury.
    AddPaperDrake(cards, "BG29_810", 1);
    AddPaperDrake(cards, "BG29_810_G", 2);

    // Boom-in-a-Box deals three damage to every other minion; golden repeats
    // the same event twice.
    AddBoomInABox(cards, "BG36_620", 1);
    AddBoomInABox(cards, "BG36_620_G", 2);
}
}  // namespace RosettaStone::Battlegrounds
