// Copyright (c) 2026 Hearthstone BG AI contributors

#ifndef ROSETTASTONE_BATTLEGROUNDS_LIFECYCLE_ENCHANTMENT_HPP
#define ROSETTASTONE_BATTLEGROUNDS_LIFECYCLE_ENCHANTMENT_HPP

#include <string_view>

#include <Rosetta/Battlegrounds/Models/Minion.hpp>

namespace RosettaStone::Battlegrounds
{
//! Apply the canonical child enchantment for a Spellcraft parent.
//!
//! The parent spell remains the source of legality and numeric payload.  This
//! registry supplies the exact pinned child identity and routes its payload
//! through Minion's typed temporary lifecycle, so expiry cannot double-apply
//! or silently lose the enchantment provenance.
bool ApplySpellcraftLifecycleEnchantment(Minion& target,
                                          std::string_view spellID,
                                          int attack = 0, int health = 0,
                                          bool applyConditionalPayload = true);

//! Apply a non-Spellcraft temporary/pending effect and retain its canonical
//! child enchantment identity.  The caller owns target legality and chooses
//! the typed payload; this helper only admits reviewed parent IDs.
bool ApplyReviewedLifecycleEnchantment(Minion& target,
                                       std::string_view parentID,
                                       int attack = 0, int health = 0);

//! Apply a reviewed parent effect whose canonical child is selected by the
//! resolver (for example a random keyword child).  Keeping the child lookup
//! here ensures the typed payload and its identity cannot drift apart.
bool ApplyReviewedLifecycleEnchantment(Minion& target,
                                       std::string_view parentID,
                                       std::string_view childID,
                                       Minion::TemporaryEnchantment payload,
                                       int attack = 0, int health = 0);

//! Apply a reviewed child whose card-data identity is itself the lifecycle
//! record.  The registry owns the child identity and typed payload together;
//! callers must not recreate the temporary keyword/stat semantics locally.
bool ApplyReviewedTemporaryChildEnchantment(Minion& target,
                                            std::string_view childID,
                                            int attack = 0, int health = 0);

//! Record a reviewed child when its payload is owned by another state
//! machine (for example a next-combat reward), so applying it here would
//! double-count stats.
bool RecordReviewedLifecycleEnchantment(Minion& target,
                                        std::string_view parentID);

//! Record an exact parent-qualified child marker when the payload is owned by
//! a Player/Season14 state machine rather than Minion temporary stats.
bool RecordReviewedLifecycleEnchantment(Minion& target,
                                        std::string_view parentID,
                                        std::string_view childID);

//! Record a child whose payload is owned by a non-Minion state machine.
//! Parent and child IDs are admitted only as exact reviewed pairs; the
//! caller remains responsible for applying the authoritative state payload.
bool RecordReviewedExternalLifecycleEnchantment(
    Minion& target, std::string_view parentID, std::string_view childID);

bool ApplyReviewedEndTurnChildEnchantment(Minion& target,
                                          std::string_view parentID,
                                          std::string_view childID);

//! Record the exact child marker for a Dark Gift whose payload lives in the
//! DarkGiftBehavior state machine.  This is deliberately parent-qualified;
//! arbitrary enchantment IDs must not become provenance through this helper.
bool RecordReviewedDarkGiftChild(Minion& target, std::string_view parentID);

//! Validate the canonical child for a deferred next-combat reward.  The
//! reward payload is player-owned, so the child identity is recorded at the
//! arm boundary rather than attached to a minion.
bool IsReviewedDeferredLifecycle(std::string_view parentID,
                                std::string_view childID);

//! Apply Ichoron's exact normal/golden child identity.  Normal Ichoron grants
//! a Divine Shield only until the next recruit turn; the golden child is a
//! permanent shield.  Keeping this distinction here prevents a generic card
//! enchantment path from losing the child identity or expiring the golden
//! payload accidentally.
bool ApplyIchoronLifecycleEnchantment(Minion& target,
                                      std::string_view childID);

//! Apply one of the reviewed persistent deathrattle-child enchantments.
//!
//! These records are real child entities (not temporary stat payloads).  Keep
//! their exact IDs on the attachment path so the child CardDef owns the
//! trigger and golden variants cannot collapse into a parent or a guessed
//! token.  The caller still owns target selection and stack-number semantics.
bool ApplyReviewedPersistentChildEnchantment(
    Minion& target, std::string_view childID, int stackNumber = 0);

// Parent-qualified overload.  The closed parent/child allowlist prevents a
// generated enchantment CardDef from being promoted without its owner path.
bool ApplyReviewedPersistentChildEnchantment(
    Minion& target, std::string_view parentID, std::string_view childID,
    int stackNumber = 0);
}

#endif
