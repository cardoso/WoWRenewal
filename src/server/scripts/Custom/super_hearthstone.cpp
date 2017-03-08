#include "ScriptMgr.h"
#include "MapManager.h"

class super_hearthstone : ItemScript
{
public:
	super_hearthstone() : ItemScript("super_hearthstone") {}

	// more code .. 

	std::vector<WorldLocation> GetPlayerWaypoints(Player* player) {

		auto waypoints = std::vector<WorldLocation>();

		std::string query = "SELECT map, x, y, z, o FROM super_hearthstone_waypoints WHERE character_guid = ";
		query += std::to_string(player->GetSession()->GetGUIDLow());
		query += ";";

		auto res = CharacterDatabase.Query(query.c_str());

		if (res) {
			do
			{
				Field* field = res->Fetch();

				uint16 id = field[0].GetUInt16();
				float x = field[1].GetFloat();
				float y = field[2].GetFloat();
				float z = field[3].GetFloat();
				float o = field[4].GetFloat();

				auto location = WorldLocation(id, x, y, z, o);

				waypoints.push_back(location);

			} while (res->NextRow());
		}

		return waypoints;
	}

	bool OnUse(Player* player, Item* item, SpellCastTargets const& /*targets*/) {

		ClearGossipMenuFor(player);

		auto waypoints = GetPlayerWaypoints(player);

		for (auto w = waypoints.begin(); w < waypoints.end(); w++) {
			auto mapEntry = sMapStore.LookupEntry(w->m_mapId);
			if (!mapEntry) continue;
			auto mapName = std::string(mapEntry->name[sWorld->GetDefaultDbcLocale()]);

			auto areaId = MapManager::instance()->GetAreaId(w->m_mapId, w->m_positionX, w->m_positionY, w->m_positionZ);
			auto areaEntry = sAreaTableStore.LookupEntry(areaId);
			if (!areaEntry) continue;
			auto areaName = std::string(areaEntry->area_name[sWorld->GetDefaultDbcLocale()]);

			AddGossipItemFor(player, 0, areaName + " (" + mapName + ")",
							 GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
		}

		SendGossipMenuFor(player, 135555, item->GetGUID());

		return true;
	}
};

void AddSC_super_hearthstone()
{
	new super_hearthstone();
}