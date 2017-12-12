#include "cbase.h"
#include "triggers.h"

class CAR_StartlineEntity : public CTriggerMultiple
{
public:
	DECLARE_CLASS(CAR_StartlineEntity, CTriggerMultiple);
	DECLARE_DATADESC();

	void Spawn();
	void StartTouch(CBaseEntity *pOther);
	void SetPlayerCheckpoint(int iPlayerIndex, int iCheckpoint);

private:
	int	m_iPlayerCheckpoint[MAX_PLAYERS];	// Stores each player's current checkpoint
	int	m_iPlayerLapCount[MAX_PLAYERS];		// Stores each player's laps completed
	int m_iLastCheckpoint;
};

LINK_ENTITY_TO_CLASS(r_startline, CAR_StartlineEntity);

BEGIN_DATADESC(CAR_StartlineEntity)
DEFINE_KEYFIELD(m_iLastCheckpoint, FIELD_INTEGER, "lastCheck"),
END_DATADESC()

ConVar ar_laps("race_laps", "1", FCVAR_REPLICATED | FCVAR_NOTIFY, "Set the number of laps each race is", true, 1, false, 0);

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