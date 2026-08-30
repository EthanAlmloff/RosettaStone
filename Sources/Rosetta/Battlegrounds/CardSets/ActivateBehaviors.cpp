#include <Rosetta/Battlegrounds/CardSets/ActivateBehaviors.hpp>

namespace RosettaStone::Battlegrounds
{
void ActivateBehaviors::AddAll(std::map<std::string, CardDef>& cards)
{
    Power bird;
    bird.AddAvenge({AvengeEffect::BUFF_RACE, 1, 1, 1, Race::BEAST});
    cards.emplace("BG21_002", CardDef{bird});
    bird.ClearData();
    bird.AddAvenge({AvengeEffect::BUFF_RACE, 1, 2, 2, Race::BEAST});
    cards.emplace("BG21_002_G", CardDef{bird});

    Power abomination;
    abomination.AddAvenge({AvengeEffect::BUFF_SELF, 1, 1, 2, Race::INVALID, true});
    cards.emplace("BG25_014", CardDef{abomination});
    abomination.ClearData();
    abomination.AddAvenge({AvengeEffect::BUFF_SELF, 1, 2, 4, Race::INVALID, true});
    cards.emplace("BG25_014_G", CardDef{abomination});

}
}  // namespace RosettaStone::Battlegrounds
