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
int SupportedSpellGold(const Spell& spell)
{
    // These are the only no-target economy spells promoted from the pinned
    // 36.4 inventory.  Every other spell remains fail-closed until its
    // target/effect semantics have a simulator implementation.
    if (spell.GetID() == "BG28_810") // Tavern Coin: Gain 1 Gold.
    {
        return 1;
    }
    if (spell.GetID() == "BG33_815") // Wealthy Bounty: Gain 2 Gold.
    {
        return 2;
    }
    return -1;
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
    return SupportedSpellGold(spell) >= 0 && spell.GetCost() >= 0 &&
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
    const int gold = SupportedSpellGold(spell);
    hand.Remove(card);
    remainCoin -= cost;
    remainCoin += gold;
    season14.Emit(Season14Event::SPELL_CAST);
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
