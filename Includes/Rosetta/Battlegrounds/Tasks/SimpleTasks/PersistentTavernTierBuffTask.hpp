#ifndef ROSETTASTONE_BATTLEGROUNDS_PERSISTENT_TAVERN_TIER_BUFF_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_PERSISTENT_TAVERN_TIER_BUFF_TASK_HPP
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player; namespace SimpleTasks {
class PersistentTavernTierBuffTask { public: PersistentTavernTierBuffTask(int a,int h,int tier):attack(a),health(h),maxTier(tier){} TaskStatus Run(Player&,Minion&); TaskStatus Run(Player&,Minion&,Minion&); private:int attack,health,maxTier;}; }}
#endif
