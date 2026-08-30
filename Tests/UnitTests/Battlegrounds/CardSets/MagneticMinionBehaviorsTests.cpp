#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/MagneticMinionBehaviors.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonTask.hpp>
#include <map>
using namespace RosettaStone::Battlegrounds;
TEST_CASE("[Magnetic] - Prosthetic Hand pair is magnetic and exact")
{
    std::map<std::string, CardDef> cards;
    MagneticMinionBehaviors::AddAll(cards);
    CHECK_EQ(cards.size(), 10);
    for (const auto id : { "BG_DEEP_015", "BG_DEEP_015_G" })
    {
        REQUIRE(cards.contains(id));
        Minion attachment{ Cards::FindCardByID(id) };
        CHECK(attachment.IsMagnetic());
        Minion mech{ Cards::FindCardByID("BG_BOT_563") };
        CHECK(attachment.CanMagnetizeTo(mech));
        Minion undead{ Cards::FindCardByID("BG22_HERO_001_Buddy") };
        CHECK(attachment.CanMagnetizeTo(undead));
        attachment.MagnetizeOnto(mech);
        CHECK_EQ(mech.GetAttack(), Cards::FindCardByID("BG_BOT_563").attack + Cards::FindCardByID(id).attack);
        CHECK_EQ(mech.GetHealth(), Cards::FindCardByID("BG_BOT_563").health + Cards::FindCardByID(id).health);
        CHECK(mech.HasReborn());
    }
}

TEST_CASE("[Magnetic] - fixed Mech pairs and deathrattle merge")
{
    std::map<std::string, CardDef> cards;
    MagneticMinionBehaviors::AddAll(cards);
    for (const auto id : { "BG27_021", "BG27_021_G", "BG31_170",
                           "BG31_170_G", "BG_BOT_563", "BG_BOT_563_G",
                           "BG_BOT_312", "TB_BaconUps_032" })
    {
        REQUIRE(cards.contains(id));
        Minion attachment{ Cards::FindCardByID(id) };
        Minion target{ Cards::FindCardByID("BG_BOT_563") };
        REQUIRE(attachment.IsMagnetic());
        REQUIRE(attachment.CanMagnetizeTo(target));
        const int attack = target.GetAttack() + attachment.GetAttack();
        const int health = target.GetHealth() + attachment.GetHealth();
        attachment.MagnetizeOnto(target);
        CHECK_EQ(target.GetAttack(), attack);
        CHECK_EQ(target.GetHealth(), health);
    }
    REQUIRE(std::holds_alternative<SimpleTasks::SummonTask>(
        cards.at("BG_BOT_312").power.GetDeathrattleTask().front()));
    const auto& normal = std::get<SimpleTasks::SummonTask>(
        cards.at("BG_BOT_312").power.GetDeathrattleTask().front());
    CHECK(normal.m_cardID == "BG_BOT_312t");
    CHECK_EQ(normal.m_amount, 3);
    const auto& golden = std::get<SimpleTasks::SummonTask>(
        cards.at("TB_BaconUps_032").power.GetDeathrattleTask().front());
    CHECK(golden.m_cardID == "TB_BaconUps_032t");
    CHECK_EQ(golden.m_amount, 3);
}

TEST_CASE("[Magnetic] - standalone and attachment modes are both legal")
{
    Player player;
    Minion attachment{ Cards::FindCardByID("BG31_170") };
    Minion target{ Cards::FindCardByID("BG_BOT_563") };

    // No target selects ordinary minion play; a supplied friendly Mech selects
    // the attachment path.  The bridge uses these same predicates for both
    // action branches, including attachment when the board is full.
    CHECK(attachment.IsPlayableByCardReq(player));
    CHECK(attachment.IsValidPlayTarget(player, -1));
    CHECK(attachment.CanMagnetizeTo(target));
}
