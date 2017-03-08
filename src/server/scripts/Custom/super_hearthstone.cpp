#include "ScriptMgr.h"
#include "MapManager.h"

class super_hearthstone : ItemScript
{
public:
	super_hearthstone() : ItemScript("super_hearthstone") {}

	// may return null_ptr
	const MapEntry* GetMapEntryFromMapId(uint32 mapId) {
		return sMapStore.LookupEntry(mapId);
	}

	const std::string GetMapNameFromMapId(uint32 mapId, LocaleConstant locale = LOCALE_enUS) {
		auto entry = GetMapEntryFromMapId(mapId);
		if (!entry)
			return "UNKNOWN_MAPID_" + std::to_string(mapId);
		return entry->name[locale];
	}

	// may return null_ptr
	const AreaTableEntry* GetAreaTableEntryFromAreaId(uint32 areaId) {
		return sAreaTableStore.LookupEntry(areaId);
	}

	// may return null_ptr
	const uint32 GetAreaIdFromWorldLocation(WorldLocation w) {
		return MapManager::instance()->GetAreaId(w.m_mapId, w.m_positionX, w.m_positionY, w.m_positionZ);
	}

	// may return null_ptr
	const AreaTableEntry* GetAreaTableEntryFromWorldLocation(WorldLocation w) {
		auto areaId = GetAreaIdFromWorldLocation(w);
		return GetAreaTableEntryFromAreaId(areaId);
	}

	const std::string GetAreaNameFromAreaId(uint32 areaId, LocaleConstant locale = LOCALE_enUS) {
		auto entry = GetAreaTableEntryFromAreaId(areaId);
		if (!entry)
			return "UNKNOWN_AREAID_" + std::to_string(areaId);
		return std::string(entry->area_name[locale]);
	}

	const std::string GetAreaNameFromWorldLocation(WorldLocation w, LocaleConstant locale = LOCALE_enUS) {
		auto areaId = GetAreaIdFromWorldLocation(w);
		return GetAreaNameFromAreaId(areaId, locale);
	}

	/* WoWRenewal Additions */

	// generic cooldown function
	/*void ApplyPlayerItemCooldown(Player* player, Item* pItem, Milliseconds cooldown)
	{
		if (pItem->GetTemplate()->Flags & ITEM_FLAG_NO_EQUIP_COOLDOWN)
			return;

		std::chrono::steady_clock::time_point now = GameTime::GetGameTimeSteadyPoint();
		for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
		{
			_Spell const& spellData = pItem->GetTemplate()->Spells[i];

			// no spell
			if (spellData.SpellId <= 0)
				continue;

			// apply proc cooldown to equip auras if we have any
			if (spellData.SpellTrigger == ITEM_SPELLTRIGGER_ON_EQUIP)
			{
				SpellProcEntry const* procEntry = sSpellMgr->GetSpellProcEntry(spellData.SpellId);
				if (!procEntry)
					continue;

				if (Aura* itemAura = player->GetAura(spellData.SpellId, player->GetGUID(), pItem->GetGUID()))
					itemAura->AddProcCooldown(now + cooldown);
				continue;
			}

			// wrong triggering type (note: ITEM_SPELLTRIGGER_ON_NO_DELAY_USE not have cooldown)
			/*if (spellData.SpellTrigger != ITEM_SPELLTRIGGER_ON_USE)
				continue;*/

			// Don't replace longer cooldowns by equip cooldown if we have any.
			/*if (player->GetSpellHistory()->GetRemainingCooldown(sSpellMgr->AssertSpellInfo(spellData.SpellId)) > 30 * IN_MILLISECONDS)
				continue;*/

			/*player->GetSpellHistory()->AddCooldown(spellData.SpellId, pItem->GetEntry(), cooldown);

			WorldPacket data(SMSG_ITEM_COOLDOWN, 8 + 4);
			data << uint64(pItem->GetGUID());
			data << uint32(spellData.SpellId);
			player->GetSession()->SendPacket(&data);
		}
	}*/

	std::vector<WorldLocation> GetPlayerWaypoints(Player* player) {

		auto waypoints = std::vector<WorldLocation>();

		std::string query = "SELECT mapId, x, y, z, o FROM super_hearthstone_waypoints WHERE character_guid = ";
		query += std::to_string(player->GetSession()->GetGUIDLow());
		query += " ORDER BY mapId;";

		auto res = CharacterDatabase.Query(query.c_str());

		if (res) {
			do
			{
				Field* field = res->Fetch();

				uint16 mapId = field[0].GetUInt16();

				if (!GetMapEntryFromMapId(mapId))
					continue;

				float x = field[1].GetFloat();
				float y = field[2].GetFloat();
				float z = field[3].GetFloat();
				float o = field[4].GetFloat();

				auto location = WorldLocation(mapId, x, y, z, o);

				if (!GetAreaTableEntryFromWorldLocation(location))
					continue;

				waypoints.push_back(location);

			} while (res->NextRow());
		}

		return waypoints;
	}

	bool OnUse(Player* player, Item* item, SpellCastTargets const& /*targets*/) {

		auto locale = player->GetSession()->GetSessionDbLocaleIndex();
		auto waypoints = GetPlayerWaypoints(player);

		ClearGossipMenuFor(player);

		auto i = 1;
		for (auto w : waypoints) {

			auto mapName = GetMapNameFromMapId(w.m_mapId, locale);
			auto areaName = GetAreaNameFromWorldLocation(w, locale);

			AddGossipItemFor(player, 0, areaName + " (" + mapName + ")",
				GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + i);

			i++;
		}

		SendGossipMenuFor(player, 135555, item->GetGUID());

		return true;
	}

	void OnGossipSelect(Player* player, Item* item, uint32 /*sender*/, uint32 action) {

		ClearGossipMenuFor(player);

		auto waypoints = GetPlayerWaypoints(player);
		auto choice = action - GOSSIP_ACTION_INFO_DEF - 1;
		auto waypoint = waypoints[choice];

		player->SetHomebind(waypoint, GetAreaIdFromWorldLocation(waypoint));
		//player->TeleportTo(waypoint);
		//ApplyPlayerItemCooldown(player, item, std::chrono::seconds(60));
		player->CastItemUseSpell(item, SpellCastTargets(), 1, 0);

		CloseGossipMenuFor(player);
	}

	bool OnRemove(Player* /*player*/, Item* /*item*/) {
		return true;
	}
};

void AddSC_super_hearthstone()
{
	new super_hearthstone();
}