#ifndef ROSETTASTONE_BATTLEGROUNDS_SPELL_CAST_SELF_SCALING_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_SPELL_CAST_SELF_SCALING_TASK_HPP
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player; namespace SimpleTasks {
class SpellCastSelfScalingTask { public: SpellCastSelfScalingTask(int a,int h):attack(a),health(h){} TaskStatus Run(Player&,Minion&); TaskStatus Run(Player&,Minion&,Minion&); private:int attack,health;}; }}
#endif
