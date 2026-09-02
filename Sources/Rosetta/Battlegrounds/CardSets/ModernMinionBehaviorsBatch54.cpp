#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch54.hpp>
#include <Rosetta/Battlegrounds/Enchants/Power.hpp>
#include <Rosetta/Battlegrounds/Cards/CardDef.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/CastTavernSpellTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/CastTavernSpellOnAdjacentTask.hpp>
#include <Rosetta/Battlegrounds/Triggers/Trigger.hpp>
namespace RosettaStone::Battlegrounds {
void ModernMinionBehaviorsBatch54::AddAll(std::map<std::string, CardDef>& cards) {
    Power fauna;
    { Trigger t{TriggerType::TURN_END}; t.SetTriggerSource(TriggerSource::SELF);
      t.SetTasks({SimpleTasks::CastTavernSpellOnAdjacentTask{"BG28_845", 1}});
      fauna.AddTrigger(std::move(t)); }
    cards.emplace("BG32_837", CardDef{std::move(fauna)});
    Power faunaGolden;
    { Trigger t{TriggerType::TURN_END}; t.SetTriggerSource(TriggerSource::SELF);
      t.SetTasks({SimpleTasks::CastTavernSpellOnAdjacentTask{"BG28_845", 2}});
      faunaGolden.AddTrigger(std::move(t)); }
    cards.emplace("BG32_837_G", CardDef{std::move(faunaGolden)});
    Power normal;
    { Trigger t{TriggerType::AFTER_CAST_SPELL};
      // The spell must target this exact copy; FRIENDLY would incorrectly fire every Gatekeeper when another copy receives the spell.
      t.SetTriggerSource(TriggerSource::SELF);
      t.SetTasks({SimpleTasks::CastTavernSpellTask{"BG28_888", 1}}); normal.AddTrigger(std::move(t)); }
    cards.emplace("BG36_640", CardDef{std::move(normal)});
    Power golden;
    { Trigger t{TriggerType::AFTER_CAST_SPELL};
      t.SetTriggerSource(TriggerSource::SELF);
      t.SetTasks({SimpleTasks::CastTavernSpellTask{"BG28_888", 2}}); golden.AddTrigger(std::move(t)); }
    cards.emplace("BG36_640_G", CardDef{std::move(golden)});
}
}
