#pragma once
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player; namespace SimpleTasks {
class BuyTavernSpellMurlocTask {
 public:
  explicit BuyTavernSpellMurlocTask(int limit): m_limit(limit) {}
  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&, Minion&, Minion&);
  // Shared purchase-event path for persistent effects (which have no Minion
  // source). `progress` is owned by the effect instance and is reset by the
  // recruit-turn boundary, while the reward itself remains this task's
  // canonical taught-Murloc/pool path.
  TaskStatus Run(Player&, int& progress);
 private: int m_limit;
};
}}
