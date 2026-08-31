#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch54.hpp>
#include <Rosetta/Battlegrounds/Enchants/Power.hpp>
#include <Rosetta/Battlegrounds/Cards/CardDef.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/CastTavernSpellTask.hpp>
#include <Rosetta/Battlegrounds/Triggers/Trigger.hpp>
namespace RosettaStone::Battlegrounds {
void ModernMinionBehaviorsBatch54::AddAll(std::map<std::string, CardDef>& cards) {
    Power normal;
    { Trigger t{TriggerType::AFTER_CAST_SPELL};
      // The spell must target this exact copy; FRIENDLY would incorrectly
      // fire every Gatekeeper when another copy receives the spell.
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
