#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch29.hpp>
#include <Rosetta/Battlegrounds/Enchants/Effects.hpp>
#include <Rosetta/Battlegrounds/Enchants/Enchant.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AddEnchantmentTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SetGameTagTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/MechDiscoverMagnetizeTask.hpp>
#include <Rosetta/Battlegrounds/Triggers/Trigger.hpp>
#include <utility>
#include <vector>
namespace RosettaStone::Battlegrounds
{
namespace
{
void AddEnchant(std::map<std::string, CardDef>& cards, const char* id, int attack, int health)
{
    std::vector<Effect> effects;
    if (attack) effects.emplace_back(Effects::AttackN(attack));
    if (health) effects.emplace_back(Effects::HealthN(health));
    Power power;
    power.AddEnchant(RosettaStone::Battlegrounds::Enchant{ std::move(effects) });
    cards.emplace(id, CardDef{ std::move(power) });
}
void AddFrenzyBuff(std::map<std::string, CardDef>& cards, const char* id,
                   const char* enchant, EntityType target)
{
    Power power; Trigger trigger{ TriggerType::TAKE_DAMAGE };
    trigger.SetTriggerSource(TriggerSource::SELF);
    trigger.SetTasks({ SimpleTasks::AddEnchantmentTask{ enchant, target } });
    power.AddTrigger(std::move(trigger)); cards.emplace(id, CardDef{ std::move(power) });
}
void AddShieldFrenzy(std::map<std::string, CardDef>& cards, const char* id, int count)
{
    Power power; Trigger trigger{ TriggerType::TAKE_DAMAGE };
    trigger.SetTriggerSource(TriggerSource::SELF);
    std::vector<TaskType> tasks;
    for (int i = 0; i < count; ++i)
        tasks.emplace_back(SimpleTasks::SetGameTagTask{ EntityType::SOURCE, GameTag::DIVINE_SHIELD, 1 });
    trigger.SetTasks(std::move(tasks)); power.AddTrigger(std::move(trigger));
    cards.emplace(id, CardDef{ std::move(power) });
}
}
void ModernMinionBehaviorsBatch29::AddAll(std::map<std::string, CardDef>& cards)
{
    // Clunker Junker uses a typed target followed by a public Mech Discover.
    // The modal is committed by Player::ApplyChoice so the selected option
    // is magnetized onto the exact target selected for the Battlecry.
    {
        Power power;
        power.AddBattlecryTask(SimpleTasks::MechDiscoverMagnetizeTask{1});
        cards.emplace("BG29_503", CardDef{std::move(power)});
    }
    {
        Power power;
        power.AddBattlecryTask(SimpleTasks::MechDiscoverMagnetizeTask{2});
        cards.emplace("BG29_503_G", CardDef{std::move(power)});
    }
    AddShieldFrenzy(cards, "BG20_204", 1); AddShieldFrenzy(cards, "BG20_204_G", 2);
    AddEnchant(cards, "BG29_800e", 1, 0); AddFrenzyBuff(cards, "BG29_800", "BG29_800e", EntityType::SOURCE);
    AddEnchant(cards, "BG29_800Ge", 2, 0); AddFrenzyBuff(cards, "BG29_800_G", "BG29_800Ge", EntityType::SOURCE);
    AddEnchant(cards, "BG29_846e", 2, 0); AddFrenzyBuff(cards, "BG29_846", "BG29_846e", EntityType::MINIONS);
    AddEnchant(cards, "BG29_846Ge", 4, 0); AddFrenzyBuff(cards, "BG29_846_G", "BG29_846Ge", EntityType::MINIONS);
    AddEnchant(cards, "BG34_312e", 1, 1); AddFrenzyBuff(cards, "BG34_312", "BG34_312e", EntityType::MINIONS_NOSOURCE);
    AddEnchant(cards, "BG34_312Ge", 2, 2); AddFrenzyBuff(cards, "BG34_312_G", "BG34_312Ge", EntityType::MINIONS_NOSOURCE);
}
}
