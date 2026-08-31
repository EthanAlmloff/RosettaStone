#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch51.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomChromadrakeToHandTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/CastTavernSpellTask.hpp>
namespace RosettaStone::Battlegrounds {
void ModernMinionBehaviorsBatch51::AddAll(std::map<std::string, CardDef>& cards) {
    // Draconic Warden: one random Chromadrake on Battlecry and Deathrattle.
    Power warden;
    warden.AddBattlecryTask(SimpleTasks::RandomChromadrakeToHandTask{1});
    warden.AddDeathrattleTask(SimpleTasks::RandomChromadrakeToHandTask{1});
    cards.emplace("BG34_633", CardDef{std::move(warden)});
    Power wardenGolden;
    wardenGolden.AddBattlecryTask(SimpleTasks::RandomChromadrakeToHandTask{2});
    wardenGolden.AddDeathrattleTask(SimpleTasks::RandomChromadrakeToHandTask{2});
    cards.emplace("BG34_633_G", CardDef{std::move(wardenGolden)});

    // Hired Mount: manual Activate (2) gets a random Chromadrake.
    Power mount;
    mount.AddActivate({ActivateEffect::RANDOM_CHROMADRAKE, 2, 0, 0, 1});
    cards.emplace("BG36_240", CardDef{std::move(mount)});
    Power mountGolden;
    mountGolden.AddActivate({ActivateEffect::RANDOM_CHROMADRAKE, 2, 0, 0, 2});
    cards.emplace("BG36_240_G", CardDef{std::move(mountGolden)});

    // Runic Arcanist: Start of Combat: Cast Shiny Ring twice.
    Power arcanist;
    arcanist.AddStartCombatTask(SimpleTasks::CastTavernSpellTask{"BG28_168", 2});
    cards.emplace("BG36_245", CardDef{std::move(arcanist)});
    Power arcanistGolden;
    arcanistGolden.AddStartCombatTask(SimpleTasks::CastTavernSpellTask{"BG28_168", 2});
    cards.emplace("BG36_245_G", CardDef{std::move(arcanistGolden)});

}
}
