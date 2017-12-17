#ifndef CAR_STARTLINE_H
#define CAR_STARTLINE_H
#pragma once

#include "cbase.h"
#include "triggers.h"

extern ConVar ar_laps;

class CAR_StartlineEntity : public CTriggerMultiple
{
public:
	DECLARE_CLASS(CAR_StartlineEntity, CTriggerMultiple);

	void Spawn(void);
	void StartTouch(CBaseEntity *pOther);
	void SetPlayerCheckpoint(int iPlayerIndex, int iCheckpoint);

private:
	int	m_iPlayerCheckpoint[MAX_PLAYERS];	// Stores each player's current checkpoint
	int	m_iPlayerLapCount[MAX_PLAYERS];		// Stores each player's laps completed
	int m_iLastCheckpoint;

	DECLARE_DATADESC();
};

#endif // CAR_STARTLINE_H