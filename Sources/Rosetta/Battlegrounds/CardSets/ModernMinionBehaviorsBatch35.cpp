#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch35.hpp>
#include <Rosetta/Battlegrounds/Enchants/Effects.hpp>
#include <Rosetta/Battlegrounds/Enchants/Enchant.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AddEnchantmentTask.hpp>
#include <Rosetta/Battlegrounds/Triggers/Trigger.hpp>
#include <utility>
#include <vector>
namespace RosettaStone::Battlegrounds {
void ModernMinionBehaviorsBatch35::AddAll(std::map<std::string, CardDef>& cards) {
    auto enchant = [](std::map<std::string, CardDef>& cards, const char* id, int attack, int health) {
        Power p; p.AddEnchant(Enchant{std::vector<Effect>{Effects::AttackN(attack), Effects::HealthN(health)}});
        cards.emplace(id, CardDef{std::move(p)});
    };
    auto add = [](std::map<std::string, CardDef>& cards, const char* id, const char* e) {
        Power p; Trigger t{TriggerType::SELL_MINION};
        // SellMinion removes the sold entity before dispatching this event.
        // FRIENDLY is therefore the precise source: the removed Elemental is
        // never an eligible owner, while MINIONS_EXCEPT_SELF compares copied
        // zone indices and can incorrectly suppress a trigger when the sold
        // slot equals this minion's slot.
        t.SetTriggerSource(TriggerSource::FRIENDLY);
        t.SetCondition(SelfCondition::IsRace(Race::ELEMENTAL));
        t.SetTasks({SimpleTasks::AddEnchantmentTask{e, EntityType::SOURCE}});
        p.AddTrigger(std::move(t)); cards.emplace(id, CardDef{std::move(p)});
    };
    enchant(cards, "BG31_843e", 4, 4);
    enchant(cards, "BG31_843Ge", 8, 8);
    add(cards, "BG31_843", "BG31_843e");
    add(cards, "BG31_843_G", "BG31_843Ge");
}
}
