#include "cbase.h"
#include "ar_startline.h"
#include "ar_player.h"
#include "fmtstr.h"

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS(r_startline, CAR_StartlineEntity);

BEGIN_DATADESC(CAR_StartlineEntity)
	DEFINE_KEYFIELD(m_iLastCheckpoint, FIELD_INTEGER, "lastCheck"),
END_DATADESC()

ConVar ar_laps("race_laps", "3", FCVAR_NOTIFY, "Set the number of laps each race is", true, 1, false, 0);

void CAR_StartlineEntity::Spawn()
{
	DevMsg("Spawned Startline: Last Checkpoint: %d\n", m_iLastCheckpoint);
	BaseClass::Spawn();
}

void CAR_StartlineEntity::StartTouch(CBaseEntity *pOther)
{
	if (stricmp(pOther->GetClassname(), "player") == 0) {

		int iPlayerIndex = pOther->entindex() - 1;
		if (m_iPlayerCheckpoint[iPlayerIndex] == m_iLastCheckpoint) {
			m_iPlayerLapCount[iPlayerIndex]++;
			m_iPlayerCheckpoint[iPlayerIndex] = 0;

			// Send lap complete message via our player class
			CAR_Player *pPlayer = dynamic_cast<CAR_Player*>(pOther);
			if (pPlayer) {
				CFmtStr str;
				hud_message msg;
				msg.type = "Lap";
				msg.valueString = str.sprintf("%d/%d", m_iPlayerLapCount[iPlayerIndex]+1, ar_laps.GetInt());
				pPlayer->SendHudMessage(msg);
			}

			if (m_iPlayerLapCount[iPlayerIndex] == ar_laps.GetInt()) {
				DevMsg("Race Finished. Player: %d wins", iPlayerIndex);
			}
		}
		DevMsg("Startline StartTouch entity:%s Laps: %d\n", pOther->GetClassname(), m_iPlayerLapCount[iPlayerIndex]);
	}
}

void CAR_StartlineEntity::SetPlayerCheckpoint(int iPlayerIndex, int iCheckpoint) {
	if (iCheckpoint == (m_iPlayerCheckpoint[iPlayerIndex] + 1)) {
		m_iPlayerCheckpoint[iPlayerIndex] = iCheckpoint;
		DevMsg("Player %d checkpoint %d\n", iPlayerIndex, iCheckpoint);
	}
}