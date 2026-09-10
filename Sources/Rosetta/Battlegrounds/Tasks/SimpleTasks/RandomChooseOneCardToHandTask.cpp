#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Cards/CardDefs.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomChooseOneCardToHandTask.hpp>
#include <effolkronium/random.hpp>
#include <vector>
using Random = effolkronium::random_thread_local;
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus RandomChooseOneCardToHandTask::Run(Player& player, Minion&) {
    if (m_amount <= 0 || player.hand.IsFull()) return TaskStatus::STOP;
    std::vector<const Card*> candidates;
    for (const auto& card : Cards::GetAllCards()) {
        if (!card.isBattlegroundsPoolMinion || card.normalDbfID != 0 ||
            !card.hasBehavior || card.GetCardType() != CardType::MINION ||
            !HasActiveTribe(player.activeTribes, card) ||
            !card.gameTags.contains(GameTag::CHOOSE_ONE) ||
            card.gameTags.at(GameTag::CHOOSE_ONE) == 0 ||
            CardDefs::FindCardDefByID(card.id).lifecycle !=
                CardLifecycle::CHOOSE_ONE_SOURCE)
            continue;
        candidates.push_back(&card);
    }
    if (candidates.empty()) return TaskStatus::STOP;
    for (int i = 0; i < m_amount && !player.hand.IsFull(); ++i) {
        Minion generated{*candidates[Random::get<std::size_t>(
            0, candidates.size() - 1)]};
        // Generated copies are still real Choose-One sources. Preserve the
        // global Trailblazer modifier on the instance so its later modal
        // commit combines both validated branches exactly like a played
        // source; do not bypass the public modal here.
        if (player.season14.trailblazerCombinedChooseOne)
            generated.SetCombinedChooseOne(true);
        player.ApplyFreshMinionModifiers(generated);
        player.hand.Add(CardData{std::move(generated)});
    }
    return TaskStatus::COMPLETE;
}
TaskStatus RandomChooseOneCardToHandTask::Run(Player& p,Minion& s,Minion&){return Run(p,s);}
}
