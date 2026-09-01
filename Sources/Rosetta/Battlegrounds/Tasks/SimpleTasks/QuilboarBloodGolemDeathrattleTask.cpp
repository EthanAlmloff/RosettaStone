#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/QuilboarBloodGolemDeathrattleTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus QuilboarBloodGolemDeathrattleTask::Run(Player& player, Minion& source)
{
    if (player.GetField().IsFull() || source.GetBloodGemCount() <= 0) return TaskStatus::STOP;
    const auto card = Cards::FindCardByID("BG30_MagicItem_442t");
    if (card.id.empty()) return TaskStatus::STOP;
    Minion golem{card};
    const auto [attack, health] = player.season14.BloodGemStats();
    golem.SetAttack(attack * source.GetBloodGemCount());
    golem.SetHealth(health * source.GetBloodGemCount());
    player.ApplyFreshMinionModifiers(golem);
    golem.getPlayerCallback = [&player]() -> Player& { return player; };
    if (player.getNextCardIndexCallback) golem.SetIndex(player.getNextCardIndexCallback());
    player.GetField().Add(golem, player.GetField().GetCount());
    Minion& added = player.GetField()[player.GetField().GetCount() - 1];
    player.GetField().ForEachAlive([&added](MinionData& data) { data.value().ActivateTrigger(TriggerType::SUMMON, added); });
    player.ApplySummonTrinkets(added);
    return TaskStatus::COMPLETE;
}
TaskStatus QuilboarBloodGolemDeathrattleTask::Run(Player& p, Minion& s, Minion&) { return Run(p, s); }
}
