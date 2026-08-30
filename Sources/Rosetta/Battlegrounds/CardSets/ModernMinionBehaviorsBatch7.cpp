// Copyright (c) 2026 Hearthstone BG AI contributors

#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch7.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonTask.hpp>

#include <utility>

namespace RosettaStone::Battlegrounds
{
namespace
{
using SimpleTasks::SummonTask;

void AddStatic(std::map<std::string, CardDef>& cards, const char* id)
{
    // Stats, keywords, and the Eternal Knight aura are metadata-owned in the
    // pinned snapshot.  This registration is deliberately empty: it makes a
    // generated companion resolvable without inventing an unsupported task.
    cards.emplace(id, CardDef{});
}

void AddDeathrattleSummon(std::map<std::string, CardDef>& cards, const char* id,
                          const char* tokenID, int amount)
{
    Power power;
    power.AddDeathrattleTask(SummonTask{ tokenID, amount });
    cards.emplace(id, CardDef{ std::move(power) });
}
}  // namespace

void ModernMinionBehaviorsBatch7::AddAll(
    std::map<std::string, CardDef>& cards)
{
    // ---------------------------------------------------------------------
    // Fixed deathrattle summon family.
    // ---------------------------------------------------------------------
    // Eternal Summoner (Patch 36.4): Reborn. Deathrattle: Summon 1 Eternal
    // Knight. The golden form summons a Golden Eternal Knight. Reborn is a
    // metadata keyword; only the deterministic deathrattle belongs here.
    AddDeathrattleSummon(cards, "BG25_009", "BG25_008", 1);
    AddDeathrattleSummon(cards, "BG25_009_G", "BG25_008_G", 1);
    AddStatic(cards, "BG25_008");
    AddStatic(cards, "BG25_008_G");

    // ---------------------------------------------------------------------
    // Static generated companions whose producers are intentionally outside
    // this batch.  They are included so an experimental full-pool lobby can
    // resolve generated entities without a fake trigger chain.
    // ---------------------------------------------------------------------
    AddStatic(cards, "BG20_HERO_301_Buddy");
    AddStatic(cards, "BG20_HERO_301_Buddy_G");
    AddStatic(cards, "BG24_004");
    AddStatic(cards, "BG24_004_G");
    AddStatic(cards, "BG27_513");
    AddStatic(cards, "BG27_513_G");
    AddStatic(cards, "BG35_150t");
    AddStatic(cards, "BG35_150t_G");
    AddStatic(cards, "TB_BaconShop_HERO_40_Buddy");
    AddStatic(cards, "TB_BaconShop_HERO_40_Buddy_G");
    AddStatic(cards, "TB_BaconShop_HERO_68_Buddy");
    AddStatic(cards, "TB_BaconShop_HERO_68_Buddy_G");
    AddStatic(cards, "TB_BaconUps_156");
    AddStatic(cards, "BG36_621");
    AddStatic(cards, "BG36_621_G");
}
}  // namespace RosettaStone::Battlegrounds
