#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomSpellcraftToHandTask.hpp>

#include <effolkronium/random.hpp>

#include <iterator>

using Random = effolkronium::random_thread_local;
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus RandomSpellcraftToHandTask::Run(Player& player)
{
    if (player.hand.IsFull()) return TaskStatus::STOP;
    constexpr const char* spells[] = {
        "BG23_000t", "BG23_004t", "BG23_007t", "BG23_008t", "BG31_830t"};
    const auto id = spells[Random::get<std::size_t>(0, std::size(spells) - 1)];
    const auto card = Cards::FindCardByID(id);
    if (card.id.empty()) return TaskStatus::STOP;
    Spell spell(card);
    spell.SetTemporary(true);
    player.hand.Add(CardData{std::move(spell)});
    return TaskStatus::COMPLETE;
}
TaskStatus RandomSpellcraftToHandTask::Run(Player& p, Minion&) { return Run(p); }
TaskStatus RandomSpellcraftToHandTask::Run(Player& p, Minion& s, Minion&) { return Run(p, s); }
}
