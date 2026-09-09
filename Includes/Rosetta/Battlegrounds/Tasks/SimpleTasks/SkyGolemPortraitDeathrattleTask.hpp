#ifndef ROSETTASTONE_BATTLEGROUNDS_SKY_GOLEM_PORTRAIT_DEATHRATTLE_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_SKY_GOLEM_PORTRAIT_DEATHRATTLE_TASK_HPP

#include <Rosetta/Common/Enums/TaskEnums.hpp>

namespace RosettaStone::Battlegrounds
{
class Player;
class Minion;

namespace SimpleTasks
{
class SkyGolemPortraitDeathrattleTask
{
 public:
    TaskStatus Run(Player&, Minion&);
    TaskStatus Run(Player&, Minion&, Minion&);
};
}
}

#endif
