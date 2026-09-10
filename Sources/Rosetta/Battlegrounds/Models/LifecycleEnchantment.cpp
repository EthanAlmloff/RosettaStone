// Copyright (c) 2026 Hearthstone BG AI contributors

#include <Rosetta/Battlegrounds/Models/LifecycleEnchantment.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/GenerateBloodGemsTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomSpellcraftToHandTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SkyGolemPortraitDeathrattleTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonTask.hpp>

namespace RosettaStone::Battlegrounds
{
namespace
{
struct LifecycleSpec
{
    std::string_view spell;
    std::string_view enchantment;
    Minion::TemporaryEnchantment payload;
};

struct ReviewedLifecycleSpec
{
    std::string_view parent;
    std::string_view enchantment;
    Minion::TemporaryEnchantment payload;
};

struct ReviewedChildLifecycleSpec
{
    std::string_view parent;
    std::string_view child;
    Minion::TemporaryEnchantment payload;
};

struct ReviewedTemporaryChildSpec
{
    std::string_view child;
    Minion::TemporaryEnchantment payload;
};

struct ReviewedExternalLifecycleSpec
{
    std::string_view parent;
    std::string_view child;
};

constexpr LifecycleSpec SPELLCRAFT_LIFECYCLES[] = {
    { "BG23_000t", "BG23_000e", Minion::TemporaryEnchantment::Stats },
    { "BG23_000_Gt", "BG23_000e", Minion::TemporaryEnchantment::Stats },
    { "BG23_004t", "BG23_004e", Minion::TemporaryEnchantment::StatsAndTaunt },
    { "BG23_004_Gt", "BG23_004e", Minion::TemporaryEnchantment::StatsAndTaunt },
    { "BG23_007t", "BG23_007e2", Minion::TemporaryEnchantment::StatsAndWindfury },
    { "BG23_007_Gt", "BG23_007e2", Minion::TemporaryEnchantment::StatsAndWindfury },
    { "BG23_008t", "BG23_008e", Minion::TemporaryEnchantment::DivineShield },
    { "BG23_008_Gt", "BG23_008e", Minion::TemporaryEnchantment::DivineShield },
    { "BG26_171t", "BG26_171e", Minion::TemporaryEnchantment::StatsAndStealth },
    { "BG26_171_Gt", "BG26_171e", Minion::TemporaryEnchantment::StatsAndStealth },
    { "BG26_501t", "BG26_501e", Minion::TemporaryEnchantment::Stats },
    { "BG26_501_Gt", "BG26_501e", Minion::TemporaryEnchantment::Stats },
    { "BG31_830t", "BG31_830te2", Minion::TemporaryEnchantment::StatsAndReborn },
    { "BG31_830_Gt", "BG31_830te2", Minion::TemporaryEnchantment::StatsAndReborn },
    { "BG31_924t", "BG31_924e", Minion::TemporaryEnchantment::Stats },
    { "BG26_502t", "BG26_502e", Minion::TemporaryEnchantment::Stats },
    { "BG31_924_Gt", "BG31_924e", Minion::TemporaryEnchantment::Stats },
    { "BG24_Reward_719t", "BG24_Reward_719te", Minion::TemporaryEnchantment::Stats },
    { "BG25_044t", "BG25_044e2", Minion::TemporaryEnchantment::Stats },
    { "BG30_MagicItem_714t", "BG30_MagicItem_714te", Minion::TemporaryEnchantment::Stats },
    // Shellemental copies the selected Tavern minion's stats onto a random
    // friendly minion. Both normal and golden Spellcraft tokens use the same
    // canonical child identity; resolved stats remain parent-owned.
    { "BG29_879t", "BG29_879e", Minion::TemporaryEnchantment::Stats },
    { "BG29_879t_G", "BG29_879e", Minion::TemporaryEnchantment::Stats },
};

// These parents have an explicit, already-tested expiry boundary in the
// Battlegrounds resolver.  Keep this table deliberately small: a card-data
// enchantment record alone is not sufficient evidence for promotion.
constexpr ReviewedLifecycleSpec REVIEWED_LIFECYCLES[] = {
    // The following card-data children are intentionally not listed here:
    // their current resolver paths apply direct stats/keywords (or a hero
    // power state change) and never attach the canonical child identity.
    // A parent/card-text match is not sufficient provenance credit.
    { "BG36_883", "BG36_883e", Minion::TemporaryEnchantment::Stats },
};

// Lift Off's upgrade spells own their payload in Season14State/Battle.  The
// child records below are provenance markers only; keeping the exact spell
// IDs here prevents a similarly-prefixed generated upgrade from being
// promoted accidentally.  Level variants intentionally share the printed
// child identity, as in the card data.
constexpr ReviewedExternalLifecycleSpec REVIEWED_EXTERNAL_LIFECYCLES[] = {
    { "BG31_HERO_801ptc", "BG31_HERO_801ptce" },
    { "BG31_HERO_801ptc2", "BG31_HERO_801ptce" },
    { "BG31_HERO_801ptc3", "BG31_HERO_801ptce" },
    { "BG31_HERO_801ptc4", "BG31_HERO_801ptce" },
    { "BG31_HERO_801ptc5", "BG31_HERO_801ptce" },
    { "BG31_HERO_801ptc6", "BG31_HERO_801ptce" },
    { "BG31_HERO_801ptc7", "BG31_HERO_801ptce" },
    { "BG31_HERO_801pte", "BG31_HERO_801ptee" },
    { "BG31_HERO_801pte2", "BG31_HERO_801ptee" },
    { "BG31_HERO_801pte3", "BG31_HERO_801ptee" },
    { "BG31_HERO_801pte4", "BG31_HERO_801ptee" },
    { "BG31_HERO_801pte5", "BG31_HERO_801ptee" },
    { "BG31_HERO_801pte6", "BG31_HERO_801ptee" },
    { "BG31_HERO_801pte7", "BG31_HERO_801ptee" },
    { "BG31_HERO_801pth", "BG31_HERO_801pthe" },
    { "BG31_HERO_801pth2", "BG31_HERO_801pthe" },
    { "BG31_HERO_801pth3", "BG31_HERO_801pthe" },
    { "BG31_HERO_801pth4", "BG31_HERO_801pthe" },
    { "BG31_HERO_801pth5", "BG31_HERO_801pthe" },
    { "BG31_HERO_801pth6", "BG31_HERO_801pthe" },
    { "BG31_HERO_801pth7", "BG31_HERO_801pthe" },
};

// Daggerspine Thrasher chooses one of these children after each successful
// Tavern-spell cast.  Keep normal and golden identities explicit: the parent
// owns the random choice, while this table owns the canonical child and typed
// expiry payload.
constexpr ReviewedChildLifecycleSpec REVIEWED_CHILD_LIFECYCLES[] = {
    // Goldrinn's deathrattle applies Soul of the Beast to each friendly
    // Beast.  The parent resolver owns the race-wide selection and golden
    // repetition; this row owns the canonical child identity and the
    // temporary stat payload/expiry.
    { "BGS_018", "BGS_018e", Minion::TemporaryEnchantment::Stats },
    // Pufferquil's trigger is owned by Player's successful targeted-spell
    // path; this entry owns the exact temporary Venomous child identity.
    { "BG25_039", "BG25_039e", Minion::TemporaryEnchantment::Venomous },
    // The golden Pufferquil child is a permanent keyword (the golden card
    // text omits "until next turn").  The resolver handles that distinction
    // below while this row keeps the parent/child identity explicit.
    { "BG25_039_G", "BG25_039_Ge", Minion::TemporaryEnchantment::Venomous },
    // Thorncaptain's hand-add trigger owns the +1 Health parent payload; the
    // zero-stat child is retained solely for typed expiry/provenance.
    { "BG25_045", "BG25_045e", Minion::TemporaryEnchantment::Stats },
    // Golden Thorncaptain has no distinct child record; it uses the same
    // canonical temporary child identity with the doubled +2 Health payload.
    { "BG25_045_G", "BG25_045e", Minion::TemporaryEnchantment::Stats },
    { "BG27_024", "BG27_024e1", Minion::TemporaryEnchantment::DivineShield },
    { "BG27_024", "BG27_024e2", Minion::TemporaryEnchantment::StatsAndWindfury },
    { "BG27_024", "BG27_024e3", Minion::TemporaryEnchantment::Venomous },
    { "BG27_024_G", "BG27_024_Ge1", Minion::TemporaryEnchantment::DivineShield },
    { "BG27_024_G", "BG27_024_Ge2", Minion::TemporaryEnchantment::StatsAndWindfury },
    { "BG27_024_G", "BG27_024_Ge3", Minion::TemporaryEnchantment::Venomous },
    // Exact children for two existing keyword lifecycle paths.  The parent
    // condition remains owned by Player; this table owns identity and the
    // typed expiry payload.
    { "BG24_Reward_115", "BG24_Reward_115e2", Minion::TemporaryEnchantment::StatsAndStealth },
    { "BG32_MagicItem_279", "BG32_MagicItem_279e", Minion::TemporaryEnchantment::DivineShield },
    // Haunted Carapace owns the +3/+1 parent payload.  The child is a
    // provenance/expiry marker, not a second stat application.
    { "BG33_112", "BG33_112e", Minion::TemporaryEnchantment::Stats },
    // Volatile Venom's generated reward owns the attack-death boundary; the
    // child records the exact +7/+7 combat payload and replay identity.
    { "BG24_Reward_364", "BG24_Reward_364e", Minion::TemporaryEnchantment::Stats },
    // Tough Tusk's first Blood Gem trigger has separate normal and golden
    // children.  The normal child expires at the next recruit turn; the
    // golden child is a permanent Divine Shield, as printed.
    { "BG20_102", "BG20_102e", Minion::TemporaryEnchantment::DivineShield },
    { "BG20_102_G", "BG20_102_Ge", Minion::TemporaryEnchantment::DivineShield },
};

// Do not admit card-data-only children here.  The Duo records below remain
// excluded because no Battlegrounds parent resolver currently invokes this
// registry for the team-pass parent.  Registering those child IDs alone would
// make AddEnchantmentTask appear executable while bypassing the parent path.

// These are persistent child entities whose own CardDef contains the
// deathrattle.  Keep normal and golden IDs explicit: the reviewed identity
// gate copies that CardDef task list onto the target instance.
constexpr std::string_view REVIEWED_PERSISTENT_CHILDREN[] = {
    "BG27_004e", "BG27_004_Ge", "BG29_875e", "BG29_875_Ge",
    "BG30_119e", "BG30_119_Ge", "BG31_325e", "BG31_325_Ge",
    "BG32_172e", "BG32_172_Ge", "BG21_000e", "BG21_000_Ge",
    // Trinket start-of-combat children.  Their parent resolver selects the
    // eligible race/targets; these IDs own the canonical deathrattle payload.
    "BG30_MagicItem_411e", "BG30_MagicItem_917e", "BG30_MagicItem_952e",
    // Sky Golem Portrait's start-of-combat grant is an exact persistent
    // child: the child owns the canonical Deathrattle payload, while the
    // portrait resolver owns timing and the copied-board target set.
    "BG35_MagicItem_740e2",
    // Sneed's starting Shredder hero power installs this exact deathrattle
    // child on the generated Shredder instance.  The parent resolver owns
    // the tier-lower pool selection; this child owns the canonical task.
    "BG21_HERO_030pe",
    // Exact active lifecycle children whose parent minion deathrattle is
    // installed through AddEnchantmentTask.  These are not generic child
    // admissions: their parent-qualified gates below are mandatory.
    "BG28_603e", "BG_BOT_312e",
};

struct ReviewedPersistentChildParent
{
    std::string_view parent;
    std::string_view child;
};

constexpr ReviewedPersistentChildParent REVIEWED_PERSISTENT_CHILD_PARENTS[] = {
    { "BG21_HERO_030p", "BG21_HERO_030pe" },
    { "BG31_803", "BG28_603e" },
    { "BG_BOT_312", "BG_BOT_312e" },
};
}

bool ApplySpellcraftLifecycleEnchantment(Minion& target,
                                          std::string_view spellID,
                                          int attack, int health,
                                          bool applyConditionalPayload)
{
    for (const auto& spec : SPELLCRAFT_LIFECYCLES)
    {
        if (spec.spell != spellID) continue;
        // Some Spellcraft cards always grant their numeric stats, but only
        // grant the keyword when the target has the card's tribe (currently
        // Waverider/Weary Mage -> Naga).  Do not claim the child keyword
        // enchantment for a non-tribe target; let Player's typed fallback
        // apply the stats-only temporary payload instead.
        if (!applyConditionalPayload &&
            spec.payload == Minion::TemporaryEnchantment::StatsAndReborn)
            return false;
        // Waverider has two authoritative child records: the stats child is
        // always applied, while the Windfury child is only applied to a
        // Naga.  Keep both identities so the conditional branch cannot
        // silently turn a stats-only cast into the keyword child.
        if (spec.spell == "BG23_007t" || spec.spell == "BG23_007_Gt")
        {
            target.ApplyTemporaryEnchantment(
                Minion::TemporaryEnchantment::Stats, attack, health);
            target.RecordTemporaryEnchantment("BG23_007e");
            if (applyConditionalPayload)
            {
                target.ApplyTemporaryEnchantment(
                    Minion::TemporaryEnchantment::StatsAndWindfury, 0, 0);
                target.RecordTemporaryEnchantment("BG23_007e2");
            }
            return true;
        }
        if (spec.enchantment == "BG24_Reward_719te" ||
            spec.enchantment == "BG25_044e2")
        {
            if (!target.MakeGoldenUntilNextTurn()) return false;
        }
        else
        {
            target.ApplyTemporaryEnchantment(spec.payload, attack, health);
        }
        target.RecordTemporaryEnchantment(spec.enchantment);
        return true;
    }
    return false;
}

bool ApplyReviewedLifecycleEnchantment(Minion& target,
                                       std::string_view parentID,
                                       int attack, int health)
{
    for (const auto& spec : REVIEWED_LIFECYCLES)
    {
        if (spec.parent != parentID) continue;
        target.ApplyTemporaryEnchantment(spec.payload, attack, health);
        target.RecordTemporaryEnchantment(spec.enchantment);
        return true;
    }
    return false;
}

bool ApplyReviewedTemporaryChildEnchantment(Minion& target,
                                            std::string_view childID,
                                            int attack, int health)
{
    // No Batch240 child is executable until its parent resolver is wired to
    // this gate.  Keep the helper fail-closed rather than treating a direct
    // child ID as proof that the parent trigger and target semantics exist.
    (void)target;
    (void)childID;
    (void)attack;
    (void)health;
    return false;
}

bool ApplyReviewedLifecycleEnchantment(Minion& target,
                                       std::string_view parentID,
                                       std::string_view childID,
                                       Minion::TemporaryEnchantment payload,
                                       int attack, int health)
{
    for (const auto& spec : REVIEWED_CHILD_LIFECYCLES)
    {
        if (spec.parent != parentID || spec.child != childID ||
            spec.payload != payload)
            continue;
        if (parentID == "BG25_039_G" && childID == "BG25_039_Ge")
        {
            target.SetGameTag(GameTag::POISONOUS, 1);
            return true;
        }
        if (parentID == "BG20_102_G" && childID == "BG20_102_Ge")
        {
            target.SetGameTag(GameTag::DIVINE_SHIELD, 1);
            return true;
        }
        target.ApplyTemporaryEnchantment(payload, attack, health);
        // Goldrinn's task records per-trigger occurrences separately so the
        // combat snapshot can carry its stacked aura back to recruit state.
        if (parentID != "BGS_018")
            target.RecordTemporaryEnchantment(childID);
        return true;
    }
    return false;
}

bool RecordReviewedLifecycleEnchantment(Minion& target,
                                        std::string_view parentID)
{
    for (const auto& spec : REVIEWED_LIFECYCLES)
    {
        if (spec.parent != parentID) continue;
        target.RecordTemporaryEnchantment(spec.enchantment);
        return true;
    }
    return false;
}

bool RecordReviewedExternalLifecycleEnchantment(
    Minion& target, std::string_view parentID, std::string_view childID)
{
    for (const auto& spec : REVIEWED_EXTERNAL_LIFECYCLES)
    {
        if (spec.parent != parentID || spec.child != childID) continue;
        target.RecordTemporaryEnchantment(childID);
        return true;
    }
    return false;
}

bool RecordReviewedDarkGiftChild(Minion& target, std::string_view parentID)
{
    // The Dark Gift executor owns the actual persistent state (steady-growth
    // counters or deathrattle stat transfer).  Keep only the exact pinned
    // child identity here for replay/provenance; never attach a second stat
    // or deathrattle payload through the generic enchantment path.
    if (parentID == "BG36_MidGameEffect_000t51")
    {
        target.RecordTemporaryEnchantment("BG36_MidGameEffect_000t51e");
        return true;
    }
    if (parentID == "BG36_MidGameEffect_000t")
    {
        target.RecordTemporaryEnchantment("BG36_MidGameEffect_000te2");
        return true;
    }
    return false;
}

bool IsReviewedDeferredLifecycle(std::string_view parentID,
                                std::string_view childID)
{
    return parentID == "BG28_884" && childID == "BG28_884e";
}

bool ApplyIchoronLifecycleEnchantment(Minion& target,
                                      std::string_view childID)
{
    // The pinned card data has separate normal and golden children.  The
    // normal child is a recruit-turn temporary keyword; the golden child has
    // no expiry marker and is therefore a permanent keyword application.
    if (childID == "BG31_812e")
    {
        target.ApplyTemporaryEnchantment(Minion::TemporaryEnchantment::DivineShield);
        target.RecordTemporaryEnchantment(childID);
        return true;
    }
    if (childID == "BG31_812e2")
    {
        target.SetGameTag(GameTag::DIVINE_SHIELD, 1);
        return true;
    }
    return false;
}

bool ApplyReviewedPersistentChildEnchantment(Minion& target,
                                             std::string_view childID,
                                             int stackNumber)
{
    for (const auto reviewed : REVIEWED_PERSISTENT_CHILDREN)
    {
        if (reviewed != childID) continue;
        // These three trinket children have intentionally empty CardDefs in
        // the generated card set.  Install their exact child task here so
        // the parent start-of-combat resolver and child provenance remain a
        // single executable path.
        if (childID == "BG30_MagicItem_411e")
        {
            target.AddDarkGiftDeathrattleTask(
                SimpleTasks::GenerateBloodGemsTask{2});
            return true;
        }
        if (childID == "BG30_MagicItem_917e")
        {
            target.AddDarkGiftDeathrattleTask(
                SimpleTasks::RandomSpellcraftToHandTask{});
            return true;
        }
        if (childID == "BG30_MagicItem_952e")
        {
            target.AddDarkGiftDeathrattleTask(
                SimpleTasks::SummonTask{"BG26_537", 1});
            return true;
        }
        if (childID == "BG35_MagicItem_740e2")
        {
            target.AddDarkGiftDeathrattleTask(
                SimpleTasks::SkyGolemPortraitDeathrattleTask{});
            return true;
        }
        const auto enchantmentCard = Cards::FindCardByID(childID);
        // These are persistent deathrattle *children*, not ordinary stat
        // enchantments.  Generic::AddEnchantment only evaluates the CardDef's
        // optional `enchant` payload; it silently drops a child whose CardDef
        // owns only deathrattle tasks.  Copy the canonical child task list
        // onto the target instance so the identity's executable payload is
        // installed exactly once per AddEnchantmentTask invocation.  Keep the
        // stack number out of this path: the child task itself owns golden
        // scaling (for example 2 vs 4 Blood Gems).
        if (enchantmentCard.id.empty() ||
            enchantmentCard.power.GetDeathrattleTask().empty())
            return false;
        // Leapfrogger's child CardDef carries the combat-persistent +1/+1 or
        // +2/+2.  Apply those stats explicitly because this gate intentionally
        // bypasses Generic::AddEnchantment's task-blind path.  Other reviewed
        // persistent children own task payloads only; their parent task owns
        // any numeric effect and must not receive an accidental +1/+1 here.
        if (childID == "BG21_000e" || childID == "BG21_000_Ge")
        {
            const int stat = childID == "BG21_000_Ge" ? 2 : 1;
            target.ApplyCombatPersistentStats(stat, stat);
        }
        for (const auto& task : enchantmentCard.power.GetDeathrattleTask())
            target.AddDarkGiftDeathrattleTask(TaskType{ task });
        return true;
    }
    return false;
}

bool ApplyReviewedPersistentChildEnchantment(Minion& target,
                                             std::string_view parentID,
                                             std::string_view childID,
                                             int stackNumber)
{
    for (const auto& pair : REVIEWED_PERSISTENT_CHILD_PARENTS)
    {
        if (pair.parent == parentID && pair.child == childID)
            return ApplyReviewedPersistentChildEnchantment(target, childID,
                                                           stackNumber);
    }
    return false;
}
}
