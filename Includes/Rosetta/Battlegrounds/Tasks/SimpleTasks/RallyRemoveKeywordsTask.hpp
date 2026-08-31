#ifndef ROSETTASTONE_BATTLEGROUNDS_RALLY_REMOVE_KEYWORDS_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_RALLY_REMOVE_KEYWORDS_TASK_HPP
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player; namespace SimpleTasks {
class RallyRemoveKeywordsTask { public:
  explicit RallyRemoveKeywordsTask(int keywordBits) : m_keywordBits(keywordBits) {}
  TaskStatus Run(Player&, Minion&); TaskStatus Run(Player&, Minion&, Minion&);
 private: int m_keywordBits = 0;
};
} }
#endif
