#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/BattlecryTavernSpellDiscountTask.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus BattlecryTavernSpellDiscountTask::Run(Player& p, Minion&) { p.season14.nextTavernSpellDiscount += m_amount; return TaskStatus::COMPLETE; }
TaskStatus BattlecryTavernSpellDiscountTask::Run(Player& p, Minion& source, Minion&) { return Run(p, source); }
}
