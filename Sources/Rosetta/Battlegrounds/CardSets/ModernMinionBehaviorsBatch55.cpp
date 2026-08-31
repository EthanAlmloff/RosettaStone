#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch55.hpp>
#include <Rosetta/Battlegrounds/Enchants/Power.hpp>
#include <utility>
namespace RosettaStone::Battlegrounds {
void ModernMinionBehaviorsBatch55::AddAll(std::map<std::string, CardDef>& cards) {
    Power normal; normal.AddActivate({ActivateEffect::GAIN_NEXT_BOUGHT_STATS, 1, 0, 0, 1});
    cards.emplace("BG36_180", CardDef{std::move(normal)});
    Power golden; golden.AddActivate({ActivateEffect::GAIN_NEXT_BOUGHT_STATS, 1, 0, 0, 2});
    cards.emplace("BG36_180_G", CardDef{std::move(golden)});
}
}
