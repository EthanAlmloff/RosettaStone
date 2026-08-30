#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch15.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonTask.hpp>

#include <utility>

namespace RosettaStone::Battlegrounds
{
namespace
{
void AddDeathrattleSummon(std::map<std::string, CardDef>& cards,
                          const char* id, const char* token, int amount)
{
    Power power;
    power.AddDeathrattleTask(SimpleTasks::SummonTask{ token, amount });
    cards.emplace(id, CardDef{ std::move(power) });
}
}  // namespace

void ModernMinionBehaviorsBatch15::AddAll(
    std::map<std::string, CardDef>& cards)
{
    // Golden token IDs and counts are explicit in HearthstoneJSON 36.4.
    AddDeathrattleSummon(cards, "BG26_800", "BG26_800t", 2);
    AddDeathrattleSummon(cards, "BG26_800_G", "BG26_800_Gt", 2);
    AddDeathrattleSummon(cards, "BG_AV_309", "BG_AV_309t", 1);
    AddDeathrattleSummon(cards, "BG_AV_309_G", "BG_AV_309_Gt", 2);
    AddDeathrattleSummon(cards, "BG_DMF_533", "BG_DMF_533t", 2);
    // Ring Matron's golden entity uses the non-pattern BaconUps ID and
    // summons golden Imps; there is no BG_DMF_533_G card in 36.4 JSON.
    AddDeathrattleSummon(cards, "TB_BaconUps_309", "TB_BaconUps_309t", 2);
    AddDeathrattleSummon(cards, "BG_EX1_534", "BG_EX1_534t", 2);
    // Savannah Highmane likewise derives to BaconUps_049 and its golden
    // Hyena token, rather than an _G suffix.
    AddDeathrattleSummon(cards, "TB_BaconUps_049", "TB_BaconUps_049t", 2);
    AddDeathrattleSummon(cards, "BG_KAR_005", "BG_KAR_005a", 1);
    AddDeathrattleSummon(cards, "TB_BaconUps_004", "TB_BaconUps_004t", 1);
}
}  // namespace RosettaStone::Battlegrounds
