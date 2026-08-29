// Copyright (c) 2017-2024 Chris Ohk

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#include <Rosetta/Battlegrounds/Loaders/CardLoader.hpp>

#include <fstream>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string_view>

namespace
{
using Json = nlohmann::json;

bool IsMetadataFlag(const Json& object, const char* key)
{
    if (!object.contains(key) || object.at(key).is_null())
    {
        return false;
    }

    return !object.at(key).is_boolean() || object.at(key).get<bool>();
}

int MetadataInt(const Json& object, const char* key)
{
    if (!object.contains(key) || object.at(key).is_null() ||
        !object.at(key).is_number_integer())
    {
        return 0;
    }

    return object.at(key).get<int>();
}

bool IsLinkKey(const std::string& key)
{
    std::string normalized;
    normalized.reserve(key.size());
    for (const char value : key)
    {
        if (value != '_' && value != '-')
        {
            normalized.push_back(static_cast<char>(
                std::tolower(static_cast<unsigned char>(value))));
        }
    }

    constexpr std::array<std::string_view, 10> parts = {
        "dbfid", "relatedcard", "normaldbfid", "premiumdbfid",
        "heropowerdbfid", "entourage", "choice", "option", "token",
        "generated"
    };
    return std::any_of(parts.begin(), parts.end(), [&normalized](const auto& part) {
        return normalized.find(part) != std::string::npos;
    });
}

bool IsChoiceKey(const std::string& key)
{
    std::string normalized;
    normalized.reserve(key.size());
    for (const char value : key)
    {
        if (value != '_' && value != '-')
        {
            normalized.push_back(static_cast<char>(
                std::tolower(static_cast<unsigned char>(value))));
        }
    }

    return normalized.find("choice") != std::string::npos ||
           normalized.find("option") != std::string::npos;
}

void CollectGeneratedChoiceLinks(const Json& value, const std::string& key,
                                 std::vector<std::string>& result)
{
    if (value.is_object())
    {
        for (const auto& [childKey, childValue] : value.items())
        {
            if (IsChoiceKey(childKey))
            {
                CollectGeneratedChoiceLinks(childValue, childKey, result);
            }
        }
    }
    else if (value.is_array())
    {
        for (const auto& child : value)
        {
            CollectGeneratedChoiceLinks(child, key, result);
        }
    }
    else if (IsChoiceKey(key) && value.is_string())
    {
        result.push_back(value.get<std::string>());
    }
}

void CollectLinkedDbfIDs(const Json& value, const std::string& key,
                         std::vector<int>& result)
{
    if (value.is_object())
    {
        for (const auto& [childKey, childValue] : value.items())
        {
            if (IsLinkKey(childKey))
            {
                CollectLinkedDbfIDs(childValue, childKey, result);
            }
        }
    }
    else if (value.is_array())
    {
        for (const auto& child : value)
        {
            CollectLinkedDbfIDs(child, key, result);
        }
    }
    else if (IsLinkKey(key) && value.is_number_integer())
    {
        result.push_back(value.get<int>());
    }
}
}  // namespace

namespace RosettaStone::Battlegrounds
{
void CardLoader::Load(std::array<Card, NUM_BATTLEGROUNDS_CARDS>& cards)
{
    // Read card data from JSON file
#ifdef ROSETTA_BATTLEGROUNDS_CARDS_JSON
    constexpr const char* configuredPath = ROSETTA_BATTLEGROUNDS_CARDS_JSON;
#else
    constexpr const char* configuredPath = RESOURCES_DIR "cards.json";
#endif
    std::ifstream cardFile(configuredPath);
    nlohmann::json j;

    if (!cardFile.is_open())
    {
        throw std::runtime_error(
            "Can't open configured Battlegrounds card snapshot: "
            + std::string(configuredPath));
    }

    cardFile >> j;

    std::size_t idx = 0;

    for (auto& cardData : j)
    {
        const int cardSet = cardData["set"].is_null()
                                ? 1
                                : static_cast<int>(StrToEnum<CardSet>(
                                      cardData["set"].get<std::string>()));

        if (static_cast<CardSet>(cardSet) == CardSet::LETTUCE)
        {
            continue;
        }

        const std::string id = cardData["id"].get<std::string>();

        const int dbfID =
            cardData["dbfId"].is_null() ? 0 : cardData["dbfId"].get<int>();
        const int normalDbfID =
            MetadataInt(cardData, "battlegroundsNormalDbfId");
        const int premiumDbfID =
            MetadataInt(cardData, "battlegroundsPremiumDbfId");
        const bool isBattlegroundsHero =
            IsMetadataFlag(cardData, "battlegroundsHero");
        const bool isBattlegroundsPoolMinion =
            IsMetadataFlag(cardData, "isBattlegroundsPoolMinion");

        const std::string name = cardData["name"].is_null()
                                     ? ""
                                     : cardData["name"].get<std::string>();
        const std::string text = cardData["text"].is_null()
                                     ? ""
                                     : cardData["text"].get<std::string>();

        const CardType type =
            cardData["type"].is_null()
                ? CardType::INVALID
                : StrToEnum<CardType>(cardData["type"].get<std::string>());
        const Race race =
            cardData["race"].is_null()
                ? Race::INVALID
                : StrToEnum<Race>(cardData["race"].get<std::string>());

        const int techLevel = cardData["techLevel"].is_null()
                                  ? 0
                                  : cardData["techLevel"].get<int>();
        const int attack =
            cardData["attack"].is_null() ? 0 : cardData["attack"].get<int>();
        const int health =
            cardData["health"].is_null() ? 0 : cardData["health"].get<int>();

        std::map<GameTag, int> gameTags;
        for (auto& mechanic : cardData["mechanics"])
        {
            GameTag gameTag = StrToEnum<GameTag>(mechanic.get<std::string>());
            gameTags.emplace(gameTag, 1);
        }

        Card card;
        card.id = id;
        card.dbfID = dbfID;
        card.normalDbfID = normalDbfID;
        card.premiumDbfID = premiumDbfID;
        card.heroPowerDbfID = MetadataInt(cardData, "heroPowerDbfId");
        card.relatedDbfID = MetadataInt(cardData, "battlegroundsRelatedCard");
        card.name = name;
        card.text = text;
        card.isCurHero = isBattlegroundsHero;
        card.isBattlegroundsPoolMinion = isBattlegroundsPoolMinion;
        card.isBattlegroundsPoolSpell =
            IsMetadataFlag(cardData, "isBattlegroundsPoolSpell");
        card.isBattlegroundsDarkGift =
            IsMetadataFlag(cardData, "isBattlegroundsDarkGift");
        card.isBattlegroundsDuosExclusive =
            IsMetadataFlag(cardData, "isBattlegroundsDuosExclusive");
        card.isBattlegroundsTrinket = type == CardType::BATTLEGROUND_TRINKET;
        if (card.isBattlegroundsTrinket && cardData.contains("spellSchool") &&
            cardData.at("spellSchool").is_string())
        {
            card.trinketType = cardData.at("spellSchool").get<std::string>();
        }
        if (cardData.contains("battlegroundsAssociatedRaces") &&
            cardData.at("battlegroundsAssociatedRaces").is_array())
        {
            for (const auto& associatedRace :
                 cardData.at("battlegroundsAssociatedRaces"))
            {
                if (associatedRace.is_string())
                {
                    card.associatedRaces.push_back(
                        associatedRace.get<std::string>());
                }
            }
        }
        CollectLinkedDbfIDs(cardData, "", card.linkedDbfIDs);
        CollectGeneratedChoiceLinks(cardData, "", card.generatedChoiceLinks);
        std::sort(card.generatedChoiceLinks.begin(),
                  card.generatedChoiceLinks.end());
        card.generatedChoiceLinks.erase(
            std::unique(card.generatedChoiceLinks.begin(),
                        card.generatedChoiceLinks.end()),
            card.generatedChoiceLinks.end());
        card.linkedDbfIDs.erase(
            std::remove(card.linkedDbfIDs.begin(), card.linkedDbfIDs.end(),
                        dbfID),
            card.linkedDbfIDs.end());
        std::sort(card.linkedDbfIDs.begin(), card.linkedDbfIDs.end());
        card.linkedDbfIDs.erase(
            std::unique(card.linkedDbfIDs.begin(), card.linkedDbfIDs.end()),
            card.linkedDbfIDs.end());

        card.gameTags = gameTags;
        card.gameTags[GameTag::CARD_SET] = static_cast<int>(cardSet);
        card.gameTags[GameTag::CARDTYPE] = static_cast<int>(type);
        card.gameTags[GameTag::CARDRACE] = static_cast<int>(race);
        card.gameTags[GameTag::TECH_LEVEL] = techLevel;
        card.gameTags[GameTag::ATK] = attack;
        card.gameTags[GameTag::HEALTH] = health;

        // NOTE: The value "isBattlegroundsHero" of Lady Vashj
        //       (TB_BaconShop_HERO_61) is missing.
        if (id == "TB_BaconShop_HERO_61")
        {
            card.isCurHero = true;
        }

        if (idx >= cards.size())
        {
            throw std::length_error(
                "Battlegrounds cards.json exceeds NUM_BATTLEGROUNDS_CARDS=" +
                std::to_string(cards.size()) + ": " + id);
        }
        cards.at(idx) = card;
        ++idx;
    }

    cardFile.close();
}
}  // namespace RosettaStone::Battlegrounds
