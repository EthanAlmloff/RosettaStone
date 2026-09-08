#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/CopyTargetBattlecryTask.hpp>

namespace RosettaStone::Battlegrounds::SimpleTasks
{
TaskStatus CopyTargetBattlecryTask::Run(Player&, Minion&)
{
    return TaskStatus::STOP;
}

TaskStatus CopyTargetBattlecryTask::Run(Player&, Minion& source, Minion& target)
{
    if (&source == &target || target.IsDestroyed())
        return TaskStatus::STOP;
    auto card = Cards::FindCardByID(target.GetCardID());
    if (card.id.empty() || card.GetCardType() != CardType::MINION)
        return TaskStatus::STOP;
    if (m_golden && card.premiumDbfID != 0)
    {
        const auto golden = Cards::FindCardByDbfID(card.premiumDbfID);
        if (!golden.id.empty()) card = golden;
    }
    const int attack = card.GetAttack();
    const int health = card.GetHealth();
    if (!source.TransformTo(card))
        return TaskStatus::STOP;
    source.SetAttack(attack);
    source.SetHealth(health);
    return TaskStatus::COMPLETE;
}
}  // namespace RosettaStone::Battlegrounds::SimpleTasks
