#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsSimpleBatch.hpp>
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

void AddSelfRally(std::map<std::string, CardDef>& cards, const char* id,
                  const char* enchantment, int attack, int repetitions = 1)
{
    Power power;
    for (int i = 0; i < repetitions; ++i)
        power.AddRallyTask(AddEnchantmentTask{ enchantment, EntityType::SOURCE });
    cards.emplace(id, CardDef{ std::move(power) });
    Power enchant;
    enchant.AddEnchant(Enchant{ std::vector<Effect>{ Effects::AttackN(attack) } });
    cards.emplace(enchantment, CardDef{ std::move(enchant) });
}

}  // namespace

void ModernMinionBehaviorsSimpleBatch::AddAll(
    std::map<std::string, CardDef>& cards)
{
    // Exact pinned 36.4 text; golden values are explicit in the metadata.
    AddSelfRally(cards, "BG29_888", "BG29_888e", 2);
    AddSelfRally(cards, "BG29_888_G", "BG29_888e", 2, 2);
}
}  // namespace RosettaStone::Battlegrounds
