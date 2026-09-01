#pragma once
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks { class DestroyLastDamageSourceTask { public:
 TaskStatus Run(Player&, Minion&); TaskStatus Run(Player&, Minion&, Minion&);
}; }}
