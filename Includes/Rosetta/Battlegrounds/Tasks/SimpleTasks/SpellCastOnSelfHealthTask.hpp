#ifndef ROSETTASTONE_BATTLEGROUNDS_SPELL_CAST_ON_SELF_HEALTH_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_SPELL_CAST_ON_SELF_HEALTH_TASK_HPP
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player; namespace SimpleTasks {
class SpellCastOnSelfHealthTask { public: explicit SpellCastOnSelfHealthTask(int h):health(h){} TaskStatus Run(Player&,Minion&); TaskStatus Run(Player&,Minion&,Minion&); private:int health;}; }}
#endif
