#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RebornAttackTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus RebornAttackTask::Run(Player&, Minion&) { return TaskStatus::COMPLETE; }
TaskStatus RebornAttackTask::Run(Player& player, Minion& source, Minion& target) {
    Minion* recipient = &target;
    if (m_rightmostUndead) {
        recipient = nullptr;
        player.GetField().ForEachAlive([&recipient](MinionData& data) {
            Minion& candidate = data.value();
            if (candidate.HasRace(Race::UNDEAD) &&
                (recipient == nullptr || candidate.GetZonePosition() > recipient->GetZonePosition())) recipient = &candidate;
        });
    }
    if (recipient != nullptr) recipient->SetAttack(recipient->GetAttack() + target.GetAttack() * m_multiplier);
    return TaskStatus::COMPLETE;
}
}
