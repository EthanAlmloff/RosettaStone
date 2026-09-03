#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/StartCombatSelfCopyTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus StartCombatSelfCopyTask::Run(Player& p, Minion& s) {
    if (m_amount <= 0 || s.IsDestroyed() || p.battleField.IsFull()) return TaskStatus::STOP;
    return SummonTask{s.GetCardID(), m_amount, SummonSide::RIGHT}.Run(p, s);
}
TaskStatus StartCombatSelfCopyTask::Run(Player& p, Minion& s, Minion&) { return Run(p,s); }
}
