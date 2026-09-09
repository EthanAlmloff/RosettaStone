#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernTokenBehaviorsBatch70.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomCardToHandTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/GoldenizeTierMinionTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomSummonFromPoolTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AddEnchantmentTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonTauntBuffSelfTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomBountyToHandTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/MinionOfferingTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/CopyTargetBattlecryTask.hpp>
#include <Rosetta/Battlegrounds/CardSets/BuddyBehaviors.hpp>
#include <map>
#include <algorithm>
#include <string>
#include <variant>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[ModernTokenBehaviorsBatch70] generated rows are registered")
{
    std::map<std::string, CardDef> cards;
    ModernTokenBehaviorsBatch70::AddAll(cards);
    for (const auto* id : {
             "BG30_121", "BG30_121_G", "BG31_148", "BG31_148_G",
             "BG31_826", "BG31_826_G", "BG30_802",
             "TB_BaconShop_HP_105t", "TB_BaconShop_HERO_10_Buddy",
             "TB_BaconShop_HERO_10_Buddy_G", "TB_BaconShop_HERO_18_Buddy",
             "TB_BaconShop_HERO_18_Buddy_G", "TB_BaconShop_HERO_25_Buddy",
             "TB_BaconShop_HERO_25_Buddy_G", "TB_BaconShop_HERO_92_Buddy",
             "TB_BaconShop_HERO_92_Buddy_G", "TB_BaconShop_HERO_53_Buddy",
             "TB_BaconShop_HERO_53_Buddy_G", "TB_BaconShop_HERO_76_Buddy",
             "TB_BaconShop_HERO_76_Buddy_G", "TB_BaconUps_045",
             "TB_BaconUps_089"})
        CHECK(cards.contains(id));
}

TEST_CASE("[ModernTokenBehaviorsBatch70] premium generated tokens preserve parent paths")
{
    std::map<std::string, CardDef> cards;
    ModernTokenBehaviorsBatch70::AddAll(cards);
    CHECK(cards.contains("TB_BaconUps_045"));
    CHECK(cards.at("TB_BaconUps_045").power.GetBattlecryTask().empty());

    const auto& tasks = cards.at("TB_BaconUps_089").power.GetBattlecryTask();
    // The second Discover is reopened by Player after the first modal commits;
    // keeping two tasks here would attempt to replace an already-open public
    // decision and silently lose the premium repeat.
    REQUIRE(tasks.size() == 1);
    for (const auto& task : tasks)
    {
        const auto* offering = std::get_if<SimpleTasks::MinionOfferingTask>(&task);
        REQUIRE(offering != nullptr);
        CHECK(offering->GetRace() == Race::MURLOC);
        CHECK(offering->GetMinTier() == 1);
        CHECK(offering->GetMaxTier() == 7);
        CHECK(offering->GetCount() == 3);
        CHECK(offering->RequiresFriendlyRace());
    }
}

TEST_CASE("[ModernTokenBehaviorsBatch70] Mini-Zerek targets Tavern and goldenizes copy")
{
    std::map<std::string, CardDef> cards;
    ModernTokenBehaviorsBatch70::AddAll(cards);

    for (const auto* id : {"BG31_HERO_005_Buddy", "BG31_HERO_005_Buddy_G"})
    {
        const auto& definition = cards.at(id);
        CHECK(definition.playReqs.contains(PlayReq::REQ_TARGET_TO_PLAY));
        CHECK(definition.playReqs.contains(PlayReq::REQ_TAVERN_MINION_TARGET));
        REQUIRE(definition.power.GetBattlecryTask().size() == 1);
        CHECK(std::holds_alternative<SimpleTasks::CopyTargetBattlecryTask>(
            definition.power.GetBattlecryTask().front()));
    }

    const auto& normal = std::get<SimpleTasks::CopyTargetBattlecryTask>(
        cards.at("BG31_HERO_005_Buddy").power.GetBattlecryTask().front());
    const auto& golden = std::get<SimpleTasks::CopyTargetBattlecryTask>(
        cards.at("BG31_HERO_005_Buddy_G").power.GetBattlecryTask().front());
    CHECK_FALSE(normal.Golden());
    CHECK(golden.Golden());
}

TEST_CASE("[ModernTokenBehaviorsBatch70] lifecycle Buddy families are registered")
{
    std::map<std::string, CardDef> cards;
    ModernTokenBehaviorsBatch70::AddAll(cards);
    // These five legacy Buddies intentionally resolve at the authoritative
    // Player/Game/Battle boundaries rather than through a fixed CardDef task.
    // Keep the registry and executable catalogue in lockstep so a future
    // card-data refresh cannot silently turn one into a metadata-only row.
    constexpr std::array expected = {
        std::pair{"TB_BaconShop_HERO_02_Buddy", 77494},
        std::pair{"TB_BaconShop_HERO_08_Buddy", 77626},
        std::pair{"TB_BaconShop_HERO_11_Buddy", 77821},
        std::pair{"TB_BaconShop_HERO_12_Buddy", 77843},
        std::pair{"TB_BaconShop_HERO_17_Buddy", 77805},
    };
    for (const auto& [id, dbf] : expected)
    {
        CHECK(cards.contains(id));
        CHECK(cards.contains(std::string{id} + "_G"));
        const auto it = std::find_if(
            BUDDY_LEGACY_BEHAVIORS.begin(), BUDDY_LEGACY_BEHAVIORS.end(),
            [id, dbf](const BuddyLegacyDefinition& definition) {
                return definition.id == id && definition.dbfID == dbf;
            });
        CHECK(it != BUDDY_LEGACY_BEHAVIORS.end());
    }
}

TEST_CASE("[ModernTokenBehaviorsBatch70] Tuskarr Raider scales all bounty hooks")
{
    std::map<std::string, CardDef> cards;
    ModernTokenBehaviorsBatch70::AddAll(cards);
    auto& normal = cards.at("TB_BaconShop_HERO_18_Buddy").power;
    auto& golden = cards.at("TB_BaconShop_HERO_18_Buddy_G").power;
    REQUIRE(normal.GetBattlecryTask().size() == 1);
    REQUIRE(normal.GetDeathrattleTask().size() == 1);
    REQUIRE(normal.GetRallyTask().size() == 1);
    REQUIRE(golden.GetBattlecryTask().size() == 1);
    REQUIRE(golden.GetDeathrattleTask().size() == 1);
    REQUIRE(golden.GetRallyTask().size() == 1);
    CHECK(std::holds_alternative<SimpleTasks::RandomBountyToHandTask>(normal.GetBattlecryTask().front()));
    CHECK(std::holds_alternative<SimpleTasks::RandomBountyToHandTask>(golden.GetRallyTask().front()));
}

TEST_CASE("[ModernTokenBehaviorsBatch70] Kil'rek golden scales demon hand generation")
{
    std::map<std::string, CardDef> cards;
    ModernTokenBehaviorsBatch70::AddAll(cards);
    const auto& normal = cards.at("TB_BaconShop_HERO_37_Buddy").power.GetDeathrattleTask();
    const auto& golden = cards.at("TB_BaconShop_HERO_37_Buddy_G").power.GetDeathrattleTask();
    REQUIRE(normal.size() == 1);
    REQUIRE(golden.size() == 1);
    CHECK(std::get<SimpleTasks::RandomCardToHandTask>(normal.front()).GetAmount() == 1);
    CHECK(std::get<SimpleTasks::RandomCardToHandTask>(golden.front()).GetAmount() == 2);
    CHECK(std::get<SimpleTasks::RandomCardToHandTask>(normal.front()).GetRace() == Race::DEMON);
}

TEST_CASE("[ModernTokenBehaviorsBatch70] generated Buddy rows carry executable tasks")
{
    std::map<std::string, CardDef> cards;
    ModernTokenBehaviorsBatch70::AddAll(cards);
    for (const auto* id : {
             "TB_BaconShop_HERO_23_Buddy", "TB_BaconShop_HERO_23_Buddy_G",
             "BG25_HERO_105_Buddy", "BG25_HERO_105_Buddy_G",
             "BG25_HERO_100_Buddy", "BG25_HERO_100_Buddy_G",
             "TB_BaconShop_HERO_702_Buddy", "TB_BaconShop_HERO_702_Buddy_G",
             "TB_BaconShop_HERO_56_Buddy", "TB_BaconShop_HERO_56_Buddy_G",
             "TB_BaconShop_HERO_36_Buddy", "TB_BaconShop_HERO_36_Buddy_G",
             "TB_BaconShop_HERO_95_Buddy", "TB_BaconShop_HERO_95_Buddy_G"})
        CHECK(cards.contains(id));

    const auto& normalMucks = cards.at("TB_BaconShop_HERO_23_Buddy").power.GetBattlecryTask();
    const auto& goldenMucks = cards.at("TB_BaconShop_HERO_23_Buddy_G").power.GetBattlecryTask();
    REQUIRE(normalMucks.size() == 1);
    REQUIRE(goldenMucks.size() == 1);
    CHECK(std::get<SimpleTasks::RandomCardToHandTask>(normalMucks.front()).GetAmount() == 1);
    CHECK(std::get<SimpleTasks::RandomCardToHandTask>(goldenMucks.front()).GetAmount() == 2);
    CHECK(std::get<SimpleTasks::RandomCardToHandTask>(normalMucks.front()).IsBattlecryOnly());

    const auto& normalTalent = cards.at("BG25_HERO_105_Buddy").power.GetBattlecryTask();
    const auto& goldenTalent = cards.at("BG25_HERO_105_Buddy_G").power.GetBattlecryTask();
    REQUIRE(normalTalent.size() == 1);
    REQUIRE(goldenTalent.size() == 1);
    CHECK(std::get<SimpleTasks::GoldenizeTierMinionTask>(normalTalent.front()).Count() == 1);
    CHECK(std::get<SimpleTasks::GoldenizeTierMinionTask>(goldenTalent.front()).Count() == 2);
    // HERO_702 is Mawsworn Soulkeeper; its normal/golden deathrattle is
    // represented by two/four independent random-Undead summon tasks.
    const auto& mawsworn = cards.at("TB_BaconShop_HERO_702_Buddy").power.GetDeathrattleTask();
    const auto& mawswornGolden = cards.at("TB_BaconShop_HERO_702_Buddy_G").power.GetDeathrattleTask();
    CHECK(mawsworn.size() == 2);
    CHECK(mawswornGolden.size() == 4);

    // HERO_56 is Vaelastrasz (Rally -> Dragon), not Chromie.  Chromie is
    // HERO_57 and is intentionally lifecycle-owned by Player::RefreshTavern.
    const auto& vaelastrasz = cards.at("TB_BaconShop_HERO_56_Buddy").power.GetTrigger();
    const auto& vaelastraszGolden = cards.at("TB_BaconShop_HERO_56_Buddy_G").power.GetTrigger();
    REQUIRE(vaelastrasz.has_value());
    REQUIRE(vaelastraszGolden.has_value());
    CHECK(vaelastrasz->GetTriggerType() == TriggerType::RALLY);
    CHECK(vaelastraszGolden->GetTriggerType() == TriggerType::RALLY);
    CHECK(vaelastrasz->GetTriggerSource() == TriggerSource::FRIENDLY);
    CHECK(vaelastraszGolden->GetTriggerSource() == TriggerSource::FRIENDLY);

    CHECK(cards.contains("TB_BaconShop_HERO_57_Buddy"));
    CHECK(cards.contains("TB_BaconShop_HERO_57_Buddy_G"));

    // Asher is a lifecycle-owned self-buff: Player::SellMinion applies the
    // idempotent 1/1 or 2/2 persistent gain after the sold entity is removed.
    CHECK(cards.contains("TB_BaconShop_HERO_36_Buddy"));
    CHECK(cards.contains("TB_BaconShop_HERO_36_Buddy_G"));

    // Wandering Treant is resolved by Battle after a friendly Taunt is
    // attacked.  It must not be represented as a summon trigger, which
    // would fire for the wrong event and buff only the observer.
    CHECK_FALSE(cards.at("TB_BaconShop_HERO_95_Buddy").power.GetTrigger().has_value());
    CHECK_FALSE(cards.at("TB_BaconShop_HERO_95_Buddy_G").power.GetTrigger().has_value());
    CHECK(cards.contains("TB_BaconShop_HERO_72_Buddy"));
    CHECK(cards.contains("TB_BaconShop_HERO_72_Buddy_G"));

    const auto& festergut = cards.at("BG25_HERO_100_Buddy").power.GetDeathrattleTask();
    const auto& festergutGolden = cards.at("BG25_HERO_100_Buddy_G").power.GetDeathrattleTask();
    REQUIRE(festergut.size() == 2);
    REQUIRE(festergutGolden.size() == 2);
    CHECK(std::holds_alternative<SimpleTasks::RandomSummonFromPoolTask>(festergut.front()));
    CHECK(std::holds_alternative<SimpleTasks::RandomCardToHandTask>(festergut.back()));
    CHECK(std::get<SimpleTasks::RandomCardToHandTask>(festergut.back()).GetRace() == Race::UNDEAD);
    CHECK(std::get<SimpleTasks::RandomCardToHandTask>(festergut.back()).GetAmount() == 1);
    CHECK(std::get<SimpleTasks::RandomCardToHandTask>(festergutGolden.back()).GetAmount() == 2);
}
