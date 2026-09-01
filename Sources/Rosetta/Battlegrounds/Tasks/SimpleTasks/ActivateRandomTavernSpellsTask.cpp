#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ActivateRandomTavernSpellsTask.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/CardSets/TavernSpellBehaviors.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <effolkronium/random.hpp>
#include <vector>
namespace RosettaStone::Battlegrounds::SimpleTasks {
using Random = effolkronium::random_static;
TaskStatus ActivateRandomTavernSpellsTask::Run(Player& p, Minion& s) { if(m_amount<=0||s.IsDestroyed()) return TaskStatus::STOP; std::vector<std::string> pool; for(const auto& c:Cards::GetAllCards()) if(c.isBattlegroundsPoolSpell&&c.normalDbfID==0&&FindTavernSpellBehavior(c.id).effect!=TavernSpellEffect::NONE) pool.emplace_back(c.id); if(pool.empty()) return TaskStatus::STOP; for(int i=0;i<m_amount;++i){ const auto& id=pool[Random::get<std::size_t>(0,pool.size()-1)]; p.CastTavernSpellFree(id,1,s.GetZonePosition()); } return TaskStatus::COMPLETE; }
TaskStatus ActivateRandomTavernSpellsTask::Run(Player& p, Minion& s, Minion&) { return Run(p,s); }
}
