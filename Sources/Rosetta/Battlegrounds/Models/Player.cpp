// Copyright (c) 2017-2024 Chris Ohk

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>

namespace RosettaStone::Battlegrounds
{
FieldZone& Player::GetField()
{
    return isInCombat ? battleField : recruitField;
}

void Player::SelectHero(std::size_t idx)
{
    const auto heroCard = Cards::FindCardByDbfID(heroChoices.at(idx));
    hero.Initialize(heroCard);

    selectHeroCallback(*this);
}

void Player::PrepareTavern()
{
    prepareTavernMinionsCallback(*this);
}

void Player::PurchaseMinion(std::size_t idx)
{
    if (remainCoin < NUM_COIN_PURCHASE_MINION)
    {
        return;
    }

    purchaseMinionCallback(*this, idx);

    remainCoin -= NUM_COIN_PURCHASE_MINION;
}

void Player::PlayCard(std::size_t handIdx, std::size_t fieldIdx, int targetIdx)
{
    if (std::holds_alternative<Minion>(hand[handIdx]))
    {
        // Check the field is full
        if (recruitField.IsFull())
        {
            return;
        }

        // Check if we can play this card and the target is valid
        if (!std::get<Minion>(hand[handIdx]).IsPlayableByCardReq(*this) ||
            !std::get<Minion>(hand[handIdx])
                 .IsValidPlayTarget(*this, targetIdx))
        {
            return;
        }

        CardData card = hand.Remove(hand[handIdx]);

        auto minion = std::get<Minion>(card);
        minion.getPlayerCallback = [this]() -> Player& { return *this; };
        minion.SetIndex(getNextCardIndexCallback());

        if (targetIdx == -1)
        {
            recruitField.Add(minion, fieldIdx);

            recruitField.ForEachAlive([&minion](MinionData& aliveMinion) {
                aliveMinion.value().ActivateTrigger(TriggerType::SUMMON,
                                                    minion);
            });

            minion.ActivateTask(PowerType::POWER, *this);
        }
        else
        {
            Minion& target = recruitField[targetIdx];

            recruitField.Add(minion, fieldIdx);

            recruitField.ForEachAlive([&minion](MinionData& aliveMinion) {
                aliveMinion.value().ActivateTrigger(TriggerType::SUMMON,
                                                    minion);
            });

            minion.ActivateTask(PowerType::POWER, *this, target);
        }

        recruitField.ForEachAlive([&minion](MinionData& aliveMinion) {
            aliveMinion.value().ActivateTrigger(TriggerType::AFTER_PLAY_MINION,
                                                minion);
        });
    }
    else
    {
        if (targetIdx == -1)
        {
            static_cast<void>(PlaySpell(handIdx));
        }
    }
}

namespace
{
enum class SpellBoardEffect
{
    NONE,
    ALL_STATS,
    LEFTMOST_STATS,
    DIVINE_SHIELD_ATTACK,
    ALL_AND_RACE,
    ALL_RACE_AND_DIVINE_SHIELD,
};

struct SupportedSpellEffect
{
    int gold = -1;
    int attack = 0;
    int health = 0;
    SpellBoardEffect board = SpellBoardEffect::NONE;
    Race race = Race::INVALID;
};

SupportedSpellEffect SupportedSpell(const Spell& spell)
{
    // These are exact no-target effects promoted from the pinned 36.4
    // inventory. Every other spell remains fail-closed until its target and
    // effect semantics have a simulator implementation.
    if (spell.GetID() == "BG28_168") // Shiny Ring: Give your minions +1/+1.
    {
        return { 0, 1, 1 };
    }
    if (spell.GetID() == "BG28_169") // Azerite Empowerment: +2/+2 twice.
    {
        return { 0, 4, 4 };
    }
    if (spell.GetID() == "BG33_813") // Selfish Bounty: left-most +6/+6.
    {
        return { 0, 6, 6, SpellBoardEffect::LEFTMOST_STATS };
    }
    if (spell.GetID() == "BG33_817") // Sanctify: Divine Shield minions +6 Attack.
    {
        return { 0, 6, 0, SpellBoardEffect::DIVINE_SHIELD_ATTACK };
    }
    if (spell.GetID() == "BG35_922") // Naga minions get another +2/+2.
    {
        return { 0, 2, 2, SpellBoardEffect::ALL_AND_RACE, Race::NAGA };
    }
    if (spell.GetID() == "BG36_246") // Dragons and Divine Shields repeat +2/+1.
    {
        return {
            0, 2, 1, SpellBoardEffect::ALL_RACE_AND_DIVINE_SHIELD,
            Race::DRAGON
        };
    }
    if (spell.GetID() == "BG28_810") // Tavern Coin: Gain 1 Gold.
    {
        return { 1, 0, 0 };
    }
    if (spell.GetID() == "BG33_815") // Wealthy Bounty: Gain 2 Gold.
    {
        return { 2, 0, 0 };
    }
    return {};
}
void ApplySpellBoardEffect(Player& player, const SupportedSpellEffect& effect)
{
    const auto addStats = [&effect](MinionData& aliveMinion) {
        Minion& minion = aliveMinion.value();
        minion.SetAttack(minion.GetAttack() + effect.attack);
        minion.SetHealth(minion.GetHealth() + effect.health);
    };

    switch (effect.board)
    {
        case SpellBoardEffect::NONE:
            return;
        case SpellBoardEffect::ALL_STATS:
            player.recruitField.ForEachAlive(addStats);
            return;
        case SpellBoardEffect::LEFTMOST_STATS:
        {
            bool applied = false;
            player.recruitField.ForEachAlive(
                [&applied, &addStats](MinionData& aliveMinion) {
                    if (!applied)
                    {
                        addStats(aliveMinion);
                        applied = true;
                    }
                });
            return;
        }
        case SpellBoardEffect::DIVINE_SHIELD_ATTACK:
            player.recruitField.ForEachAlive(
                [&effect](MinionData& aliveMinion) {
                    Minion& minion = aliveMinion.value();
                    if (minion.HasDivineShield())
                    {
                        minion.SetAttack(minion.GetAttack() + effect.attack);
                    }
                });
            return;
        case SpellBoardEffect::ALL_AND_RACE:
            player.recruitField.ForEachAlive(
                [&effect, &addStats](MinionData& aliveMinion) {
                    addStats(aliveMinion);
                    Minion& minion = aliveMinion.value();
                    if (minion.GetRace() == effect.race)
                    {
                        addStats(aliveMinion);
                    }
                });
            return;
        case SpellBoardEffect::ALL_RACE_AND_DIVINE_SHIELD:
            player.recruitField.ForEachAlive(
                [&effect, &addStats](MinionData& aliveMinion) {
                    addStats(aliveMinion);
                    Minion& minion = aliveMinion.value();
                    if (minion.GetRace() == effect.race ||
                        minion.HasDivineShield())
                    {
                        addStats(aliveMinion);
                    }
                });
            return;
    }
}
}  // namespace

bool Player::CanPlaySpell(std::size_t handIdx) const
{
    if (handIdx >= static_cast<std::size_t>(hand.GetCount()))
    {
        return false;
    }
    const CardData& card = hand[static_cast<int>(handIdx)];
    if (!std::holds_alternative<Spell>(card))
    {
        return false;
    }
    const Spell& spell = std::get<Spell>(card);
    return SupportedSpell(spell).gold >= 0 && spell.GetCost() >= 0 &&
           remainCoin >= spell.GetCost();
}

bool Player::PlaySpell(std::size_t handIdx)
{
    if (!CanPlaySpell(handIdx))
    {
        return false;
    }

    CardData& card = hand[static_cast<int>(handIdx)];
    const Spell& spell = std::get<Spell>(card);
    const int cost = spell.GetCost();
    const SupportedSpellEffect effect = SupportedSpell(spell);
    hand.Remove(card);
    remainCoin -= cost;
    remainCoin += effect.gold;
    season14.Emit(Season14Event::SPELL_CAST);
    ApplySpellBoardEffect(*this, effect);
    return true;
}

void Player::SellMinion(std::size_t idx)
{
    const auto minion = recruitField.Remove(recruitField[idx]);
    returnMinionCallback(minion.GetPoolIndex());

    remainCoin += 1;
}

void Player::UpgradeTavern()
{
    if (currentTier == TIER_UPPER_LIMIT || remainCoin < coinToUpgradeTavern)
    {
        return;
    }

    remainCoin -= coinToUpgradeTavern;
    upgradeTavernCallback(*this);
}

void Player::RefreshTavern()
{
    if (remainCoin < NUM_COIN_REFRESH_TAVERN)
    {
        return;
    }

    clearTavernMinionsCallback(*this);
    remainCoin -= NUM_COIN_REFRESH_TAVERN;

    prepareTavernMinionsCallback(*this);
}

void Player::FreezeTavern()
{
    freezeTavern = !freezeTavern;
    tavern.fieldZone.ForEach(
        [this](MinionData& minion) { minion.value().SetFrozen(freezeTavern); });
}

void Player::RearrangeMinion(std::size_t curIdx, std::size_t newIdx)
{
    if (curIdx == newIdx ||
        static_cast<int>(curIdx) >= recruitField.GetCount() ||
        static_cast<int>(newIdx) >= recruitField.GetCount())
    {
        return;
    }

    recruitField.Move(static_cast<int>(curIdx), static_cast<int>(newIdx));
}

void Player::CompleteRecruit() const
{
    completeRecruitCallback();
}

void Player::ProcessDefeat()
{
    // Dead players may be selected as Battlegrounds ghosts and can receive
    // combat damage again. Defeat must be idempotent: processing the same
    // player twice corrupts the remaining-player count and placement ranks.
    if (playState != PlayState::PLAYING)
    {
        return;
    }
    processDefeatCallback(*this);
}
}  // namespace RosettaStone::Battlegrounds
