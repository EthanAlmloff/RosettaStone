#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/CardSets/TavernSpellBehaviors.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomBountyToHandTask.hpp>
#include <effolkronium/random.hpp>
#include <algorithm>
#include <array>
#include <string_view>
#include <vector>
using Random=effolkronium::random_thread_local;
namespace RosettaStone::Battlegrounds::SimpleTasks
{
TaskStatus RandomBountyToHandTask::Run(Player& player, Minion&)
{
    if (m_amount <= 0 || player.hand.IsFull()) return TaskStatus::STOP;

    // The canonical Bounty pool is exactly BG33_811..815.  Do not use a
    // prefix: later BG33_81x effects are not Bounties.
    constexpr std::array<std::string_view, 5> bountyIds = {
        "BG33_811", "BG33_812", "BG33_813", "BG33_814", "BG33_815"};
    std::vector<const Card*> pool;
    for (const auto& card : Cards::GetAllCards())
    {
        if (card.normalDbfID != 0 ||
            card.GetCardType() != CardType::BATTLEGROUND_SPELL ||
            FindTavernSpellBehavior(card.id).effect == TavernSpellEffect::NONE)
            continue;
        if (std::find(bountyIds.begin(), bountyIds.end(), card.id) !=
            bountyIds.end())
            pool.push_back(&card);
    }
    if (pool.empty()) return TaskStatus::STOP;
    for (int i = 0; i < m_amount && !player.hand.IsFull(); ++i)
        player.hand.Add(CardData{Spell{*pool[Random::get<std::size_t>(
            0, pool.size() - 1)]}});
    return TaskStatus::COMPLETE;
}

TaskStatus RandomBountyToHandTask::Run(Player& player, Minion& source,
                                       Minion&)
{
    return Run(player, source);
}
}  // namespace RosettaStone::Battlegrounds::SimpleTasks
