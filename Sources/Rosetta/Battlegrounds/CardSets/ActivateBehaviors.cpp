#include <Rosetta/Battlegrounds/CardSets/ActivateBehaviors.hpp>

namespace RosettaStone::Battlegrounds
{
void ActivateBehaviors::AddAll(std::map<std::string, CardDef>& cards)
{
    // Suspicious Prisonguard: Activate (1): Give another minion +3/+3.
    Power buff;
    buff.AddActivate({ActivateEffect::BUFF_TARGET, 1, 3, 3});
    cards.emplace("BG36_345", CardDef{buff});
    buff.ClearData();
    buff.AddActivate({ActivateEffect::BUFF_TARGET, 1, 6, 6});
    cards.emplace("BG36_345_G", CardDef{buff});

    // Tyrael: Activate (2): Set another minion's stats to 50/50.
    Power setStats;
    setStats.AddActivate({ActivateEffect::SET_TARGET_STATS, 2, 50, 50});
    cards.emplace("BG36_356", CardDef{setStats});
    setStats.ClearData();
    setStats.AddActivate({ActivateEffect::SET_TARGET_STATS, 2, 100, 100});
    cards.emplace("BG36_356_G", CardDef{setStats});

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
