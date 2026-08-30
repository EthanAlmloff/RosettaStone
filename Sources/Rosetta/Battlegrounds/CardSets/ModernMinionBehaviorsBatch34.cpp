#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch34.hpp>
#include <Rosetta/Battlegrounds/Enchants/Effects.hpp>
#include <Rosetta/Battlegrounds/Enchants/Enchant.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AddEnchantmentTask.hpp>
#include <Rosetta/Battlegrounds/Triggers/Trigger.hpp>
#include <utility>
#include <vector>
namespace RosettaStone::Battlegrounds {
void ModernMinionBehaviorsBatch34::AddAll(std::map<std::string, CardDef>& cards) {
    auto enchant = [](std::map<std::string, CardDef>& cards, const char* id, int attack, int health) {
        Power p; p.AddEnchant(Enchant{std::vector<Effect>{Effects::AttackN(attack), Effects::HealthN(health)}});
        cards.emplace(id, CardDef{std::move(p)});
    };
    auto add = [](std::map<std::string, CardDef>& cards, const char* id, const char* e) {
        Power p; Trigger t{TriggerType::AFTER_PLAY_MINION};
        t.SetTriggerSource(TriggerSource::MINIONS_EXCEPT_SELF);
        t.SetCondition(SelfCondition::IsRace(Race::MECHANICAL));
        t.SetTasks({SimpleTasks::AddEnchantmentTask{e, EntityType::SOURCE}});
        p.AddTrigger(std::move(t)); cards.emplace(id, CardDef{std::move(p)});
    };
    enchant(cards, "BG22_HERO_200_Buddy_e", 3, 3);
    enchant(cards, "BG22_HERO_200_Buddy_Ge", 6, 6);
    add(cards, "BG22_HERO_200_Buddy", "BG22_HERO_200_Buddy_e");
    add(cards, "BG22_HERO_200_Buddy_G", "BG22_HERO_200_Buddy_Ge");
}
}
