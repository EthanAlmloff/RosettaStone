#include <Rosetta/Battlegrounds/CardSets/MagneticMinionBehaviors.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonTask.hpp>

#include <utility>
namespace RosettaStone::Battlegrounds
{
namespace
{
void AddDeathrattleSummon(std::map<std::string, CardDef>& cards,
                          const char* id, const char* tokenID, int amount)
{
    Power power;
    power.AddDeathrattleTask(
        SimpleTasks::SummonTask{ tokenID, amount });
    cards.emplace(id, CardDef{ std::move(power) });
}
}

void MagneticMinionBehaviors::AddAll(std::map<std::string, CardDef>& cards)
{
    // Prosthetic Hand's complete behavior is implemented by the shared
    // Magnetic play path; these entries keep the supported pair explicit.
    cards.emplace("BG_DEEP_015", CardDef{});
    cards.emplace("BG_DEEP_015_G", CardDef{});

    // Fixed-stat/keyword Magnetic Mechs.  Their static keywords come from
    // card metadata; the deathrattle pair uses the canonical summon task.
    cards.emplace("BG27_021", CardDef{});       // The Boommobile
    cards.emplace("BG27_021_G", CardDef{});
    cards.emplace("BG31_170", CardDef{});       // Frantic Alarm-o-Bot
    cards.emplace("BG31_170_G", CardDef{});
    cards.emplace("BG_BOT_563", CardDef{});     // Wargear
    cards.emplace("BG_BOT_563_G", CardDef{});
    AddDeathrattleSummon(cards, "BG_BOT_312", "BG_BOT_312t", 3);
    AddDeathrattleSummon(cards, "TB_BaconUps_032", "TB_BaconUps_032t", 3);
}
}
