#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomChromadrakeToHandTask.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <effolkronium/random.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus RandomChromadrakeToHandTask::Run(Player& p, Minion&) { static constexpr const char* ids[] = {"BG34_634t","BG34_635t","BG34_636t","BG34_637t","BG34_638t"}; if (p.hand.IsFull()) return TaskStatus::STOP; for (int i=0;i<m_amount && !p.hand.IsFull();++i) { const Card c=Cards::FindCardByID(ids[effolkronium::random_thread_local::get<std::size_t>(0,4)]); if (c.id.empty()) return TaskStatus::STOP; Minion m{c}; p.ApplyFreshMinionModifiers(m); p.hand.Add(CardData{std::move(m)}); } return TaskStatus::COMPLETE; }
TaskStatus RandomChromadrakeToHandTask::Run(Player& p, Minion& s, Minion&) { return Run(p,s); }
}
