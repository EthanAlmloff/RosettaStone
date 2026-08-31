#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/CastTavernSpellTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus CastTavernSpellTask::Run(Player& player, Minion&) {
    return player.CastTavernSpellFree(m_cardID, m_amount) ? TaskStatus::COMPLETE : TaskStatus::STOP;
}
TaskStatus CastTavernSpellTask::Run(Player& player, Minion& source, Minion&) { return Run(player, source); }
}
