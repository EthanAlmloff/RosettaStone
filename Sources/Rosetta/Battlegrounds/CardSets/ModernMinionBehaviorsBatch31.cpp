#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch31.hpp>
#include <Rosetta/Battlegrounds/Enchants/Effects.hpp>
#include <Rosetta/Battlegrounds/Enchants/Enchant.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AddEnchantmentTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RebornAttackTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SetGameTagTask.hpp>
#include <Rosetta/Battlegrounds/Triggers/Trigger.hpp>
#include <utility>
#include <vector>

namespace RosettaStone::Battlegrounds
{
void ModernMinionBehaviorsBatch31::AddAll(
    std::map<std::string, CardDef>& cards)
{
    auto enchant = [](std::map<std::string, CardDef>& cards, const char* id, int attack, int health) {
        std::vector<Effect> effects{Effects::AttackN(attack), Effects::HealthN(health)};
        Power power;
        power.AddEnchant(Enchant{std::move(effects)});
        cards.emplace(id, CardDef{std::move(power)});
    };
    auto barrier = [&enchant](std::map<std::string, CardDef>& cards, const char* id, const char* e, int attack) {
        enchant(cards, e, attack, attack);
        Power power;
        Trigger trigger{TriggerType::REBORN};
        trigger.SetTriggerSource(TriggerSource::FRIENDLY);
        trigger.SetTasks({SimpleTasks::AddEnchantmentTask{e, EntityType::SOURCE},
                          SimpleTasks::SetGameTagTask{EntityType::SOURCE, GameTag::DIVINE_SHIELD, 1}});
        power.AddTrigger(std::move(trigger));
        cards.emplace(id, CardDef{std::move(power)});
    };
    barrier(cards, "BG36_514", "BG36_514e", 7);
    barrier(cards, "BG36_514_G", "BG36_514Ge", 14);
    auto transfer = [](std::map<std::string, CardDef>& cards, const char* id, int multiplier, bool rightmost) {
        Power power;
        Trigger trigger{TriggerType::REBORN};
        trigger.SetTriggerSource(TriggerSource::FRIENDLY);
        trigger.SetTasks({SimpleTasks::RebornAttackTask{multiplier, rightmost}});
        power.AddTrigger(std::move(trigger));
        cards.emplace(id, CardDef{std::move(power)});
    };
    transfer(cards, "BG36_515", 1, true);
    transfer(cards, "BG36_515_G", 2, true);
    transfer(cards, "TB_BaconShop_HERO_22_Buddy", 1, false);
    transfer(cards, "TB_BaconShop_HERO_22_Buddy_G", 2, false);
}
}  // namespace RosettaStone::Battlegrounds
