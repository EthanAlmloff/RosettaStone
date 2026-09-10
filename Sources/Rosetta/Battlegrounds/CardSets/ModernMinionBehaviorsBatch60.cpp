#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch60.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/PersistentSelfStatsTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/PersistentRaceBuffTask.hpp>
namespace RosettaStone::Battlegrounds {
void ModernMinionBehaviorsBatch60::AddAll(std::map<std::string, CardDef>& cards) {
  // Titus is a board aura; deathrattle repetition is resolved centrally by
  // Minion::ActivateTask so every existing and future deathrattle receives the
  // same normal (one extra) / golden (two extra) treatment.
  cards.emplace("BG25_354", CardDef{});
  cards.emplace("BG25_354_G", CardDef{});

  // Echoing Roar is a linked end-of-turn child.  The normal 36.4 child is
  // the only executable child: the pinned source has no BG28_814_G parent,
  // and BG28_814e2 is a +0/+0 generated record rather than a golden +4/+4
  // definition.  Its amount must never be guessed here.
  {
    Power power;
    Trigger trigger{TriggerType::TURN_END};
    trigger.SetTriggerSource(TriggerSource::SELF);
    trigger.SetTasks({SimpleTasks::PersistentSelfStatsTask{2, 2}});
    power.AddTrigger(std::move(trigger));
    cards.emplace("BG28_814e", CardDef{std::move(power)});
  }

  // Springy Spriggan's normal/golden children permanently buff the other
  // Mechanical minions at turn end.  Excluding the source is essential: the
  // child is attached to the Spriggan's owner/source instance.
  {
    Power power;
    Trigger trigger{TriggerType::TURN_END};
    trigger.SetTriggerSource(TriggerSource::SELF);
    trigger.SetTasks({SimpleTasks::PersistentRaceBuffTask{
        Race::MECHANICAL, 1, 0, true}});
    power.AddTrigger(std::move(trigger));
    cards.emplace("BG32_171e2", CardDef{std::move(power)});
  }
  {
    Power power;
    Trigger trigger{TriggerType::TURN_END};
    trigger.SetTriggerSource(TriggerSource::SELF);
    trigger.SetTasks({SimpleTasks::PersistentRaceBuffTask{
        Race::MECHANICAL, 2, 0, true}});
    power.AddTrigger(std::move(trigger));
    cards.emplace("BG32_171_Ge2", CardDef{std::move(power)});
  }

  // The minion itself must own the end-of-turn trigger as well.  The child
  // enchantment records above are provenance identities for magnetic
  // attachment/replay; registering only those children would leave an
  // ordinary Springy Spriggan with no executable trigger at all.
  {
    Power power;
    Trigger trigger{TriggerType::TURN_END};
    trigger.SetTriggerSource(TriggerSource::SELF);
    trigger.SetTasks({SimpleTasks::PersistentRaceBuffTask{
        Race::MECHANICAL, 1, 0, true}});
    power.AddTrigger(std::move(trigger));
    cards.emplace("BG32_171", CardDef{std::move(power)});
  }
  {
    Power power;
    Trigger trigger{TriggerType::TURN_END};
    trigger.SetTriggerSource(TriggerSource::SELF);
    trigger.SetTasks({SimpleTasks::PersistentRaceBuffTask{
        Race::MECHANICAL, 2, 0, true}});
    power.AddTrigger(std::move(trigger));
    cards.emplace("BG32_171_G", CardDef{std::move(power)});
  }
}
}
