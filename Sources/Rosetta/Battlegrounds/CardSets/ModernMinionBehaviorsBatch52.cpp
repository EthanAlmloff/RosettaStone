#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch52.hpp>
#include <Rosetta/Battlegrounds/Enchants/Power.hpp>
namespace RosettaStone::Battlegrounds {
void ModernMinionBehaviorsBatch52::AddAll(std::map<std::string, CardDef>& cards) {
    Power normal;
    normal.AddActivate({ActivateEffect::TAVERN_STATS_RANDOM_KEYWORD, 1, 8, 8, 1});
    cards.emplace("BG36_621", CardDef{std::move(normal)});
    Power golden;
    golden.AddActivate({ActivateEffect::TAVERN_STATS_RANDOM_KEYWORD, 1, 16, 16, 1});
    cards.emplace("BG36_621_G", CardDef{std::move(golden)});
}
}
