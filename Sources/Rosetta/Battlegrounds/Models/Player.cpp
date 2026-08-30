// Copyright (c) 2017-2024 Chris Ohk

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/CardSets/TavernSpellBehaviors.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>

#include <effolkronium/random.hpp>

#include <algorithm>
#include <set>
#include <vector>

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

    // Hero powers are metadata-only cards in RosettaStone.  Install their
    // cost and lifecycle state on the owning player at selection time; the
    // bridge still decides whether a target-dependent power is exposed.
    const auto* batch1 =
        FindSeason14HeroPowerBehavior(hero.card.heroPowerDbfID);
    const auto* batch2 =
        FindSeason14HeroPowerBehaviorBatch2(hero.card.heroPowerDbfID);
    const auto* batch3 =
        FindSeason14HeroPowerBehaviorBatch3(hero.card.heroPowerDbfID);
    const int heroPowerCost = batch1 != nullptr
                                  ? batch1->cost
                                  : (batch2 != nullptr
                                         ? batch2->cost
                                         : (batch3 != nullptr ? batch3->cost
                                                              : 0));
    season14.SetHeroPower(hero.card.heroPowerDbfID, heroPowerCost,
                          hero.card.heroPowerDbfID != 0);

    selectHeroCallback(*this);
}

void Player::PrepareTavern()
{
    // Preserve the identity of cards already in the Tavern.  Independently
    // frozen cards survive a normal fill and must not receive a persistent
    // spell bonus more than once on every subsequent turn.
    std::set<int> existingPoolIndices;
    tavern.fieldZone.ForEach([&existingPoolIndices](MinionData& minion) {
        existingPoolIndices.insert(minion.value().GetPoolIndex());
    });
    prepareTavernMinionsCallback(*this);
    if (season14.persistentShopAttack != 0 ||
        season14.persistentShopHealth != 0)
    {
        tavern.fieldZone.ForEach(
            [this, &existingPoolIndices](MinionData& minion) {
                if (existingPoolIndices.contains(
                        minion.value().GetPoolIndex()))
                {
                    return;
                }
                minion.value().SetAttack(minion.value().GetAttack() +
                                         season14.persistentShopAttack);
                minion.value().SetHealth(minion.value().GetHealth() +
                                         season14.persistentShopHealth);
            });
    }
}

void Player::PurchaseMinion(std::size_t idx)
{
    if (idx >= static_cast<std::size_t>(tavern.fieldZone.GetCount()))
    {
        return;
    }

    const int cost = season14.MinionPurchaseCost(NUM_COIN_PURCHASE_MINION);
    if (remainCoin < cost)
    {
        return;
    }

    const bool purchasedPirate =
        tavern.fieldZone[idx].GetRace() == Race::PIRATE;
    purchaseMinionCallback(*this, idx);

    remainCoin -= cost;
    remainCoin += season14.OnBuyMinion(purchasedPirate);
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

        if (minion.GetRace() == Race::ELEMENTAL)
        {
            const auto result = season14.OnPlayElemental();
            coinToUpgradeTavern = std::max(
                0, coinToUpgradeTavern + result.upgradeCostDelta);
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
using Random = effolkronium::random_thread_local;

bool ValidFriendlyBoardTarget(const Player& player, int targetIdx)
{
    return targetIdx >= 0 && targetIdx < player.recruitField.GetCount() &&
           !player.recruitField[static_cast<std::size_t>(targetIdx)]
                .IsDestroyed();
}

void ApplySpellBoardEffect(Player& player, const TavernSpellBehavior& effect,
                           int targetIdx)
{
    const auto addStats = [&effect](MinionData& aliveMinion) {
        Minion& minion = aliveMinion.value();
        minion.SetAttack(minion.GetAttack() + effect.attack);
        minion.SetHealth(minion.GetHealth() + effect.health);
    };

    switch (effect.effect)
    {
        case TavernSpellEffect::NONE:
            return;
        case TavernSpellEffect::ALL_STATS:
            player.recruitField.ForEachAlive(addStats);
            return;
        case TavernSpellEffect::ALL_STATS_AND_GOLDEN:
            player.recruitField.ForEachAlive(
                [&effect, &addStats](MinionData& aliveMinion) {
                    addStats(aliveMinion);
                    if (aliveMinion.value().IsGolden())
                    {
                        addStats(aliveMinion);
                    }
                });
            return;
        case TavernSpellEffect::LEFTMOST_STATS:
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
        case TavernSpellEffect::DIVINE_SHIELD_ATTACK:
            player.recruitField.ForEachAlive(
                [&effect](MinionData& aliveMinion) {
                    Minion& minion = aliveMinion.value();
                    if (minion.HasDivineShield())
                    {
                        minion.SetAttack(minion.GetAttack() + effect.attack);
                    }
                });
            return;
        case TavernSpellEffect::ALL_AND_RACE:
            player.recruitField.ForEachAlive(
                [&effect, &addStats](MinionData& aliveMinion) {
                    addStats(aliveMinion);
                    Minion& minion = aliveMinion.value();
                    // Card metadata can contain multiple gameplay tribes (or
                    // ALL for an amalgam).  Effects that say "Naga" must use
                    // the card's complete tribe predicate, not only its
                    // primary race field.
                    if (minion.HasRace(effect.race))
                    {
                        addStats(aliveMinion);
                    }
                });
            return;
        case TavernSpellEffect::ALL_RACE_AND_DIVINE_SHIELD:
            player.recruitField.ForEachAlive(
                [&effect, &addStats](MinionData& aliveMinion) {
                    addStats(aliveMinion);
                    Minion& minion = aliveMinion.value();
                    if (minion.HasRace(effect.race))
                    {
                        addStats(aliveMinion);
                    }
                    if (minion.HasDivineShield())
                    {
                        addStats(aliveMinion);
                    }
                });
            return;
        case TavernSpellEffect::RANDOM_STATS:
        {
            std::vector<Minion*> candidates;
            player.recruitField.ForEachAlive(
                [&candidates](MinionData& aliveMinion) {
                    candidates.push_back(&aliveMinion.value());
                });
            Random::shuffle(candidates.begin(), candidates.end());
            const auto count = std::min<std::size_t>(
                static_cast<std::size_t>(std::max(0, effect.randomCount)),
                candidates.size());
            for (std::size_t i = 0; i < count; ++i)
            {
                candidates[i]->SetAttack(candidates[i]->GetAttack() +
                                         effect.attack);
                candidates[i]->SetHealth(candidates[i]->GetHealth() +
                                         effect.health);
            }
            return;
        }
        case TavernSpellEffect::MENAGERIE_STATS:
        {
            std::set<Race> races;
            player.recruitField.ForEachAlive(
                [&races](MinionData& aliveMinion) {
                    const Minion& minion = aliveMinion.value();
                    // A dual-tribe or ALL minion contributes every concrete
                    // Battlegrounds type it belongs to.  Counting only
                    // GetRace() under-counts these cards and changes the
                    // number of Menagerie Tableware repeats.
                    for (const Race race : RACES_IN_BATTLEGROUNDS)
                    {
                        if (minion.HasRace(race))
                        {
                            races.insert(race);
                        }
                    }
                });
            const auto repeats = MenagerieTablewareRepeatCount(races.size());
            for (std::size_t i = 0; i < repeats; ++i)
            {
                player.recruitField.ForEachAlive(addStats);
            }
            return;
        }
        case TavernSpellEffect::ONE_PER_RACE_STATS:
        {
            // Resolve one friendly minion independently for every concrete
            // tribe.  A dual-tribe/ALL minion may therefore satisfy more
            // than one type, matching the game's per-type targeting model.
            for (const Race race : RACES_IN_BATTLEGROUNDS)
            {
                bool applied = false;
                player.recruitField.ForEachAlive(
                    [&effect, race, &applied](MinionData& aliveMinion) {
                        Minion& minion = aliveMinion.value();
                        if (!applied && minion.HasRace(race))
                        {
                            minion.SetAttack(minion.GetAttack() +
                                             effect.attack);
                            minion.SetHealth(minion.GetHealth() +
                                             effect.health);
                            applied = true;
                        }
                    });
            }
            return;
        }
        case TavernSpellEffect::SHOP_STATS:
            player.tavern.fieldZone.ForEach(
                [&effect](MinionData& minion) {
                    minion.value().SetAttack(minion.value().GetAttack() +
                                             effect.attack);
                    minion.value().SetHealth(minion.value().GetHealth() +
                                             effect.health);
                });
            return;
        case TavernSpellEffect::TARGET_STATS:
        {
            Minion& minion =
                player.recruitField[static_cast<std::size_t>(targetIdx)];
            minion.SetAttack(minion.GetAttack() + effect.attack);
            minion.SetHealth(minion.GetHealth() + effect.health);
            return;
        }
        case TavernSpellEffect::SET_TARGET_STATS:
        {
            Minion& minion =
                player.recruitField[static_cast<std::size_t>(targetIdx)];
            minion.SetAttack(effect.attack);
            minion.SetHealth(effect.health);
            return;
        }
        case TavernSpellEffect::TARGET_AND_RACE:
        {
            Minion& target =
                player.recruitField[static_cast<std::size_t>(targetIdx)];
            target.SetAttack(target.GetAttack() + effect.attack);
            target.SetHealth(target.GetHealth() + effect.health);
            player.recruitField.ForEachAlive(
                [&effect, &addStats](MinionData& aliveMinion) {
                    if (aliveMinion.value().HasRace(effect.race))
                    {
                        addStats(aliveMinion);
                    }
                });
            return;
        }
        case TavernSpellEffect::TARGET_STATS_REPEAT:
        {
            Minion& minion =
                player.recruitField[static_cast<std::size_t>(targetIdx)];
            const int repeats = minion.HasRace(effect.race) ? 3 : 2;
            minion.SetAttack(minion.GetAttack() + repeats * effect.attack);
            minion.SetHealth(minion.GetHealth() + repeats * effect.health);
            return;
        }
        case TavernSpellEffect::TARGET_STATS_AND_TAUNT:
        {
            Minion& minion =
                player.recruitField[static_cast<std::size_t>(targetIdx)];
            minion.SetAttack(minion.GetAttack() + effect.attack);
            minion.SetHealth(minion.GetHealth() + effect.health);
            minion.SetTaunt(true);
            return;
        }
        case TavernSpellEffect::TARGET_DIVINE_SHIELD:
        {
            Minion& minion =
                player.recruitField[static_cast<std::size_t>(targetIdx)];
            minion.SetGameTag(GameTag::DIVINE_SHIELD, 1);
            return;
        }
        case TavernSpellEffect::TARGET_STATS_TOGGLE_TAUNT:
        {
            Minion& minion =
                player.recruitField[static_cast<std::size_t>(targetIdx)];
            const bool alreadyTaunted = minion.HasTaunt();
            minion.SetAttack(minion.GetAttack() + effect.attack);
            minion.SetHealth(minion.GetHealth() + effect.health);
            minion.SetTaunt(!alreadyTaunted);
            return;
        }
        case TavernSpellEffect::SET_PLAYER_ARMOR:
            player.armor = effect.value;
            return;
        case TavernSpellEffect::NEXT_TURN_GOLD:
            player.season14.AddNextTurnGold(effect.value);
            return;
        case TavernSpellEffect::INCREASE_MAX_GOLD:
            player.season14.IncreaseMaxGold(effect.value);
            return;
        case TavernSpellEffect::FREE_REFRESHES:
            player.season14.AddFreeRefreshes(effect.value);
            return;
        case TavernSpellEffect::SHOP_STATS_PERSISTENT:
            player.tavern.fieldZone.ForEach(
                [&effect](MinionData& minion) {
                    minion.value().SetAttack(minion.value().GetAttack() +
                                             effect.attack);
                    minion.value().SetHealth(minion.value().GetHealth() +
                                             effect.health);
                });
            player.season14.AddPersistentShopStats(effect.attack,
                                                   effect.health);
            return;
        case TavernSpellEffect::SPELL_COSTS_HEALTH:
            return;
    }
}
}  // namespace

bool Player::CanPlaySpell(std::size_t handIdx) const
{
    return CanPlaySpell(handIdx, -1);
}

bool Player::CanPlaySpell(std::size_t handIdx, int targetIdx) const
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
    const TavernSpellBehavior behavior = FindTavernSpellBehavior(spell.GetID());
    if (behavior.gold < 0 ||
        TavernSpellRequiresTarget(behavior.effect) != (targetIdx >= 0))
    {
        return false;
    }
    if (targetIdx >= 0 && !ValidFriendlyBoardTarget(*this, targetIdx))
    {
        return false;
    }
    const int baseCost = spell.GetCost();
    const int cost = season14.TavernSpellCost(baseCost);
    if (baseCost < 0)
    {
        return false;
    }
    if (behavior.effect == TavernSpellEffect::SPELL_COSTS_HEALTH)
    {
        // This is a health payment, not damage: Armor must not absorb it.
        // It also cannot reduce the hero to zero; a lethal payment is not a
        // legal purchase and must not partially resolve the spell.
        return hero.health > cost;
    }
    return remainCoin >= cost;
}

bool Player::PlaySpell(std::size_t handIdx)
{
    return PlaySpell(handIdx, -1);
}

bool Player::PlaySpell(std::size_t handIdx, int targetIdx)
{
    if (!CanPlaySpell(handIdx, targetIdx))
    {
        return false;
    }

    CardData& card = hand[static_cast<int>(handIdx)];
    const Spell& spell = std::get<Spell>(card);
    const int cost = season14.TavernSpellCost(spell.GetCost());
    const TavernSpellBehavior effect = FindTavernSpellBehavior(spell.GetID());
    hand.Remove(card);
    if (effect.effect == TavernSpellEffect::SPELL_COSTS_HEALTH)
    {
        // Hasty Excavation explicitly costs Health instead of Gold, so do
        // not route this through Hero::TakeDamage (which would consume Armor).
        hero.health -= cost;
    }
    else
    {
        remainCoin -= cost;
    }
    remainCoin += effect.gold;
    season14.Emit(Season14Event::SPELL_CAST);
    ApplySpellBoardEffect(*this, effect, targetIdx);
    // Arcane Knowledge and other one-shot Tavern-spell discounts are
    // consumed only after a supported spell has actually resolved.  The
    // legality check above ensures unaffordable/unsupported attempts leave
    // the discount untouched.
    season14.OnTavernSpellResolved(true);
    return true;
}

bool Player::ApplySeason14HeroPowerBatch3Activation(
    const Season14HeroPowerBatch3Activation& activation)
{
    std::vector<Minion*> candidates;
    recruitField.ForEachAlive([&candidates](MinionData& minion) {
        candidates.push_back(&minion.value());
    });
    if (candidates.empty())
    {
        return false;
    }

    Random::shuffle(candidates.begin(), candidates.end());
    const auto count = activation.randomCount <= 0
                           ? candidates.size()
                           : std::min<std::size_t>(
                                 static_cast<std::size_t>(activation.randomCount),
                                 candidates.size());
    for (std::size_t i = 0; i < count; ++i)
    {
        candidates[i]->SetAttack(candidates[i]->GetAttack() +
                                 activation.attack);
        candidates[i]->SetHealth(candidates[i]->GetHealth() +
                                 activation.health);
    }
    return true;
}

void Player::SellMinion(std::size_t idx)
{
    const auto minion = recruitField.Remove(recruitField[idx]);
    returnMinionCallback(minion.GetPoolIndex());

    remainCoin += 1;
    season14.OnSellMinion();
}

void Player::UpgradeTavern()
{
    const int cost = season14.UpgradeCost(coinToUpgradeTavern);
    if (currentTier == TIER_UPPER_LIMIT || remainCoin < cost)
    {
        return;
    }

    remainCoin -= cost;
    upgradeTavernCallback(*this);
    const auto result = season14.OnUpgradeTavern();
    remainCoin += result.goldDelta;
}

void Player::RefreshTavern(bool freeRefresh)
{
    const bool allowanceRefresh = !freeRefresh && season14.HasFreeRefresh();
    const int cost = freeRefresh || allowanceRefresh
                         ? 0
                         : season14.RefreshCost(NUM_COIN_REFRESH_TAVERN);
    if (remainCoin < cost)
    {
        return;
    }

    clearTavernMinionsCallback(*this);
    remainCoin -= cost;

    if (allowanceRefresh)
    {
        season14.ConsumeFreeRefresh();
    }

    PrepareTavern();
    season14.OnRefreshTavern(true);
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
