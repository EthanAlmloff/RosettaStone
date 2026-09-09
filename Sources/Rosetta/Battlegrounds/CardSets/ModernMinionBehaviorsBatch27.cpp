#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch27.hpp>

#include <Rosetta/Battlegrounds/Enchants/Effects.hpp>
#include <Rosetta/Battlegrounds/Enchants/Enchant.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/FriendlyRaceEnchantmentTask.hpp>

#include <utility>
#include <vector>

namespace RosettaStone::Battlegrounds
{
void ModernMinionBehaviorsBatch27::AddAll(
    std::map<std::string, CardDef>& cards)
{
    // Leapfrogger's enchantment is a real persistent child: its deathrattle
    // gives one random friendly Beast the exact same child.  The reviewed
    // persistent-child gate applies the child stats and task atomically.
    auto child = [&cards](const char* id, int stat) {
        Power power;
        power.AddEnchant(Enchant{std::vector<Effect>{Effects::AttackN(stat),
                                                       Effects::HealthN(stat)}});
        power.AddDeathrattleTask(
            SimpleTasks::FriendlyRaceEnchantmentTask{id, Race::BEAST, false});
        cards.emplace(id, CardDef{std::move(power)});
    };

    child("BG21_000e", 1);
    child("BG21_000_Ge", 2);

    Power normal;
    normal.AddDeathrattleTask(
        SimpleTasks::FriendlyRaceEnchantmentTask{"BG21_000e", Race::BEAST,
                                                  true});
    cards.emplace("BG21_000", CardDef{std::move(normal)});

    Power golden;
    golden.AddDeathrattleTask(
        SimpleTasks::FriendlyRaceEnchantmentTask{"BG21_000_Ge", Race::BEAST,
                                                  true});
    cards.emplace("BG21_000_G", CardDef{std::move(golden)});
}
}
