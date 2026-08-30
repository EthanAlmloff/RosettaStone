// This code is based on Sabberstone project.
// Copyright (c) 2017-2021 SabberStone Team, darkfriend77 & rnilva
// RosettaStone is hearthstone simulator using C++ with reinforcement learning.
// Copyright (c) 2017-2024 Chris Ohk

#include <Rosetta/Battlegrounds/CardSets/BattlegroundsCardsGen.hpp>
#include <Rosetta/Battlegrounds/CardSets/DarkGiftBehaviors.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviors.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch2.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch3.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch4.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch5.hpp>
#include <Rosetta/Battlegrounds/Cards/CardDefs.hpp>

namespace RosettaStone::Battlegrounds
{
std::map<std::string, CardDef> CardDefs::m_data;

CardDefs::CardDefs()
{
    BattlegroundsCardsGen::AddAll(m_data);
    DarkGiftBehaviors::AddAll(m_data);
    ModernMinionBehaviors::AddAll(m_data);
    ModernMinionBehaviorsBatch2::AddAll(m_data);
    ModernMinionBehaviorsBatch3::AddAll(m_data);
    ModernMinionBehaviorsBatch4::AddAll(m_data);
    ModernMinionBehaviorsBatch5::AddAll(m_data);
}

CardDefs::~CardDefs()
{
    m_data.clear();
}

CardDefs& CardDefs::GetInstance()
{
    static CardDefs instance;
    return instance;
}

CardDef CardDefs::FindCardDefByID(const std::string_view& id)
{
    for (auto& data : m_data)
    {
        if (data.first == id)
        {
            return data.second;
        }
    }

    return CardDef();
}

bool CardDefs::HasDefinition(const std::string_view& id)
{
    (void)GetInstance();
    return m_data.contains(std::string(id));
}
}  // namespace RosettaStone::Battlegrounds
