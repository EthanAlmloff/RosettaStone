#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/CastSpellBuffTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus ApplyMinionStatBuffTask::Run(Player& p,Minion&) {
  switch (m_target) {
    case MinionBuffTarget::FRIENDLY_BOARD_RACE: p.ApplySpellRaceBuff(m_race,m_attack,m_health,false); break;
    case MinionBuffTarget::FRIENDLY_BOARD_AND_HAND_RACE: p.ApplySpellRaceBuff(m_race,m_attack,m_health,true); break;
    case MinionBuffTarget::LEFTMOST_HAND: p.ApplySpellSpecialBuff(1,m_attack,m_health); break;
    case MinionBuffTarget::TAVERN_RACE_PERSISTENT: p.ApplyTavernRaceBuff(m_race,m_attack,m_health); break;
    case MinionBuffTarget::TAVERN_ALL_PERSISTENT: p.ApplyTavernRaceBuff(Race::INVALID,m_attack,m_health); break;
  }
  return TaskStatus::COMPLETE;
}
TaskStatus ApplyMinionStatBuffTask::Run(Player& p,Minion& source,Minion&) { return Run(p,source); }
}
