#ifndef ROSETTASTONE_BATTLEGROUNDS_RANDOM_CHROMADRAKE_TO_HAND_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_RANDOM_CHROMADRAKE_TO_HAND_TASK_HPP
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Player; class Minion; namespace SimpleTasks {
class RandomChromadrakeToHandTask { public: explicit RandomChromadrakeToHandTask(int amount): m_amount(amount) {} TaskStatus Run(Player&, Minion&); TaskStatus Run(Player&, Minion&, Minion&); int GetAmount() const noexcept { return m_amount; } private: int m_amount; }; }}
#endif
