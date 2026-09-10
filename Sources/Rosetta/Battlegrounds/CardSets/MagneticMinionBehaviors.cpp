#include <Rosetta/Battlegrounds/CardSets/MagneticMinionBehaviors.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AddEnchantmentTask.hpp>

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

void AddDeathrattleChild(std::map<std::string, CardDef>& cards,
                         const char* id, const char* childID)
{
    Power power;
    // The parent owns the deathrattle boundary.  The exact child task list is
    // installed by LifecycleEnchantment, which preserves one executable
    // source of truth and rejects sibling child IDs.
    power.AddDeathrattleTask(
        SimpleTasks::AddEnchantmentTask{ childID, EntityType::SOURCE });
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
    AddDeathrattleChild(cards, "BG_BOT_312", "BG_BOT_312e");
    AddDeathrattleSummon(cards, "TB_BaconUps_032", "TB_BaconUps_032t", 3);
}
}
